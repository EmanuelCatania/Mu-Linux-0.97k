(function () {
  const root = document.getElementById('gate-editor');
  if (!root) return;

  const FLAG_LABELS = {
    0: 'Normal',
    1: 'Entrada',
    2: 'Salida'
  };

  const state = {
    maps: [],
    gates: [],
    selectedMap: 0,
    selectedGate: null,
    terrainCache: new Map(),
    placeMode: false,
    resizeMode: false,
    dragging: false,
    dragStart: null,
    dragEnd: null
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

  function isStarOrNumber(value, min, max) {
    const raw = String(value ?? '').trim();
    if (!raw || raw === '*') return true;
    const num = toNumber(raw);
    return isInRange(num, min, max);
  }

  function parseStar(value) {
    if (value === '*') return '*';
    const num = Number(value);
    return Number.isNaN(num) ? '*' : num;
  }

  function formatStar(value) {
    if (value === '*' || value === '' || value === null || value === undefined) return '*';
    return Number(value);
  }

  function isGateValid(gate) {
    if (!isInRange(gate.index, 0, 65535)) return false;
    if (!isInRange(gate.flag, 0, 2)) return false;
    if (!isInRange(gate.map, 0, 255)) return false;
    if (!isInRange(gate.x, 0, 255)) return false;
    if (!isInRange(gate.y, 0, 255)) return false;
    if (!isInRange(gate.tx, 0, 255)) return false;
    if (!isInRange(gate.ty, 0, 255)) return false;
    if (!isInRange(gate.target, 0, 65535)) return false;
    if (!isInRange(gate.dir, -1, 7)) return false;
    if (!isInRange(gate.minLvl, 0, 1000)) return false;
    if (!isInRange(gate.maxLvl, 0, 1000)) return false;
    if (gate.minLvl > gate.maxLvl) return false;
    if (!isStarOrNumber(gate.minRes, 0, 1000)) return false;
    if (!isStarOrNumber(gate.maxRes, 0, 1000)) return false;
    if (gate.minRes !== '*' && gate.maxRes !== '*' && gate.minRes > gate.maxRes) return false;
    if (!isInRange(gate.accountLvl, 0, 3)) return false;
    return true;
  }

  function validateGateForm() {
    const indexInput = root.querySelector('#gate-index');
    const flagInput = root.querySelector('#gate-flag');
    const mapInput = root.querySelector('#gate-map');
    const xInput = root.querySelector('#gate-x');
    const yInput = root.querySelector('#gate-y');
    const txInput = root.querySelector('#gate-tx');
    const tyInput = root.querySelector('#gate-ty');
    const targetInput = root.querySelector('#gate-target');
    const dirInput = root.querySelector('#gate-dir');
    const minLvlInput = root.querySelector('#gate-minlvl');
    const maxLvlInput = root.querySelector('#gate-maxlvl');
    const minResInput = root.querySelector('#gate-minres');
    const maxResInput = root.querySelector('#gate-maxres');
    const accInput = root.querySelector('#gate-acclvl');

    const index = toNumber(indexInput?.value);
    const flag = toNumber(flagInput?.value);
    const map = toNumber(mapInput?.value);
    const x = toNumber(xInput?.value);
    const y = toNumber(yInput?.value);
    const tx = toNumber(txInput?.value);
    const ty = toNumber(tyInput?.value);
    const target = toNumber(targetInput?.value);
    const dir = toNumber(dirInput?.value);
    const minLvl = toNumber(minLvlInput?.value);
    const maxLvl = toNumber(maxLvlInput?.value);
    const minResRaw = String(minResInput?.value ?? '').trim() || '*';
    const maxResRaw = String(maxResInput?.value ?? '').trim() || '*';
    const acc = toNumber(accInput?.value);

    const minRes = minResRaw === '*' ? '*' : toNumber(minResRaw);
    const maxRes = maxResRaw === '*' ? '*' : toNumber(maxResRaw);

    const okIndex = setFieldInvalid(indexInput, isInRange(index, 0, 65535));
    const okFlag = setFieldInvalid(flagInput, isInRange(flag, 0, 2));
    const okMap = setFieldInvalid(mapInput, isInRange(map, 0, 255));
    const okX = setFieldInvalid(xInput, isInRange(x, 0, 255));
    const okY = setFieldInvalid(yInput, isInRange(y, 0, 255));
    const okTX = setFieldInvalid(txInput, isInRange(tx, 0, 255));
    const okTY = setFieldInvalid(tyInput, isInRange(ty, 0, 255));
    const okTarget = setFieldInvalid(targetInput, isInRange(target, 0, 65535));
    const okDir = setFieldInvalid(dirInput, isInRange(dir, -1, 7));
    const okMinLvl = setFieldInvalid(minLvlInput, isInRange(minLvl, 0, 1000));
    const okMaxLvl = setFieldInvalid(maxLvlInput, isInRange(maxLvl, 0, 1000) && minLvl <= maxLvl);
    const okMinRes = setFieldInvalid(minResInput, isStarOrNumber(minResRaw, 0, 1000));
    const okMaxRes = setFieldInvalid(maxResInput, isStarOrNumber(maxResRaw, 0, 1000));
    const okResRange =
      minRes !== '*' &&
      maxRes !== '*' &&
      Number.isFinite(minRes) &&
      Number.isFinite(maxRes)
        ? minRes <= maxRes
        : true;
    setFieldInvalid(minResInput, okMinRes && okResRange);
    setFieldInvalid(maxResInput, okMaxRes && okResRange);
    const okAcc = setFieldInvalid(accInput, isInRange(acc, 0, 3));

    const ok =
      okIndex &&
      okFlag &&
      okMap &&
      okX &&
      okY &&
      okTX &&
      okTY &&
      okTarget &&
      okDir &&
      okMinLvl &&
      okMaxLvl &&
      okMinRes &&
      okMaxRes &&
      okResRange &&
      okAcc;

    return {
      ok,
      gate: {
        index,
        flag,
        map,
        x,
        y,
        tx,
        ty,
        target,
        dir,
        minLvl,
        maxLvl,
        minRes: minResRaw === '*' ? '*' : minRes,
        maxRes: maxResRaw === '*' ? '*' : maxRes,
        accountLvl: acc
      }
    };
  }

  function parseGateTxt(content) {
    const lines = String(content || '').split(/\r?\n/);
    const gates = [];
    for (const rawLine of lines) {
      const line = rawLine.trim();
      if (!line || line.startsWith('//')) continue;
      const parts = rawLine.split('//');
      const data = parts[0].trim();
      if (!data) continue;
      const tokens = data.match(/\S+/g);
      if (!tokens || tokens.length < 14) continue;
      gates.push({
        index: Number(tokens[0]),
        flag: Number(tokens[1]),
        map: Number(tokens[2]),
        x: Number(tokens[3]),
        y: Number(tokens[4]),
        tx: Number(tokens[5]),
        ty: Number(tokens[6]),
        target: Number(tokens[7]),
        dir: Number(tokens[8]),
        minLvl: Number(tokens[9]),
        maxLvl: Number(tokens[10]),
        minRes: parseStar(tokens[11]),
        maxRes: parseStar(tokens[12]),
        accountLvl: Number(tokens[13])
      });
    }
    return gates;
  }

  function buildGateTxt() {
    const lines = [];
    lines.push('//Index\tFlag\tMap\tX\tY\tTX\tTY\tTarget\tDir\tMinLvl\tMaxLvl\tMinRes\tMaxRes\tAccountLvl');
    const gates = state.gates.slice().sort((a, b) => a.index - b.index);
    for (const gate of gates) {
      lines.push([
        gate.index,
        gate.flag,
        gate.map,
        gate.x,
        gate.y,
        gate.tx,
        gate.ty,
        gate.target,
        gate.dir,
        gate.minLvl,
        gate.maxLvl,
        formatStar(gate.minRes),
        formatStar(gate.maxRes),
        gate.accountLvl
      ].join('\t'));
    }
    return lines.join('\n');
  }

  function getMapName(mapId) {
    const map = state.maps.find((m) => m.id === mapId);
    return map ? map.name : `Mapa ${mapId}`;
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

  function getTerrain(mapId) {
    return state.terrainCache.get(mapId) || null;
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

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Mapa</label>
          <select id="map-select"></select>
          <label>Zoom</label>
          <input type="range" id="zoom-range" min="1" max="3" step="1" value="2" />
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="add-gate">Agregar gate</button>
          <button type="button" id="resize-gate">Ajustar area</button>
          <button type="button" id="save-gates">Guardar</button>
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
            <h3>Gates</h3>
            <ul id="gate-list"></ul>
          </div>
          <div class="spawn-form">
            <h3>Detalle</h3>
            <label>Index</label>
            <input type="number" id="gate-index" min="0" max="65535" />
            <label>Flag</label>
            <select id="gate-flag">
              <option value="0">0 - Normal</option>
              <option value="1">1 - Entrada</option>
              <option value="2">2 - Salida</option>
            </select>
            <label>Map</label>
            <input type="number" id="gate-map" min="0" max="255" />
            <label>X</label>
            <input type="number" id="gate-x" min="0" max="255" />
            <label>Y</label>
            <input type="number" id="gate-y" min="0" max="255" />
            <label>TX</label>
            <input type="number" id="gate-tx" min="0" max="255" />
            <label>TY</label>
            <input type="number" id="gate-ty" min="0" max="255" />
            <label>Target</label>
            <input type="number" id="gate-target" min="0" max="65535" />
            <label>Dir</label>
            <input type="number" id="gate-dir" min="-1" max="7" />
            <label>Min Lvl</label>
            <input type="number" id="gate-minlvl" min="0" max="1000" />
            <label>Max Lvl</label>
            <input type="number" id="gate-maxlvl" min="0" max="1000" />
            <label>Min Reset</label>
            <input type="text" id="gate-minres" placeholder="*" />
            <label>Max Reset</label>
            <input type="text" id="gate-maxres" placeholder="*" />
            <label>Account Lvl</label>
            <input type="number" id="gate-acclvl" min="0" max="3" />
            <div class="spawn-actions">
              <button type="button" id="update-gate">Actualizar</button>
              <button type="button" id="delete-gate" class="link-button">Eliminar</button>
            </div>
          </div>
        </div>
      </div>
    `;
  }

  function populateSelectors() {
    const mapSelect = root.querySelector('#map-select');
    if (!mapSelect) return;
    mapSelect.innerHTML = '';
    state.maps.forEach((map) => {
      const option = document.createElement('option');
      option.value = map.id;
      option.textContent = `${map.id} - ${map.name}`;
      mapSelect.appendChild(option);
    });
    mapSelect.value = String(state.selectedMap);
  }

  function renderGateList() {
    const list = root.querySelector('#gate-list');
    if (!list) return;
    list.innerHTML = '';
    const gates = state.gates.filter((gate) => gate.map === state.selectedMap);
    gates.forEach((gate) => {
      const item = document.createElement('li');
      const label = `${gate.index} (${gate.x},${gate.y}) → (${gate.tx},${gate.ty})`;
      item.textContent = label;
      item.dataset.index = String(gate.index);
      if (state.selectedGate && state.selectedGate.index === gate.index) {
        item.classList.add('selected');
      }
      list.appendChild(item);
    });
  }

  function selectGate(gate) {
    state.selectedGate = gate;
    if (!gate) {
      renderGateList();
      return;
    }
    root.querySelector('#gate-index').value = gate.index;
    root.querySelector('#gate-flag').value = gate.flag;
    root.querySelector('#gate-map').value = gate.map;
    root.querySelector('#gate-x').value = gate.x;
    root.querySelector('#gate-y').value = gate.y;
    root.querySelector('#gate-tx').value = gate.tx;
    root.querySelector('#gate-ty').value = gate.ty;
    root.querySelector('#gate-target').value = gate.target;
    root.querySelector('#gate-dir').value = gate.dir;
    root.querySelector('#gate-minlvl').value = gate.minLvl;
    root.querySelector('#gate-maxlvl').value = gate.maxLvl;
    root.querySelector('#gate-minres').value = gate.minRes === '*' ? '*' : gate.minRes;
    root.querySelector('#gate-maxres').value = gate.maxRes === '*' ? '*' : gate.maxRes;
    root.querySelector('#gate-acclvl').value = gate.accountLvl;
    renderGateList();
    drawMap();
  }

  function getGateAt(x, y) {
    const gates = state.gates.filter((gate) => gate.map === state.selectedMap);
    for (const gate of gates) {
      const x1 = Math.min(gate.x, gate.tx);
      const y1 = Math.min(gate.y, gate.ty);
      const x2 = Math.max(gate.x, gate.tx);
      const y2 = Math.max(gate.y, gate.ty);
      if (x >= x1 && x <= x2 && y >= y1 && y <= y2) return gate;
    }
    return null;
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

    const gates = state.gates.filter((gate) => gate.map === state.selectedMap);
    for (const gate of gates) {
      const selected = state.selectedGate && state.selectedGate.index === gate.index;
      ctx.strokeStyle = selected ? '#7dd3fc' : '#facc15';
      ctx.lineWidth = 1;
      const x = Math.min(gate.x, gate.tx);
      const y = Math.min(gate.y, gate.ty);
      const w = Math.abs(gate.tx - gate.x) + 1;
      const h = Math.abs(gate.ty - gate.y) + 1;
      ctx.strokeRect(x * canvasScale, y * canvasScale, w * canvasScale, h * canvasScale);
    }

    if (state.dragging && state.dragStart && state.dragEnd) {
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

  function getAttrLabel(attr) {
    const labels = [];
    if (attr & 0x04) labels.push('Bloqueado');
    if (attr & 0x01) labels.push('Safe');
    if (attr & 0x10) labels.push('Agua');
    if (attr & 0x08) labels.push('Hueco');
    if (labels.length === 0) return 'Libre';
    return labels.join(', ');
  }

  function clampCoord(value, max) {
    if (Number.isNaN(value)) return 0;
    if (value < 0) return 0;
    if (value > max) return max;
    return value;
  }

  function bindEvents() {
    root.querySelector('#map-select').addEventListener('change', (event) => {
      state.selectedMap = Number(event.target.value);
      loadTerrain(state.selectedMap);
      renderGateList();
      drawMap();
    });

    root.querySelector('#zoom-range').addEventListener('input', (event) => {
      canvasScale = Number(event.target.value) || 2;
      drawMap();
    });

    root.querySelector('#add-gate').addEventListener('click', () => {
      state.placeMode = true;
      setMessage('Arrastra en el mapa para crear el gate.', 'success');
    });

    root.querySelector('#resize-gate').addEventListener('click', () => {
      if (!state.selectedGate) {
        setMessage('Selecciona un gate para ajustar.', 'error');
        return;
      }
      state.resizeMode = true;
      setMessage('Arrastra en el mapa para ajustar el area.', 'success');
    });

    root.querySelector('#save-gates').addEventListener('click', async () => {
      try {
        const invalidGate = state.gates.find((gate) => !isGateValid(gate));
        if (invalidGate) {
          state.selectedMap = invalidGate.map;
          root.querySelector('#map-select').value = String(state.selectedMap);
          await loadTerrain(state.selectedMap);
          selectGate(invalidGate);
          setMessage('Hay gates con valores inválidos. Revisar el detalle antes de guardar.', 'error');
          return;
        }
        const content = buildGateTxt();
        const res = await fetch('/admin/server-editor/api/gates', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ content })
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'No se pudo guardar');
        const reload = await fetch('/admin/server-editor/api/reload', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ target: 'move' })
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

    root.querySelector('#update-gate').addEventListener('click', () => {
      if (!state.selectedGate) return;
      const validation = validateGateForm();
      if (!validation.ok) {
        setMessage('Hay campos inválidos. Revisar resaltados.', 'error');
        return;
      }
      const gate = state.selectedGate;
      const formGate = validation.gate;
      gate.index = formGate.index;
      gate.flag = formGate.flag;
      gate.map = formGate.map;
      gate.x = formGate.x;
      gate.y = formGate.y;
      gate.tx = formGate.tx;
      gate.ty = formGate.ty;
      gate.target = formGate.target;
      gate.dir = formGate.dir;
      gate.minLvl = formGate.minLvl;
      gate.maxLvl = formGate.maxLvl;
      gate.minRes = formGate.minRes;
      gate.maxRes = formGate.maxRes;
      gate.accountLvl = formGate.accountLvl;
      state.selectedMap = gate.map;
      root.querySelector('#map-select').value = String(state.selectedMap);
      renderGateList();
      drawMap();
      setMessage('Gate actualizado.', 'success');
    });

    root.querySelector('#delete-gate').addEventListener('click', () => {
      if (!state.selectedGate) return;
      state.gates = state.gates.filter((g) => g.index !== state.selectedGate.index);
      state.selectedGate = null;
      renderGateList();
      drawMap();
    });

    root.querySelector('#gate-list').addEventListener('click', (event) => {
      const item = event.target.closest('li');
      if (!item) return;
      const index = Number(item.dataset.index);
      const gate = state.gates.find((g) => g.index === index);
      if (gate) selectGate(gate);
    });

    const canvas = root.querySelector('#map-canvas');
    const coordLabel = root.querySelector('.spawn-coords');

    canvas.addEventListener('mousedown', (event) => {
      const { x, y } = getMapCoordsFromEvent(event, canvas);
      if (state.placeMode || state.resizeMode || event.shiftKey) {
        state.dragging = true;
        state.dragStart = { x, y };
        state.dragEnd = { x, y };
        drawMap();
        return;
      }

      const gate = getGateAt(x, y);
      if (gate) selectGate(gate);
    });

    canvas.addEventListener('mousemove', (event) => {
      if (state.dragging) {
        const { x, y } = getMapCoordsFromEvent(event, canvas);
        state.dragEnd = { x, y };
        drawMap();
      }

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
      if (!state.dragging) return;
      const terrain = getTerrain(state.selectedMap);
      const maxX = (terrain?.width || 256) - 1;
      const maxY = (terrain?.height || 256) - 1;
      const start = state.dragStart;
      const end = state.dragEnd || start;
      const x1 = clampCoord(Math.min(start.x, end.x), maxX);
      const y1 = clampCoord(Math.min(start.y, end.y), maxY);
      const x2 = clampCoord(Math.max(start.x, end.x), maxX);
      const y2 = clampCoord(Math.max(start.y, end.y), maxY);

      if (state.placeMode) {
        const nextIndex = state.gates.reduce((max, g) => Math.max(max, g.index), 0) + 1;
        const newGate = {
          index: nextIndex,
          flag: 0,
          map: state.selectedMap,
          x: x1,
          y: y1,
          tx: x2,
          ty: y2,
          target: 0,
          dir: -1,
          minLvl: 0,
          maxLvl: 400,
          minRes: '*',
          maxRes: '*',
          accountLvl: 0
        };
        state.gates.push(newGate);
        selectGate(newGate);
        state.placeMode = false;
        setMessage('Gate creado.', 'success');
      } else if (state.resizeMode || state.selectedGate) {
        const gate = state.selectedGate;
        if (gate) {
          gate.x = x1;
          gate.y = y1;
          gate.tx = x2;
          gate.ty = y2;
          selectGate(gate);
          setMessage('Area ajustada.', 'success');
        }
      }

      state.dragging = false;
      state.dragStart = null;
      state.dragEnd = null;
      state.resizeMode = false;
      drawMap();
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
    const [mapsRes, gatesRes] = await Promise.all([
      fetch('/admin/server-editor/api/map-defs'),
      fetch('/admin/server-editor/api/gates')
    ]);
    const mapsData = await mapsRes.json();
    const gatesData = await gatesRes.json();
    if (!mapsRes.ok) throw new Error(mapsData.error || 'No se pudo cargar mapas');
    if (!gatesRes.ok) throw new Error(gatesData.error || 'No se pudo cargar gates');
    state.maps = Array.isArray(mapsData.maps) ? mapsData.maps : [];
    state.gates = parseGateTxt(gatesData.content || '');
    state.selectedMap = state.maps[0]?.id ?? 0;
  }

  async function init() {
    buildLayout();
    bindEvents();
    try {
      await loadInitialData();
      populateSelectors();
      renderGateList();
      await loadTerrain(state.selectedMap);
      drawMap();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
