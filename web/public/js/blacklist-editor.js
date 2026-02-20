(function () {
  const root = document.getElementById('blacklist-editor');
  if (!root) return;

  const state = {
    ips: [],
    hwids: []
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

  function parseBlacklist(content) {
    const lines = String(content || '').split(/\r?\n/);
    let section = null;
    const ips = [];
    const hwids = [];
    for (const rawLine of lines) {
      const line = rawLine.split('//')[0].trim();
      if (!line) continue;
      if (line === '0' || line === '1') {
        section = line;
        continue;
      }
      if (line.toLowerCase() === 'end') {
        section = null;
        continue;
      }
      const match = line.match(/"([^"]+)"/);
      if (!match) continue;
      if (section === '0') ips.push(match[1]);
      if (section === '1') hwids.push(match[1]);
    }
    return { ips, hwids };
  }

  function buildBlacklist() {
    const lines = [];
    lines.push('0');
    lines.push('// IpAddress');
    state.ips.forEach((ip) => {
      const value = String(ip || '').replace(/"/g, '').trim();
      if (value) lines.push(`"${value}"`);
    });
    lines.push('end');
    lines.push('');
    lines.push('1');
    lines.push('// HardwareID');
    state.hwids.forEach((hwid) => {
      const value = String(hwid || '').replace(/"/g, '').trim();
      if (value) lines.push(`"${value}"`);
    });
    lines.push('end');
    return lines.join('\n');
  }

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>IP</label>
          <input type="text" id="ip-input" placeholder="127.0.0.1" />
          <button type="button" id="add-ip">Agregar IP</button>
        </div>
        <div class="spawn-toolbar-group">
          <label>HWID</label>
          <input type="text" id="hwid-input" placeholder="C46A1ADE-..." />
          <button type="button" id="add-hwid">Agregar HWID</button>
          <button type="button" id="save-blacklist">Guardar</button>
        </div>
      </div>
      <div class="spawn-editor">
        <div class="spawn-list">
          <h3>IPs</h3>
          <ul id="ip-list" class="simple-list"></ul>
        </div>
        <div class="spawn-list">
          <h3>Hardware IDs</h3>
          <ul id="hwid-list" class="simple-list"></ul>
        </div>
      </div>
    `;
  }

  function renderList(listEl, items, type) {
    listEl.innerHTML = '';
    items.forEach((value, index) => {
      const item = document.createElement('li');
      item.className = 'list-item-row';
      const text = document.createElement('span');
      text.textContent = value;
      const btn = document.createElement('button');
      btn.type = 'button';
      btn.textContent = 'Eliminar';
      btn.className = 'link-button';
      btn.addEventListener('click', () => {
        if (type === 'ip') {
          state.ips.splice(index, 1);
        } else {
          state.hwids.splice(index, 1);
        }
        refresh();
      });
      item.appendChild(text);
      item.appendChild(btn);
      listEl.appendChild(item);
    });
  }

  function refresh() {
    renderList(root.querySelector('#ip-list'), state.ips, 'ip');
    renderList(root.querySelector('#hwid-list'), state.hwids, 'hwid');
  }

  function bindEvents() {
    root.querySelector('#add-ip').addEventListener('click', () => {
      const value = String(root.querySelector('#ip-input').value || '').trim();
      if (!value) return;
      state.ips.push(value);
      root.querySelector('#ip-input').value = '';
      refresh();
    });

    root.querySelector('#add-hwid').addEventListener('click', () => {
      const value = String(root.querySelector('#hwid-input').value || '').trim();
      if (!value) return;
      state.hwids.push(value);
      root.querySelector('#hwid-input').value = '';
      refresh();
    });

    root.querySelector('#save-blacklist').addEventListener('click', async () => {
      try {
        const content = buildBlacklist();
        const res = await fetch('/admin/server-editor/api/blacklist', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ content })
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'No se pudo guardar');
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
    });
  }

  async function loadInitialData() {
    const res = await fetch('/admin/server-editor/api/blacklist');
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || 'No se pudo cargar');
    const parsed = parseBlacklist(data.content || '');
    state.ips = parsed.ips;
    state.hwids = parsed.hwids;
  }

  async function init() {
    buildLayout();
    bindEvents();
    try {
      await loadInitialData();
      refresh();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
