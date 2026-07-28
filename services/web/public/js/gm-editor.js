(function () {
  const root = document.getElementById('gm-editor');
  if (!root) return;

  const state = {
    entries: [],
    selected: null,
    filter: ''
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

  function parseGameMaster(content) {
    const lines = String(content || '').split(/\r?\n/);
    const entries = [];
    for (const rawLine of lines) {
      const line = rawLine.split('//')[0].trim();
      if (!line) continue;
      if (line.toLowerCase() === 'end') break;
      const match = line.match(/"([^"]+)"\s+"([^"]+)"\s+(\d+)/);
      if (!match) continue;
      entries.push({
        account: match[1],
        name: match[2],
        level: Number(match[3])
      });
    }
    return entries;
  }

  function buildGameMaster() {
    const lines = [];
    lines.push('//Account\tName\t\t\t\tLevel');
    const entries = state.entries.slice().sort((a, b) => {
      if (a.account === b.account) return a.name.localeCompare(b.name);
      return a.account.localeCompare(b.account);
    });
    for (const entry of entries) {
      const acc = String(entry.account || '').replace(/"/g, '').trim();
      const name = String(entry.name || '').replace(/"/g, '').trim();
      lines.push(`"${acc}"\t\t"${name}"\t\t\t${entry.level}`);
    }
    lines.push('end');
    return lines.join('\n');
  }

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Cuenta</label>
          <input type="text" id="gm-account" placeholder="account" list="gm-account-list" />
          <datalist id="gm-account-list"></datalist>
          <button type="button" id="load-characters">Cargar personajes</button>
          <label>Personaje</label>
          <select id="gm-character">
            <option value="">-</option>
          </select>
          <label>Nivel GM</label>
          <input type="number" id="gm-level-add" min="0" max="3" value="1" />
          <button type="button" id="add-gm">Agregar</button>
        </div>
        <div class="spawn-toolbar-group">
          <label>Buscar</label>
          <input type="text" id="gm-search" placeholder="Buscar..." />
          <button type="button" id="save-gm">Guardar</button>
        </div>
      </div>
      <div class="spawn-editor">
        <div class="spawn-list">
          <h3>GM</h3>
          <ul id="gm-list" class="simple-list"></ul>
        </div>
        <div class="spawn-form">
          <h3>Detalle</h3>
          <label>Cuenta</label>
          <input type="text" id="gm-account-edit" />
          <label>Personaje</label>
          <input type="text" id="gm-name-edit" />
          <label>Nivel</label>
          <input type="number" id="gm-level-edit" min="0" max="3" />
          <div class="spawn-actions">
            <button type="button" id="update-gm">Actualizar</button>
            <button type="button" id="delete-gm" class="link-button">Eliminar</button>
          </div>
        </div>
      </div>
    `;
  }

  function renderList() {
    const list = root.querySelector('#gm-list');
    if (!list) return;
    list.innerHTML = '';
    const filter = state.filter.toLowerCase();
    const entries = state.entries.slice().sort((a, b) => {
      if (a.account === b.account) return a.name.localeCompare(b.name);
      return a.account.localeCompare(b.account);
    });
    for (const entry of entries) {
      const label = `${entry.account} - ${entry.name} (GM ${entry.level})`;
      if (filter && !label.toLowerCase().includes(filter)) continue;
      const item = document.createElement('li');
      item.textContent = label;
      item.dataset.account = entry.account;
      item.dataset.name = entry.name;
      if (state.selected && state.selected.account === entry.account && state.selected.name === entry.name) {
        item.classList.add('selected');
      }
      list.appendChild(item);
    }
  }

  function selectEntry(entry) {
    state.selected = entry;
    if (!entry) {
      renderList();
      return;
    }
    root.querySelector('#gm-account-edit').value = entry.account;
    root.querySelector('#gm-name-edit').value = entry.name;
    root.querySelector('#gm-level-edit').value = entry.level;
    renderList();
  }

  async function loadCharacters() {
    const account = String(root.querySelector('#gm-account').value || '').trim();
    if (!account) {
      setMessage('Ingresa una cuenta.', 'error');
      return;
    }
    const select = root.querySelector('#gm-character');
    select.innerHTML = '<option value="">Cargando...</option>';
    try {
      const res = await fetch(`/admin/server-editor/api/characters?account=${encodeURIComponent(account)}`);
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudo cargar');
      select.innerHTML = '<option value="">-</option>';
      (data.characters || []).forEach((name) => {
        const option = document.createElement('option');
        option.value = name;
        option.textContent = name;
        select.appendChild(option);
      });
      if ((data.characters || []).length === 0) {
        setMessage('No se encontraron personajes.', 'error');
      }
    } catch (err) {
      setMessage(err.message, 'error');
      select.innerHTML = '<option value="">-</option>';
    }
  }

  function addGM() {
    const account = String(root.querySelector('#gm-account').value || '').trim();
    const name = String(root.querySelector('#gm-character').value || '').trim();
    const level = Number(root.querySelector('#gm-level-add').value);
    if (!account || !name) {
      setMessage('Selecciona cuenta y personaje.', 'error');
      return;
    }
    const existing = state.entries.find((e) => e.account === account && e.name === name);
    if (existing) {
      existing.level = level;
      selectEntry(existing);
      setMessage('GM actualizado.', 'success');
      return;
    }
    const entry = { account, name, level };
    state.entries.push(entry);
    selectEntry(entry);
    setMessage('GM agregado.', 'success');
  }

  function bindEvents() {
    root.querySelector('#gm-search').addEventListener('input', (event) => {
      state.filter = event.target.value || '';
      renderList();
    });

    root.querySelector('#load-characters').addEventListener('click', loadCharacters);
    root.querySelector('#add-gm').addEventListener('click', addGM);

    root.querySelector('#gm-list').addEventListener('click', (event) => {
      const item = event.target.closest('li');
      if (!item) return;
      const account = item.dataset.account;
      const name = item.dataset.name;
      const entry = state.entries.find((e) => e.account === account && e.name === name);
      if (entry) selectEntry(entry);
    });

    root.querySelector('#update-gm').addEventListener('click', () => {
      if (!state.selected) return;
      state.selected.account = String(root.querySelector('#gm-account-edit').value || '').trim();
      state.selected.name = String(root.querySelector('#gm-name-edit').value || '').trim();
      state.selected.level = Number(root.querySelector('#gm-level-edit').value);
      renderList();
      setMessage('GM actualizado.', 'success');
    });

    root.querySelector('#delete-gm').addEventListener('click', () => {
      if (!state.selected) return;
      state.entries = state.entries.filter(
        (e) => !(e.account === state.selected.account && e.name === state.selected.name)
      );
      state.selected = null;
      renderList();
      setMessage('GM eliminado.', 'success');
    });

    root.querySelector('#save-gm').addEventListener('click', async () => {
      try {
        const content = buildGameMaster();
        const res = await fetch('/admin/server-editor/api/gamemaster', {
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

  async function loadAccounts() {
    const dataList = root.querySelector('#gm-account-list');
    if (!dataList) return;
    dataList.innerHTML = '';
    try {
      const res = await fetch('/admin/server-editor/api/accounts');
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudieron cargar las cuentas');
      (data.accounts || []).forEach((account) => {
        const option = document.createElement('option');
        option.value = account;
        dataList.appendChild(option);
      });
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  async function loadInitialData() {
    const res = await fetch('/admin/server-editor/api/gamemaster');
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || 'No se pudo cargar');
    state.entries = parseGameMaster(data.content || '');
  }

  async function init() {
    buildLayout();
    bindEvents();
    try {
      await loadInitialData();
      await loadAccounts();
      renderList();
      if (state.entries[0]) selectEntry(state.entries[0]);
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
