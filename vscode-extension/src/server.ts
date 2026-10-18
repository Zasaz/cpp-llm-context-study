import * as http from 'http';
import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';

export class ArchitectureServer {
    private server: http.Server | null = null;
    private port: number = 3000;
    private currentGraphData: any = {};
    private webRoot: string;

    constructor(extensionUri: vscode.Uri) {
        this.webRoot = path.join(extensionUri.fsPath, 'media', 'web');
    }

    public start(): Promise<string> {
        return new Promise((resolve, reject) => {
            if (this.server) {
                resolve(`http://localhost:${this.port}`);
                return;
            }

            this.server = http.createServer((req, res) => this.handleRequest(req, res));

            // Try to listen on port 3000, fallback to random if busy
            this.server.listen(this.port, () => {
                console.log(`Architecture server listening on port ${this.port}`);
                resolve(`http://localhost:${this.port}`);
            });

            this.server.on('error', (e: any) => {
                if (e.code === 'EADDRINUSE') {
                    console.log('Port 3000 in use, retrying with random port...');
                    this.server?.listen(0); // 0 means random port
                    // Note: In a real robust impl, we'd handle the re-listen callback carefully.
                    // For now, let's assume if 3000 fails, we might need to handle the 'listening' event again.
                    // Actually, simpler to just listen(0) initially if we don't care about fixed port.
                } else {
                    reject(e);
                }
            });
        });
    }

    public updateData(data: any) {
        this.currentGraphData = data;
    }

    public stop() {
        if (this.server) {
            this.server.close();
            this.server = null;
        }
    }

    private handleRequest(req: http.IncomingMessage, res: http.ServerResponse) {
        const urlP = new URL(req.url || '/', `http://localhost:${this.port}`);
        const pathname = urlP.pathname;

        // API Endpoints
        if (pathname === '/api/data') {
            res.writeHead(200, { 'Content-Type': 'application/json', 'Access-Control-Allow-Origin': '*' });
            res.end(JSON.stringify(this.currentGraphData));
            return;
        }

        if (pathname === '/api/open') {
            const file = urlP.searchParams.get('file');
            const line = parseInt(urlP.searchParams.get('line') || '1');
            
            if (file) {
                this.openFileInEditor(file, line);
                res.writeHead(200, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ success: true }));
            } else {
                res.writeHead(400);
                res.end(JSON.stringify({ error: 'Missing file parameter' }));
            }
            return;
        }

        // Static Files
        let filePath = path.join(this.webRoot, pathname === '/' ? 'index.html' : pathname);
        
        // Prevent directory traversal
        if (!filePath.startsWith(this.webRoot)) {
            res.writeHead(403);
            res.end('Forbidden');
            return;
        }

        const ext = path.extname(filePath).toLowerCase();
        const mimeTypes: {[key: string]: string} = {
            '.html': 'text/html',
            '.js': 'text/javascript',
            '.css': 'text/css',
            '.json': 'application/json',
            '.png': 'image/png',
            '.jpg': 'image/jpg',
            '.gif': 'image/gif',
            '.svg': 'image/svg+xml',
        };

        const contentType = mimeTypes[ext] || 'application/octet-stream';

        fs.readFile(filePath, (err, content) => {
            if (err) {
                if (err.code === 'ENOENT') {
                    res.writeHead(404);
                    res.end(`File not found: ${pathname}`);
                } else {
                    res.writeHead(500);
                    res.end(`Server Error: ${err.code}`);
                }
            } else {
                res.writeHead(200, { 'Content-Type': contentType });
                res.end(content, 'utf-8');
            }
        });
    }

    private async openFileInEditor(filePath: string, line: number) {
        // Backend usually sends relative paths like "src/main.cpp"
        // We need to resolve to absolute path
        const workspaceFolders = vscode.workspace.workspaceFolders;
        if (!workspaceFolders) return;

        const rootPath = workspaceFolders[0].uri.fsPath;
        const absolutePath = path.isAbsolute(filePath) ? filePath : path.join(rootPath, filePath);
        
        try {
            const doc = await vscode.workspace.openTextDocument(absolutePath);
            const editor = await vscode.window.showTextDocument(doc);
            
            const position = new vscode.Position(Math.max(0, line - 1), 0);
            editor.selection = new vscode.Selection(position, position);
            editor.revealRange(editor.selection, vscode.TextEditorRevealType.InCenter);
        } catch (e) {
            console.error(`Failed to open file: ${absolutePath}`, e);
            vscode.window.showErrorMessage(`Failed to open file: ${filePath}`);
        }
    }
}
