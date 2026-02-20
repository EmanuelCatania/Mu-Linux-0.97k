(function () {
  const root = document.getElementById('mapmanager-editor');
  if (!root) return;

  const state = {
    maps: [],
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

  function parseMaybeStar(value) {
    if (value === '*') return '*';
    const num = Number(value);
    return Number.isNaN(num) ? '*' : num;
  }

  function formatMaybeStar(value) {
    if (value === '*' || value === '' || value === null || value === undefined) return '*';
    return Number(value);
  }

  function parseMapManager(content) {
    const lines = String(content || '').split(/\r?\n/);
    const maps = [];
    for (const rawLine of lines) {
      const base = rawLine.split('//')[0].trim();
      if (!base) continue;
      if (base.toLowerCase() === 'end') break;
      const nameMatch = base.match(/"([^"]+)"/);
      if (!nameMatch) continue;
      const name = nameMatch[1];
      const left = base.replace(/"[^"]+"/, '').trim();
      const tokens = left.match(/\S+/g) || [];
      if (tokens.length < 9) continue;
      maps.push({
        index: Number(tokens[0]),
        nonPk: parseMaybeStar(tokens[1]),
        view: Number(tokens[2]),
        expRate: Number(tokens[3]),
        itemDropRate: Number(tokens[4]),
        excDropRate: Number(tokens[5]),
        deadGate: Number(tokens[6]),
        spawnInPlace: Number(tokens[7]),
        flyingDragons: Number(tokens[8]),
        name
      });
    }
    return maps;
  }

  function buildMapManager() {
    const lines = [];
    lines.push('//Index\tNonPK\tView\tExperienceRate\tItemDropRate\tExcDropRate\tDeadGate\tSpawnInPlace\tFlyingDragons\tName');
    const maps = state.maps.slice().sort((a, b) => a.index - b.index);
    for (const map of maps) {
      const cleanName = String(map.name || '').replace(/"/g, '').trim();
      lines.push([
        map.index,
        formatMaybeStar(map.nonPk),
        map.view,
        map.expRate,
        map.itemDropRate,
        map.excDropRate,
        map.deadGate,
        map.spawnInPlace,
        map.flyingDragons,
        `"${cleanName}"`
      ].join('\t'));
    }
    lines.push('end');
    return lines.join('\n');
  }

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Buscar</label>
          <input type="text" id="map-search" placeholder="Buscar mapa..." />
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="add-map">Agregar mapa</button>
          <button type="button" id="save-map">Guardar</button>
        </div>
      </div>
      <div class="spawn-editor">
        <div class="spawn-list">
          <h3>Mapas</h3>
          <ul id="map-list" class="simple-list"></ul>
        </div>
        <div class="spawn-form">
          <h3>Detalle</h3>
          <label>Index</label>
          <input type="number" id="map-index" min="0" max="255" />
          <label>Nombre</label>
          <input type="text" id="map-name" />
          <label>NonPK</label>
          <input type="text" id="map-nonpk" placeholder="*" />
          <label>View</label>
          <input type="number" id="map-view" min="0" max="255" />
          <label>Experience Rate</label>
          <input type="number" id="map-exp" min="0" />
          <label>Item Drop Rate</label>
          <input type="number" id="map-drop" min="0" />
          <label>Exc Drop Rate</label>
          <input type="number" id="map-exc" min="0" />
          <label>Dead Gate</label>
          <input type="number" id="map-deadgate" min="0" />
          <label>Spawn In Place</label>
          <input type="number" id="map-spawn" min="0" max="1" />
          <label>Flying Dragons</label>
          <input type="number" id="map-dragons" min="0" max="1" />
          <div class="spawn-actions">
            <button type="button" id="update-map">Actualizar</button>
            <button type="button" id="delete-map" class="link-button">Eliminar</button>
          </div>
        </div>
      </div>
    `;
  }

  function renderList() {
    const list = root.querySelector('#map-list');
    if (!list) return;
    list.innerHTML = '';
    const filter = state.filter.toLowerCase();
    const maps = state.maps.slice().sort((a, b) => a.index - b.index);
    for (const map of maps) {
      const label = `${map.index} - ${map.name}`;
      if (filter && !label.toLowerCase().includes(filter)) continue;
      const item = document.createElement('li');
      item.textContent = label;
      item.dataset.index = String(map.index);
      if (state.selected && state.selected.index === map.index) {
        item.classList.add('selected');
      }
      list.appendChild(item);
    }
  }

  function selectMap(map) {
    state.selected = map;
    if (!map) {
      renderList();
      return;
    }
    root.querySelector('#map-index').value = map.index;
    root.querySelector('#map-name').value = map.name;
    root.querySelector('#map-nonpk').value = map.nonPk === '*' ? '*' : map.nonPk;
    root.querySelector('#map-view').value = map.view;
    root.querySelector('#map-exp').value = map.expRate;
    root.querySelector('#map-drop').value = map.itemDropRate;
    root.querySelector('#map-exc').value = map.excDropRate;
    root.querySelector('#map-deadgate').value = map.deadGate;
    root.querySelector('#map-spawn').value = map.spawnInPlace;
    root.querySelector('#map-dragons').value = map.flyingDragons;
    renderList();
  }

  function bindEvents() {
    root.querySelector('#map-search').addEventListener('input', (event) => {
      state.filter = event.target.value || '';
      renderList();
    });

    root.querySelector('#map-list').addEventListener('click', (event) => {
      const item = event.target.closest('li');
      if (!item) return;
      const index = Number(item.dataset.index);
      const map = state.maps.find((m) => m.index === index);
      if (map) selectMap(map);
    });

    root.querySelector('#add-map').addEventListener('click', () => {
      const nextIndex = state.maps.reduce((max, m) => Math.max(max, m.index), 0) + 1;
      const newMap = {
        index: nextIndex,
        nonPk: '*',
        view: 15,
        expRate: 100,
        itemDropRate: 100,
        excDropRate: 0,
        deadGate: 0,
        spawnInPlace: 0,
        flyingDragons: 1,
        name: 'NuevoMapa'
      };
      state.maps.push(newMap);
      selectMap(newMap);
      setMessage('Mapa creado.', 'success');
    });

    root.querySelector('#update-map').addEventListener('click', () => {
      if (!state.selected) return;
      const map = state.selected;
      map.index = Number(root.querySelector('#map-index').value);
      map.name = String(root.querySelector('#map-name').value || '').trim();
      map.nonPk = parseMaybeStar(String(root.querySelector('#map-nonpk').value || '').trim() || '*');
      map.view = Number(root.querySelector('#map-view').value);
      map.expRate = Number(root.querySelector('#map-exp').value);
      map.itemDropRate = Number(root.querySelector('#map-drop').value);
      map.excDropRate = Number(root.querySelector('#map-exc').value);
      map.deadGate = Number(root.querySelector('#map-deadgate').value);
      map.spawnInPlace = Number(root.querySelector('#map-spawn').value);
      map.flyingDragons = Number(root.querySelector('#map-dragons').value);
      renderList();
      setMessage('Mapa actualizado.', 'success');
    });

    root.querySelector('#delete-map').addEventListener('click', () => {
      if (!state.selected) return;
      state.maps = state.maps.filter((m) => m.index !== state.selected.index);
      state.selected = null;
      renderList();
      setMessage('Mapa eliminado.', 'success');
    });

    root.querySelector('#save-map').addEventListener('click', async () => {
      try {
        const content = buildMapManager();
        const res = await fetch('/admin/server-editor/api/mapmanager', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ content })
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'No se pudo guardar');
        const reload = await fetch('/admin/server-editor/api/reload', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ target: 'map' })
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
    const res = await fetch('/admin/server-editor/api/mapmanager');
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || 'No se pudo cargar');
    state.maps = parseMapManager(data.content || '');
  }

  async function init() {
    buildLayout();
    bindEvents();
    try {
      await loadInitialData();
      renderList();
      if (state.maps[0]) selectMap(state.maps[0]);
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
