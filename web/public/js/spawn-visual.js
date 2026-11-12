// Editor visual de spawns: el mapa (.att) con los spawns encima, la lista y el
// detalle. Lo usan el editor de spawns de todos los mapas (monsterset-editor.js)
// y la pestaña Spawns del editor por mapa (map-folder-editor.js).
//
//   const view = window.MuSpawnVisual.create(container, {
//     maps, monsters,            // [{ id, name }]
//     entries,                   // spawns, ver más abajo
//     map,                       // mapa que se muestra al abrir
//     lockMap,                   // true: un solo mapa (sin selector, Map fijo)
//     types,                     // tipos que se pueden editar (por defecto todos)
//     setMessage(text, type),
//     onChange()                 // después de cada cambio en los spawns
//   });
//   view.getEntries(), view.setEntries(list), view.findInvalid(), view.toolbar
//
// Cada spawn: { id, type, monsterId, map, dis, x, y, dir } y, para los de área
// (tipos 1 y 3), x2, y2, count; las invasiones (3) llevan además value.
(function () {
  const TYPE_META = [
    { type: 0, label: 'NPCs', mode: 'point' },
    { type: 1, label: 'Spots', mode: 'box' },
    { type: 2, label: 'Monsters', mode: 'point-random' },
    { type: 3, label: 'Invasiones', mode: 'box-value' },
    { type: 4, label: 'Eventos', mode: 'point' }
  ];

  const t = (text, params) => window.MuText.t(text, params);
  const isBoxType = (type) => type === 1 || type === 3;

  function toNumber(value) {
    const raw = String(value ?? '').trim();
    if (!raw) return NaN;
    const num = Number(raw);
    return Number.isFinite(num) ? num : NaN;
  }

  function isInRange(value, min, max) {
    return Number.isFinite(value) && value >= min && value <= max;
  }

  function clampCoord(value, max) {
    if (Number.isNaN(value)) return 0;
    if (value < 0) return 0;
    if (value > max) return max;
    return value;
  }

  function setFieldInvalid(el, ok) {
    if (!el) return ok;
    const wrapper = el.closest('.field') || el;
    wrapper.classList.toggle('invalid', !ok);
    if (wrapper !== el) {
      el.classList.toggle('invalid', !ok);
    }
    return ok;
  }

  function decodeTerrain(base64) {
    if (!base64) return null;
    const binary = atob(base64);
    const bytes = new Uint8Array(binary.length);
    for (let i = 0; i < binary.length; i++) {
      bytes[i] = binary.charCodeAt(i);
    }
    if (bytes.length < 3) return null;
    const width = bytes[1] + 1;
    const height = bytes[2] + 1;
    const data = bytes.slice(3);
    return { width, height, data };
  }

  function getAttrLabel(attr) {
    const labels = [];
    if (attr & 0x04) labels.push(t('Bloqueado'));
    if (attr & 0x01) labels.push('Safe');
    if (attr & 0x10) labels.push(t('Agua'));
    if (attr & 0x08) labels.push('Hueco');
    if (labels.length === 0) return 'Libre';
    return labels.join(', ');
  }

  // El .att de cada mapa se pide una sola vez por página.
  const terrainCache = new Map();

  function create(root, options) {
    const types = TYPE_META.filter((meta) => !options.types || options.types.includes(meta.type));
    const lockMap = !!options.lockMap;
    const setMessage = options.setMessage || (() => {});
    const changed = () => { if (options.onChange) options.onChange(); };

    const state = {
      maps: options.maps || [],
      monsters: options.monsters || [],
      entries: options.entries || [],
      selectedMap: options.map ?? options.maps?.[0]?.id ?? 0,
      selectedType: types[0].type,
      selectedEntry: null,
      placeMode: false,
      resizeMode: false,
      dragging: false,
      dragStart: null,
      dragEnd: null
    };
    let canvasScale = 2;

    const $ = (selector) => root.querySelector(selector);

    function monsterExists(monsterId) {
      return state.monsters.some((monster) => monster.id === monsterId);
    }

    function getMonsterName(monsterId) {
      const found = state.monsters.find((m) => m.id === monsterId);
      return found ? found.name : `Monster ${monsterId}`;
    }

    function isEntryValid(entry) {
      if (!monsterExists(entry.monsterId)) return false;
      if (!isInRange(entry.map, 0, 255)) return false;
      if (!isInRange(entry.dis, 0, 255)) return false;
      if (!isInRange(entry.x, 0, 255)) return false;
      if (!isInRange(entry.y, 0, 255)) return false;
      if (!isInRange(entry.dir, -1, 7)) return false;
      if (isBoxType(entry.type)) {
        if (!isInRange(entry.x2, 0, 255)) return false;
        if (!isInRange(entry.y2, 0, 255)) return false;
        // Mismas reglas que el GameServer: la cantidad va de 0 a 10000 y un
        // área con tx < x o ty < y vale (la toma como una línea), hay mapas así.
        if (!isInRange(entry.count, 0, 10000)) return false;
      }
      if (entry.type === 3) {
        if (!isInRange(entry.value, 0, 255)) return false;
      }
      return true;
    }

    function validateEntryForm() {
      const monsterInput = $('#monster-select');
      const mapInput = $('#spawn-map');
      const disInput = $('#spawn-dis');
      const xInput = $('#spawn-x');
      const yInput = $('#spawn-y');
      const x2Input = $('#spawn-x2');
      const y2Input = $('#spawn-y2');
      const countInput = $('#spawn-count');
      const valueInput = $('#spawn-value');
      const dirInput = $('#spawn-dir');

      const monsterId = toNumber(monsterInput?.value);
      const map = lockMap ? state.selectedMap : toNumber(mapInput?.value);
      const dis = toNumber(disInput?.value);
      const x = toNumber(xInput?.value);
      const y = toNumber(yInput?.value);
      const x2 = toNumber(x2Input?.value);
      const y2 = toNumber(y2Input?.value);
      const count = toNumber(countInput?.value);
      const value = toNumber(valueInput?.value);
      const dir = toNumber(dirInput?.value);

      const okMonster = setFieldInvalid(monsterInput, monsterExists(monsterId));
      const okMap = setFieldInvalid(mapInput, isInRange(map, 0, 255));
      const okDis = setFieldInvalid(disInput, isInRange(dis, 0, 255));
      const okX = setFieldInvalid(xInput, isInRange(x, 0, 255));
      const okY = setFieldInvalid(yInput, isInRange(y, 0, 255));
      const okDir = setFieldInvalid(dirInput, isInRange(dir, -1, 7));

      let okX2 = true;
      let okY2 = true;
      let okCount = true;
      let okValue = true;
      if (isBoxType(state.selectedType)) {
        okX2 = setFieldInvalid(x2Input, isInRange(x2, 0, 255));
        okY2 = setFieldInvalid(y2Input, isInRange(y2, 0, 255));
        okCount = setFieldInvalid(countInput, isInRange(count, 0, 10000));
      } else {
        setFieldInvalid(x2Input, true);
        setFieldInvalid(y2Input, true);
        setFieldInvalid(countInput, true);
      }
      if (state.selectedType === 3) {
        okValue = setFieldInvalid(valueInput, isInRange(value, 0, 255));
      } else {
        setFieldInvalid(valueInput, true);
      }

      const ok = okMonster && okMap && okDis && okX && okY && okDir && okX2 && okY2 && okCount && okValue;
      return { ok, entry: { monsterId, map, dis, x, y, x2, y2, count, value, dir } };
    }

    function buildLayout() {
      root.innerHTML = window.MuText.html`
        <div class="spawn-toolbar">
          <div class="spawn-toolbar-group">
            <label class="spawn-map-picker">Mapa</label>
            <select id="map-select" class="spawn-map-picker"></select>
            <label>Tipo</label>
            <select id="type-select"></select>
            <label>Zoom</label>
            <input type="range" id="zoom-range" min="1" max="3" step="1" value="2" />
          </div>
          <div class="spawn-toolbar-group spawn-toolbar-actions">
            <button type="button" id="add-spawn">Agregar spawn</button>
            <button type="button" id="resize-area">Ajustar area</button>
          </div>
        </div>
        <div class="spawn-editor">
          <div class="spawn-map">
            <canvas id="map-canvas"></canvas>
            <div class="spawn-coords">X: -- Y: --</div>
            <div class="spawn-map-legend">
              <span class="legend legend-block">Bloqueado</span>
              <span class="legend legend-safe">Safe</span>
              <span class="legend legend-water">Agua</span>
            </div>
          </div>
          <div class="spawn-panel">
            <div class="spawn-list">
              <h3>Spawns</h3>
              <ul id="spawn-list"></ul>
            </div>
            <div class="spawn-form">
              <h3>Detalle</h3>
              <label>Monster</label>
              <select id="monster-select"></select>
              <label>Map</label>
              <input type="number" id="spawn-map" min="0" max="255" />
              <label>Range</label>
              <input type="number" id="spawn-dis" min="0" max="255" />
              <label>X</label>
              <input type="number" id="spawn-x" min="0" max="255" />
              <label>Y</label>
              <input type="number" id="spawn-y" min="0" max="255" />
              <label class="spawn-field-box">X2</label>
              <input type="number" id="spawn-x2" min="0" max="255" />
              <label class="spawn-field-box">Y2</label>
              <input type="number" id="spawn-y2" min="0" max="255" />
              <label class="spawn-field-count">Cantidad</label>
              <input type="number" id="spawn-count" min="0" max="10000" />
              <label class="spawn-field-value">Value</label>
              <input type="number" id="spawn-value" min="0" max="255" />
              <label>Dir</label>
              <input type="number" id="spawn-dir" min="-1" max="7" />
              <div class="spawn-actions">
                <button type="button" id="update-spawn">Actualizar</button>
                <button type="button" id="delete-spawn" class="link-button">Eliminar</button>
              </div>
            </div>
          </div>
        </div>
      `;
      if (lockMap) {
        root.querySelectorAll('.spawn-map-picker').forEach((el) => { el.style.display = 'none'; });
        $('#spawn-map').disabled = true;
      }
    }

    function populateSelectors() {
      const mapSelect = $('#map-select');
      mapSelect.innerHTML = '';
      state.maps.forEach((map) => {
        const option = document.createElement('option');
        option.value = map.id;
        option.textContent = `${map.id} - ${map.name}`;
        mapSelect.appendChild(option);
      });
      mapSelect.value = String(state.selectedMap);

      const typeSelect = $('#type-select');
      typeSelect.innerHTML = '';
      types.forEach((meta) => {
        const option = document.createElement('option');
        option.value = meta.type;
        option.textContent = t(meta.label);
        typeSelect.appendChild(option);
      });
      typeSelect.value = String(state.selectedType);

      const monsterSelect = $('#monster-select');
      monsterSelect.innerHTML = '';
      state.monsters.forEach((monster) => {
        const option = document.createElement('option');
        option.value = monster.id;
        option.textContent = `[${monster.id}] ${monster.name}`;
        monsterSelect.appendChild(option);
      });
    }

    const visibleEntries = () => state.entries.filter((entry) => entry.map === state.selectedMap && entry.type === state.selectedType);

    function renderSpawnList() {
      const list = $('#spawn-list');
      list.innerHTML = '';
      visibleEntries().forEach((entry) => {
        const item = document.createElement('li');
        item.textContent = `${getMonsterName(entry.monsterId)} (${entry.x},${entry.y})`;
        item.dataset.id = String(entry.id);
        if (state.selectedEntry && state.selectedEntry.id === entry.id) {
          item.classList.add('selected');
        }
        list.appendChild(item);
      });
    }

    function updateFormVisibility() {
      const meta = TYPE_META.find((m) => m.type === state.selectedType);
      const isBox = meta?.mode === 'box' || meta?.mode === 'box-value';
      const hasValue = meta?.mode === 'box-value';
      root.querySelectorAll('.spawn-field-box, #spawn-x2, #spawn-y2').forEach((el) => {
        el.style.display = isBox ? 'block' : 'none';
      });
      root.querySelectorAll('.spawn-field-count, #spawn-count').forEach((el) => {
        el.style.display = isBox ? 'block' : 'none';
      });
      root.querySelectorAll('.spawn-field-value, #spawn-value').forEach((el) => {
        el.style.display = hasValue ? 'block' : 'none';
      });
    }

    function getTerrain(mapId) {
      return terrainCache.get(mapId) || null;
    }

    function drawMap() {
      const canvas = $('#map-canvas');
      const terrain = getTerrain(state.selectedMap);
      const width = terrain?.width || 256;
      const height = terrain?.height || 256;
      canvas.width = width * canvasScale;
      canvas.height = height * canvasScale;
      const ctx = canvas.getContext('2d');
      if (!ctx) return;
      ctx.imageSmoothingEnabled = false;

      const offscreen = document.createElement('canvas');
      offscreen.width = width;
      offscreen.height = height;
      const offCtx = offscreen.getContext('2d');
      if (!offCtx) return;
      const imgData = offCtx.createImageData(width, height);

      for (let i = 0; i < width * height; i++) {
        const attr = terrain ? terrain.data[i] || 0 : 0;
        let r = 20;
        let g = 28;
        let b = 36;
        if (attr & 0x04) {
          r = 8; g = 10; b = 12;
        } else if (attr & 0x01) {
          r = 20; g = 60; b = 30;
        } else if (attr & 0x10) {
          r = 20; g = 40; b = 80;
        } else if (attr & 0x08) {
          r = 80; g = 60; b = 20;
        }
        const idx = i * 4;
        imgData.data[idx] = r;
        imgData.data[idx + 1] = g;
        imgData.data[idx + 2] = b;
        imgData.data[idx + 3] = 255;
      }

      offCtx.putImageData(imgData, 0, 0);
      ctx.drawImage(offscreen, 0, 0, canvas.width, canvas.height);

      for (const entry of visibleEntries()) {
        const isSelected = state.selectedEntry && state.selectedEntry.id === entry.id;
        ctx.strokeStyle = isSelected ? '#7dd3fc' : '#f87171';
        ctx.lineWidth = 1;

        if (isBoxType(state.selectedType)) {
          const x = Math.min(entry.x, entry.x2);
          const y = Math.min(entry.y, entry.y2);
          const w = Math.abs(entry.x2 - entry.x) + 1;
          const h = Math.abs(entry.y2 - entry.y) + 1;
          ctx.strokeRect(x * canvasScale, y * canvasScale, w * canvasScale, h * canvasScale);
        } else if (state.selectedType === 2) {
          ctx.strokeRect((entry.x - 3) * canvasScale, (entry.y - 3) * canvasScale, 7 * canvasScale, 7 * canvasScale);
        } else {
          ctx.fillStyle = isSelected ? '#7dd3fc' : '#f87171';
          ctx.fillRect(entry.x * canvasScale, entry.y * canvasScale, canvasScale, canvasScale);
        }
      }

      if (state.dragging && state.dragStart && state.dragEnd && isBoxType(state.selectedType)) {
        const x = Math.min(state.dragStart.x, state.dragEnd.x);
        const y = Math.min(state.dragStart.y, state.dragEnd.y);
        const w = Math.abs(state.dragEnd.x - state.dragStart.x) + 1;
        const h = Math.abs(state.dragEnd.y - state.dragStart.y) + 1;
        ctx.strokeStyle = '#7dd3fc';
        ctx.lineWidth = 1;
        ctx.setLineDash([4, 2]);
        ctx.strokeRect(x * canvasScale, y * canvasScale, w * canvasScale, h * canvasScale);
        ctx.setLineDash([]);
      }
    }

    function selectEntry(entry) {
      state.selectedEntry = entry;
      if (!entry) {
        renderSpawnList();
        drawMap();
        return;
      }
      $('#monster-select').value = String(entry.monsterId);
      $('#spawn-map').value = entry.map;
      $('#spawn-dis').value = entry.dis ?? 0;
      $('#spawn-x').value = entry.x ?? 0;
      $('#spawn-y').value = entry.y ?? 0;
      $('#spawn-dir').value = entry.dir ?? 0;
      $('#spawn-x2').value = entry.x2 ?? entry.x ?? 0;
      $('#spawn-y2').value = entry.y2 ?? entry.y ?? 0;
      $('#spawn-count').value = entry.count ?? 1;
      $('#spawn-value').value = entry.value ?? 0;
      renderSpawnList();
      drawMap();
    }

    function getMapCoordsFromEvent(event, canvas) {
      const rect = canvas.getBoundingClientRect();
      const terrain = getTerrain(state.selectedMap);
      const mapW = terrain?.width || 256;
      const mapH = terrain?.height || 256;
      const x = Math.floor(((event.clientX - rect.left) / rect.width) * mapW);
      const y = Math.floor(((event.clientY - rect.top) / rect.height) * mapH);
      return { x, y };
    }

    function getEntryAt(x, y) {
      for (const entry of visibleEntries()) {
        if (isBoxType(entry.type)) {
          const x1 = Math.min(entry.x, entry.x2);
          const y1 = Math.min(entry.y, entry.y2);
          const x2 = Math.max(entry.x, entry.x2);
          const y2 = Math.max(entry.y, entry.y2);
          if (x >= x1 && x <= x2 && y >= y1 && y <= y2) return entry;
        } else if (entry.type === 2) {
          if (x >= entry.x - 3 && x <= entry.x + 3 && y >= entry.y - 3 && y <= entry.y + 3) return entry;
        } else if (entry.x === x && entry.y === y) {
          return entry;
        }
      }
      return null;
    }

    function createEntryAt(x, y) {
      const meta = TYPE_META.find((m) => m.type === state.selectedType);
      if (!meta) return;
      const entry = {
        id: Date.now() + Math.floor(Math.random() * 1000),
        type: state.selectedType,
        monsterId: state.monsters[0]?.id || 0,
        map: state.selectedMap,
        dis: 0,
        x,
        y,
        dir: -1
      };
      if (meta.mode === 'box' || meta.mode === 'box-value') {
        entry.x2 = Math.min(255, x + 5);
        entry.y2 = Math.min(255, y + 5);
        entry.count = 1;
      }
      if (meta.mode === 'box-value') {
        entry.value = 0;
      }
      state.entries.push(entry);
      selectEntry(entry);
      changed();
    }

    async function loadTerrain(mapId) {
      if (!terrainCache.has(mapId)) {
        try {
          const res = await fetch(`/admin/server-editor/api/terrain?map=${mapId}`);
          const data = await res.json();
          if (!res.ok) throw new Error(data.error || t('No se pudo cargar'));
          terrainCache.set(mapId, decodeTerrain(data.base64));
        } catch (err) {
          setMessage(err.message, 'error');
        }
      }
      drawMap();
    }

    function showType(type) {
      state.selectedType = type;
      $('#type-select').value = String(type);
      updateFormVisibility();
      renderSpawnList();
      drawMap();
    }

    function bindEvents() {
      $('#map-select').addEventListener('change', (event) => {
        state.selectedMap = Number(event.target.value);
        loadTerrain(state.selectedMap);
        renderSpawnList();
      });

      $('#type-select').addEventListener('change', (event) => showType(Number(event.target.value)));

      $('#zoom-range').addEventListener('input', (event) => {
        canvasScale = Number(event.target.value) || 2;
        drawMap();
      });

      $('#add-spawn').addEventListener('click', () => {
        state.placeMode = true;
        setMessage(t('Selecciona una posicion en el mapa para crear el spawn.'), 'success');
      });

      $('#resize-area').addEventListener('click', () => {
        if (!state.selectedEntry || !isBoxType(state.selectedType)) {
          setMessage(t('Selecciona un spawn de area (Spots/Invasiones) para ajustar.'), 'error');
          return;
        }
        state.resizeMode = true;
        setMessage(t('Arrastra en el mapa para definir el area.'), 'success');
      });

      $('#update-spawn').addEventListener('click', () => {
        if (!state.selectedEntry) {
          setMessage(t('Selecciona un spawn.'), 'error');
          return;
        }
        const validation = validateEntryForm();
        if (!validation.ok) {
          setMessage(t('Hay campos inválidos. Revisar resaltados.'), 'error');
          return;
        }
        const entry = state.selectedEntry;
        const formEntry = validation.entry;
        entry.monsterId = formEntry.monsterId;
        entry.map = formEntry.map;
        entry.dis = formEntry.dis;
        entry.x = formEntry.x;
        entry.y = formEntry.y;
        entry.dir = formEntry.dir;
        if (isBoxType(entry.type)) {
          entry.x2 = formEntry.x2;
          entry.y2 = formEntry.y2;
          entry.count = formEntry.count;
        }
        if (entry.type === 3) {
          entry.value = formEntry.value;
        }
        if (state.selectedMap !== entry.map) {
          state.selectedMap = entry.map;
          $('#map-select').value = String(state.selectedMap);
          loadTerrain(state.selectedMap);
        }
        renderSpawnList();
        drawMap();
        changed();
        setMessage(t('Spawn actualizado.'), 'success');
      });

      $('#delete-spawn').addEventListener('click', () => {
        if (!state.selectedEntry) return;
        state.entries.splice(state.entries.indexOf(state.selectedEntry), 1);
        selectEntry(null);
        changed();
      });

      $('#spawn-list').addEventListener('click', (event) => {
        const item = event.target.closest('li');
        if (!item) return;
        const entry = state.entries.find((e) => e.id === Number(item.dataset.id));
        if (entry) selectEntry(entry);
      });

      const canvas = $('#map-canvas');
      const coordLabel = $('.spawn-coords');
      canvas.addEventListener('mousedown', (event) => {
        const { x, y } = getMapCoordsFromEvent(event, canvas);
        if ((state.resizeMode || event.shiftKey) && state.selectedEntry && isBoxType(state.selectedType)) {
          state.dragging = true;
          state.dragStart = { x, y };
          state.dragEnd = { x, y };
          drawMap();
          return;
        }
        if (state.placeMode) {
          state.placeMode = false;
          createEntryAt(x, y);
          setMessage(t('Spawn creado.'), 'success');
          return;
        }
        const entry = getEntryAt(x, y);
        if (entry) selectEntry(entry);
      });

      canvas.addEventListener('mousemove', (event) => {
        const { x, y } = getMapCoordsFromEvent(event, canvas);
        if (state.dragging) {
          state.dragEnd = { x, y };
          drawMap();
        }
        const terrain = getTerrain(state.selectedMap);
        const maxX = (terrain?.width || 256) - 1;
        const maxY = (terrain?.height || 256) - 1;
        if (x < 0 || y < 0 || x > maxX || y > maxY) {
          coordLabel.textContent = 'X: -- Y: --';
          return;
        }
        let attrText = 'Libre';
        if (terrain && terrain.data) {
          attrText = getAttrLabel(terrain.data[y * (terrain.width || 256) + x] || 0);
        }
        coordLabel.textContent = `X: ${x} Y: ${y} | ${attrText}`;
      });

      canvas.addEventListener('mouseup', () => {
        if (!state.dragging || !state.selectedEntry) return;
        const terrain = getTerrain(state.selectedMap);
        const maxX = (terrain?.width || 256) - 1;
        const maxY = (terrain?.height || 256) - 1;
        const start = state.dragStart || { x: state.selectedEntry.x, y: state.selectedEntry.y };
        const end = state.dragEnd || start;
        const x1 = clampCoord(Math.min(start.x, end.x), maxX);
        const y1 = clampCoord(Math.min(start.y, end.y), maxY);
        const x2 = clampCoord(Math.max(start.x, end.x), maxX);
        const y2 = clampCoord(Math.max(start.y, end.y), maxY);

        Object.assign(state.selectedEntry, { x: x1, y: y1, x2, y2, map: state.selectedMap });
        $('#spawn-x').value = x1;
        $('#spawn-y').value = y1;
        $('#spawn-x2').value = x2;
        $('#spawn-y2').value = y2;

        state.dragging = false;
        state.dragStart = null;
        state.dragEnd = null;
        state.resizeMode = false;
        renderSpawnList();
        drawMap();
        changed();
        setMessage(t('Area ajustada.'), 'success');
      });
    }

    buildLayout();
    populateSelectors();
    bindEvents();
    updateFormVisibility();
    renderSpawnList();
    loadTerrain(state.selectedMap);

    return {
      // Botones propios de la página, al lado de "Agregar spawn".
      toolbar: $('.spawn-toolbar-actions'),

      getEntries: () => state.entries,

      setEntries(entries) {
        state.entries = entries;
        selectEntry(null);
      },

      // El primer spawn inválido, ya seleccionado en el mapa, o null.
      findInvalid() {
        const entry = state.entries.find((e) => !isEntryValid(e));
        if (!entry) return null;
        if (entry.map !== state.selectedMap) {
          state.selectedMap = entry.map;
          $('#map-select').value = String(entry.map);
          loadTerrain(entry.map);
        }
        showType(entry.type);
        selectEntry(entry);
        setMessage(t('Hay spawns con valores inválidos. Revisar el detalle antes de guardar.'), 'error');
        return entry;
      }
    };
  }

  window.MuSpawnVisual = { TYPE_META, create };
})();
