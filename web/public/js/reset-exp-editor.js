(function () {
  const root = document.getElementById('reset-exp-editor');
  if (!root) return;

  const state = {
    active: 'reset',
    reset: { columns: [], rows: [], rawHeader: [] },
    experience: { columns: [], rows: [], rawHeader: [] }
  };

  function setMessage(text, type) {
    const box = root.querySelector('.editor-message');
    if (!box) return;
    if (!text) {
      box.textContent = '';
      box.classList.add('hidden');
      return;
    }
    box.textContent = text;
    box.classList.remove('hidden');
    box.classList.toggle('success', type === 'success');
    box.classList.toggle('error', type === 'error');
  }

  function parseTable(content) {
    const lines = String(content || '').split(/\r?\n/);
    const headerLines = [];
    const rows = [];
    for (const raw of lines) {
      const line = raw.trim();
      if (!line) continue;
      if (line.startsWith('//')) {
        if (headerLines.length < 2 && !/^\/\/=+/.test(line)) {
          headerLines.push(line.replace(/^\/\//, '').trim());
        }
        continue;
      }
      if (line.toLowerCase() === 'end') break;
      const tokens = line.split(/\s+/);
      if (tokens.length) rows.push(tokens);
    }
    let columns = [];
    if (headerLines.length >= 2) {
      const parts1 = headerLines[0].split(/\s+/);
      const parts2 = headerLines[1].split(/\s+/);
      const max = Math.max(parts1.length, parts2.length);
      columns = Array.from({ length: max }).map((_, i) => {
        const a = parts1[i] || '';
        const b = parts2[i] || '';
        return `${a} ${b}`.trim() || `Col${i + 1}`;
      });
    } else if (headerLines.length === 1) {
      columns = headerLines[0].split(/\s+/);
    }
    return { columns, rows, rawHeader: headerLines };
  }

  function buildTable(data) {
    const lines = [];
    if (data.rawHeader.length) {
      data.rawHeader.forEach((h) => lines.push(`//${h}`));
    }
    data.rows.forEach((row) => {
      lines.push(row.join('\t'));
    });
    lines.push('end');
    return lines.join('\n');
  }

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <button type="button" class="tab-btn" data-tab="reset">ResetTable</button>
          <button type="button" class="tab-btn" data-tab="experience">ExperienceTable</button>
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="add-row">Agregar fila</button>
          <button type="button" id="save-table">Guardar</button>
        </div>
      </div>
      <div id="table-container"></div>
    `;
  }

  function renderTable() {
    const container = root.querySelector('#table-container');
    const data = state[state.active];
    if (!container || !data) return;
    container.innerHTML = '';
    const table = document.createElement('table');
    table.className = 'cfg-table';
    const thead = document.createElement('thead');
    const headRow = document.createElement('tr');
    data.columns.forEach((col) => {
      const th = document.createElement('th');
      th.textContent = col;
      headRow.appendChild(th);
    });
    thead.appendChild(headRow);
    table.appendChild(thead);
    const tbody = document.createElement('tbody');
    data.rows.forEach((row, rowIndex) => {
      const tr = document.createElement('tr');
      data.columns.forEach((_, colIndex) => {
        const td = document.createElement('td');
        const input = document.createElement('input');
        input.type = 'text';
        input.value = row[colIndex] ?? '';
        input.addEventListener('input', () => {
          row[colIndex] = input.value.trim();
        });
        td.appendChild(input);
        tr.appendChild(td);
      });
      tbody.appendChild(tr);
    });
    table.appendChild(tbody);
    container.appendChild(table);
  }

  function setTab(tab) {
    state.active = tab;
    root.querySelectorAll('.tab-btn').forEach((btn) => {
      btn.classList.toggle('active', btn.dataset.tab === tab);
    });
    renderTable();
  }

  async function saveActive() {
    try {
      const data = state[state.active];
      const content = buildTable(data);
      const res = await fetch(`/admin/server-editor/api/reset-exp?kind=${state.active}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ content })
      });
      const json = await res.json();
      if (!res.ok) throw new Error(json.error || 'No se pudo guardar');
      const reload = await fetch('/admin/server-editor/api/reload', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ target: 'util' })
      });
      if (!reload.ok) {
        setMessage('Guardado, pero no se pudo recargar en el server.', 'error');
      } else {
        setMessage('Guardado y recargado en el servidor.', 'success');
      }
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  async function loadAll() {
    const [resetRes, expRes] = await Promise.all([
      fetch('/admin/server-editor/api/reset-exp?kind=reset'),
      fetch('/admin/server-editor/api/reset-exp?kind=experience')
    ]);
    const resetData = await resetRes.json();
    const expData = await expRes.json();
    if (!resetRes.ok) throw new Error(resetData.error || 'No se pudo cargar ResetTable');
    if (!expRes.ok) throw new Error(expData.error || 'No se pudo cargar ExperienceTable');
    state.reset = parseTable(resetData.content || '');
    state.experience = parseTable(expData.content || '');
  }

  async function init() {
    buildLayout();
    root.querySelectorAll('.tab-btn').forEach((btn) => {
      btn.addEventListener('click', () => setTab(btn.dataset.tab));
    });
    root.querySelector('#add-row').addEventListener('click', () => {
      const data = state[state.active];
      const row = Array.from({ length: data.columns.length }).map(() => '0');
      data.rows.push(row);
      renderTable();
    });
    root.querySelector('#save-table').addEventListener('click', saveActive);
    try {
      await loadAll();
      setTab(state.active);
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
