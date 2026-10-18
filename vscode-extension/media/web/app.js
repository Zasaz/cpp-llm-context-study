(function () {
    let cy = null;
    let selected = null;
    let currentLayout = 'cluster';
    let detailLevel = 'symbols';
    let theme = 'light';
    let matches = null;
    let matchIndex = -1;

    const PALETTES = {
        light: {
            ink: '#0f172a',
            surface: '#f8fafc',
            fileBg: '#ffffff', fileBorder: '#d3dce6', fileInk: '#64748b',
            classBg: '#eef2ff', classBorder: '#a5b4fc', classInk: '#4338ca',
            fnBg: '#ffffff', fnBorder: '#94a3b8',
            call: '#4f46e5', inherit: '#059669', include: '#b9c4d4',
            select: '#4f46e5', match: '#d97706'
        },
        dark: {
            ink: '#e2e8f0',
            surface: '#0f172a',
            fileBg: '#16203a', fileBorder: '#33436a', fileInk: '#8ba3c7',
            classBg: '#272b63', classBorder: '#6366f1', classInk: '#c7d2fe',
            fnBg: '#1e293b', fnBorder: '#64748b',
            call: '#6366f1', inherit: '#0ea774', include: '#526079',
            select: '#a5b4fc', match: '#fbbf24'
        }
    };

    const EDGE_META = {
        function_call: { label: 'Function calls', color: 'call', dashed: false },
        inheritance: { label: 'Inheritance', color: 'inherit', dashed: false },
        includes: { label: 'Includes', color: 'include', dashed: true }
    };

    document.addEventListener('DOMContentLoaded', () => {
        try {
            const saved = localStorage.getItem('arch-theme');
            if (saved === 'dark' || saved === 'light') theme = saved;
        } catch (e) {}
        applyThemeChrome();
        fetchData();
        setupEventListeners();
    });

    function pal() {
        return PALETTES[theme];
    }

    async function fetchData() {
        try {
            const response = await fetch('/api/data');
            const data = await response.json();
            const elements = data.elements || data;
            const hasNodes = elements && Array.isArray(elements.nodes) && elements.nodes.length > 0;
            if (hasNodes) {
                updateStats(elements);
                buildRelationsPanel(elements);
                renderGraph(elements);
            } else {
                document.getElementById('empty-state').style.display = 'flex';
            }
        } catch (error) {
            console.error('Failed to fetch graph data:', error);
            document.getElementById('empty-state').style.display = 'flex';
        }
    }

    function updateStats(elements) {
        const nodes = elements.nodes || [];
        const edges = elements.edges || [];
        const count = t => nodes.filter(n => n.data && n.data.type === t).length;
        document.getElementById('stat-files').textContent = count('file');
        document.getElementById('stat-classes').textContent = count('class') + count('struct');
        document.getElementById('stat-functions').textContent = count('function');
        document.getElementById('stat-edges').textContent = edges.length;
    }

    function edgeMeta(type) {
        if (EDGE_META[type]) return EDGE_META[type];
        const label = String(type || 'other').replace(/_/g, ' ');
        return { label: label.charAt(0).toUpperCase() + label.slice(1), color: 'include', dashed: true };
    }

    function buildRelationsPanel(elements) {
        const edges = elements.edges || [];
        const counts = {};
        for (const e of edges) {
            const t = (e.data && e.data.type) || 'other';
            counts[t] = (counts[t] || 0) + 1;
        }
        const order = Object.keys(EDGE_META).filter(t => counts[t]);
        for (const t of Object.keys(counts)) {
            if (!order.includes(t)) order.push(t);
        }
        const host = document.getElementById('relations');
        host.innerHTML = '';
        if (!order.length) {
            const p = document.createElement('p');
            p.className = 'placeholder';
            p.textContent = 'No relations in this project.';
            host.appendChild(p);
            return;
        }
        for (const t of order) {
            const meta = edgeMeta(t);
            const label = document.createElement('label');
            label.className = 'toggle';
            label.innerHTML =
                '<input type="checkbox" checked data-type="' + t + '">' +
                '<span class="slider"></span>' +
                '<span class="swatch' + (meta.dashed ? ' dashed' : '') + '"></span>' +
                '<span class="label">' + meta.label + '</span>' +
                '<span class="count">' + counts[t] + '</span>';
            label.querySelector('.swatch').style.borderColor = pal()[meta.color];
            label.querySelector('input').addEventListener('change', e => {
                if (!cy) return;
                cy.edges('[type="' + t + '"]').style('display', e.target.checked ? 'element' : 'none');
            });
            host.appendChild(label);
        }
    }

    function refreshSwatches() {
        document.querySelectorAll('#relations .toggle').forEach(row => {
            const t = row.querySelector('input').getAttribute('data-type');
            row.querySelector('.swatch').style.borderColor = pal()[edgeMeta(t).color];
        });
    }

    function renderGraph(elements) {
        if (cy) cy.destroy();

        cy = cytoscape({
            container: document.getElementById('cy'),
            elements: elements,
            style: getStylesheet(),
            layout: { name: 'preset' },
            wheelSensitivity: 0.25,
            minZoom: 0.06,
            maxZoom: 4
        });

        cy.on('tap', 'node', evt => selectNode(evt.target));
        cy.on('tap', 'edge', evt => selectEdge(evt.target));
        cy.on('tap', evt => {
            if (evt.target === cy) clearSelection();
        });
        cy.on('dbltap', 'node', evt => {
            const scope = evt.target.union(evt.target.descendants()).union(evt.target.closedNeighborhood());
            cy.animate({ fit: { eles: scope, padding: 70 }, duration: 350, easing: 'ease-in-out-quad' });
        });
        cy.on('mouseover', 'node', evt => {
            document.getElementById('cy').style.cursor = 'pointer';
            evt.target.addClass('hovered');
            showTooltip(evt.target);
        });
        cy.on('mouseout', 'node', evt => {
            document.getElementById('cy').style.cursor = 'default';
            evt.target.removeClass('hovered');
            hideTooltip();
        });
        cy.on('viewport', hideTooltip);
        cy.on('drag', 'node', hideTooltip);
        window.__cy = cy;
        window.__selectById = id => {
            const n = cy.getElementById(id);
            if (n && n.length) selectNode(n);
        };

        runLayout(layoutOptions());
    }

    function layoutOptions() {
        return currentLayout === 'tree' ? treeLayout() : clusterLayout();
    }

    function runLayout(options) {
        const layout = cy.layout(options);
        layout.one('layoutstop', () => {
            cy.fit(cy.elements(':visible'), 40);
            setTimeout(() => {
                cy.fit(cy.elements(':visible'), 40);
                window.__graphReady = true;
            }, 80);
        });
        layout.run();
    }

    function showTooltip(node) {
        const tip = document.getElementById('tooltip');
        const data = node.data();
        const line = data.line_start ? ' · line ' + data.line_start : '';
        tip.innerHTML = '<div class="t-name">' + escapeHtml(data.id) + '</div>' +
            '<div class="t-sub">' + escapeHtml(data.type || 'node') + line + '</div>';
        tip.style.display = 'block';
        const bb = node.renderedBoundingBox({ includeLabels: false });
        const host = document.getElementById('cy').getBoundingClientRect();
        let x = bb.x1 + (bb.w - tip.offsetWidth) / 2;
        let y = bb.y1 - tip.offsetHeight - 8;
        x = Math.max(8, Math.min(x, host.width - tip.offsetWidth - 8));
        if (y < 8) y = bb.y2 + 8;
        tip.style.left = x + 'px';
        tip.style.top = y + 'px';
    }

    function hideTooltip() {
        document.getElementById('tooltip').style.display = 'none';
    }

    function escapeHtml(s) {
        return String(s == null ? '' : s).replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
    }

    function kindChip(type) {
        const t = type === 'struct' ? 'class' : type;
        const cls = t === 'file' ? 'kind-file' : t === 'class' ? 'kind-class' : 'kind-function';
        return '<span class="kind-chip ' + cls + '">' + escapeHtml(type) + '</span>';
    }

    function findFile(node) {
        const data = node.data();
        if (data.type === 'file') return data.id;
        if (data.file) return data.file;
        let current = node;
        while (current.parent().length) {
            current = current.parent();
            if (current.data('type') === 'file') return current.data('id');
        }
        return null;
    }

    function neighborRow(edge, other) {
        const meta = edgeMeta(edge.data('type'));
        return '<div class="neighbor" data-node="' + escapeHtml(other.id()) + '">' +
            '<span class="edot" style="background:' + pal()[meta.color] + '"></span>' +
            '<span class="nlabel">' + escapeHtml(other.id()) + '</span></div>';
    }

    function neighborSection(title, rows, total) {
        if (!rows.length) return '';
        let html = '<div class="neighbors"><h4>' + title + ' · ' + total + '</h4>' + rows.join('');
        if (total > rows.length) html += '<div class="more-note">+ ' + (total - rows.length) + ' more</div>';
        return html + '</div>';
    }

    function selectNode(node) {
        selected = node;
        const data = node.data();
        const host = document.getElementById('details-content');
        const file = findFile(node);

        const out = node.outgoers('edge');
        const inn = node.incomers('edge');
        const outRows = out.slice(0, 8).map(e => neighborRow(e, e.target()));
        const inRows = inn.slice(0, 8).map(e => neighborRow(e, e.source()));

        let sub = '';
        if (data.type === 'file') {
            sub = node.descendants().length + ' symbols';
        } else {
            sub = file ? 'in <span class="mono">' + escapeHtml(file) + '</span>' : '';
            if (data.line_start) sub += (sub ? ' · ' : '') + 'line ' + data.line_start;
            if (data.type === 'class' || data.type === 'struct') {
                sub += (sub ? ' · ' : '') + node.children().length + ' members';
            }
        }

        host.innerHTML =
            kindChip(data.type || 'node') +
            '<div class="info-name">' + escapeHtml(data.id) + '</div>' +
            '<div class="info-sub">' + sub + '</div>' +
            neighborSection('Outgoing', outRows, out.length) +
            neighborSection('Incoming', inRows, inn.length) +
            (file ? '<button id="btn-open-code" class="btn primary">Open in editor</button>' : '');
        wireDetails(host);
        showDetails();

        focusOn(node);
    }

    function selectEdge(edge) {
        selected = edge;
        const meta = edgeMeta(edge.data('type'));
        const host = document.getElementById('details-content');
        host.innerHTML =
            '<span class="kind-chip kind-relation" style="color:' + pal()[meta.color] + ';border-color:' + pal()[meta.color] + '">' + escapeHtml(meta.label) + '</span>' +
            '<div class="neighbors"><h4>From</h4>' +
            '<div class="neighbor" data-node="' + escapeHtml(edge.source().id()) + '"><span class="edot" style="background:' + pal()[meta.color] + '"></span><span class="nlabel">' + escapeHtml(edge.source().id()) + '</span></div>' +
            '<h4>To</h4>' +
            '<div class="neighbor" data-node="' + escapeHtml(edge.target().id()) + '"><span class="edot" style="background:' + pal()[meta.color] + '"></span><span class="nlabel">' + escapeHtml(edge.target().id()) + '</span></div></div>';
        wireDetails(host);
        showDetails();

        cy.elements().addClass('faded').removeClass('focus');
        edge.removeClass('faded').addClass('focus');
        edge.connectedNodes().removeClass('faded');
        edge.connectedNodes().ancestors().removeClass('faded');
    }

    function wireDetails(host) {
        host.querySelectorAll('.neighbor').forEach(el => {
            el.addEventListener('click', () => {
                const target = cy.getElementById(el.getAttribute('data-node'));
                if (target && target.length) {
                    selectNode(target);
                    cy.animate({ center: { eles: target }, duration: 300, easing: 'ease-in-out-quad' });
                }
            });
        });
        const openBtn = host.querySelector('#btn-open-code');
        if (openBtn) openBtn.addEventListener('click', openInEditor);
    }

    function showDetails() {
        document.getElementById('details-content').style.display = 'block';
        document.querySelector('#node-info .placeholder').style.display = 'none';
    }

    function focusOn(node) {
        const keep = node.closedNeighborhood()
            .union(node.ancestors())
            .union(node.descendants())
            .union(node.descendants().connectedEdges().filter(e => {
                return node.descendants().contains(e.source()) && node.descendants().contains(e.target());
            }));
        cy.elements().addClass('faded').removeClass('focus');
        keep.removeClass('faded');
        keep.connectedNodes().removeClass('faded');
        node.connectedEdges().addClass('focus').removeClass('faded');
        node.connectedEdges().connectedNodes().removeClass('faded');
    }

    function clearSelection() {
        selected = null;
        document.getElementById('details-content').style.display = 'none';
        document.querySelector('#node-info .placeholder').style.display = 'block';
        if (cy) cy.elements().removeClass('faded').removeClass('focus');
    }

    async function openInEditor() {
        if (!selected || !selected.isNode()) return;
        const data = selected.data();
        let filepath = findFile(selected);
        if (!filepath) filepath = data.id;
        try {
            await fetch('/api/open?file=' + encodeURIComponent(filepath) + '&line=' + (data.line_start || 1));
        } catch (e) {
            console.error('Failed to open file:', e);
        }
    }

    function applySearch(query) {
        const countEl = document.getElementById('search-count');
        if (!cy) return;
        cy.elements().removeClass('dimmed').removeClass('match');
        matches = null;
        matchIndex = -1;
        if (!query || query.length < 2) {
            countEl.textContent = '';
            return;
        }
        clearSelection();
        const q = query.toLowerCase();
        matches = cy.nodes().filter(n => String(n.data('id')).toLowerCase().includes(q));
        countEl.textContent = matches.length ? matches.length : '0';
        if (!matches.length) return;
        cy.elements().addClass('dimmed');
        matches.removeClass('dimmed').addClass('match');
        matches.ancestors().removeClass('dimmed');
    }

    function cycleMatch() {
        if (!matches || !matches.length) return;
        matchIndex = (matchIndex + 1) % matches.length;
        const node = matches[matchIndex];
        cy.animate({ center: { eles: node }, duration: 280, easing: 'ease-in-out-quad' });
        document.getElementById('search-count').textContent = (matchIndex + 1) + '/' + matches.length;
    }

    function pillWidth(ele) {
        const label = ele.data('label') || '';
        return Math.min(180, label.length * 6.4 + 22);
    }

    function getStylesheet() {
        const p = pal();
        return [
            {
                selector: 'node',
                style: {
                    'label': 'data(label)',
                    'font-family': 'Inter, system-ui, sans-serif',
                    'text-valign': 'center',
                    'text-halign': 'center',
                    'color': p.ink,
                    'font-size': '11px',
                    'font-weight': 500,
                    'text-wrap': 'ellipsis',
                    'text-max-width': '170px',
                    'border-style': 'solid'
                }
            },
            {
                selector: 'node[type="function"]',
                style: {
                    'shape': 'round-rectangle',
                    'background-color': p.fnBg,
                    'border-width': 1.3,
                    'border-color': p.fnBorder,
                    'width': pillWidth,
                    'height': 26
                }
            },
            {
                selector: 'node:parent',
                style: {
                    'text-valign': 'top',
                    'text-halign': 'center',
                    'padding': '16px',
                    'text-margin-y': -6
                }
            },
            {
                selector: 'node[type="class"], node[type="struct"]',
                style: {
                    'shape': 'round-rectangle',
                    'background-color': p.classBg,
                    'background-opacity': theme === 'dark' ? 0.55 : 0.72,
                    'border-width': 1.4,
                    'border-color': p.classBorder,
                    'font-size': '12px',
                    'font-weight': 600,
                    'color': p.classInk,
                    'text-background-color': p.surface,
                    'text-background-opacity': 1,
                    'text-background-shape': 'round-rectangle',
                    'text-background-padding': '3px'
                }
            },
            {
                selector: 'node[type="file"]',
                style: {
                    'shape': 'round-rectangle',
                    'background-color': p.fileBg,
                    'background-opacity': 0.55,
                    'border-width': 1,
                    'border-color': p.fileBorder,
                    'font-size': '11px',
                    'font-weight': 600,
                    'color': p.fileInk,
                    'text-background-color': p.surface,
                    'text-background-opacity': 1,
                    'text-background-shape': 'round-rectangle',
                    'text-background-padding': '3px'
                }
            },
            {
                selector: 'node[type="class"]:childless, node[type="struct"]:childless',
                style: {
                    'width': pillWidth,
                    'height': 28,
                    'text-valign': 'center',
                    'text-margin-y': 0,
                    'background-opacity': 1
                }
            },
            {
                selector: 'node[type="file"]:childless',
                style: {
                    'width': pillWidth,
                    'height': 28,
                    'text-valign': 'center',
                    'text-margin-y': 0,
                    'border-style': 'dashed'
                }
            },
            {
                selector: 'edge',
                style: {
                    'width': 1.7,
                    'curve-style': 'bezier',
                    'target-arrow-shape': 'triangle',
                    'arrow-scale': 1.05
                }
            },
            {
                selector: 'edge[type="function_call"]',
                style: {
                    'line-color': p.call,
                    'target-arrow-color': p.call,
                    'line-opacity': 0.85
                }
            },
            {
                selector: 'edge[type="inheritance"]',
                style: {
                    'line-color': p.inherit,
                    'target-arrow-color': p.inherit,
                    'target-arrow-shape': 'triangle',
                    'target-arrow-fill': 'hollow',
                    'arrow-scale': 1.25,
                    'line-opacity': 0.9
                }
            },
            {
                selector: 'edge[type="includes"]',
                style: {
                    'line-color': p.include,
                    'target-arrow-color': p.include,
                    'line-style': 'dashed',
                    'line-dash-pattern': [5, 4],
                    'width': 1.2,
                    'arrow-scale': 0.8,
                    'line-opacity': 0.8
                }
            },
            {
                selector: 'node.hovered',
                style: {
                    'border-color': p.select,
                    'border-width': 1.8
                }
            },
            {
                selector: 'node:selected',
                style: {
                    'border-color': p.select,
                    'border-width': 2.4
                }
            },
            {
                selector: 'node.match',
                style: {
                    'border-color': p.match,
                    'border-width': 2.2
                }
            },
            {
                selector: '.faded, .dimmed',
                style: {
                    'opacity': 0.13
                }
            },
            {
                selector: 'edge.focus',
                style: {
                    'width': 2.6,
                    'line-opacity': 1
                }
            }
        ];
    }

    function clusterLayout() {
        return {
            name: 'fcose',
            quality: 'proof',
            randomize: true,
            animate: false,
            nodeDimensionsIncludeLabels: true,
            packComponents: true,
            tile: true,
            tilingPaddingVertical: 14,
            tilingPaddingHorizontal: 14,
            nodeRepulsion: () => 50000,
            idealEdgeLength: () => 85,
            edgeElasticity: () => 0.45,
            nestingFactor: 0.1,
            gravity: 0.25,
            gravityCompound: 1.5,
            gravityRangeCompound: 1.8,
            numIter: 2500,
            padding: 40
        };
    }

    function treeLayout() {
        return {
            name: 'dagre',
            rankDir: 'TB',
            rankSep: 90,
            nodeSep: 28,
            edgeSep: 16,
            padding: 40,
            animate: true,
            animationDuration: 450
        };
    }

    function setDetailLevel(level) {
        if (!cy || level === detailLevel) return;
        detailLevel = level;
        document.getElementById('detail-symbols').classList.toggle('active', level === 'symbols');
        document.getElementById('detail-classes').classList.toggle('active', level === 'classes');
        clearSelection();
        cy.nodes('[type="function"]').style('display', level === 'classes' ? 'none' : 'element');
        runLayout(layoutOptions());
    }

    function applyThemeChrome() {
        document.documentElement.setAttribute('data-theme', theme);
        document.getElementById('icon-moon').style.display = theme === 'light' ? '' : 'none';
        document.getElementById('icon-sun').style.display = theme === 'dark' ? '' : 'none';
    }

    function toggleTheme() {
        theme = theme === 'light' ? 'dark' : 'light';
        try {
            localStorage.setItem('arch-theme', theme);
        } catch (e) {}
        applyThemeChrome();
        refreshSwatches();
        if (cy) cy.style(getStylesheet());
        if (selected) {
            if (selected.isNode()) selectNode(selected);
            else selectEdge(selected);
        }
    }

    function setupEventListeners() {
        document.getElementById('btn-theme').addEventListener('click', toggleTheme);

        document.getElementById('btn-layout-cluster').addEventListener('click', e => {
            currentLayout = 'cluster';
            updateActiveButton(e.currentTarget);
            runLayout(clusterLayout());
        });

        document.getElementById('btn-layout-tree').addEventListener('click', e => {
            currentLayout = 'tree';
            updateActiveButton(e.currentTarget);
            runLayout(treeLayout());
        });

        document.getElementById('btn-fit').addEventListener('click', () => {
            if (cy) cy.animate({ fit: { eles: cy.elements(':visible'), padding: 40 }, duration: 300 });
        });

        document.getElementById('btn-export').addEventListener('click', () => {
            if (!cy) return;
            const blob = cy.png({ output: 'blob', full: true, scale: 2, bg: pal().surface });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'architecture-graph.png';
            a.click();
            URL.revokeObjectURL(url);
        });

        document.getElementById('detail-symbols').addEventListener('click', () => setDetailLevel('symbols'));
        document.getElementById('detail-classes').addEventListener('click', () => setDetailLevel('classes'));

        document.getElementById('zoom-in').addEventListener('click', () => {
            if (cy) cy.zoom({ level: cy.zoom() * 1.25, renderedPosition: { x: cy.width() / 2, y: cy.height() / 2 } });
        });
        document.getElementById('zoom-out').addEventListener('click', () => {
            if (cy) cy.zoom({ level: cy.zoom() * 0.8, renderedPosition: { x: cy.width() / 2, y: cy.height() / 2 } });
        });

        const search = document.getElementById('search');
        search.addEventListener('input', e => applySearch(e.target.value.trim()));
        search.addEventListener('keydown', e => {
            if (e.key === 'Enter') cycleMatch();
            if (e.key === 'Escape') {
                search.value = '';
                applySearch('');
                search.blur();
            }
        });
    }

    function updateActiveButton(clickedBtn) {
        document.getElementById('btn-layout-cluster').classList.remove('active');
        document.getElementById('btn-layout-tree').classList.remove('active');
        clickedBtn.classList.add('active');
    }
})();
