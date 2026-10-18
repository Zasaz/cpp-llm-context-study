(function () {
    // Access the VS Code API acquired in the main script
    const vscode = window.vscodeApi;

    console.log("Architecture Visualization Script Loaded");

    let cy = null;

    if (!vscode) {
        console.error("VS Code API not found in window.vscodeApi");
        return;
    }

    window.addEventListener('message', event => {
        const message = event.data;
        console.log("Architecture Script Received Message:", message.command, message);
        switch (message.command) {
            case 'renderArchitecture':
                console.log("Rendering Architecture with data:", message.data);
                if (!message.data || (!message.data.nodes && !message.data.elements)) {
                    console.error("Invalid data received for architecture graph:", message.data);
                    document.getElementById('cy-container').innerHTML = '<div style="color: red; padding: 20px;">Error: No graph data received from backend.</div>';
                    return;
                }

                // Ensure container is visible and has size before rendering
                setTimeout(() => {
                    renderGraph(message.data);
                }, 100);
                break;
        }
    });

    function renderGraph(data) {
        if (cy) {
            cy.destroy();
        }

        // Filter Data (Only files, classes, functions, structs)
        const allowedTypes = ['file', 'class', 'function', 'struct'];
        const elements = data.nodes ? {
            nodes: data.nodes.filter(n => allowedTypes.includes(n.data.type)),
            edges: data.edges // Edges will be filtered automatically by Cytoscape if nodes are missing
        } : data;

        // Initialize Cytoscape
        cy = cytoscape({
            container: document.getElementById('cy-container'),
            elements: elements,
            style: [
                // --- Node Styles ---
                {
                    selector: 'node',
                    style: {
                        // Indicate clickability
                        'overlay-padding': '5px'
                    }
                },
                {
                    selector: 'node[type="file"]',
                    style: {
                        'background-color': '#e8f5e9', // Light green
                        'border-color': '#2e7d32',
                        'border-width': 2,
                        'label': 'data(label)',
                        'color': '#1b5e20',
                        'shape': 'round-rectangle',
                        'text-valign': 'top',
                        'text-halign': 'center',
                        'font-size': '12px',
                        'font-weight': 'bold',
                        'padding': '10px'
                    }
                },
                {
                    selector: 'node[type="class"]',
                    style: {
                        'background-color': '#e3f2fd', // Light blue
                        'border-color': '#1565c0',
                        'border-width': 2,
                        'label': 'data(label)',
                        'color': '#0d47a1',
                        'shape': 'rectangle',
                        'text-valign': 'center',
                        'text-halign': 'center',
                        'font-size': '11px'
                    }
                },
                {
                    selector: 'node[type="function"]',
                    style: {
                        'background-color': '#fff3e0', // Light orange
                        'border-color': '#ef6c00',
                        'border-width': 1,
                        'label': 'data(label)',
                        'color': '#e65100',
                        'shape': 'ellipse',
                        'font-size': '10px'
                    }
                },
                {
                    selector: 'node[type="struct"]',
                    style: {
                        'background-color': '#f3e5f5', // Light Purple
                        'border-color': '#7b1fa2',
                        'border-width': 1,
                        'label': 'data(label)',
                        'color': '#4a148c',
                        'shape': 'pentagon',
                        'font-size': '10px'
                    }
                },

                // --- Edge Styles ---
                {
                    selector: 'edge',
                    style: {
                        'width': 2,
                        'line-color': '#ccc',
                        'target-arrow-color': '#ccc',
                        'target-arrow-shape': 'triangle',
                        'curve-style': 'bezier',
                        'label': 'data(label)',
                        'font-size': '9px',
                        'color': '#555',
                        'text-background-opacity': 1,
                        'text-background-color': '#ffffff',
                        'text-background-padding': '2px'
                    }
                },
                {
                    selector: 'edge[type="includes"]',
                    style: {
                        'line-style': 'dotted',
                        'line-color': '#2196f3', // Blue
                        'target-arrow-color': '#2196f3'
                    }
                },
                {
                    selector: 'edge[type="function_call"]',
                    style: {
                        'line-style': 'solid',
                        'line-color': '#ff9800', // Orange
                        'target-arrow-color': '#ff9800'
                    }
                },
                {
                    selector: 'edge[type="inheritance"]',
                    style: {
                        'line-style': 'dashed',
                        'line-color': '#9c27b0', // Purple
                        'target-arrow-color': '#9c27b0'
                    }
                }
            ],
            layout: {
                name: 'breadthfirst', // Built-in tree layout (guaranteed to work)
                directed: true, // Follow edge direction for hierarchy
                padding: 10,
                spacingFactor: 0.8, // Compact spacing to allow larger zoom
                animate: false, // Cleaner initial render
                nodeDimensionsIncludeLabels: true,
                avoidOverlap: true
            }
        });

        // Click-to-Jump Navigation
        cy.on('tap', 'node', function (evt) {
            const node = evt.target;
            const data = node.data();

            console.log("Clicked node:", data);

            // Robust click handling: Use data.file OR fallback to data.id for file nodes
            let filepath = data.file;

            // If it's a file node and no specific 'file' prop, assume ID is the path (e.g. "src/main.cpp")
            if (!filepath && data.type === 'file') {
                filepath = data.id;
            }

            if (filepath) {
                vscode.postMessage({
                    command: 'jumpToCode',
                    filepath: filepath,
                    line: data.line_start || 1 // Default to line 1 if missing
                });
            } else {
                console.log("No file path found for clicked node");
            }
        });

        // Initialize Filters and Controls
        initControls();
    }

    function initControls() {
        // Zoom Controls
        const zoomInBtn = document.getElementById('zoom-in');
        if (zoomInBtn) zoomInBtn.onclick = () => { if (cy) cy.zoom(cy.zoom() * 1.2); };

        const zoomOutBtn = document.getElementById('zoom-out');
        if (zoomOutBtn) zoomOutBtn.onclick = () => { if (cy) cy.zoom(cy.zoom() * 0.8); };

        const fitBtn = document.getElementById('fit-graph');
        if (fitBtn) fitBtn.onclick = () => { if (cy) cy.fit(); };

        // Filters
        const filters = {
            'filter-includes': 'edge[type="includes"]',
            'filter-calls': 'edge[type="function_call"]',
            'filter-inheritance': 'edge[type="inheritance"]'
        };

        for (const [id, selector] of Object.entries(filters)) {
            const checkbox = document.getElementById(id);
            if (checkbox) {
                checkbox.addEventListener('change', (e) => {
                    if (e.target.checked) {
                        cy.elements(selector).style('display', 'element');
                    } else {
                        cy.elements(selector).style('display', 'none');
                    }
                });
            }
        }
    }

})();
