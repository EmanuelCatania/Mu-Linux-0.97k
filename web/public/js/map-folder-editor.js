// Editor por mapa: los TOML de una carpeta de Data/Maps, una pestaña por
// archivo. El Spawns.toml tiene dos modos, Visual (el mapa con los spawns,
// spawn-visual.js) y Texto; los dos editan el mismo archivo y cambiar de modo
// no pierde lo editado. Los demás archivos se editan como texto.
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

  // Tipos de spawn de un Spawns.toml de mapa (las invasiones van en otro archivo).
  const MAP_SPAWN_TYPES = [0, 1, 2, 4];

  const state = {
    folders: [],
    kinds: {},
    folder: null,
    kind: 'map',
    path: '',
    exists: false,
    loaded: '', // contenido tal como vino del server, para saber si hay cambios
    busy: false,
    mode: 'visual', // modo de los spawns: 'visual' | 'text'
    monsters: null, // Monster.txt, se pide la primera vez que se abre el modo visual
    view: null, // el editor visual abierto
    visualBase: '', // texto del que salió el modo visual
    visualSnapshot: '' // los spawns tal como se abrieron, para saber si cambiaron
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
  const visual = () => state.kind === 'spawns' && state.mode === 'visual' && !!state.view;

  // Spawns del modo visual <-> los del Spawns.toml.
  const toServer = (e) => ({ section: e.type, monster: e.monsterId, dis: e.dis, x: e.x, y: e.y, dir: e.dir, tx: e.x2, ty: e.y2, count: e.count });
  const toVisual = (e, map, index) => ({
    id: index + 1, type: e.section, monsterId: e.monster, map, dis: e.dis, x: e.x, y: e.y, dir: e.dir, x2: e.tx, y2: e.ty, count: e.count, value: e.value
  });
  const snapshot = () => JSON.stringify(state.view.getEntries().map(toServer));
  const visualChanged = () => visual() && snapshot() !== state.visualSnapshot;

  function dirty() {
    if (visual()) return visualChanged() || state.visualBase !== state.loaded;
    return textarea().value !== state.loaded;
  }

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
            <div id="folder-modes" class="map-folder-modes hidden">
              <button type="button" class="map-folder-mode" data-mode="visual">Visual</button>
              <button type="button" class="map-folder-mode" data-mode="text">Texto</button>
            </div>
            <a id="folder-full-editor" class="button-link" href="#">Abrir el editor completo</a>
          </div>
          <div id="folder-note" class="alert warning hidden"></div>
          <textarea id="folder-content" class="map-folder-text" spellcheck="false" rows="28"></textarea>
          <div id="folder-visual" class="map-folder-visual hidden"></div>
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
    root.querySelector('#folder-revert').addEventListener('click', revert);
    root.querySelector('#folder-delete').addEventListener('click', remove);
    root.querySelectorAll('.map-folder-mode').forEach((btn) => {
      btn.addEventListener('click', () => switchMode(btn.dataset.mode));
    });
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

  // Muestra el texto o el mapa según el modo.
  function renderMode() {
    const editable = !textarea().disabled;
    const spawns = state.kind === 'spawns' && editable;
    const showVisual = spawns && visual();
    root.querySelector('#folder-modes').classList.toggle('hidden', !spawns);
    root.querySelectorAll('.map-folder-mode').forEach((btn) => {
      btn.classList.toggle('selected', btn.dataset.mode === (showVisual ? 'visual' : 'text'));
    });
    textarea().classList.toggle('hidden', showVisual);
    root.querySelector('#folder-visual').classList.toggle('hidden', !showVisual);
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
    renderMode();
  }

  async function monsters() {
    if (!state.monsters) {
      const data = await api('GET', '/admin/server-editor/api/monster-defs');
      state.monsters = Array.isArray(data.monsters) ? data.monsters : [];
    }
    return state.monsters;
  }

  // Abre el modo visual sobre `text`. Si el texto no es válido tira el error y
  // la página queda en modo texto.
  async function openVisual(text) {
    const folder = current();
    const [list, data] = await Promise.all([
      monsters(),
      api('POST', '/admin/server-editor/api/map-folders/spawns/parse', { folder: state.folder, content: text })
    ]);
    const box = root.querySelector('#folder-visual');
    state.view = window.MuSpawnVisual.create(box, {
      maps: [{ id: folder.map, name: folder.folder }],
      monsters: list,
      entries: data.entries.map((e, i) => toVisual(e, data.map, i)),
      map: data.map,
      lockMap: true,
      types: MAP_SPAWN_TYPES,
      setMessage
    });
    state.visualBase = text;
    state.visualSnapshot = snapshot();
  }

  function closeVisual() {
    state.view = null;
    root.querySelector('#folder-visual').innerHTML = '';
  }

  // El texto actual del archivo, venga del modo que venga. Si en el modo
  // visual no se tocó nada, devuelve el texto original (con sus comentarios).
  async function currentText() {
    if (!visual()) return textarea().value;
    if (!visualChanged()) return state.visualBase;
    const data = await api('POST', '/admin/server-editor/api/map-folders/spawns/build', {
      folder: state.folder,
      entries: state.view.getEntries().map(toServer)
    });
    return data.content;
  }

  async function switchMode(mode) {
    if (state.busy) return;
    if (mode === (visual() ? 'visual' : 'text')) {
      state.mode = mode;
      return;
    }
    state.busy = true;
    try {
      if (mode === 'text') {
        const text = await currentText();
        if (visualChanged()) setMessage(t('El texto se armó de nuevo desde el mapa: los comentarios de cada línea son los nombres de los monstruos.'), 'success');
        else setMessage('');
        closeVisual();
        textarea().value = text;
      } else {
        await openVisual(textarea().value);
        setMessage('');
      }
      state.mode = mode;
      renderMode();
    } catch (err) {
      setMessage(mode === 'visual' ? t('No se puede abrir en el mapa: {error}', { error: err.message }) : err.message, 'error');
    } finally {
      state.busy = false;
    }
  }

  async function showContent() {
    closeVisual();
    textarea().value = state.loaded;
    if (state.kind === 'spawns' && state.mode === 'visual' && !textarea().disabled) {
      try {
        await openVisual(state.loaded);
      } catch (err) {
        setMessage(t('No se puede abrir en el mapa: {error}', { error: err.message }), 'error');
      }
    }
    renderMode();
  }

  async function load() {
    renderList();
    renderTabs();
    setMessage('');
    closeVisual();
    try {
      const q = new URLSearchParams({ folder: state.folder, kind: state.kind });
      const data = await api('GET', `/admin/server-editor/api/map-folders/file?${q}`);
      state.path = data.path;
      state.exists = data.exists;
      state.loaded = data.content.replace(/\r\n/g, '\n');
      renderFile();
      await showContent();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  async function revert() {
    if (state.busy) return;
    setMessage('');
    await showContent();
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
    if (visual() && state.view.findInvalid()) return;
    state.busy = true;
    try {
      const content = await currentText();
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
      textarea().value = content;
      if (visual()) {
        state.visualBase = content;
        state.visualSnapshot = snapshot();
      }
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
      // ?folder=02-Devias&kind=spawns&mode=text abre directo ahí.
      const params = new URLSearchParams(window.location.search);
      const folder = state.folders.find((f) => f.folder === params.get('folder'));
      state.folder = (folder || state.folders[0]).folder;
      if (KINDS.some((k) => k[0] === params.get('kind'))) state.kind = params.get('kind');
      if (params.get('mode') === 'text') state.mode = 'text';
      await load();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
