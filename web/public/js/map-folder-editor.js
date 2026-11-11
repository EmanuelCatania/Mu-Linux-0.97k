(function () {
  const root = document.getElementById('map-folder-editor');
  if (!root) return;
  const t = (text, params) => window.MuText.t(text, params);

  // Orden de las pestañas, nombre y editor completo de cada tipo.
  const KINDS = [
    ['map', 'Reglas', '/admin/server-editor/mapmanager'],
    ['spawns', 'Spawns', '/admin/server-editor/spawns'],
    ['drops', 'Drops', '/admin/server-editor/drop'],
    ['gates', 'Gates', '/admin/server-editor/gates'],
    ['zones', 'Zonas', '/admin/server-editor/customs']
  ];

  const state = {
    folders: [],
    kinds: {},
    folder: null,
    kind: 'map',
    path: '',
    exists: false,
    loaded: '', // contenido tal como vino del server, para saber si hay cambios
    busy: false
  };

  function setMessage(text, type) {
    const box = root.querySelector('.editor-message');
    box.textContent = text || '';
    box.classList.toggle('hidden', !text);
    box.classList.toggle('success', type === 'success');
    box.classList.toggle('error', type === 'error');
  }

  async function api(method, url, body) {
    const res = await fetch(url, {
      method,
      headers: body ? { 'Content-Type': 'application/json' } : undefined,
      body: body ? JSON.stringify(body) : undefined
    });
    const data = await res.json().catch(() => ({}));
    if (!res.ok) throw new Error(data.error || `HTTP ${res.status}`);
    return data;
  }

  const textarea = () => root.querySelector('#folder-content');
  const dirty = () => textarea().value !== state.loaded;

  function confirmDiscard() {
    return !dirty() || window.confirm(t('Hay cambios sin guardar. ¿Descartarlos?'));
  }

  function current() {
    return state.folders.find((f) => f.folder === state.folder);
  }

  function renderShell() {
    root.innerHTML = window.MuText.html`
      <div class="editor-message alert hidden"></div>
      <div class="map-folder-editor">
        <div class="spawn-list">
          <h3>Mapas</h3>
          <ul id="folder-list" class="simple-list map-folder-list"></ul>
        </div>
        <div class="spawn-form map-folder-form">
          <div id="folder-tabs" class="map-folder-tabs"></div>
          <div class="spawn-toolbar">
            <code id="folder-path"></code>
            <a id="folder-full-editor" class="button-link" href="#">Abrir el editor completo</a>
          </div>
          <div id="folder-note" class="alert warning hidden"></div>
          <textarea id="folder-content" class="map-folder-text" spellcheck="false" rows="28"></textarea>
          <div class="spawn-actions">
            <button type="button" id="folder-check">Validar</button>
            <button type="button" id="folder-save">Guardar y recargar</button>
            <button type="button" id="folder-revert" class="link-button">Descartar cambios</button>
            <button type="button" id="folder-delete" class="link-button">Borrar archivo</button>
          </div>
        </div>
      </div>
    `;
    root.querySelector('#folder-check').addEventListener('click', () => save(true));
    root.querySelector('#folder-save').addEventListener('click', () => save(false));
    root.querySelector('#folder-revert').addEventListener('click', () => {
      textarea().value = state.loaded;
      setMessage('');
    });
    root.querySelector('#folder-delete').addEventListener('click', remove);
    window.addEventListener('beforeunload', (event) => {
      if (dirty()) event.preventDefault();
    });
  }

  function renderList() {
    const list = root.querySelector('#folder-list');
    list.innerHTML = '';
    for (const f of state.folders) {
      const li = document.createElement('li');
      li.textContent = f.folder;
      li.title = f.kinds.map((k) => state.kinds[k].file).join(', ');
      li.classList.toggle('selected', f.folder === state.folder);
      li.addEventListener('click', () => {
        if (f.folder === state.folder || !confirmDiscard()) return;
        state.folder = f.folder;
        load();
      });
      list.appendChild(li);
    }
  }

  function renderTabs() {
    const box = root.querySelector('#folder-tabs');
    const folder = current();
    box.innerHTML = '';
    for (const [kind, label] of KINDS) {
      const btn = document.createElement('button');
      btn.type = 'button';
      const has = folder && folder.kinds.includes(kind);
      btn.textContent = has ? t(label) : `${t(label)} (${t('no tiene')})`;
      btn.className = 'map-folder-tab' + (kind === state.kind ? ' selected' : '') + (has ? '' : ' missing');
      btn.addEventListener('click', () => {
        if (kind === state.kind || !confirmDiscard()) return;
        state.kind = kind;
        load();
      });
      box.appendChild(btn);
    }
  }

  function renderFile() {
    const def = state.kinds[state.kind];
    root.querySelector('#folder-path').textContent = state.path;
    root.querySelector('#folder-full-editor').href = KINDS.find((k) => k[0] === state.kind)[2];
    root.querySelector('#folder-save').textContent = state.exists ? t('Guardar y recargar') : t('Crear y recargar');
    root.querySelector('#folder-delete').classList.toggle('hidden', !state.exists || !def.deletable);
    const note = root.querySelector('#folder-note');
    let text = '';
    if (!state.exists && !def.toml) {
      text = t('Este tipo todavía está en el formato de un solo archivo. Pasalo a archivos por mapa desde el editor completo para poder editarlo acá.');
    } else if (!state.exists) {
      text = t('Este mapa no tiene el archivo. Se crea al guardar.');
    }
    note.textContent = text;
    note.classList.toggle('hidden', !text);
    textarea().value = state.loaded;
    textarea().disabled = !state.exists && !def.toml;
  }

  async function load() {
    renderList();
    renderTabs();
    setMessage('');
    try {
      const q = new URLSearchParams({ folder: state.folder, kind: state.kind });
      const data = await api('GET', `/admin/server-editor/api/map-folders/file?${q}`);
      state.path = data.path;
      state.exists = data.exists;
      state.loaded = data.content.replace(/\r\n/g, '\n');
      renderFile();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  async function refreshFolders() {
    const data = await api('GET', '/admin/server-editor/api/map-folders');
    state.folders = data.folders;
    state.kinds = data.kinds;
  }

  async function reload(target) {
    try {
      await api('POST', '/admin/server-editor/api/reload', { target });
      return t('El GameServer lo aplica en unos segundos.');
    } catch (err) {
      return t('No se pudo pedir la recarga: {error}', { error: err.message });
    }
  }

  async function save(check) {
    if (state.busy) return;
    state.busy = true;
    try {
      const content = textarea().value;
      const data = await api('POST', '/admin/server-editor/api/map-folders/file', {
        folder: state.folder, kind: state.kind, content, check
      });
      if (check) {
        setMessage(t('El archivo es válido.'), 'success');
        return;
      }
      if (data.unchanged) {
        setMessage(t('No hay cambios que guardar.'), 'success');
        return;
      }
      state.loaded = content;
      const created = !state.exists;
      await refreshFolders();
      state.exists = true;
      renderList();
      renderTabs();
      renderFile();
      setMessage(`${created ? t('Archivo creado.') : t('Guardado.')} ${await reload(data.reload)}`, 'success');
    } catch (err) {
      setMessage(err.message, 'error');
    } finally {
      state.busy = false;
    }
  }

  async function remove() {
    if (state.busy) return;
    if (!window.confirm(t('¿Borrar {path}? Queda una copia de respaldo.', { path: state.path }))) return;
    state.busy = true;
    try {
      const data = await api('POST', '/admin/server-editor/api/map-folders/delete', { folder: state.folder, kind: state.kind });
      await refreshFolders();
      state.busy = false;
      await load();
      setMessage(`${t('Archivo borrado.')} ${await reload(data.reload)}`, 'success');
    } catch (err) {
      setMessage(err.message, 'error');
    } finally {
      state.busy = false;
    }
  }

  async function init() {
    renderShell();
    try {
      await refreshFolders();
      if (!state.folders.length) {
        setMessage(t('No hay carpetas de mapa en Data/Maps.'), 'error');
        return;
      }
      state.folder = state.folders[0].folder;
      await load();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
