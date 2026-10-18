"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = __importStar(require("vscode"));
const axios_1 = __importDefault(require("axios"));
const util_1 = require("util"); // Use built-in Node.js util
// IMPORTANT: This must match the URL your Flask API is running on.
const API_BASE_URL = 'http://127.0.0.1:5000';
// This function is called when your extension is activated
function activate(context) {
    let disposable = vscode.commands.registerCommand('cpp-helper-bot.start', () => {
        // Create and show a new webview panel
        const panel = vscode.window.createWebviewPanel('cppHelperBot', // Identifies the type of the webview. Used internally
        'C++ Helper Bot', // Title shown to the user
        vscode.ViewColumn.Two, // Show it in a side-by-side column
        {
            // Enable scripts in the webview
            enableScripts: true,
            // We don't need to restrict local resources for this simple UI
            localResourceRoots: []
        });
        // Set the webview's initial HTML content
        panel.webview.html = getWebviewContent();
        // Handle messages from the webview (the HTML UI)
        panel.webview.onDidReceiveMessage(async (message) => {
            switch (message.command) {
                // This case is triggered when a button is clicked in the UI
                case 'analyze':
                    // This will gather code, call the API, and send the result back
                    await handleAnalyzeCommand(panel, message.feature, message.payload || {});
                    return;
                case 'showError':
                    // Utility to show VS Code error popups
                    vscode.window.showErrorMessage(message.text);
                    return;
            }
        }, undefined, context.subscriptions);
    });
    context.subscriptions.push(disposable);
}
/**
 * Handles the main analysis logic when a feature is triggered from the webview.
 */
async function handleAnalyzeCommand(panel, feature, payload) {
    // 1. Get the currently active text editor
    const activeEditor = vscode.window.activeTextEditor;
    let activeFileRelativePath = null;
    if (activeEditor) {
        const workspaceFolder = vscode.workspace.getWorkspaceFolder(activeEditor.document.uri);
        if (workspaceFolder) {
            // Get the relative path of the file, e.g., "src/main.cpp"
            // This is what our API's 'active_file' parameter expects
            activeFileRelativePath = vscode.workspace.asRelativePath(activeEditor.document.uri);
        }
    }
    // Show a loading message in the webview UI
    panel.webview.postMessage({ command: 'showLoading', text: `Analyzing: ${feature}...` });
    try {
        // 2. Find all C++ files in the workspace
        const filesData = await getAllCppFiles();
        if (filesData.length === 0) {
            throw new Error("No C++ files (.cpp, .h, .hpp) found in the current workspace.");
        }
        // 3. Construct the JSON payload for our Flask API
        const apiPayload = {
            feature: feature,
            files: filesData,
            active_file: activeFileRelativePath,
            ...payload // Add any extra payload, e.g., test_suite_code
        };
        // 4. Send all the code to the Flask API
        const response = await axios_1.default.post(`${API_BASE_URL}/api/analyze`, apiPayload, {
            timeout: 60000 // 60 second timeout for long analyses
        });
        // 5. Send the successful result back to the webview to display
        panel.webview.postMessage({ command: 'showResult', result: response.data, feature: feature });
    }
    catch (error) {
        let errorMessage = `Failed to run analysis: ${error.message}`;
        if (axios_1.default.isAxiosError(error) && error.response) {
            // If the Flask API returned an error (like 500), show it
            errorMessage = `API Error: ${error.response.data?.error || error.response.statusText}`;
        }
        else if (axios_1.default.isAxiosError(error) && error.code === 'ECONNREFUSED') {
            errorMessage = `Connection Refused: Is the Python API server running at ${API_BASE_URL}?`;
        }
        // Send the error to the webview
        panel.webview.postMessage({ command: 'showError', text: errorMessage });
        // Also show a native VS Code error notification
        vscode.window.showErrorMessage(errorMessage);
    }
}
/**
 * Finds all C++ files in the workspace, reads them, and returns them
 * in the format our API expects.
 */
async function getAllCppFiles() {
    // Find all files matching these patterns, excluding node_modules
    const fileUris = await vscode.workspace.findFiles('**/*.{cpp,h,hpp,cxx,cc}', '**/node_modules/**');
    const filesData = [];
    for (const uri of fileUris) {
        // Read the file content
        const fileContent = await vscode.workspace.fs.readFile(uri);
        // Get the relative path (e.g., "src/my_class.h")
        const relativePath = vscode.workspace.asRelativePath(uri);
        filesData.push({
            name: relativePath,
            content: new util_1.TextDecoder().decode(fileContent) // Decode bytes to string
        });
    }
    return filesData;
}
/**
 * This function provides the HTML/CSS/JS for the side panel's UI.
 */
function getWebviewContent() {
    // We use an inline style and script to keep this simple
    return `<!DOCTYPE html>
	<html lang="en">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>C++ Helper Bot</title>
		<style>
			:root {
				--vscode-font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol";
				--vscode-editor-background: #1e1e1e;
				--vscode-sideBar-background: #252526;
				--vscode-button-background: #0e639c;
				--vscode-button-hoverBackground: #1177bb;
				--vscode-text-default: #d4d4d4;
				--vscode-text-secondary: #8c8c8c;
			}
			body {
				font-family: var(--vscode-font-family);
				color: var(--vscode-text-default);
				background-color: var(--vscode-sideBar-background);
				padding: 0;
				margin: 0;
				overflow: hidden;
			}
			.container {
				padding: 20px;
				height: 100vh;
				display: flex;
				flex-direction: column;
				box-sizing: border-box;
			}
			#chat-history {
				flex-grow: 1;
				overflow-y: auto;
				padding-bottom: 20px;
			}
			.chat-message {
				margin-bottom: 15px;
				padding: 10px 15px;
				border-radius: 10px;
				background-color: var(--vscode-editor-background);
				max-width: 95%;
				word-wrap: break-word;
			}
			.bot-message {
				background-color: var(--vscode-editor-background);
			}
			.loading-message {
				color: var(--vscode-text-secondary);
				font-style: italic;
			}
			.error-message {
				color: #f48771;
				font-weight: bold;
			}
			pre {
				background-color: #2a2a2a;
				padding: 10px;
				border-radius: 5px;
				overflow-x: auto;
				white-space: pre-wrap;
				word-wrap: break-word;
			}
			.button-grid {
				display: grid;
				grid-template-columns: 1fr 1fr;
				gap: 10px;
				flex-shrink: 0;
			}
			button {
				padding: 12px;
				border: none;
				border-radius: 5px;
				cursor: pointer;
				font-size: 14px;
				background-color: var(--vscode-button-background);
				color: white;
				transition: background-color 0.2s;
			}
			button:hover {
				background-color: var(--vscode-button-hoverBackground);
			}
			h4 { margin-top: 0; }
		</style>
	</head>
	<body>
		<div class="container">
			<div class="button-grid">
				<button data-feature="run_project">Run Project</button>
				<button data-feature="explain_code">Explain Active File</button>
				<button data-feature="generate_tests">Generate Tests</button>
				<button data-feature="analyze_performance">Check Performance</button>
				<button data-feature="simulate_logic">Simulate Logic</button>
				<button data-feature="optimize_code">Find Optimizations</button>
			</div>

			<div id="chat-history">
				<div class="chat-message bot-message">
					<p>Welcome! Select an action to analyze your workspace.</p>
				</div>
			</div>
		</div>

		<script>
			// This script runs inside the webview
			const vscode = acquireVsCodeApi();
			const chatHistory = document.getElementById('chat-history');

			// Add click listeners to all buttons
			document.querySelectorAll('.button-grid button').forEach(button => {
				button.addEventListener('click', () => {
					const feature = button.dataset.feature;
					if (feature) {
						// Send a message to the extension's "backend" (extension.ts)
						vscode.postMessage({
							command: 'analyze',
							feature: feature
						});
					}
				});
			});

			// Handle messages *from* the extension (e.g., results or errors)
			window.addEventListener('message', event => {
				const message = event.data;
				switch (message.command) {
					case 'showLoading':
						addMessage(message.text, 'loading-message');
						break;
					case 'showResult':
						// Remove the "Loading..." message
						const loading = chatHistory.querySelector('.loading-message');
						if (loading) {
							loading.remove();
						}
						// Add the bot's response
						addMessage(formatResult(message.result, message.feature), 'bot-message');
						break;
					case 'showError':
						// Remove loading message
						const loadingErr = chatHistory.querySelector('.loading-message');
						if (loadingErr) {
							loadingErr.remove();
						}
						addMessage(escapeHtml(message.text), 'error-message');
						break;
				}
			});

			/**
			 * Appends a new message to the chat history UI.
			 */
			function addMessage(content, className) {
				const div = document.createElement('div');
				div.className = 'chat-message ' + className;
				div.innerHTML = content;
				chatHistory.appendChild(div);
				chatHistory.scrollTop = chatHistory.scrollHeight; // Auto-scroll
			}
			
			/**
			 * Basic HTML escaping
			 */
			function escapeHtml(str) {
				if (!str) return '';
				return str.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;").replace(/'/g, "&#039;");
			}

			/**
			 * This function formats the JSON from your API into readable HTML.
			 * You should expand this to make it look nicer!
			 */
			function formatResult(result, feature) {
				if (result.error) {
					return \`<strong>Error:</strong> <pre>\${escapeHtml(result.error)}</pre>\`;
				}
				
				// This uses the exact field names from your Python API's JSON response
				switch(feature) {
					case 'run_project':
						let output = '<h4>Execution Result</h4>';
						output += \`<strong>Compilation:</strong><pre>\${escapeHtml(result.compilation || '[No output]')}</pre>\`;
						output += \`<strong>Execution:</strong><pre>\${escapeHtml(result.execution || '[No output]')}</pre>\`;
						return output;
					
					case 'explain__code':
						// Your API already returns HTML, so we can inject it
						return '<h4>Code Explanation</h4>' + result.explanation_html; 
					
					case 'generate_tests':
						return \`<h4>Generated Test Suite</h4><pre>\${escapeHtml(result.generated_test_suite_code)}</pre>\`;
						
					case 'analyze_performance':
						let perf = '<h4>Performance Report</h4>';
						perf += \`<div><strong>Execution Time:</strong> \${escapeHtml(result.execution_time_str)}</div>\`;
						perf += \`<div><strong>Peak Memory:</strong> \${escapeHtml(result.peak_memory_str)}</div>\`;
						perf += \`<div><strong>CPU Time:</strong> \${escapeHtml(result.cpu_time_str)}</div>\`;
						perf += '<hr><h4>AI Analysis</h4>' + result.ai_analysis; // API returns HTML
						return perf;
						
					case 'simulate_logic':
						let sim = '<h4>Logic Simulation</h4>';
						if (result.simulation_steps && result.simulation_steps.length > 0) {
							result.simulation_steps.forEach((step, index) => {
								sim += \`<strong>Step \${index + 1}: \${escapeHtml(step.title)}</strong>\`;
								sim += step.narrative_html; // API returns HTML
								if(step.state_changes && step.state_changes.length > 0) {
									sim += '<ul>';
									step.state_changes.forEach(change => {
										sim += \`<li>\${escapeHtml(change.variable)}: "\${escapeHtml(change.from_value)}" &rarr; "\${escapeHtml(change.to_value)}"</li>\`;
									});
									sim += '</ul>';
								}
							});
						} else {
							sim += '<p>No simulation steps were generated.</p>';
						}
						return sim;

					case 'optimize_code':
						let opt = '<h4>Optimization Insights</h4>';
						if (result.optimizations && result.optimizations.length > 0) {
							result.optimizations.forEach((item, index) => {
								opt += \`<hr><strong>\${index + 1}. \${escapeHtml(item.title)}</strong>\`;
								opt += item.explanation_html; // API returns HTML
								if (item.suggested_code_raw) {
									opt += \`<strong>Suggestion:</strong><pre>\${escapeHtml(item.suggested_code_raw)}</pre>\`;
								}
							});
						} else {
							opt += '<p>No optimization suggestions found.</p>';
						}
						return opt;
						
					default:
						// Fallback for any other feature
						return '<pre>'all' + escapeHtml(JSON.stringify(result, null, 2)) + '</pre>';
				}
			}
		</script>
	</body>
	</html>`;
}
// This method is called when your extension is deactivated
function deactivate() { }
//# sourceMappingURL=extension.js.map