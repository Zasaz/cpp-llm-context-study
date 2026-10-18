import * as vscode from 'vscode';
import axios from 'axios';
import { TextDecoder } from 'util';
import * as path from 'path';
import { ArchitectureServer } from './server';

// --- PORTABLE GOOGLE TEST IMPLEMENTATION ---
const MINI_GTEST_CONTENT = `
#ifndef MINI_GTEST_H
#define MINI_GTEST_H

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <sstream>

// Minimal implementation of Google Test macros for portable usage

namespace testing {

class Test {
public:
    virtual void SetUp() {}
    virtual void TearDown() {}
    virtual void TestBody() = 0;
    virtual ~Test() {}
};

struct TestInfo {
    std::string test_case_name;
    std::string test_name;
    std::function<Test*()> factory;
};

class UnitTest {
public:
    static UnitTest* GetInstance() {
        static UnitTest instance;
        return &instance;
    }

    void RegisterTest(const std::string& test_case_name, const std::string& test_name, std::function<Test*()> factory) {
        tests_.push_back({test_case_name, test_name, factory});
    }

    int Run() {
        int passed = 0;
        int failed = 0;
        
        std::cout << "[==========] Running " << tests_.size() << " tests." << std::endl;

        for (const auto& test_info : tests_) {
            std::cout << "[ RUN      ] " << test_info.test_case_name << "." << test_info.test_name << std::endl;
            
            Test* test = test_info.factory();
            bool current_test_failed = false;
            
            try {
                test->SetUp();
                test->TestBody();
                test->TearDown();
            } catch (const std::exception& e) {
                std::cout << "[  FAILED  ] " << test_info.test_case_name << "." << test_info.test_name << "\\n" 
                          << "             Exception: " << e.what() << std::endl;
                current_test_failed = true;
            } catch (...) {
                std::cout << "[  FAILED  ] " << test_info.test_case_name << "." << test_info.test_name << "\\n" 
                          << "             Unknown exception" << std::endl;
                current_test_failed = true;
            }

            if (current_test_failed) {
                failed++;
            } else {
                std::cout << "[       OK ] " << test_info.test_case_name << "." << test_info.test_name << std::endl;
                passed++;
            }
            delete test;
        }

        std::cout << "[==========] " << tests_.size() << " tests ran." << std::endl;
        std::cout << "[  PASSED  ] " << passed << " tests." << std::endl;
        if (failed > 0) {
            std::cout << "[  FAILED  ] " << failed << " tests." << std::endl;
        }

        return failed == 0 ? 0 : 1;
    }

private:
    std::vector<TestInfo> tests_;
};

void InitGoogleTest(int* argc, char** argv) {}

} // namespace testing

// Macros
#define TEST(test_case_name, test_name) \\
    class test_case_name##_##test_name##_Test : public testing::Test { \\
    public: \\
        void TestBody() override; \\
        static testing::Test* Create() { return new test_case_name##_##test_name##_Test; } \\
    }; \\
    bool test_case_name##_##test_name##_registered = []() { \\
        testing::UnitTest::GetInstance()->RegisterTest(#test_case_name, #test_name, test_case_name##_##test_name##_Test::Create); \\
        return true; \\
    }(); \\
    void test_case_name##_##test_name##_Test::TestBody()

#define ASSERT_EQ(val1, val2) \\
    if (!((val1) == (val2))) { \\
        std::stringstream ss; \\
        ss << "Expected equality of these values:\\n  " << #val1 << "\\n    Which is: " << (val1) << "\\n  " << #val2 << "\\n    Which is: " << (val2); \\
        throw std::runtime_error(ss.str()); \\
    }

#define EXPECT_EQ(val1, val2) ASSERT_EQ(val1, val2)
#define ASSERT_TRUE(condition) \\
    if (!(condition)) throw std::runtime_error("Value of: " #condition "\\n  Actual: false\\nExpected: true");
#define EXPECT_TRUE(condition) ASSERT_TRUE(condition)
#define ASSERT_FALSE(condition) \\
    if (condition) throw std::runtime_error("Value of: " #condition "\\n  Actual: true\\nExpected: false");
#define EXPECT_FALSE(condition) ASSERT_FALSE(condition)
#define ASSERT_NEAR(val1, val2, abs_error) \\
    if (std::abs((val1) - (val2)) > (abs_error)) throw std::runtime_error("The difference between " #val1 " and " #val2 " exceeds " #abs_error);
#define EXPECT_NEAR(val1, val2, abs_error) ASSERT_NEAR(val1, val2, abs_error)

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return testing::UnitTest::GetInstance()->Run();
}
#endif
`;

// --- CONFIGURATION ---
const API_BASE_URL = 'http://127.0.0.1:5000';
// const API_BASE_URL = 'http://127.0.0.1:5000'; 

// Global state to persist chat history across tab switches
class ChatStateManager {
    private static _instance: ChatStateManager;
    private _chatHistory: any[] = [];
    private _currentFile: string | null = null;

    private constructor() { }

    public static getInstance(): ChatStateManager {
        if (!ChatStateManager._instance) {
            ChatStateManager._instance = new ChatStateManager();
        }
        return ChatStateManager._instance;
    }

    public getChatHistory(): any[] {
        return this._chatHistory;
    }

    public addMessage(message: any): void {
        this._chatHistory.push(message);
    }

    public clearChatHistory(): void {
        this._chatHistory = [];
    }

    public setCurrentFile(file: string | null): void {
        this._currentFile = file;
    }

    public getCurrentFile(): string | null {
        return this._currentFile;
    }

    public saveState(context: vscode.ExtensionContext): void {
        context.globalState.update('chatHistory', this._chatHistory);
        context.globalState.update('currentFile', this._currentFile);
    }

    public loadState(context: vscode.ExtensionContext): void {
        this._chatHistory = context.globalState.get('chatHistory', []);
        this._currentFile = context.globalState.get('currentFile', null);
    }
}

export function activate(context: vscode.ExtensionContext) {
    // Initialize chat state manager and load saved state
    const chatStateManager = ChatStateManager.getInstance();
    chatStateManager.loadState(context);

    // 1. Create the Sidebar View Provider
    const provider = new CppHelperProvider(context.extensionUri, context.subscriptions, context);

    // 2. Register the Sidebar View
    context.subscriptions.push(
        vscode.window.registerWebviewViewProvider(CppHelperProvider.viewType, provider)
    );

    // 3. Register the Command Palette command
    let commandDisposable = vscode.commands.registerCommand('cpp-helper-bot.start', () => {
        vscode.commands.executeCommand('cpp-helper-sidebar-view.focus');
    });
    context.subscriptions.push(commandDisposable);

    // --- File watcher to keep the file list up to date ---
    const fileWatcher = vscode.workspace.createFileSystemWatcher('**/*.{cpp,h,hpp,cxx,cc}');

    fileWatcher.onDidCreate(() => provider.updateFileListAndSend());
    fileWatcher.onDidDelete(() => provider.updateFileListAndSend());

    context.subscriptions.push(fileWatcher);

    // --- Watch for active file changes ---
    context.subscriptions.push(
        vscode.window.onDidChangeActiveTextEditor(editor => {
            provider.updateActiveFile(editor?.document);
        })
    );

    // Save state when extension is deactivated
    context.subscriptions.push({
        dispose: () => {
            chatStateManager.saveState(context);
        }
    });
}

class CppHelperProvider implements vscode.WebviewViewProvider {

    public static readonly viewType = 'cpp-helper-sidebar-view';
    private _view?: vscode.WebviewView;
    private _fileList: string[] = [];
    private _chatStateManager: ChatStateManager;
    private _suggestionDecorations: vscode.TextEditorDecorationType[] = [];
    private _architectureServer: ArchitectureServer;

    constructor(
        private readonly _extensionUri: vscode.Uri,
        private readonly _subscriptions: vscode.Disposable[],
        private readonly _context: vscode.ExtensionContext
    ) {
        this._chatStateManager = ChatStateManager.getInstance();
        this._architectureServer = new ArchitectureServer(_extensionUri);

        // Clean up decorations when extension is deactivated
        this._subscriptions.push({
            dispose: () => {
                this.clearAllDecorations();
                this._architectureServer.stop();
            }
        });
    }

    public resolveWebviewView(
        webviewView: vscode.WebviewView,
        context: vscode.WebviewViewResolveContext,
        _token: vscode.CancellationToken,
    ) {
        this._view = webviewView;

        webviewView.webview.options = {
            enableScripts: true,
            localResourceRoots: [
                this._extensionUri,
                vscode.Uri.joinPath(this._extensionUri, 'media')
            ]
        };

        webviewView.webview.html = this._getWebviewContent(webviewView.webview);

        // Handle messages from the webview
        webviewView.webview.onDidReceiveMessage(
            async message => {
                switch (message.command) {
                    case 'jumpToCode':
                        await this.jumpToCodeLocation(message.filepath, message.line);
                        return;
                    case 'runTestsLocally':
                        if (!vscode.window.activeTextEditor) {
                            vscode.window.showErrorMessage('No active editor found.');
                            return;
                        }

                        const activeDocPath = vscode.window.activeTextEditor.document.fileName;
                        const activeDir = path.dirname(activeDocPath);
                        const activeFileName = path.basename(activeDocPath);
                        const testFilePath = path.join(activeDir, 'test_suite.cpp');
                        const testExePath = path.join(activeDir, 'test_suite');

                        // Create a temporary source file with main() renamed to user_main()
                        // This prevents "multiple definition of main" errors during linking
                        const tempSourcePath = path.join(activeDir, `temp_${activeFileName}`);

                        try {
                            // Read original source code
                            const sourceDoc = vscode.window.activeTextEditor.document;
                            let sourceCode = sourceDoc.getText();

                            // Rename main to user_main
                            sourceCode = sourceCode.replace(/int\s+main\s*\(/g, 'int user_main(');

                            // Save modified source to temp file
                            await vscode.workspace.fs.writeFile(
                                vscode.Uri.file(tempSourcePath),
                                Buffer.from(sourceCode, 'utf8')
                            );

                            // Save the test code to file
                            await vscode.workspace.fs.writeFile(
                                vscode.Uri.file(testFilePath),
                                Buffer.from(message.payload.test_suite_code, 'utf8')
                            );

                            // Create or reuse terminal
                            let terminal = vscode.window.terminals.find(t => t.name === 'C++ Helper Tests');
                            if (!terminal) {
                                terminal = vscode.window.createTerminal('C++ Helper Tests');
                            }
                            terminal.show();

                            // Construct compile and run command
                            // Detect OS for executable extension and path separator
                            const isWin = process.platform === 'win32';
                            const exeExtension = isWin ? '.exe' : '';
                            const fullExePath = testExePath + exeExtension;

                            // Quote paths to handle spaces
                            const quotedTempSource = `"${tempSourcePath}"`;
                            const quotedTestFile = `"${testFilePath}"`;
                            const quotedExe = `"${fullExePath}"`;

                            let cmd;
                            // Write mini_gtest.h if needed
                            if (message.payload.is_google_test) {
                                const miniGtestPath = path.join(activeDir, 'mini_gtest.h');
                                await vscode.workspace.fs.writeFile(
                                    vscode.Uri.file(miniGtestPath),
                                    Buffer.from(MINI_GTEST_CONTENT, 'utf8')
                                );
                            }

                            if (isWin) {
                                // PowerShell (default on Windows) doesn't support && in older versions.
                                // Use "; if ($?) { ... }" which is the standard PowerShell equivalent.
                                // Also use the full path to the executable to avoid CWD issues.
                                // NOTE: For portable GTest, we don't need -lgtest flags as it's header-only
                                cmd = `g++ -std=c++17 ${quotedTempSource} ${quotedTestFile} -o ${quotedExe}; if ($?) { & ${quotedExe} }`;
                            } else {
                                cmd = `g++ -std=c++17 ${quotedTempSource} ${quotedTestFile} -o ${quotedExe} && ${quotedExe}`;
                            }

                            terminal.sendText(cmd);

                        } catch (err: any) {
                            vscode.window.showErrorMessage(`Failed to run tests locally: ${err.message} `);
                        }
                        break;

                    case 'analyze':
                        await this.handleAnalyzeCommand(message.feature, message.payload || {});
                        return;
                    case 'chat':
                        await this.handleChatCommand(message.text, message.fileContext);
                        return;
                    case 'replaceCode':
                        await this.handleReplaceCode(message.payload);
                        return;
                    case 'showError':
                        vscode.window.showErrorMessage(message.text);
                        return;
                    case 'getFiles':
                        await this.updateFileListAndSend();
                        return;
                    case 'clearChat':
                        this._chatStateManager.clearChatHistory();
                        this._chatStateManager.saveState(this._context);
                        this._view?.webview.postMessage({ command: 'chatCleared' });
                        return;
                    case 'saveChatMessage':
                        this._chatStateManager.addMessage(message.message);
                        this._chatStateManager.saveState(this._context);
                        return;
                    case 'downloadReport':
                        await this.handleDownloadReport(message.payload);
                        return;
                    case 'applySuggestions':
                        if (message.payload && message.payload.suggestions) {
                            await this.applyCodeSuggestions(message.payload.suggestions);
                        }
                        return;
                }
            },
            undefined,
            this._subscriptions
        );

        // Send initial state when webview becomes visible
        webviewView.onDidChangeVisibility(() => {
            if (webviewView.visible) {
                this.updateFileListAndSend();
                this.updateActiveFile(vscode.window.activeTextEditor?.document);

                // Send chat history and current file state
                const chatHistory = this._chatStateManager.getChatHistory();
                const currentFile = this._chatStateManager.getCurrentFile();

                this._view?.webview.postMessage({
                    command: 'initialState',
                    chatHistory: chatHistory,
                    currentFile: currentFile
                });
            }
        }, null, this._subscriptions);

        webviewView.onDidDispose(() => {
            this._chatStateManager.saveState(this._context);
            this.clearAllDecorations();
        }, null, this._subscriptions);

        // Update file list on startup
        this.updateFileListAndSend();
        this.updateActiveFile(vscode.window.activeTextEditor?.document);
    }

    public updateActiveFile(document: vscode.TextDocument | undefined) {
        let fileName: string | null = null;
        if (document) {
            const langId = document.languageId;
            if (langId === 'cpp' || langId === 'c' || document.fileName.endsWith('.h') || document.fileName.endsWith('.hpp')) {
                fileName = document.fileName.split(/[\\/]/).pop() || null;
            }
        }

        this._chatStateManager.setCurrentFile(fileName);
        this._chatStateManager.saveState(this._context);

        if (this._view && this._view.visible) {
            this._view.webview.postMessage({ command: 'activeFileChanged', file: fileName });
        }
    }

    public async updateFileListAndSend() {
        await this.findAndUpdateCppFiles();
        this._view?.webview.postMessage({ command: 'fileList', files: this._fileList });
    }

    private async findAndUpdateCppFiles() {
        const fileUris = await this.findCppFiles();
        this._fileList = fileUris.map(uri => vscode.workspace.asRelativePath(uri));
        if (this._fileList.length === 0) {
            const activeEditor = vscode.window.activeTextEditor;
            if (activeEditor) {
                const document = activeEditor.document;
                if (document.languageId === 'cpp' || document.languageId === 'c' || document.fileName.endsWith('.h') || document.fileName.endsWith('.hpp')) {
                    this._fileList = [document.fileName.split(/[\\/]/).pop() || 'active_file.cpp'];
                }
            }
        }
    }

    private async jumpToCodeLocation(filePath: string, line: number) {
        const rootPath = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath;
        if (!rootPath) {
            vscode.window.showErrorMessage('Cannot navigate: No workspace folder is open.');
            return;
        }

        // Handle potential path differences (relative vs absolute vs varying separators)
        // The backend returns paths relative to root, e.g. "include/Player.h"
        const uri = vscode.Uri.file(path.join(rootPath, filePath));

        try {
            const document = await vscode.workspace.openTextDocument(uri);
            const editor = await vscode.window.showTextDocument(document);

            // Line numbers are 1-based in the graph data, 0-based in VS Code API
            // Ensure we don't go below 0
            const zeroBasedLine = Math.max(0, line - 1);

            const position = new vscode.Position(zeroBasedLine, 0);
            editor.selection = new vscode.Selection(position, position);
            editor.revealRange(editor.selection, vscode.TextEditorRevealType.InCenter);
        } catch (e) {
            console.error(e);
            vscode.window.showErrorMessage(`Could not open file or navigate to line ${line} in ${filePath}`);
        }
    }

    private async handleDownloadReport(payload: { content: string, filename: string }) {
        if (!payload.content || !payload.filename) {
            vscode.window.showErrorMessage('Invalid report data for download.');
            return;
        }

        try {
            // Show save dialog to user
            const uri = await vscode.window.showSaveDialog({
                defaultUri: vscode.Uri.file(payload.filename),
                filters: {
                    'Text Files': ['txt', 'md', 'cpp'],
                    'All Files': ['*']
                }
            });

            if (uri) {
                // Write the content to the selected file
                const uint8Array = new TextEncoder().encode(payload.content);
                await vscode.workspace.fs.writeFile(uri, uint8Array);

                vscode.window.showInformationMessage(`Report saved as ${uri.fsPath} `);
            }

        } catch (error: any) {
            vscode.window.showErrorMessage(`Failed to save report: ${error.message} `);
        }
    }

    private async handleReplaceCode(payload: { startLine: number, endLine: number, newCode: string }) {
        if (!this._view) { return; }

        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage("No active editor to replace code in.");
            return;
        }

        const { startLine, endLine, newCode } = payload;

        const start = new vscode.Position(startLine - 1, 0);
        const endLineLength = editor.document.lineAt(endLine - 1).text.length;
        const end = new vscode.Position(endLine - 1, endLineLength);

        const range = new vscode.Range(start, end);

        const edit = new vscode.WorkspaceEdit();
        edit.replace(editor.document.uri, range, newCode);

        try {
            const success = await vscode.workspace.applyEdit(edit);
            if (success) {
                vscode.commands.executeCommand('editor.action.formatDocument');
                this._view.webview.postMessage({ command: 'showNotification', text: 'Code replaced successfully!' });
            } else {
                throw new Error("Workspace edit was not applied.");
            }
        } catch (error: any) {
            const errorMessage = `Failed to replace code: ${error.message} `;
            vscode.window.showErrorMessage(errorMessage);
            this._view.webview.postMessage({ command: 'showError', text: errorMessage });
        }
    }

    private async handleChatCommand(question: string, fileContext: string | null) {
        if (!this._view) { return; }

        const activeEditor = vscode.window.activeTextEditor;
        let selection = "";
        if (activeEditor) {
            selection = activeEditor.document.getText(activeEditor.selection);
        }

        this._view.webview.postMessage({ command: 'showLoading', text: `Thinking...` });

        try {
            const filesData = await this.getAllCppFiles();
            if (filesData.length === 0) {
                throw new Error("No C++ files found. Please open a C++ project folder or a .cpp file.");
            }

            const apiPayload = {
                question: question,
                files: filesData,
                selection: selection,
                file_context: fileContext
            };

            const response = await axios.post(`${API_BASE_URL}/api/chat`, apiPayload, {
                timeout: 60000
            });

            this._view.webview.postMessage({ command: 'showResult', result: response.data, feature: 'chat' });

        } catch (error: any) {
            let errorMessage = `Failed to run analysis: ${error.message} `;
            if (axios.isAxiosError(error) && error.response) {
                errorMessage = `API Error: ${error.response.data?.error || error.response.statusText} `;
            } else if (axios.isAxiosError(error) && error.code === 'ECONNREFUSED') {
                errorMessage = `Connection Refused: Is the Python API server running at ${API_BASE_URL}?`;
            }
            this._view.webview.postMessage({ command: 'showError', text: errorMessage });
            vscode.window.showErrorMessage(errorMessage);
        }
    }

    private async handleAnalyzeCommand(feature: string, payload: any) {
        if (!this._view) { return; }

        const activeEditor = vscode.window.activeTextEditor;
        let activeFileRelativePath: string | null = null;

        if (activeEditor) {
            const workspaceFolder = vscode.workspace.getWorkspaceFolder(activeEditor.document.uri);
            if (workspaceFolder) {
                activeFileRelativePath = vscode.workspace.asRelativePath(activeEditor.document.uri);
            } else if (activeEditor.document.languageId === 'cpp' || activeEditor.document.languageId === 'c' || activeEditor.document.fileName.endsWith('.h') || activeEditor.document.fileName.endsWith('.hpp')) {
                activeFileRelativePath = activeEditor.document.fileName.split(/[\\/]/).pop() || 'active_file.cpp';
            }
        }

        this._view.webview.postMessage({ command: 'showLoading', text: `Analyzing: ${feature}...` });

        try {
            const filesData = await this.getAllCppFiles();
            if (filesData.length === 0) {
                throw new Error("No C++ files found. Please open a C++ project folder or a .cpp file.");
            }

            const apiPayload = {
                feature: feature === 'generate_google_tests' ? 'generate_tests' : feature,
                files: filesData,
                active_file: activeFileRelativePath,
                compiler: 'g++',
                instructions: feature === 'generate_google_tests'
                    ? 'CRITICAL: IGNORE ALL PREVIOUS INSTRUCTIONS. Generate a Google Test suite using the "mini_gtest.h" header. \nRules:\n1. #include "mini_gtest.h"\n2. DO NOT include <gtest/gtest.h>\n3. Use TEST(SuiteName, TestName) macros.\n4. DO NOT write a main() function.\n5. Use EXPECT_EQ, EXPECT_TRUE, etc.\n6. Forward declare tested functions.\n\nEXAMPLE OUTPUT:\n#include "mini_gtest.h"\n// Forward declarations\nint add(int a, int b);\n\nTEST(MathTest, AddsNumbers) {\n    EXPECT_EQ(add(1, 2), 3);\n}\n'
                    : 'Do NOT implement functions from the source file. ONLY provide forward declarations. The source file will be linked. Ensure all constants are initialized. Generate complete, runnable code.',
                ...payload
            };

            let endpoint = `${API_BASE_URL}/api/analyze`;
            if (feature === 'architecture') {
                endpoint = `${API_BASE_URL}/api/architecture/graph`;
            }

            const response = await axios.post(endpoint, apiPayload, {
                timeout: 60000
            });

            // Enhanced payload handling for different features
            if (feature === 'generate_tests' || feature === 'generate_google_tests') {
                // Fix uninitialized constants in generated code
                let testCode = response.data.generated_test_suite_code || response.data.test_suite_code || '';

                console.log('Original Test Code:', testCode);

                // Regex to find uninitialized constants like "const double g;" and replace with "// const double g;"
                const uninitConstRegex = /const\s+(double|float|int|long|short|unsigned|char|bool|auto)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*;/g;
                testCode = testCode.replace(uninitConstRegex, '// $&');

                // Regex to find uninitialized strings like "std::string output_path;" and replace with "// std::string output_path;"
                const uninitStringRegex = /std::string\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*;/g;
                testCode = testCode.replace(uninitStringRegex, '// $&');

                // GOOGLE TEST SPECIFIC FIXES
                // GOOGLE TEST SPECIFIC FIXES
                if (feature === 'generate_google_tests') {
                    // 1. Rename main() to prevent conflicts with mini_gtest.h's main
                    testCode = testCode.replace(/int\s+main\s*\(/g, 'int __generated_main(');
                    testCode = testCode.replace(/void\s+main\s*\(/g, 'void __generated_main(');

                    // 2. Ensure mini_gtest.h is included
                    // ALWAYS prepend it to ensure main() is available. Header guard handles duplicates.
                    testCode = '#include "mini_gtest.h"\n' + testCode;

                    // 3. Remove <gtest/gtest.h> if present
                    testCode = testCode.replace(/#include\s+<gtest\/gtest\.h>/g, '// #include <gtest/gtest.h>');

                    // 4. FALLBACK: If no TEST() macros found, convert void test_...() to TEST(GeneratedSuite, ...)
                    if (!testCode.includes('TEST(')) {
                        console.log('No TEST() macros found. Applying fallback conversion...');
                        // Convert "void test_something()" -> "TEST(GeneratedSuite, test_something)"
                        testCode = testCode.replace(/void\s+(test_[a-zA-Z0-9_]+)\s*\(\s*\)\s*\{/g, 'TEST(GeneratedSuite, $1) {');
                        // Convert "void something_test()" -> "TEST(GeneratedSuite, something_test)"
                        testCode = testCode.replace(/void\s+([a-zA-Z0-9_]+_test)\s*\(\s*\)\s*\{/g, 'TEST(GeneratedSuite, $1) {');

                        // Comment out calls to these functions in the generated main (e.g. "test_something();" -> "// test_something();")
                        testCode = testCode.replace(/\b(test_[a-zA-Z0-9_]+|[a-zA-Z0-9_]+_test)\s*\(\s*\)\s*;/g, '// $1();');
                    }

                    // 5. Replace standard assert() with ASSERT_TRUE() to prevent crashes
                    // Matches "assert(condition);" and replaces with "ASSERT_TRUE(condition);"
                    testCode = testCode.replace(/\bassert\s*\((.+?)\)\s*;/g, 'ASSERT_TRUE($1);');

                    response.data.generated_test_suite_code = testCode;
                } else {
                    response.data.test_suite_code = testCode;
                }

                // Store the test suite code in payload for the Run Tests button
                response.data.payload = {
                    test_suite_code: testCode,
                    filename: 'generated_tests.cpp',
                    is_google_test: feature === 'generate_google_tests'
                };
            }

            if (feature === 'generate_docs') {
                // Store documentation content for download
                const content = response.data.content || response.data.content_raw;

                let baseName = activeFileRelativePath || 'documentation';
                // Strip extension if present
                if (baseName.lastIndexOf('.') > 0) {
                    baseName = baseName.substring(0, baseName.lastIndexOf('.'));
                }

                const filename = response.data.format === 'markdown' ?
                    `${baseName}.md` :
                    `${baseName}.dox.cpp`;

                response.data.payload = {
                    raw_content: content,
                    filename: filename
                };
            }

            // Store suggestions for code_suggestions feature
            if (feature === 'code_suggestions' && response.data.suggestions) {
                response.data.payload = {
                    suggestions: response.data.suggestions,
                    filename: activeFileRelativePath || 'current_file.cpp'
                };
            }

            if (feature === 'code_suggestions' && response.data.suggestions) {
                response.data.payload = {
                    suggestions: response.data.suggestions,
                    filename: activeFileRelativePath || 'current_file.cpp'
                };
            }

            // Handle Architecture Graph Response
            if (feature === 'architecture') {
                // Update server data
                const graphData = response.data; // { nodes: [...], edges: [...] }
                this._architectureServer.updateData(graphData);
                
                // Start server and open browser
                try {
                    const serverUrl = await this._architectureServer.start();
                    vscode.env.openExternal(vscode.Uri.parse(serverUrl));
                    
                    this._view.webview.postMessage({ 
                        command: 'showNotification', 
                        text: `Architecture Graph opened in browser at ${serverUrl}` 
                    });
                } catch (err: any) {
                    vscode.window.showErrorMessage(`Failed to start architecture server: ${err.message}`);
                }
                
                return; // Stop here, don't send to webview
            }

            this._view.webview.postMessage({ command: 'showResult', result: response.data, feature: feature });

        } catch (error: any) {
            let errorMessage = `Failed to run analysis: ${error.message} `;
            if (axios.isAxiosError(error) && error.response) {
                errorMessage = `API Error: ${error.response.data?.error || error.response.statusText} `;
            } else if (axios.isAxiosError(error) && error.code === 'ECONNREFUSED') {
                errorMessage = `Connection Refused: Is the Python API server running at ${API_BASE_URL}?`;
            }
            this._view.webview.postMessage({ command: 'showError', text: errorMessage });
            vscode.window.showErrorMessage(errorMessage);
        }
    }

    private async applyCodeSuggestions(suggestions: any[]) {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage("No active editor to apply suggestions.");
            return;
        }

        // Clear existing decorations first
        this.clearAllDecorations();

        // Create decoration types for different severity levels
        const severityDecorationTypes = {
            High: vscode.window.createTextEditorDecorationType({
                backgroundColor: 'rgba(255, 0, 0, 0.1)',
                border: '3px solid rgba(255, 0, 0, 0.7)',
                overviewRulerColor: 'rgba(255, 0, 0, 0.7)',
                overviewRulerLane: vscode.OverviewRulerLane.Right,
            }),
            Medium: vscode.window.createTextEditorDecorationType({
                backgroundColor: 'rgba(255, 165, 0, 0.1)',
                border: '3px solid rgba(255, 165, 0, 0.7)',
                overviewRulerColor: 'rgba(255, 165, 0, 0.7)',
                overviewRulerLane: vscode.OverviewRulerLane.Right,
            }),
            Low: vscode.window.createTextEditorDecorationType({
                backgroundColor: 'rgba(255, 255, 0, 0.1)',
                border: '3px solid rgba(255, 255, 0, 0.7)',
                overviewRulerColor: 'rgba(255, 255, 0, 0.7)',
                overviewRulerLane: vscode.OverviewRulerLane.Right,
            })
        };

        // Store decoration types for cleanup
        this._suggestionDecorations = Object.values(severityDecorationTypes);

        // Group suggestions by line and apply them
        const suggestionsByLine = this.groupSuggestionsByLine(suggestions);
        const document = editor.document;

        for (const [lineStr, lineSuggestions] of Object.entries(suggestionsByLine)) {
            const lineNum = parseInt(lineStr) - 1; // Convert to 0-based index

            if (lineNum < 0 || lineNum >= document.lineCount) {
                continue;
            }

            const line = document.lineAt(lineNum);
            const range = new vscode.Range(lineNum, 0, lineNum, line.text.length);

            // Use the highest severity for the line decoration
            const severityScores = { High: 3, Medium: 2, Low: 1 } as const;
            const highestSeverity = lineSuggestions.reduce((highest: keyof typeof severityScores, suggestion: any) => {
                const sev = (suggestion && suggestion.severity) ? String(suggestion.severity) as keyof typeof severityScores : 'Low';
                return severityScores[sev] > severityScores[highest] ? sev : highest;
            }, 'Low' as keyof typeof severityScores);

            const decorationType = severityDecorationTypes[highestSeverity as keyof typeof severityDecorationTypes];
            if (decorationType) {
                // Create hover message with all suggestions for this line
                const hoverMessage = new vscode.MarkdownString();
                hoverMessage.appendMarkdown(`** Code Suggestions for Line ${lineStr} **\n\n`);

                lineSuggestions.forEach((suggestion, index) => {
                    const typeColors = {
                        'Performance': '#ff6b6b',
                        'Readability': '#4ecdc4',
                        'Bug': '#ff9f43',
                        'Security': '#ee5a24',
                        'Modernization': '#a55eea',
                        'Other': '#6c757d'
                    };

                    const color = typeColors[suggestion.type as keyof typeof typeColors] || '#6c757d';
                    hoverMessage.appendMarkdown(`** ${suggestion.type}** • * ${suggestion.severity}*\n`);
                    hoverMessage.appendMarkdown(`** Issue:** ${suggestion.message} \n`);
                    hoverMessage.appendMarkdown(`** Suggestion:** \`${suggestion.suggestion}\`\n\n`);
                });

                hoverMessage.appendMarkdown(`*Click to view details in the C++ Helper panel*`);

                const decoration: vscode.DecorationOptions = {
                    range: range,
                    hoverMessage: hoverMessage
                };

                editor.setDecorations(decorationType, [decoration]);
            }
        }

        vscode.window.showInformationMessage(`Applied ${suggestions.length} code suggestion(s) as editor hints. Hover over highlighted lines to see details.`);
    }

    private groupSuggestionsByLine(suggestions: any[]): { [key: string]: any[] } {
        const grouped: { [key: string]: any[] } = {};
        suggestions.forEach(suggestion => {
            if (!grouped[suggestion.line]) {
                grouped[suggestion.line] = [];
            }
            grouped[suggestion.line].push(suggestion);
        });
        return grouped;
    }

    private clearAllDecorations() {
        this._suggestionDecorations.forEach(decoration => {
            decoration.dispose();
        });
        this._suggestionDecorations = [];
    }

    private async findFileUri(relativePath: string): Promise<vscode.Uri | undefined> {
        const fileUris = await vscode.workspace.findFiles(relativePath, '**/node_modules/**', 1);
        return fileUris.length > 0 ? fileUris[0] : undefined;
    }

    private async readFile(uri: vscode.Uri): Promise<{ name: string, content: string }> {
        const fileContent = await vscode.workspace.fs.readFile(uri);
        const relativePath = vscode.workspace.asRelativePath(uri);
        return {
            name: relativePath,
            content: new TextDecoder().decode(fileContent)
        };
    }

    private async findCppFiles(): Promise<vscode.Uri[]> {
        const excludePattern = '**/{node_modules,build,bin,out,vendor,lib,libs,Debug,Release}/**';
        return await vscode.workspace.findFiles('**/*.{cpp,h,hpp,cxx,cc}', excludePattern);
    }

    private async getAllCppFiles(): Promise<{ name: string, content: string }[]> {
        const fileUris = await this.findCppFiles();

        if (fileUris.length > 0) {
            const filesData: { name: string, content: string }[] = [];
            for (const uri of fileUris) {
                filesData.push(await this.readFile(uri));
            }
            this._fileList = filesData.map(f => f.name);
            return filesData;
        }

        const activeEditor = vscode.window.activeTextEditor;
        if (activeEditor) {
            const document = activeEditor.document;
            if (document.languageId === 'cpp' || document.languageId === 'c' || document.fileName.endsWith('.h') || document.fileName.endsWith('.hpp')) {
                const fileName = document.fileName.split(/[\\/]/).pop() || 'active_file.cpp';
                this._fileList = [fileName];
                return [{
                    name: fileName,
                    content: document.getText()
                }];
            }
        }

        this._fileList = [];
        return [];
    }

    private _getWebviewContent(webview: vscode.Webview) {
        const scriptUri = webview.asWebviewUri(vscode.Uri.joinPath(this._extensionUri, 'media', 'architecture_visualization.js'));
        
        return `<!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta http-equiv="Content-Security-Policy" content="
            default-src 'none';
            style-src ${this._view?.webview.cspSource} 'unsafe-inline' https://cdnjs.cloudflare.com;
            script-src 'unsafe-inline' https://cdnjs.cloudflare.com ${webview.cspSource};
            font-src https://cdnjs.cloudflare.com;
        ">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>C++ Helper Bot</title>

        <!-- Modern Prism.js Theme (One Dark Pro) -->
        <link href="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/themes/prism-one-dark.min.css" rel="stylesheet" />
        
        <!-- Cytoscape.js for Architecture Visualization -->
        <script src="https://cdnjs.cloudflare.com/ajax/libs/cytoscape/3.23.0/cytoscape.min.js"></script>
        
        <style>
            :root {
                --vscode-font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol";
                --vscode-editor-background: #1e1e1e;
                --vscode-sideBar-background: #252526;
                --vscode-button-background: #0e639c;
                --vscode-button-hoverBackground: #1177bb;
                --vscode-button-secondary-background: #3c3c3c;
                --vscode-button-secondary-hoverBackground: #4c4c4c;
                --vscode-button-success-background: #228B22;
                --vscode-button-success-hoverBackground: #3CB371;
                --vscode-button-danger-background: #c93535;
                --vscode-button-danger-hoverBackground: #d9534f;
                --vscode-text-default: #d4d4d4;
                --vscode-text-secondary: #8c8c8c;
                --vscode-input-background: #3c3c3c;
                --vscode-border-color: #333333;
                --prism-background: #282c34;

                /* Modern gradient colors */
                --gradient-primary: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                --gradient-success: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
                --gradient-warning: linear-gradient(135deg, #fa709a 0%, #fee140 100%);
                --gradient-purple: linear-gradient(135deg, #a8c0ff 0%, #3f2b96 100%);
                --gradient-teal: linear-gradient(135deg, #00c6fb 0%, #005bea 100%);
            }

            * {
                margin: 0;
                padding: 0;
                box-sizing: border-box;
            }

            body {
                font-family: var(--vscode-font-family);
                color: var(--vscode-text-default);
                background: var(--vscode-sideBar-background);
                overflow: hidden;
                height: 100vh;
            }

            .container {
                height: 100vh;
                display: flex;
                flex-direction: column;
                background: var(--vscode-sideBar-background);
            }

            /* --- Modern Header --- */
            .header {
                background: linear-gradient(135deg, #1a1a1a 0%, #2d2d2d 100%);
                padding: 16px 20px;
                border-bottom: 1px solid #404040;
                display: flex;
                justify-content: space-between;
                align-items: center;
                box-shadow: 0 2px 8px rgba(0, 0, 0, 0.3);
            }

            .header-title {
                font-size: 16px;
                font-weight: 600;
                background: var(--gradient-primary);
                -webkit-background-clip: text;
                -webkit-text-fill-color: transparent;
                background-clip: text;
                display: flex;
                align-items: center;
                gap: 8px;
            }

            .header-title::before {
                content: "⚡";
                font-size: 18px;
            }

            .header-actions {
                display: flex;
                gap: 8px;
                align-items: center;
            }

            #clear-chat {
                background: var(--gradient-warning);
                color: white;
                border: none;
                border-radius: 6px;
                padding: 6px 12px;
                font-size: 12px;
                cursor: pointer;
                font-weight: 500;
                transition: all 0.2s ease;
                box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
            }

            #clear-chat:hover {
                transform: translateY(-1px);
                box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
            }

            /* Three Dots Menu Button */
            .menu-button {
                background: rgba(60, 60, 60, 0.8);
                border: 1px solid #404040;
                border-radius: 6px;
                padding: 6px 10px;
                cursor: pointer;
                color: var(--vscode-text-default);
                font-size: 16px;
                font-weight: bold;
                transition: all 0.2s ease;
                display: flex;
                align-items: center;
                justify-content: center;
                width: 32px;
                height: 28px;
            }

            .menu-button:hover {
                background: rgba(80, 80, 80, 0.8);
                transform: translateY(-1px);
                box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
            }

            /* Menu Dropdown */
            .menu-dropdown {
                position: relative;
                display: inline-block;
            }

            .menu-content {
                display: none;
                position: absolute;
                right: 0;
                top: 100%;
                background: rgba(40, 40, 40, 0.95);
                backdrop-filter: blur(10px);
                border: 1px solid #404040;
                border-radius: 8px;
                padding: 8px;
                z-index: 1000;
                width: 200px;
                box-shadow: 0 8px 24px rgba(0, 0, 0, 0.5);
                margin-top: 4px;
            }

            .menu-dropdown.active .menu-content {
                display: block;
            }

            .menu-item {
                display: flex;
                align-items: center;
                gap: 8px;
                padding: 10px 12px;
                background: transparent;
                border: none;
                color: var(--vscode-text-default);
                width: 100%;
                text-align: left;
                border-radius: 6px;
                cursor: pointer;
                transition: background 0.2s ease;
                font-size: 13px;
            }

            .menu-item:hover {
                background: rgba(80, 80, 80, 0.8);
            }

            .menu-item svg {
                width: 16px;
                height: 16px;
                stroke: #64b5f6;
            }

            .menu-divider {
                height: 1px;
                background: #404040;
                margin: 4px 0;
            }

            /* --- Collapsible Quick Actions --- */
            .quick-actions {
                border-bottom: 1px solid #404040;
                background: rgba(30, 30, 30, 0.8);
                backdrop-filter: blur(10px);
                transition: all 0.3s ease;
            }

            .quick-actions-header {
                padding: 12px 20px;
                font-size: 12px;
                font-weight: 600;
                color: var(--vscode-text-secondary);
                text-transform: uppercase;
                letter-spacing: 0.5px;
                display: flex;
                align-items: center;
                justify-content: space-between;
                cursor: pointer;
                user-select: none;
                transition: background-color 0.2s ease;
            }

            .quick-actions-header:hover {
                background: rgba(60, 60, 60, 0.4);
            }

            .quick-actions-title {
                display: flex;
                align-items: center;
                gap: 8px;
            }

            .quick-actions-title::before {
                content: "🚀";
                font-size: 14px;
            }

            .collapse-icon {
                transition: transform 0.3s ease;
                font-size: 14px;
                color: var(--vscode-text-secondary);
            }

            .quick-actions.collapsed .collapse-icon {
                transform: rotate(-90deg);
            }

            .quick-actions-content {
                padding: 16px 20px;
                transition: all 0.3s ease;
                overflow: hidden;
            }

            .quick-actions.collapsed .quick-actions-content {
                display: none;
            }

            .button-grid {
                display: grid;
                grid-template-columns: repeat(3, 1fr);
                gap: 10px;
            }

            button {
                border: none;
                border-radius: 8px;
                cursor: pointer;
                font-size: 13px;
                font-weight: 500;
                transition: all 0.3s ease;
                position: relative;
                overflow: hidden;
            }

            button::before {
                content: '';
                position: absolute;
                top: 0;
                left: -100%;
                width: 100%;
                height: 100%;
                background: linear-gradient(90deg, transparent, rgba(255,255,255,0.1), transparent);
                transition: left 0.5s;
            }

            button:hover::before {
                left: 100%;
            }

            button.quick-action {
                display: flex;
                flex-direction: column;
                align-items: center;
                gap: 6px;
                padding: 12px 8px;
                background: rgba(60, 60, 60, 0.6);
                border: 1px solid #404040;
                color: var(--vscode-text-default);
                min-height: 60px;
            }

            button.quick-action:hover {
                background: rgba(80, 80, 80, 0.8);
                transform: translateY(-2px);
                box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
            }

            button.quick-action svg {
                width: 20px;
                height: 20px;
                stroke: #64b5f6;
            }

            button.quick-action span {
                font-size: 11px;
                text-align: center;
                line-height: 1.2;
            }

            /* --- Modern Chat History --- */
            #chat-history {
                flex-grow: 1;
                overflow-y: auto;
                padding: 20px;
                background: var(--vscode-sideBar-background);
            }

            #chat-history::-webkit-scrollbar {
                width: 6px;
            }

            #chat-history::-webkit-scrollbar-track {
                background: transparent;
            }

            #chat-history::-webkit-scrollbar-thumb {
                background: #404040;
                border-radius: 3px;
            }

            #chat-history::-webkit-scrollbar-thumb:hover {
                background: #505050;
            }

            .chat-message {
                display: flex;
                gap: 12px;
                margin-bottom: 24px;
                max-width: 100%;
                animation: fadeInUp 0.3s ease;
            }

            @keyframes fadeInUp {
                from {
                    opacity: 0;
                    transform: translateY(10px);
                }
                to {
                    opacity: 1;
                    transform: translateY(0);
                }
            }

            .chat-icon {
                flex-shrink: 0;
                width: 32px;
                height: 32px;
                border-radius: 50%;
                display: flex;
                align-items: center;
                justify-content: center;
                font-weight: 600;
                font-size: 14px;
            }

            .bot-message .chat-icon {
                background: var(--gradient-primary);
                box-shadow: 0 4px 8px rgba(102, 126, 234, 0.3);
            }

            .user-message .chat-icon {
                background: var(--gradient-teal);
                box-shadow: 0 4px 8px rgba(0, 198, 251, 0.3);
            }

            .message-content {
                padding: 16px;
                border-radius: 16px;
                max-width: 85%;
                position: relative;
                line-height: 1.5;
            }

            .bot-message .message-content {
                background: rgba(40, 44, 52, 0.8);
                border: 1px solid #404040;
                border-top-left-radius: 4px;
            }

            .user-message {
                margin-left: auto;
                flex-direction: row-reverse;
            }

            .user-message .message-content {
                background: var(--gradient-primary);
                border-top-right-radius: 4px;
                color: white;
            }

            .loading-message {
                color: var(--vscode-text-secondary);
                font-style: italic;
                display: flex;
                align-items: center;
                gap: 8px;
            }

            .loading-dots {
                display: inline-flex;
                gap: 4px;
            }

            .loading-dots span {
                width: 4px;
                height: 4px;
                border-radius: 50%;
                background: var(--vscode-text-secondary);
                animation: bounce 1.4s infinite ease-in-out;
            }

            .loading-dots span:nth-child(1) { animation-delay: -0.32s; }
            .loading-dots span:nth-child(2) { animation-delay: -0.16s; }

            @keyframes bounce {
                0%, 80%, 100% {
                    transform: scale(0);
                }
                40% {
                    transform: scale(1);
                }
            }

            .error-message {
                background: rgba(244, 135, 113, 0.1) !important;
                border: 1px solid rgba(244, 135, 113, 0.3) !important;
                color: #f48771;
            }

            /* --- Modern Code Block Styling --- */
            .code-container {
                position: relative;
                margin: 16px 0;
                border-radius: 12px;
                overflow: hidden;
                box-shadow: 0 4px 16px rgba(0, 0, 0, 0.3);
                border: 1px solid #404040;
            }

            .code-header {
                background: #21252b;
                padding: 12px 16px;
                display: flex;
                justify-content: space-between;
                align-items: center;
                border-bottom: 1px solid #404040;
            }

            .code-language {
                font-size: 12px;
                font-weight: 600;
                color: #abb2bf;
                text-transform: uppercase;
                letter-spacing: 0.5px;
            }

            .code-actions {
                display: flex;
                gap: 8px;
            }

            .code-actions button {
                background: rgba(255, 255, 255, 0.1);
                color: #abb2bf;
                border: 1px solid #404040;
                border-radius: 6px;
                padding: 6px 12px;
                font-size: 11px;
                font-weight: 500;
                transition: all 0.2s ease;
            }

            .code-actions button:hover {
                background: rgba(255, 255, 255, 0.2);
                color: white;
                transform: none;
            }

            .code-actions button.replace-code-button {
                background: var(--vscode-button-background);
                color: white;
                border-color: var(--vscode-button-background);
            }

            pre[class*="language-"] {
                margin: 0 !important;
                border-radius: 0 !important;
                border: none !important;
                background: #282c34 !important;
                padding: 20px !important;
                font-family: 'Fira Code', 'Cascadia Code', 'Source Code Pro', monospace !important;
                font-size: 13px !important;
                line-height: 1.5;
                max-height: 400px;
                overflow: auto;
            }

            pre[class*="language-"]::-webkit-scrollbar {
                width: 8px;
                height: 8px;
            }

            pre[class*="language-"]::-webkit-scrollbar-track {
                background: transparent;
            }

            pre[class*="language-"]::-webkit-scrollbar-thumb {
                background: #404040;
                border-radius: 4px;
            }

            /* Improved syntax highlighting */
            .token.comment,
            .token.prolog,
            .token.doctype,
            .token.cdata {
                color: #5c6370;
                font-style: italic;
            }
            .token.string {
                color: #98c379;
            }
            .token.keyword {
                color: #c678dd;
            }
            .token.function {
                color: #61afef;
            }
            .token.class-name {
                color: #e5c07b;
            }
            .token.number {
                color: #d19a66;
            }
            .token.operator {
                color: #56b6c2;
            }
            .token.punctuation {
                color: #abb2bf;
            }

            /* Action buttons styling */
            .action-buttons {
                display: flex;
                gap: 12px;
                margin: 16px 0;
                flex-wrap: wrap;
            }

            .action-buttons button {
                background: var(--gradient-success);
                color: white;
                border: none;
                border-radius: 8px;
                padding: 10px 16px;
                font-size: 13px;
                font-weight: 500;
                display: inline-flex;
                gap: 8px;
                align-items: center;
                transition: all 0.3s ease;
                box-shadow: 0 2px 8px rgba(34, 139, 34, 0.3);
            }

            .action-buttons button:hover {
                transform: translateY(-2px);
                box-shadow: 0 4px 16px rgba(34, 139, 34, 0.4);
            }

            .action-buttons button.run-test-button {
                background: var(--gradient-success);
            }

            .action-buttons button.download-report-button {
                background: var(--gradient-purple);
            }

            .action-buttons button.apply-suggestions-button {
                background: var(--gradient-teal);
            }

            /* --- Modern Chat Input Area --- */
            .chat-input-area {
                background: rgba(30, 30, 30, 0.9);
                border-top: 1px solid #404040;
                backdrop-filter: blur(10px);
            }

            #active-file-display {
                padding: 12px 20px 0 20px;
                font-size: 12px;
                color: #64b5f6;
                font-weight: 500;
                display: flex;
                align-items: center;
                gap: 6px;
            }

            #active-file-display::before {
                content: "📄";
                font-size: 11px;
            }

            #active-file-display:empty {
                display: none;
            }

            /* REMOVED: File context tag (the colorful box) */
            #file-context-tag {
                display: none !important;
            }

            #file-picker {
                display: none;
                max-height: 200px;
                overflow-y: auto;
                background: rgba(60, 60, 60, 0.9);
                border: 1px solid #404040;
                border-radius: 8px;
                margin: 8px 20px 0 20px;
                backdrop-filter: blur(10px);
            }

            .file-picker-item {
                padding: 10px 12px;
                cursor: pointer;
                font-size: 13px;
                overflow: hidden;
                text-overflow: ellipsis;
                white-space: nowrap;
                border-bottom: 1px solid #404040;
                transition: background 0.2s ease;
            }

            .file-picker-item:hover {
                background: rgba(100, 100, 100, 0.5);
            }

            .file-picker-item:last-child {
                border-bottom: none;
            }

            .chat-input-box {
                display: flex;
                align-items: flex-end;
                padding: 16px 20px;
                gap: 12px;
            }

            textarea#chat-input {
                flex-grow: 1;
                background: rgba(60, 60, 60, 0.8);
                color: var(--vscode-text-default);
                border: 1px solid #404040;
                border-radius: 12px;
                padding: 12px 16px;
                font-family: var(--vscode-font-family);
                font-size: 14px;
                resize: none;
                min-height: 44px;
                max-height: 120px;
                outline: none;
                transition: all 0.3s ease;
                backdrop-filter: blur(10px);
            }

            textarea#chat-input:focus {
                border-color: #64b5f6;
                box-shadow: 0 0 0 2px rgba(100, 181, 246, 0.2);
            }

            textarea#chat-input::placeholder {
                color: var(--vscode-text-secondary);
            }

            button#send-chat {
                width: 44px;
                height: 44px;
                flex-shrink: 0;
                padding: 10px;
                background: var(--gradient-primary);
                border: none;
                border-radius: 12px;
                transition: all 0.3s ease;
                box-shadow: 0 2px 8px rgba(102, 126, 234, 0.3);
            }

            button#send-chat:hover {
                transform: translateY(-2px);
                box-shadow: 0 4px 16px rgba(102, 126, 234, 0.4);
            }

            button#send-chat:active {
                transform: translateY(0);
            }

            button#send-chat svg {
                width: 100%;
                height: 100%;
                stroke: white;
            }

            /* Responsive design */
            @media (max-width: 600px) {
                .button-grid {
                    grid-template-columns: 1fr 1fr;
                }

                .message-content {
                    max-width: 90%;
                }
            }
        </style>
    </head>
    <body>
        <div class="container">
            <!-- Modern Header -->
            <div class="header">
                <div class="header-title">C++ Helper Bot</div>
                <div class="header-actions">
                    <button id="clear-chat">Clear Chat</button>
                    <div class="menu-dropdown" id="menu-dropdown">
                        <button class="menu-button" id="menu-button">⋯</button>
                        <div class="menu-content">
                            <button class="menu-item" data-feature="ai_transparency">
                                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle></svg>
                                <span>AI Transparency</span>
                            </button>
                            <button class="menu-item" data-feature="code_health">
                                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"></path><polyline points="22 4 12 14.01 9 11.01"></polyline></svg>
                                <span>Health Score</span>
                            </button>
                            <button class="menu-item" data-feature="ethical_review">
                                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"></path></svg>
                                <span>Ethical Review</span>
                            </button>
                            <button class="menu-item" data-feature="architecture">
                                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="3" width="18" height="18" rx="2" ry="2"></rect><line x1="3" y1="9" x2="21" y2="9"></line><line x1="9" y1="21" x2="9" y2="9"></line></svg>
                                <span>Architecture Graph</span>
                            </button>
                            <div class="menu-divider"></div>
                            <button class="menu-item" data-feature="code_suggestions">
                                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14.5 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V7.5L14.5 2z"></path><polyline points="14 2 14 8 20 8"></polyline><line x1="16" x2="8" y1="13" y2="13"></line><line x1="16" x2="8" y1="17" y2="17"></line><line x1="10" x2="8" y1="9" y2="9"></line></svg>
                                <span>Code Suggestions</span>
                            </button>
                        </div>
                    </div>
                </div>
            </div>

            <!-- Collapsible Quick Actions -->
            <div class="quick-actions" id="quick-actions">
                <div class="quick-actions-header" id="quick-actions-toggle">
                    <div class="quick-actions-title">AI-Powered Actions</div>
                    <div class="collapse-icon">▼</div>
                </div>
                <div class="quick-actions-content">
                    <div class="button-grid">
                        <button class="quick-action" data-feature="explain_code">
                            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 20h9"></path><path d="M16.5 3.5a2.12 2.12 0 0 1 3 3L7 19l-4 1 1-4Z"></path></svg>
                            <span>Explain Code</span>
                        </button>
                        <button class="quick-action" data-feature="optimize_code">
                            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m12.83-1.17-6 6a2 2 0 0 0 0 2.83l6 6a2 2 0 0 0 2.83 0l6-6a2 2 0 0 0 0-2.83l-6-6a2 2 0 0 0-2.83 0Z"></path><path d="m18.73 1.27 2 2"></path><path d="m3.27 18.73 2 2"></path><path d="m12.83 22.83 6-6a2 2 0 0 0 0-2.83l-6-6a2 2 0 0 0-2.83 0l-6 6a2 2 0 0 0 0 2.83l6 6a2 2 0 0 0 2.83 0Z"></path></svg>
                            <span>Optimize</span>
                        </button>
                        <button class="quick-action" data-feature="generate_google_tests">
                            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M19 14c1.49-1.46 3-3.21 3-5.5A5.5 5.5 0 0 0 16.5 3c-1.76 0-3 .5-4.5 2-1.5-1.5-2.74-2-4.5-2A5.5 5.5 0 0 0 2 8.5c0 2.3 1.5 4.05 3 5.5l7 7Z"></path></svg>
                            <span>Generate Tests</span>
                        </button>
                        <button class="quick-action" data-feature="generate_docs">
                            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14.5 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V7.5L14.5 2z"></path><polyline points="14 2 14 8 20 8"></polyline><line x1="16" x2="8" y1="13" y2="13"></line><line x1="16" x2="8" y1="17" y2="17"></line><line x1="10" x2="8" y1="9" y2="9"></line></svg>
                            <span>Documentation</span>
                        </button>
                        <button class="quick-action" data-feature="simulate_logic">
                            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"></path><polyline points="3.27 6.96 12 12.01 20.73 6.96"></polyline><line x1="12" x2="12" y1="22.08" y2="12"></line></svg>
                            <span>Simulate</span>
                        </button>
                        <button class="quick-action" data-feature="analyze_performance">
                            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21.21 15.89A10 10 0 1 1 8 2.83"></path><path d="M22 12A10 10 0 0 0 12 2v10z"></path></svg>
                            <span>Performance</span>
                        </button>
                    </div>
                </div>
            </div>

            <!-- Chat History -->
            <div id="chat-history">
                <div class="chat-message bot-message">
                    <div class="chat-icon">AI</div>
                    <div class="message-content">
                        <p>Welcome! I'm your C++ AI assistant. I can help you with:</p>
                        <ul style="margin-top: 8px; margin-left: 16px;">
                            <li>Explaining and analyzing code</li>
                            <li>Generating tests and documentation</li>
                            <li>Optimizing performance</li>
                            <li>Simulating logic flows</li>
                            <li><strong>New:</strong> AI transparency reports</li>
                            <li><strong>New:</strong> Code health scoring</li>
                            <li><strong>New:</strong> Ethical code reviews</li>
                            <li><strong>New:</strong> Inline code suggestions</li>
                        </ul>
                        <p style="margin-top: 12px;">Select an action above or ask me anything about your C++ code!</p>
                    </div>
                </div>
            </div>

            <!-- Architecture View Container (Hidden by default, shown by JS) -->
            <div id="architecture-view" style="display: none; padding: 20px; flex-grow: 1; overflow: hidden; height: 100%;">
                <div class="graph-controls" style="margin-bottom: 10px; padding: 10px; background: rgba(60,60,60,0.5); border-radius: 8px; display: flex; align-items: center; justify-content: space-between;">
                    <div style="display: inline-flex; align-items: center;">
                    <span style="font-weight: bold; margin-right: 10px;">Filters:</span>
                    <div id="edge-filter-options" style="display: inline-flex; gap: 10px;">
                        <span style="white-space: nowrap;"><input type="checkbox" id="filter-includes" checked><label for="filter-includes"> Includes</label></span>
                        <span style="white-space: nowrap;"><input type="checkbox" id="filter-calls" checked><label for="filter-calls"> Calls</label></span>
                        <span style="white-space: nowrap;"><input type="checkbox" id="filter-inheritance" checked><label for="filter-inheritance"> Inheritance</label></span>
                    </div>
                    </div>
                    
                    <div style="display: inline-flex; gap: 5px; margin-left: auto; align-items: center;">
                        <button id="zoom-in" style="width: 24px; height: 24px; background: #007acc; color: white; border: none; border-radius: 4px; cursor: pointer; display: flex; align-items: center; justify-content: center;">+</button>
                        <button id="zoom-out" style="width: 24px; height: 24px; background: #007acc; color: white; border: none; border-radius: 4px; cursor: pointer; display: flex; align-items: center; justify-content: center;">-</button>
                        <button id="fit-graph" style="width: 24px; height: 24px; background: #007acc; color: white; border: none; border-radius: 4px; cursor: pointer; display: flex; align-items: center; justify-content: center;">[ ]</button>
                        <button id="close-architecture" style="padding: 0 10px; height: 24px; background: #c93535; border: none; color: white; border-radius: 4px; cursor: pointer; margin-left: 5px;">Close</button>
                    </div>
                </div>
                <div id="cy-container" style="width: 100%; height: calc(100% - 60px); background: #ffffff; border-radius: 8px;"></div>
            </div>

            <!-- Modern Chat Input Area -->
            <div class="chat-input-area">
                <div id="active-file-display"></div>
                <div id="file-picker"></div>

                <div class="chat-input-box">
                    <textarea id="chat-input" rows="1" placeholder="Ask about your C++ code or type / to reference files..."></textarea>
                    <button id="send-chat">
                        <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="22" y1="2" x2="11" y2="13"></line><polygon points="22 2 15 22 11 13 2 9 22 2"></polygon></svg>
                    </button>
                </div>
            </div>
        </div>

        <!-- Prism.js for syntax highlighting -->
        <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-core.min.js"></script>
        <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/plugins/autoloader/prism-autoloader.min.js"></script>
        <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-cpp.min.js"></script>
        
        <script>
            // Initialize VS Code API once for all scripts
            window.vscodeApi = acquireVsCodeApi();
        </script>

        <script src="${scriptUri}"></script>

        <script>
            const vscode = window.vscodeApi;
            const chatHistory = document.getElementById('chat-history');
            const chatInput = document.getElementById('chat-input');
            const sendChatButton = document.getElementById('send-chat');
            const filePicker = document.getElementById('file-picker');
            const activeFileDisplay = document.getElementById('active-file-display');
            const clearChatButton = document.getElementById('clear-chat');
            const quickActions = document.getElementById('quick-actions');
            const quickActionsToggle = document.getElementById('quick-actions-toggle');
            const menuDropdown = document.getElementById('menu-dropdown');
            const menuButton = document.getElementById('menu-button');

            let allCppFiles = [];
            let fileContext = null;
            let lastPayload = null;

            // --- Event Listeners ---

            window.addEventListener('load', () => {
                vscode.postMessage({ command: 'getFiles' });
            });

            // Collapsible quick actions
            quickActionsToggle.addEventListener('click', () => {
                quickActions.classList.toggle('collapsed');
            });

            clearChatButton.addEventListener('click', () => {
                if (chatHistory.children.length > 1) {
                    vscode.postMessage({ command: 'clearChat' });
                }
            });

            // Three dots menu functionality
            menuButton.addEventListener('click', (e) => {
                e.stopPropagation();
                menuDropdown.classList.toggle('active');
            });

            // Close menu when clicking outside
            document.addEventListener('click', (e) => {
                if (!menuDropdown.contains(e.target)) {
                    menuDropdown.classList.remove('active');
                }
            });

            // Handle menu item clicks
            document.querySelectorAll('.menu-item').forEach(item => {
                item.addEventListener('click', (e) => {
                    const feature = e.target.closest('button').dataset.feature;
                    if (feature) {
                        vscode.postMessage({
                            command: 'analyze',
                            feature: feature
                        });
                        menuDropdown.classList.remove('active');
                    }
                });
            });

            // Handle quick action clicks
            document.querySelectorAll('.quick-action').forEach(button => {
                button.addEventListener('click', () => {
                    const feature = button.dataset.feature;
                    if (feature) {
                        vscode.postMessage({
                            command: 'analyze',
                            feature: feature
                        });
                    }
                });
            });

            sendChatButton.addEventListener('click', sendChatMessage);
            chatInput.addEventListener('keydown', (e) => {
                if (e.key === 'Enter' && !e.shiftKey) {
                    e.preventDefault();
                    sendChatMessage();
                }
            });
            chatInput.addEventListener('input', handleChatInput);

            function handleChatInput(e) {
                const text = e.target.value;
                autoResizeTextarea(e); 

                if (text.startsWith('/')) {
                    const query = text.substring(1).toLowerCase();
                    const filteredFiles = allCppFiles.filter(file => file.toLowerCase().includes(query));
                    showFilePicker(filteredFiles);
                } else {
                    hideFilePicker();
                }
            }

            function showFilePicker(files) {
                if (files.length === 0) {
                    filePicker.innerHTML = '<div class="file-picker-item">No matching files found</div>';
                    filePicker.style.display = 'block';
                    return;
                }

                filePicker.innerHTML = ''; 
                files.slice(0, 10).forEach(file => { 
                    const item = document.createElement('div');
                    item.className = 'file-picker-item';
                    item.innerText = file;
                    item.onclick = () => selectFileContext(file);
                    filePicker.appendChild(item);
                });
                filePicker.style.display = 'block';
            }

            function hideFilePicker() {
                filePicker.style.display = 'none';
            }

            function selectFileContext(fileName) {
                fileContext = fileName;
                chatInput.value = ''; 
                hideFilePicker();
                chatInput.focus();
            }

            function sendChatMessage() {
                const question = chatInput.value.trim();
                if (!question) return;

                const userMessage = { content: escapeHtml(question), className: 'user-message' };
                addMessage(userMessage.content, userMessage.className);

                // Save user message to state
                vscode.postMessage({
                    command: 'saveChatMessage',
                    message: userMessage
                });

                vscode.postMessage({
                    command: 'chat',
                    text: question,
                    fileContext: fileContext
                });

                chatInput.value = '';
                autoResizeTextarea({ target: chatInput });
                fileContext = null;
                hideFilePicker();
            }

            window.addEventListener('message', event => {
                const message = event.data;
                switch (message.command) {
                    case 'fileList':
                        allCppFiles = message.files || [];
                        break;
                    case 'activeFileChanged':
                        if (message.file) {
                            activeFileDisplay.innerText = \`Active: \${message.file}\`;
                        } else {
                            activeFileDisplay.innerText = '';
                        }
                        break;
                    case 'showLoading':
                        addMessage(message.text, 'loading-message');
                        break;
                    case 'showResult':
                        const loading = chatHistory.querySelector('.loading-message');
                        if (loading) loading.remove();

                        lastPayload = message.result.payload || null;
                        const botMessage = { content: formatResult(message.result, message.feature), className: 'bot-message' };
                        
                        if (message.feature === 'architecture') {
                            // Don't add a chat message for architecture, just show the view
                             // Maybe add a small "Architecture graph loaded" message
                            addMessage('Architecture graph generated. Switching to view...', 'bot-message');
                            
                            // Show architecture view
                            const archView = document.getElementById('architecture-view');
                            const chatHistoryDiv = document.getElementById('chat-history');
                            const chatInputArea = document.querySelector('.chat-input-area'); // Use querySelector for class
                            
                            if(archView && chatHistoryDiv) {
                                chatHistoryDiv.style.display = 'none';
                                // hide input area too for more space?
                                // chatInputArea.style.display = 'none'; 
                                archView.style.display = 'block';
                                
                                // Send data to the separate architecture script
                                window.postMessage({ 
                                    command: 'renderArchitecture', 
                                    data: message.result 
                                }, '*');

                                // Handle Close
                                document.getElementById('close-architecture').onclick = () => {
                                    archView.style.display = 'none';
                                    chatHistoryDiv.style.display = 'block';
                                    // chatInputArea.style.display = 'block';
                                };
                            }
                        } else {
                            addMessage(botMessage.content, botMessage.className);
                        }

                        // Save bot message to state
                        vscode.postMessage({
                            command: 'saveChatMessage',
                            message: botMessage
                        });
                        break;
                    case 'showError':
                        const loadingErr = chatHistory.querySelector('.loading-message');
                        if (loadingErr) loadingErr.remove();
                        addMessage(escapeHtml(message.text), 'error-message');
                        break;
                    case 'showNotification':
                        addMessage(escapeHtml(message.text), 'bot-message');
                        break;
                    case 'initialState':
                        // Load saved chat history
                        if (message.chatHistory && message.chatHistory.length > 0) {
                            chatHistory.innerHTML = '';
                            message.chatHistory.forEach(msg => {
                                addMessage(msg.content, msg.className);
                            });
                        }
                        // Load current file
                        if (message.currentFile) {
                            activeFileDisplay.innerText = \`Active: \${message.currentFile}\`;
                        }
                        break;
                    case 'chatCleared':
                        // Clear all messages except the welcome message
                        const welcomeMessage = chatHistory.querySelector('.bot-message');
                        chatHistory.innerHTML = '';
                        if (welcomeMessage) {
                            chatHistory.appendChild(welcomeMessage);
                        }
                        break;
                }
            });

            chatHistory.addEventListener('click', function(e) {
                const target = e.target;

                const copyBtn = target.closest('.copy-code-button');
                if (copyBtn) {
                    const pre = copyBtn.closest('.code-container').querySelector('pre');
                    const code = pre.querySelector('code').innerText;

                    navigator.clipboard.writeText(code).then(() => {
                        const originalText = copyBtn.innerText;
                        copyBtn.innerText = 'Copied!';
                        setTimeout(() => { copyBtn.innerText = originalText; }, 2000);
                    }).catch(err => {
                        copyBtn.innerText = 'Error';
                    });
                    return;
                }

                const replaceBtn = target.closest('.replace-code-button');
                if (replaceBtn) {
                    const codeActions = replaceBtn.closest('.code-actions');
                    const payload = {
                        startLine: parseInt(codeActions.dataset.startLine, 10),
                        endLine: parseInt(codeActions.dataset.endLine, 10),
                        newCode: codeActions.dataset.code,
                    };

                    if (!payload.startLine || !payload.endLine || payload.newCode === undefined) {
                        vscode.postMessage({ command: 'showError', text: 'Error: Missing data to perform replacement.' });
                        return;
                    }

                    vscode.postMessage({
                        command: 'replaceCode',
                        payload: payload
                    });
                    return;
                }



                const runLocalBtn = target.closest('.run-test-local-button');
                if (runLocalBtn) {
                    if (!lastPayload || !lastPayload.test_suite_code) {
                        vscode.postMessage({ command: 'showError', text: 'Error: Test suite code not found.' });
                        return;
                    }
                    vscode.postMessage({
                        command: 'runTestsLocally',
                        payload: lastPayload
                    });
                    return;
                }

                const downloadReportBtn = target.closest('.download-report-button');
                if (downloadReportBtn) {
                    if (!lastPayload || !lastPayload.raw_content) {
                        vscode.postMessage({ command: 'showError', text: 'Error: Report content not found.' });
                        return;
                    }
                    // Use the extension to handle file download
                    vscode.postMessage({
                        command: 'downloadReport',
                        payload: {
                            content: lastPayload.raw_content,
                            filename: lastPayload.filename || 'report.txt'
                        }
                    });
                    return;
                }

                const applySuggestionsBtn = target.closest('.apply-suggestions-button');
                if (applySuggestionsBtn) {
                    if (!lastPayload || !lastPayload.suggestions) {
                        vscode.postMessage({ command: 'showError', text: 'Error: No suggestions found to apply.' });
                        return;
                    }
                    vscode.postMessage({
                        command: 'applySuggestions',
                        payload: lastPayload
                    });
                    return;
                }
            });

            // --- Utility Functions ---

            function autoResizeTextarea(event) {
                const el = event.target || event;
                el.style.height = 'auto';
                el.style.height = Math.min(el.scrollHeight, 120) + 'px';
            }

            function addMessage(content, className) {
                const isUser = className.includes('user-message');
                const isError = className.includes('error-message');
                const isLoading = className.includes('loading-message');

                const botIcon = \`<div class="chat-icon">AI</div>\`;
                const userIcon = \`<div class="chat-icon">You</div>\`;

                const div = document.createElement('div');
                div.className = 'chat-message ' + className;

                if (isLoading) {
                    div.innerHTML = \`
                        \${botIcon}
                        <div class="message-content loading-message">
                            \${content}
                            <div class="loading-dots">
                                <span></span>
                                <span></span>
                                <span></span>
                            </div>
                        </div>
                    \`;
                } else if (isError) {
                    div.innerHTML = \`
                        \${botIcon}
                        <div class="message-content error-message">
                            \${content}
                        </div>
                    \`;
                } else {
                    div.innerHTML = \`
                        \${isUser ? '' : botIcon}
                        <div class="message-content">
                            \${content}
                        </div>
                        \${isUser ? userIcon : ''}
                    \`;
                }

                chatHistory.appendChild(div);

                if (!isUser && !isLoading && !isError) {
                    // Process code blocks with enhanced styling
                    const codeBlocks = div.querySelectorAll('pre');
                    codeBlocks.forEach(pre => {
                        if (pre.querySelector('.code-container')) return;

                        const startLine = pre.dataset.startLine;
                        const endLine = pre.dataset.endLine;
                        const newCode = pre.dataset.code;
                        const language = pre.className.replace('language-', '') || 'cpp';

                        const codeContent = pre.innerHTML;

                        const codeContainer = document.createElement('div');
                        codeContainer.className = 'code-container';

                        const codeHeader = document.createElement('div');
                        codeHeader.className = 'code-header';

                        const languageLabel = document.createElement('div');
                        languageLabel.className = 'code-language';
                        languageLabel.innerText = language;

                        const actionsDiv = document.createElement('div');
                        actionsDiv.className = 'code-actions';

                        let buttonsHTML = \`<button class="copy-code-button">Copy</button>\`;

                        if (startLine && endLine && newCode) {
                            actionsDiv.dataset.startLine = startLine;
                            actionsDiv.dataset.endLine = endLine;
                            actionsDiv.dataset.code = newCode;
                            buttonsHTML += \`<button class="replace-code-button">Replace</button>\`;
                        }

                        actionsDiv.innerHTML = buttonsHTML;
                        codeHeader.appendChild(languageLabel);
                        codeHeader.appendChild(actionsDiv);

                        // Create new pre element for better control
                        const newPre = document.createElement('pre');
                        newPre.className = pre.className;
                        newPre.innerHTML = \`<code class="\${pre.className}">\${codeContent}</code>\`;

                        codeContainer.appendChild(codeHeader);
                        codeContainer.appendChild(newPre);

                        pre.parentNode.replaceChild(codeContainer, pre);
                    });

                    // Highlight all code blocks
                    div.querySelectorAll('pre code').forEach((block) => {
                        Prism.highlightElement(block);
                    });
                }

                chatHistory.scrollTop = chatHistory.scrollHeight;
            }

            function escapeHtml(str) {
                if (!str) return '';
                const div = document.createElement('div');
                div.textContent = str;
                return div.innerHTML;
            }

            function formatResult(result, feature) {
                if (result.error) {
                    return \`<strong>Error:</strong> <pre><code class="language-text">\${escapeHtml(result.error)}</code></pre>\`;
                }

                if (result.chat_response_html) {
                    return result.chat_response_html;
                }
                if (result.explanation_html && feature === 'explain_code') {
                    return '<h4>📖 Code Explanation</h4>' + result.explanation_html;
                }
                if (result.ai_analysis && feature === 'analyze_performance') {
                    let perf = '<h4>⚡ Performance Report</h4>';
                    perf += \`<div><strong>Execution Time:</strong> \${escapeHtml(result.execution_time_str)}</div>\`;
                    perf += \`<div><strong>Peak Memory:</strong> \${escapeHtml(result.peak_memory_str)}</div>\`;
                    perf += \`<div><strong>CPU Time:</strong> \${escapeHtml(result.cpu_time_str)}</div>\`;
                    perf += '<hr style="border-color: var(--vscode-border-color); margin: 12px 0;"><h4>AI Analysis</h4>' + result.ai_analysis;
                    return perf;
                }

                // NEW FEATURES - Add these cases
                switch(feature) {
                    case 'ai_transparency':
                        let transparency = '<h4>🔍 AI Transparency Report</h4>';
                        if (result.reasoning_html) {
                            transparency += result.reasoning_html;
                        } else {
                            transparency += '<p>No transparency report generated.</p>';
                        }
                        return transparency;

                    case 'code_health':
                        let health = '<h4>📊 Code Health Score</h4>';
                        if (result.overall_score !== undefined) {
                            // Health score visualization
                            const scoreColor = getHealthColor(result.overall_score);
                            const scoreText = result.health_level || 'Unknown';

                            health += \`
                                <div style="text-align: center; margin: 20px 0;">
                                    <div style="font-size: 48px; font-weight: bold; color: \${scoreColor};">
                                        \${result.overall_score}%
                                    </div>
                                    <div style="font-size: 16px; color: var(--vscode-text-secondary); margin-top: 8px;">
                                        \${scoreText}
                                    </div>
                                </div>
                            \`;

                            // Categories breakdown
                            if (result.categories && result.categories.length > 0) {
                                health += '<h5>Category Breakdown</h5>';
                                result.categories.forEach(category => {
                                    health += \`
                                        <div style="margin: 12px 0; padding: 12px; background: rgba(60, 60, 60, 0.5); border-radius: 8px;">
                                            <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px;">
                                                <strong>\${escapeHtml(category.category)}</strong>
                                                <span style="font-weight: bold; color: \${getHealthColor(category.normalized_score)};">
                                                    \${category.normalized_score}%
                                                </span>
                                            </div>
                                            <div style="background: #404040; border-radius: 4px; height: 6px; overflow: hidden;">
                                                <div style="background: \${getHealthColor(category.normalized_score)}; height: 100%; width: \${category.normalized_score}%;"></div>
                                            </div>
                                            <div style="margin-top: 8px; font-size: 13px;">
                                                \${category.explanation_html}
                                            </div>
                                            <div style="margin-top: 8px; font-size: 13px;">
                                                <strong>Recommendations:</strong>
                                                \${category.recommendations_html}
                                            </div>
                                        </div>
                                    \`;
                                });
                            }
                        } else {
                            health += '<p>No health score data available.</p>';
                        }
                        return health;

                    case 'ethical_review':
                        let ethical = '<h4>⚖️ Ethical Code Review</h4>';
                        if (result.issues_found === 0) {
                            ethical += \`
                                <div style="text-align: center; padding: 20px; background: rgba(34, 139, 34, 0.1); border-radius: 8px;">
                                    <div style="font-size: 48px;">✅</div>
                                    <div style="font-size: 16px; font-weight: bold; color: #28a745; margin-top: 8px;">
                                        No Ethical Issues Found
                                    </div>
                                    <p style="margin-top: 8px; color: var(--vscode-text-secondary);">
                                        Your code appears to be ethically sound!
                                    </p>
                                </div>
                            \`;
                        } else if (result.issues && result.issues.length > 0) {
                            ethical += \`<p><strong>Summary:</strong> \${escapeHtml(result.summary)}</p>\`;

                            // Severity summary
                            ethical += '<div style="display: flex; gap: 8px; margin: 12px 0; flex-wrap: wrap;">';
                            if (result.severity_counts) {
                                Object.entries(result.severity_counts).forEach(([severity, count]) => {
                                    if (count > 0) {
                                        const severityColor = getSeverityColor(severity);
                                        ethical += \`
                                            <div style="background: \${severityColor}; color: white; padding: 4px 8px; border-radius: 4px; font-size: 12px; font-weight: bold;">
                                                \${severity}: \${count}
                                            </div>
                                        \`;
                                    }
                                });
                            }
                            ethical += '</div>';

                            // Issues list
                            result.issues.forEach((issue, index) => {
                                const severityColor = getSeverityColor(issue.severity);
                                ethical += \`
                                    <div style="margin: 16px 0; padding: 16px; background: rgba(60, 60, 60, 0.5); border-radius: 8px; border-left: 4px solid \${severityColor};">
                                        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px;">
                                            <strong>\${index + 1}. \${escapeHtml(issue.issue_type)}</strong>
                                            <div style="background: \${severityColor}; color: white; padding: 4px 8px; border-radius: 4px; font-size: 11px; font-weight: bold;">
                                                \${issue.severity}
                                            </div>
                                        </div>
                                        <div style="font-size: 13px; color: var(--vscode-text-secondary); margin-bottom: 8px;">
                                            <strong>Location:</strong> \${escapeHtml(issue.location)}
                                        </div>
                                        <div style="margin-bottom: 8px;">
                                            <strong>Description:</strong>
                                            \${issue.description_html}
                                        </div>
                                        <div style="margin-bottom: 8px;">
                                            <strong>Potential Impact:</strong>
                                            \${issue.impact_html}
                                        </div>
                                        <div>
                                            <strong>Remediation:</strong>
                                            \${issue.remediation_html}
                                        </div>
                                    </div>
                                \`;
                            });
                        } else {
                            ethical += '<p>No ethical review data available.</p>';
                        }
                        return ethical;

                    case 'code_suggestions':
                        let suggestions = '<h4>💡 Inline Code Suggestions</h4>';
                        if (result.suggestions && result.suggestions.length > 0) {
                            suggestions += \`<p>Found \${result.suggestions.length} suggestion(s). These will appear as inline hints in your editor.</p>\`;

                            // Group suggestions by line
                            const suggestionsByLine = {};
                            result.suggestions.forEach(suggestion => {
                                if (!suggestionsByLine[suggestion.line]) {
                                    suggestionsByLine[suggestion.line] = [];
                                }
                                suggestionsByLine[suggestion.line].push(suggestion);
                            });

                            // Display suggestions
                            Object.entries(suggestionsByLine).forEach(([line, lineSuggestions]) => {
                                const severityColor = getSeverityColor(lineSuggestions[0].severity);
                                suggestions += \`
                                    <div style="margin: 12px 0; padding: 12px; background: rgba(60, 60, 60, 0.5); border-radius: 8px; border-left: 4px solid \${severityColor};">
                                        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px;">
                                            <strong>Line \${line}</strong>
                                            <div style="background: \${severityColor}; color: white; padding: 4px 8px; border-radius: 4px; font-size: 11px; font-weight: bold;">
                                                \${lineSuggestions[0].severity}
                                            </div>
                                        </div>
                                \`;

                                lineSuggestions.forEach((suggestion, index) => {
                                    const typeColor = getSuggestionTypeColor(suggestion.type);
                                    suggestions += \`
                                        <div style="margin: 8px 0; padding: 8px; background: rgba(40, 40, 40, 0.8); border-radius: 6px;">
                                            <div style="display: flex; align-items: center; gap: 8px; margin-bottom: 4px;">
                                                <div style="background: \${typeColor}; color: white; padding: 2px 6px; border-radius: 3px; font-size: 10px; font-weight: bold;">
                                                    \${suggestion.type}
                                                </div>
                                                <span style="font-size: 13px;">\${suggestion.message}</span>
                                            </div>
                                            <div style="font-size: 12px; color: #64b5f6; margin-top: 4px;">
                                                <strong>Suggestion:</strong> \${escapeHtml(suggestion.suggestion)}
                                            </div>
                                        </div>
                                    \`;
                                });

                                suggestions += \`</div>\`;
                            });

                            // Add action button to apply suggestions
                            suggestions += \`
                                <div class="action-buttons">
                                    <button class="apply-suggestions-button">
                                        <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"></path><polyline points="22 4 12 14.01 9 11.01"></polyline></svg>
                                        <span>Apply Suggestions in Editor</span>
                                    </button>
                                </div>
                            \`;
                        } else {
                            suggestions += \`
                                <div style="text-align: center; padding: 20px; background: rgba(34, 139, 34, 0.1); border-radius: 8px;">
                                    <div style="font-size: 48px;">✅</div>
                                    <div style="font-size: 16px; font-weight: bold; color: #28a745; margin-top: 8px;">
                                        No Suggestions Needed
                                    </div>
                                    <p style="margin-top: 8px; color: var(--vscode-text-secondary);">
                                        Your code appears well-written!
                                    </p>
                                </div>
                            \`;
                        }
                        return suggestions;

                    case 'run_project':
                        let output = '<h4>🚀 Execution Result</h4>';
                        output += \`<strong>Compilation:</strong><pre><code class="language-bash">\${escapeHtml(result.compilation || '[No output]')}</code></pre>\`;
                        output += \`<strong>Execution:</strong><pre><code class="language-bash">\${escapeHtml(result.execution || '[No output]')}</code></pre>\`;
                        return output;

                    case 'generate_tests':
                    case 'generate_google_tests':
                        const testCode = result.generated_test_suite_code || result.test_suite_code;
                        if (!testCode) {
                            return '<div class="error-message">Error: No test suite code was generated.</div>';
                        }
                        return \`
                            <h4>🧪 Generated Test Suite</h4>
                            <div class="action-buttons">

                                <button class="run-test-local-button">
                                    <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M4 17v2a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2v-2"></path><polyline points="7 11 12 16 17 11"></polyline><line x1="12" y1="16" x2="12" y2="4"></line></svg>
                                    <span>Run Locally</span>
                                </button>
                            </div>
                            <pre><code class="language-cpp">\${escapeHtml(testCode)}</code></pre>
                        \`;

                    case 'generate_docs':
                        const docContent = result.content || result.content_raw;
                        if (!docContent) {
                            return '<div class="error-message">Error: No documentation content was generated.</div>';
                        }

                        let docHtml = \`
                            <h4>📚 Documentation Report</h4>
                            <div class="action-buttons">
                                <button class="download-report-button">
                                    <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path><polyline points="7 10 12 15 17 10"></polyline><line x1="12" x2="12" y1="15" y2="3"></line></svg>
                                    <span>Download Report</span>
                                </button>
                            </div>
                        \`;

                        if (result.format === 'markdown') {
                            docHtml += result.content_html || \`<pre><code class="language-markdown">\${escapeHtml(docContent)}</code></pre>\`;
                        } else {
                            docHtml += \`<pre><code class="language-cpp">\${escapeHtml(docContent)}</code></pre>\`;
                        }
                        return docHtml;

                    case 'simulate_logic':
                        let sim = '<h4>🔍 Logic Simulation</h4>';
                        if (result.simulation_steps && result.simulation_steps.length > 0) {
                            result.simulation_steps.forEach((step, index) => {
                                sim += \`<div style="margin: 16px 0; padding: 12px; background: rgba(60, 60, 60, 0.5); border-radius: 8px;"><strong>Step \${index + 1}: \${escapeHtml(step.title)}</strong>\`;
                                sim += step.narrative_html;
                                if(step.state_changes && step.state_changes.length > 0) {
                                    sim += '<ul style="margin-top: 8px;">';
                                    step.state_changes.forEach(change => {
                                        sim += \`<li style="margin: 4px 0;"><code>\${escapeHtml(change.variable)}</code>: "\${escapeHtml(change.from_value)}" → "\${escapeHtml(change.to_value)}"</li>\`;
                                    });
                                    sim += '</ul>';
                                }
                                sim += '</div>';
                            });
                        } else {
                            sim += '<p>No simulation steps were generated.</p>';
                        }
                        return sim;

                    case 'optimize_code':
                        let opt = '<h4>⚡ Optimization Insights</h4>';
                        if (result.optimizations && result.optimizations.length === 1 && (result.optimizations[0].category === 'Optimal' || result.optimizations[0].category === 'General')) {
                             opt += result.optimizations[0].explanation_html; 
                        }
                        else if (result.optimizations && result.optimizations.length > 0) {
                            result.optimizations.forEach((item, index) => {
                                opt += \`<div style="margin: 16px 0; padding: 12px; background: rgba(60, 60, 60, 0.5); border-radius: 8px;"><strong>\${index + 1}. \${escapeHtml(item.title)}</strong>\`;
                                opt += item.explanation_html;
                                if (item.suggested_code_raw) {
                                    const start = item.start_line || '';
                                    const end = item.end_line || '';
                                    const code = item.suggested_code_raw || '';
                                    opt += \`<pre data-start-line="\${start}" data-end-line="\${end}" data-code="\${escapeHtml(code)}"><code class="language-cpp">\${escapeHtml(item.suggested_code_raw)}</code></pre>\`;
                                }
                                opt += '</div>';
                            });
                        } else {
                            opt += '<p>No optimization suggestions found.</p>';
                        }
                        return opt;

                    case 'run_test_suite':
                        let testResult = '<h4>🧪 Test Run Results</h4>';
                        if (result.system_error) {
                            testResult += \`<strong>System Error:</strong><pre><code class="language-bash">\${escapeHtml(result.system_error)}</code></pre>\`;
                        }
                        if (result.execution_error) {
                            testResult += \`<strong>Execution Error:</strong><pre><code class="language-bash">\${escapeHtml(result.execution_error)}</code></pre>\`;
                        }
                        testResult += \`<strong>Test Output:</strong><pre><code class="language-bash">\${escapeHtml(result.output || '[No output]')}</code></pre>\`;
                        testResult += \`<strong>Compilation Log:</strong><pre><code class="language-bash">\${escapeHtml(result.compilation_log || '[No log]')}</code></pre>\`;
                        return testResult;

                    default:
                        return '<pre><code class="language-json">' + escapeHtml(JSON.stringify(result, null, 2)) + '</code></pre>';
                }
            }

            // Helper functions for new features
            function getHealthColor(score) {
                if (score >= 90) return '#28a745';
                if (score >= 75) return '#17a2b8';
                if (score >= 60) return '#ffc107';
                return '#dc3545';
            }

            function getSeverityColor(severity) {
                switch(severity) {
                    case 'Critical': return '#dc3545';
                    case 'High': return '#fd7e14';
                    case 'Medium': return '#ffc107';
                    case 'Low': return '#20c997';
                    default: return '#6c757d';
                }
            }

            function getSuggestionTypeColor(type) {
                switch(type) {
                    case 'Performance': return '#ff6b6b';
                    case 'Readability': return '#4ecdc4';
                    case 'Bug': return '#ff9f43';
                    case 'Security': return '#ee5a24';
                    case 'Modernization': return '#a55eea';
                    default: return '#6c757d';
                }
            }
        </script>
    </body>
    </html>`;
    }
}

export function deactivate() { }
