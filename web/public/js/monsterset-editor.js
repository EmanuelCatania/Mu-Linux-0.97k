(function () {
  const root = document.getElementById('monsterset-editor');
  if (!root) return;

  const TYPE_META = [
    { type: 0, label: 'NPCs', mode: 'point' },
    { type: 1, label: 'Spots', mode: 'box' },
    { type: 2, label: 'Monsters', mode: 'point-random' },
    { type: 3, label: 'Invasiones', mode: 'box-value' },
    { type: 4, label: 'Eventos', mode: 'point' }
  ];

  const state = {
    maps: [],
    monsters: [],
    entries: [],
    selectedMap: 0,
    selectedType: 0,
    selectedEntry: null,
    terrainCache: new Map(),
    placeMode: false,
    resizeMode: false,
    dragging: false,
    dragStart: null,
    dragEnd: null,
    lastClick: { x: 0, y: 0 }
  };

  let canvasScale = 2;

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

  function setFieldInvalid(el, ok) {
    if (!el) return ok;
    const wrapper = el.closest('.field') || el;
    wrapper.classList.toggle('invalid', !ok);
    if (wrapper !== el) {
      el.classList.toggle('invalid', !ok);
    }
    return ok;
  }

  function toNumber(value) {
    const raw = String(value ?? '').trim();
    if (!raw) return NaN;
    const num = Number(raw);
    return Number.isFinite(num) ? num : NaN;
  }

  function isInRange(value, min, max) {
    return Number.isFinite(value) && value >= min && value <= max;
  }

  function monsterExists(monsterId) {
    return state.monsters.some((monster) => monster.id === monsterId);
  }

  function isEntryValid(entry) {
    if (!monsterExists(entry.monsterId)) return false;
    if (!isInRange(entry.map, 0, 255)) return false;
    if (!isInRange(entry.dis, 0, 255)) return false;
    if (!isInRange(entry.x, 0, 255)) return false;
    if (!isInRange(entry.y, 0, 255)) return false;
    if (!isInRange(entry.dir, -1, 7)) return false;
    if (entry.type === 1 || entry.type === 3) {
      if (!isInRange(entry.x2, 0, 255)) return false;
      if (!isInRange(entry.y2, 0, 255)) return false;
      if (!isInRange(entry.count, 1, 255)) return false;
      if (entry.x2 < entry.x || entry.y2 < entry.y) return false;
    }
    if (entry.type === 3) {
      if (!isInRange(entry.value, 0, 255)) return false;
    }
    return true;
  }

  function validateEntryForm() {
    const monsterInput = root.querySelector('#monster-select');
    const mapInput = root.querySelector('#spawn-map');
    const disInput = root.querySelector('#spawn-dis');
    const xInput = root.querySelector('#spawn-x');
    const yInput = root.querySelector('#spawn-y');
    const x2Input = root.querySelector('#spawn-x2');
    const y2Input = root.querySelector('#spawn-y2');
    const countInput = root.querySelector('#spawn-count');
    const valueInput = root.querySelector('#spawn-value');
    const dirInput = root.querySelector('#spawn-dir');

    const monsterId = toNumber(monsterInput?.value);
    const map = toNumber(mapInput?.value);
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
    if (state.selectedType === 1 || state.selectedType === 3) {
      okX2 = setFieldInvalid(x2Input, isInRange(x2, 0, 255));
      okY2 = setFieldInvalid(y2Input, isInRange(y2, 0, 255));
      okCount = setFieldInvalid(countInput, isInRange(count, 1, 255));
      if (okX2 && okY2 && okX && okY) {
        const okRange = x2 >= x && y2 >= y;
        setFieldInvalid(x2Input, okRange);
        setFieldInvalid(y2Input, okRange);
        okX2 = okX2 && okRange;
        okY2 = okY2 && okRange;
      }
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

    const ok =
      okMonster &&
      okMap &&
      okDis &&
      okX &&
      okY &&
      okDir &&
      okX2 &&
      okY2 &&
      okCount &&
      okValue;

    return {
      ok,
      entry: {
        monsterId,
        map,
        dis,
        x,
        y,
        x2,
        y2,
        count,
        value,
        dir
      }
    };
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

  function parseMonsterSet(content) {
    const lines = String(content || '').split(/\r?\n/);
    const entries = [];
    let section = null;
    let nextId = 1;
    for (const rawLine of lines) {
      const line = rawLine.trim();
      if (!line || line.startsWith('//')) continue;
      if (line.toLowerCase() === 'end') {
        section = null;
        continue;
      }
      if (section === null && /^-?\d+$/.test(line)) {
        section = Number(line);
        continue;
      }
      if (section === null) continue;
      const nums = rawLine.match(/-?\d+/g);
      if (!nums) continue;
      const values = nums.map((n) => Number(n));
      let entry = null;
      if (section === 0 || section === 4 || section === 2) {
        if (values.length < 6) continue;
        entry = {
          id: nextId++,
          type: section,
          monsterId: values[0],
          map: values[1],
          dis: values[2],
          x: values[3],
          y: values[4],
          dir: values[5]
        };
      } else if (section === 1) {
        if (values.length < 9) continue;
        entry = {
          id: nextId++,
          type: section,
          monsterId: values[0],
          map: values[1],
          dis: values[2],
          x: values[3],
          y: values[4],
          x2: values[5],
          y2: values[6],
          dir: values[7],
          count: values[8]
        };
      } else if (section === 3) {
        if (values.length < 10) continue;
        entry = {
          id: nextId++,
          type: section,
          monsterId: values[0],
          map: values[1],
          dis: values[2],
          x: values[3],
          y: values[4],
          x2: values[5],
          y2: values[6],
          dir: values[7],
          count: values[8],
          value: values[9]
        };
      }
      if (entry) entries.push(entry);
    }
    return entries;
  }

  function getMonsterName(monsterId) {
    const found = state.monsters.find((m) => m.id === monsterId);
    return found ? found.name : `Monster ${monsterId}`;
  }

  function getMapName(mapId) {
    const found = state.maps.find((m) => m.id === mapId);
    return found ? found.name : `Mapa ${mapId}`;
  }

  function buildMonsterSet() {
    const lines = [];
    lines.push('//======================================//');
    lines.push('//\t\tMONSTER SET BASE\t\t//');
    lines.push('//======================================//');
    lines.push('');

    for (const meta of TYPE_META) {
      const type = meta.type;
      lines.push('//========================================================================================================================================='); 
      lines.push(`// ${meta.label}`);
      lines.push('//=========================================================================================================================================');

      const byMap = state.entries
        .filter((entry) => entry.type === type)
        .reduce((acc, entry) => {
          acc[entry.map] = acc[entry.map] || [];
          acc[entry.map].push(entry);
          return acc;
        }, {});

      const mapIds = Object.keys(byMap)
        .map((n) => Number(n))
        .sort((a, b) => a - b);

      if (mapIds.length === 0) {
        lines.push(String(type));
        lines.push('end');
        lines.push('');
        continue;
      }

      for (const mapId of mapIds) {
        lines.push(`// ${getMapName(mapId)}`);
        lines.push(String(type));
        const entries = byMap[mapId];
        for (const entry of entries) {
          const name = getMonsterName(entry.monsterId);
          if (type === 0 || type === 2 || type === 4) {
            lines.push(`${entry.monsterId}\t${entry.map}\t${entry.dis}\t${entry.x}\t${entry.y}\t${entry.dir}\t// ${name}`);
          } else if (type === 1) {
            lines.push(`${entry.monsterId}\t${entry.map}\t${entry.dis}\t${entry.x}\t${entry.y}\t${entry.x2}\t${entry.y2}\t${entry.dir}\t${entry.count}\t// ${name}`);
          } else if (type === 3) {
            lines.push(`${entry.monsterId}\t${entry.map}\t${entry.dis}\t${entry.x}\t${entry.y}\t${entry.x2}\t${entry.y2}\t${entry.dir}\t${entry.count}\t${entry.value}\t// ${name}`);
          }
        }
        lines.push('end');
        lines.push('');
      }
    }

    return lines.join('\n');
  }

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Mapa</label>
          <select id="map-select"></select>
          <label>Tipo</label>
          <select id="type-select"></select>
          <label>Zoom</label>
          <input type="range" id="zoom-range" min="1" max="3" step="1" value="2" />
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="add-spawn">Agregar spawn</button>
          <button type="button" id="resize-area">Ajustar area</button>
          <button type="button" id="save-spawns">Guardar</button>
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
            <input type="number" id="spawn-count" min="1" max="255" />
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
  }

  function populateSelectors() {
    const mapSelect = root.querySelector('#map-select');
    const typeSelect = root.querySelector('#type-select');
    const monsterSelect = root.querySelector('#monster-select');
    if (mapSelect) {
      mapSelect.innerHTML = '';
      state.maps.forEach((map) => {
        const option = document.createElement('option');
        option.value = map.id;
        option.textContent = `${map.id} - ${map.name}`;
        mapSelect.appendChild(option);
      });
      mapSelect.value = String(state.selectedMap);
    }
    if (typeSelect) {
      typeSelect.innerHTML = '';
      TYPE_META.forEach((meta) => {
        const option = document.createElement('option');
        option.value = meta.type;
        option.textContent = meta.label;
        typeSelect.appendChild(option);
      });
      typeSelect.value = String(state.selectedType);
    }
    if (monsterSelect) {
      monsterSelect.innerHTML = '';
      state.monsters.forEach((monster) => {
        const option = document.createElement('option');
        option.value = monster.id;
        option.textContent = `[${monster.id}] ${monster.name}`;
        monsterSelect.appendChild(option);
      });
    }
  }

  function renderSpawnList() {
    const list = root.querySelector('#spawn-list');
    if (!list) return;
    list.innerHTML = '';
    const entries = state.entries.filter((entry) => entry.map === state.selectedMap && entry.type === state.selectedType);
    entries.forEach((entry) => {
      const item = document.createElement('li');
      const name = getMonsterName(entry.monsterId);
      item.textContent = `${name} (${entry.x},${entry.y})`;
      item.dataset.id = String(entry.id);
      if (state.selectedEntry && state.selectedEntry.id === entry.id) {
        item.classList.add('selected');
      }
      list.appendChild(item);
    });
  }

  function isBoxType(type) {
    return type === 1 || type === 3;
  }

  function updateFormVisibility() {
    const meta = TYPE_META.find((t) => t.type === state.selectedType);
    const isBox = meta?.mode === 'box' || meta?.mode === 'box-value';
    const hasCount = meta?.mode === 'box' || meta?.mode === 'box-value';
    const hasValue = meta?.mode === 'box-value';
    root.querySelectorAll('.spawn-field-box').forEach((el) => {
      el.style.display = isBox ? 'block' : 'none';
    });
    root.querySelectorAll('#spawn-x2, #spawn-y2').forEach((el) => {
      el.style.display = isBox ? 'block' : 'none';
    });
    root.querySelectorAll('.spawn-field-count, #spawn-count').forEach((el) => {
      el.style.display = hasCount ? 'block' : 'none';
    });
    root.querySelectorAll('.spawn-field-value, #spawn-value').forEach((el) => {
      el.style.display = hasValue ? 'block' : 'none';
    });
  }

  function selectEntry(entry) {
    state.selectedEntry = entry;
    if (!entry) {
      renderSpawnList();
      return;
    }
    root.querySelector('#monster-select').value = String(entry.monsterId);
    root.querySelector('#spawn-map').value = entry.map;
    root.querySelector('#spawn-dis').value = entry.dis ?? 0;
    root.querySelector('#spawn-x').value = entry.x ?? 0;
    root.querySelector('#spawn-y').value = entry.y ?? 0;
    root.querySelector('#spawn-dir').value = entry.dir ?? 0;
    root.querySelector('#spawn-x2').value = entry.x2 ?? entry.x ?? 0;
    root.querySelector('#spawn-y2').value = entry.y2 ?? entry.y ?? 0;
    root.querySelector('#spawn-count').value = entry.count ?? 1;
    root.querySelector('#spawn-value').value = entry.value ?? 0;
    renderSpawnList();
    drawMap();
  }

  function clampCoord(value, max) {
    if (Number.isNaN(value)) return 0;
    if (value < 0) return 0;
    if (value > max) return max;
    return value;
  }

  function getAttrLabel(attr) {
    const labels = [];
    if (attr & 0x04) labels.push('Bloqueado');
    if (attr & 0x01) labels.push('Safe');
    if (attr & 0x10) labels.push('Agua');
    if (attr & 0x08) labels.push('Hueco');
    if (labels.length === 0) return 'Libre';
    return labels.join(', ');
  }

  function getTerrain(mapId) {
    return state.terrainCache.get(mapId) || null;
  }

  function drawMap() {
    const canvas = root.querySelector('#map-canvas');
    if (!canvas) return;
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

    if (terrain) {
      for (let i = 0; i < width * height; i++) {
        const attr = terrain.data[i] || 0;
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
    } else {
      for (let i = 0; i < width * height; i++) {
        const idx = i * 4;
        imgData.data[idx] = 20;
        imgData.data[idx + 1] = 28;
        imgData.data[idx + 2] = 36;
        imgData.data[idx + 3] = 255;
      }
    }

    offCtx.putImageData(imgData, 0, 0);
    ctx.drawImage(offscreen, 0, 0, canvas.width, canvas.height);

    const entries = state.entries.filter((entry) => entry.map === state.selectedMap && entry.type === state.selectedType);
    for (const entry of entries) {
      const isSelected = state.selectedEntry && state.selectedEntry.id === entry.id;
      ctx.strokeStyle = isSelected ? '#7dd3fc' : '#f87171';
      ctx.lineWidth = 1;

      if (state.selectedType === 1 || state.selectedType === 3) {
        const x = Math.min(entry.x, entry.x2);
        const y = Math.min(entry.y, entry.y2);
        const w = Math.abs(entry.x2 - entry.x) + 1;
        const h = Math.abs(entry.y2 - entry.y) + 1;
        ctx.strokeRect(x * canvasScale, y * canvasScale, w * canvasScale, h * canvasScale);
      } else if (state.selectedType === 2) {
        const x = entry.x - 3;
        const y = entry.y - 3;
        ctx.strokeRect(x * canvasScale, y * canvasScale, 7 * canvasScale, 7 * canvasScale);
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

  function getMapCoordsFromEvent(event, canvas) {
    const rect = canvas.getBoundingClientRect();
    const terrain = getTerrain(state.selectedMap);
    const mapW = terrain?.width || 256;
    const mapH = terrain?.height || 256;
    const relX = (event.clientX - rect.left) / rect.width;
    const relY = (event.clientY - rect.top) / rect.height;
    const x = Math.floor(relX * mapW);
    const y = Math.floor(relY * mapH);
    return { x, y };
  }

  function getEntryAt(x, y) {
    const entries = state.entries.filter((entry) => entry.map === state.selectedMap && entry.type === state.selectedType);
    for (const entry of entries) {
      if (entry.type === 1 || entry.type === 3) {
        const x1 = Math.min(entry.x, entry.x2);
        const y1 = Math.min(entry.y, entry.y2);
        const x2 = Math.max(entry.x, entry.x2);
        const y2 = Math.max(entry.y, entry.y2);
        if (x >= x1 && x <= x2 && y >= y1 && y <= y2) return entry;
      } else if (entry.type === 2) {
        const x1 = entry.x - 3;
        const y1 = entry.y - 3;
        const x2 = entry.x + 3;
        const y2 = entry.y + 3;
        if (x >= x1 && x <= x2 && y >= y1 && y <= y2) return entry;
      } else {
        if (entry.x === x && entry.y === y) return entry;
      }
    }
    return null;
  }

  function createEntryAt(x, y) {
    const meta = TYPE_META.find((t) => t.type === state.selectedType);
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
    renderSpawnList();
    drawMap();
  }

  function bindEvents() {
    root.querySelector('#map-select').addEventListener('change', (event) => {
      state.selectedMap = Number(event.target.value);
      loadTerrain(state.selectedMap);
      renderSpawnList();
      drawMap();
    });

    root.querySelector('#type-select').addEventListener('change', (event) => {
      state.selectedType = Number(event.target.value);
      updateFormVisibility();
      renderSpawnList();
      drawMap();
    });

    root.querySelector('#zoom-range').addEventListener('input', (event) => {
      canvasScale = Number(event.target.value) || 2;
      drawMap();
    });

    root.querySelector('#add-spawn').addEventListener('click', () => {
      state.placeMode = true;
      setMessage('Selecciona una posicion en el mapa para crear el spawn.', 'success');
    });

    root.querySelector('#resize-area').addEventListener('click', () => {
      if (!state.selectedEntry || !isBoxType(state.selectedType)) {
        setMessage('Selecciona un spawn de area (Spots/Invasiones) para ajustar.', 'error');
        return;
      }
      state.resizeMode = true;
      setMessage('Arrastra en el mapa para definir el area.', 'success');
    });

    root.querySelector('#save-spawns').addEventListener('click', async () => {
      try {
        const invalidEntry = state.entries.find((entry) => !isEntryValid(entry));
        if (invalidEntry) {
          state.selectedMap = invalidEntry.map;
          state.selectedType = invalidEntry.type;
          root.querySelector('#map-select').value = String(state.selectedMap);
          root.querySelector('#type-select').value = String(state.selectedType);
          updateFormVisibility();
          renderSpawnList();
          await loadTerrain(state.selectedMap);
          selectEntry(invalidEntry);
          setMessage('Hay spawns con valores inválidos. Revisar el detalle antes de guardar.', 'error');
          return;
        }
        const content = buildMonsterSet();
        const res = await fetch('/admin/server-editor/api/monster-set', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ content })
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'No se pudo guardar');
        const reload = await fetch('/admin/server-editor/api/reload', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ target: 'monster' })
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

    root.querySelector('#update-spawn').addEventListener('click', () => {
      if (!state.selectedEntry) {
        setMessage('Selecciona un spawn.', 'error');
        return;
      }
      const validation = validateEntryForm();
      if (!validation.ok) {
        setMessage('Hay campos inválidos. Revisar resaltados.', 'error');
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
      if (entry.type === 1 || entry.type === 3) {
        entry.x2 = formEntry.x2;
        entry.y2 = formEntry.y2;
        entry.count = formEntry.count;
      }
      if (entry.type === 3) {
        entry.value = formEntry.value;
      }
      state.selectedMap = entry.map;
      root.querySelector('#map-select').value = String(state.selectedMap);
      renderSpawnList();
      drawMap();
      setMessage('Spawn actualizado.', 'success');
    });

    root.querySelector('#delete-spawn').addEventListener('click', () => {
      if (!state.selectedEntry) return;
      state.entries = state.entries.filter((entry) => entry.id !== state.selectedEntry.id);
      state.selectedEntry = null;
      renderSpawnList();
      drawMap();
    });

    root.querySelector('#spawn-list').addEventListener('click', (event) => {
      const item = event.target.closest('li');
      if (!item) return;
      const id = Number(item.dataset.id);
      const entry = state.entries.find((e) => e.id === id);
      if (entry) selectEntry(entry);
    });

    const canvas = root.querySelector('#map-canvas');
    const coordLabel = root.querySelector('.spawn-coords');
    canvas.addEventListener('mousedown', (event) => {
      const { x, y } = getMapCoordsFromEvent(event, canvas);
      state.lastClick = { x, y };
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
        setMessage('Spawn creado.', 'success');
        return;
      }
      const entry = getEntryAt(x, y);
      if (entry) {
        selectEntry(entry);
      }
    });

    canvas.addEventListener('mousemove', (event) => {
      if (!state.dragging) return;
      const { x, y } = getMapCoordsFromEvent(event, canvas);
      state.dragEnd = { x, y };
      drawMap();
    });

    canvas.addEventListener('mousemove', (event) => {
      if (!coordLabel) return;
      const { x, y } = getMapCoordsFromEvent(event, canvas);
      const terrain = getTerrain(state.selectedMap);
      const maxX = (terrain?.width || 256) - 1;
      const maxY = (terrain?.height || 256) - 1;
      if (x < 0 || y < 0 || x > maxX || y > maxY) {
        coordLabel.textContent = 'X: -- Y: --';
        return;
      }
      let attrText = 'Libre';
      if (terrain && terrain.data) {
        const index = y * (terrain.width || 256) + x;
        const attr = terrain.data[index] || 0;
        attrText = getAttrLabel(attr);
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

      state.selectedEntry.x = x1;
      state.selectedEntry.y = y1;
      state.selectedEntry.x2 = x2;
      state.selectedEntry.y2 = y2;
      state.selectedEntry.map = state.selectedMap;

      root.querySelector('#spawn-x').value = x1;
      root.querySelector('#spawn-y').value = y1;
      root.querySelector('#spawn-x2').value = x2;
      root.querySelector('#spawn-y2').value = y2;

      state.dragging = false;
      state.dragStart = null;
      state.dragEnd = null;
      state.resizeMode = false;
      renderSpawnList();
      drawMap();
      setMessage('Area ajustada.', 'success');
    });
  }

  async function loadTerrain(mapId) {
    if (state.terrainCache.has(mapId)) {
      drawMap();
      return;
    }
    try {
      const res = await fetch(`/admin/server-editor/api/terrain?map=${mapId}`);
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudo cargar');
      const terrain = decodeTerrain(data.base64);
      state.terrainCache.set(mapId, terrain);
      drawMap();
    } catch (err) {
      setMessage(err.message, 'error');
      drawMap();
    }
  }

  async function loadInitialData() {
    const [mapsRes, monstersRes, setRes] = await Promise.all([
      fetch('/admin/server-editor/api/map-defs'),
      fetch('/admin/server-editor/api/monster-defs'),
      fetch('/admin/server-editor/api/monster-set')
    ]);

    const mapsData = await mapsRes.json();
    const monstersData = await monstersRes.json();
    const setData = await setRes.json();

    if (!mapsRes.ok) throw new Error(mapsData.error || 'No se pudo cargar mapas');
    if (!monstersRes.ok) throw new Error(monstersData.error || 'No se pudo cargar monstruos');
    if (!setRes.ok) throw new Error(setData.error || 'No se pudo cargar MonsterSetBase');

    state.maps = Array.isArray(mapsData.maps) ? mapsData.maps : [];
    state.monsters = Array.isArray(monstersData.monsters) ? monstersData.monsters : [];
    state.entries = parseMonsterSet(setData.content || '');
    state.selectedMap = state.maps[0]?.id ?? 0;
    state.selectedType = TYPE_META[0].type;
  }

  async function init() {
    buildLayout();
    bindEvents();
    try {
      await loadInitialData();
      populateSelectors();
      updateFormVisibility();
      renderSpawnList();
      await loadTerrain(state.selectedMap);
      drawMap();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
