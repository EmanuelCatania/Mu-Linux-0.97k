(function () {
  const root = document.getElementById('move-editor');
  if (!root) return;

  const state = {
    moves: [],
    gates: [],
    maps: [],
    selectedMove: null,
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

  function hasGate(gateId) {
    return state.gates.some((gate) => gate.index === gateId);
  }

  function isMoveValid(move) {
    if (!isInRange(move.index, 0, 65535)) return false;
    if (!move.name || !String(move.name).trim()) return false;
    if (!Number.isFinite(move.money) || move.money < 0) return false;
    if (!Number.isFinite(move.minLvl) || move.minLvl < 0) return false;
    if (!isStarOrNumber(move.maxLvl, 0, 1000)) return false;
    if (!isStarOrNumber(move.minRes, 0, 1000)) return false;
    if (!isStarOrNumber(move.maxRes, 0, 1000)) return false;
    if (move.maxLvl !== '*' && move.minLvl > move.maxLvl) return false;
    if (move.minRes !== '*' && move.maxRes !== '*' && move.minRes > move.maxRes) return false;
    if (!isInRange(move.accountLvl, 0, 3)) return false;
    if (!isInRange(move.gate, 0, 65535)) return false;
    if (!hasGate(move.gate)) return false;
    return true;
  }

  function validateMoveForm() {
    const indexInput = root.querySelector('#move-index');
    const nameInput = root.querySelector('#move-name');
    const moneyInput = root.querySelector('#move-money');
    const minLvlInput = root.querySelector('#move-minlvl');
    const maxLvlInput = root.querySelector('#move-maxlvl');
    const minResInput = root.querySelector('#move-minres');
    const maxResInput = root.querySelector('#move-maxres');
    const accInput = root.querySelector('#move-acclvl');
    const gateInput = root.querySelector('#move-gate');

    const index = toNumber(indexInput?.value);
    const name = String(nameInput?.value || '').trim();
    const money = toNumber(moneyInput?.value);
    const minLvl = toNumber(minLvlInput?.value);
    const maxLvlRaw = String(maxLvlInput?.value ?? '').trim() || '*';
    const minResRaw = String(minResInput?.value ?? '').trim() || '*';
    const maxResRaw = String(maxResInput?.value ?? '').trim() || '*';
    const acc = toNumber(accInput?.value);
    const gate = toNumber(gateInput?.value);

    const maxLvl = maxLvlRaw === '*' ? '*' : toNumber(maxLvlRaw);
    const minRes = minResRaw === '*' ? '*' : toNumber(minResRaw);
    const maxRes = maxResRaw === '*' ? '*' : toNumber(maxResRaw);

    const okIndex = setFieldInvalid(indexInput, isInRange(index, 0, 65535));
    const okName = setFieldInvalid(nameInput, !!name);
    const okMoney = setFieldInvalid(moneyInput, Number.isFinite(money) && money >= 0);
    const okMinLvl = setFieldInvalid(minLvlInput, Number.isFinite(minLvl) && minLvl >= 0);
    const okMaxLvl = setFieldInvalid(maxLvlInput, isStarOrNumber(maxLvlRaw, 0, 1000) && (maxLvl === '*' || minLvl <= maxLvl));
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
    const okGate = setFieldInvalid(gateInput, isInRange(gate, 0, 65535) && hasGate(gate));

    const ok =
      okIndex &&
      okName &&
      okMoney &&
      okMinLvl &&
      okMaxLvl &&
      okMinRes &&
      okMaxRes &&
      okResRange &&
      okAcc &&
      okGate;

    return {
      ok,
      move: {
        index,
        name,
        money,
        minLvl,
        maxLvl: maxLvlRaw === '*' ? '*' : maxLvl,
        minRes: minResRaw === '*' ? '*' : minRes,
        maxRes: maxResRaw === '*' ? '*' : maxRes,
        accountLvl: acc,
        gate
      }
    };
  }

  function parseMoveTxt(content) {
    const lines = String(content || '').split(/\r?\n/);
    const moves = [];
    for (const rawLine of lines) {
      const line = rawLine.split('//')[0].trim();
      if (!line || line.startsWith('//')) continue;
      if (line.toLowerCase() === 'end') break;
      let index = 0;
      let name = '';
      let rest = '';
      const quotedMatch = line.match(/^\s*(\d+)\s+"([^"]+)"\s+(.*)$/);
      if (quotedMatch) {
        index = Number(quotedMatch[1]);
        name = quotedMatch[2];
        rest = quotedMatch[3] || '';
      } else {
        const tokens = line.match(/\S+/g) || [];
        if (tokens.length < 9) continue;
        index = Number(tokens[0]);
        name = String(tokens[1] || '').replace(/^"|"$/g, '');
        rest = tokens.slice(2).join(' ');
      }
      const tokens = rest.match(/\S+/g) || [];
      if (tokens.length < 7) continue;
      moves.push({
        index,
        name,
        money: Number(tokens[0]),
        minLvl: Number(tokens[1]),
        maxLvl: parseStar(tokens[2]),
        minRes: parseStar(tokens[3]),
        maxRes: parseStar(tokens[4]),
        accountLvl: Number(tokens[5]),
        gate: Number(tokens[6])
      });
    }
    return moves;
  }

  function buildMoveTxt() {
    const lines = [];
    lines.push('//Index\tName\tMoney\tMinLvl\tMaxLvl\tMinRes\tMaxRes\tAccLvl\tGate');
    const moves = state.moves.slice().sort((a, b) => a.index - b.index);
    for (const move of moves) {
      const cleanName = String(move.name || '').replace(/"/g, '').trim();
      lines.push([
        move.index,
        `"${cleanName}"`,
        move.money,
        move.minLvl,
        formatStar(move.maxLvl),
        formatStar(move.minRes),
        formatStar(move.maxRes),
        move.accountLvl,
        move.gate
      ].join('\t'));
    }
    lines.push('end');
    return lines.join('\n');
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
      if (!tokens || tokens.length < 3) continue;
      gates.push({
        index: Number(tokens[0]),
        map: Number(tokens[2])
      });
    }
    return gates;
  }

  function getMapName(mapId) {
    const map = state.maps.find((m) => m.id === mapId);
    return map ? map.name : `Mapa ${mapId}`;
  }

  function getGateLabel(gateId) {
    const gate = state.gates.find((g) => g.index === gateId);
    if (!gate) return `Gate ${gateId}`;
    return `Gate ${gateId} (${getMapName(gate.map)})`;
  }

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Buscar</label>
          <input type="text" id="move-search" placeholder="Buscar move..." />
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="add-move">Agregar move</button>
          <button type="button" id="save-moves">Guardar</button>
        </div>
      </div>
      <div class="spawn-editor">
        <div class="spawn-list">
          <h3>Moves</h3>
          <ul id="move-list"></ul>
        </div>
        <div class="spawn-form">
          <h3>Detalle</h3>
          <label>Index</label>
          <input type="number" id="move-index" min="0" max="65535" />
          <label>Nombre</label>
          <input type="text" id="move-name" />
          <label>Costo (Zen)</label>
          <input type="number" id="move-money" min="0" />
          <label>Min Lvl</label>
          <input type="number" id="move-minlvl" min="0" />
          <label>Max Lvl</label>
          <input type="text" id="move-maxlvl" placeholder="*" />
          <label>Min Reset</label>
          <input type="text" id="move-minres" placeholder="*" />
          <label>Max Reset</label>
          <input type="text" id="move-maxres" placeholder="*" />
          <label>Account Lvl</label>
          <input type="number" id="move-acclvl" min="0" max="3" />
          <label>Gate</label>
          <input type="number" id="move-gate" min="0" max="65535" />
          <div class="muted" id="move-gate-hint"></div>
          <div class="spawn-actions">
            <button type="button" id="update-move">Actualizar</button>
            <button type="button" id="delete-move" class="link-button">Eliminar</button>
          </div>
        </div>
      </div>
    `;
  }

  function renderMoveList() {
    const list = root.querySelector('#move-list');
    if (!list) return;
    list.innerHTML = '';
    const filter = state.filter.toLowerCase();
    const moves = state.moves.slice().sort((a, b) => a.index - b.index);
    for (const move of moves) {
      const label = `${move.index} - ${move.name} (${getGateLabel(move.gate)})`;
      if (filter && !label.toLowerCase().includes(filter)) continue;
      const item = document.createElement('li');
      item.textContent = label;
      item.dataset.index = String(move.index);
      if (state.selectedMove && state.selectedMove.index === move.index) {
        item.classList.add('selected');
      }
      list.appendChild(item);
    }
  }

  function updateGateHint() {
    const hint = root.querySelector('#move-gate-hint');
    if (!hint) return;
    const gateValue = Number(root.querySelector('#move-gate').value);
    if (Number.isNaN(gateValue)) {
      hint.textContent = '';
      return;
    }
    hint.textContent = getGateLabel(gateValue);
  }

  function selectMove(move) {
    state.selectedMove = move;
    if (!move) {
      renderMoveList();
      return;
    }
    root.querySelector('#move-index').value = move.index;
    root.querySelector('#move-name').value = move.name;
    root.querySelector('#move-money').value = move.money;
    root.querySelector('#move-minlvl').value = move.minLvl;
    root.querySelector('#move-maxlvl').value = move.maxLvl === '*' ? '*' : move.maxLvl;
    root.querySelector('#move-minres').value = move.minRes === '*' ? '*' : move.minRes;
    root.querySelector('#move-maxres').value = move.maxRes === '*' ? '*' : move.maxRes;
    root.querySelector('#move-acclvl').value = move.accountLvl;
    root.querySelector('#move-gate').value = move.gate;
    updateGateHint();
    renderMoveList();
  }

  function bindEvents() {
    root.querySelector('#move-search').addEventListener('input', (event) => {
      state.filter = event.target.value || '';
      renderMoveList();
    });

    root.querySelector('#add-move').addEventListener('click', () => {
      const nextIndex = state.moves.reduce((max, m) => Math.max(max, m.index), 0) + 1;
      const newMove = {
        index: nextIndex,
        name: 'NuevoMove',
        money: 0,
        minLvl: 0,
        maxLvl: '*',
        minRes: '*',
        maxRes: '*',
        accountLvl: 0,
        gate: 0
      };
      state.moves.push(newMove);
      selectMove(newMove);
      setMessage('Move creado.', 'success');
    });

    root.querySelector('#save-moves').addEventListener('click', async () => {
      try {
        const invalidMove = state.moves.find((move) => !isMoveValid(move));
        if (invalidMove) {
          selectMove(invalidMove);
          setMessage('Hay moves con valores inválidos. Revisar el detalle antes de guardar.', 'error');
          return;
        }
        const content = buildMoveTxt();
        const res = await fetch('/admin/server-editor/api/moves', {
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

    root.querySelector('#update-move').addEventListener('click', () => {
      if (!state.selectedMove) return;
      const validation = validateMoveForm();
      if (!validation.ok) {
        setMessage('Hay campos inválidos. Revisar resaltados.', 'error');
        return;
      }
      const move = state.selectedMove;
      const formMove = validation.move;
      move.index = formMove.index;
      move.name = formMove.name;
      move.money = formMove.money;
      move.minLvl = formMove.minLvl;
      move.maxLvl = formMove.maxLvl;
      move.minRes = formMove.minRes;
      move.maxRes = formMove.maxRes;
      move.accountLvl = formMove.accountLvl;
      move.gate = formMove.gate;
      updateGateHint();
      renderMoveList();
      setMessage('Move actualizado.', 'success');
    });

    root.querySelector('#delete-move').addEventListener('click', () => {
      if (!state.selectedMove) return;
      state.moves = state.moves.filter((m) => m.index !== state.selectedMove.index);
      state.selectedMove = null;
      renderMoveList();
      setMessage('Move eliminado.', 'success');
    });

    root.querySelector('#move-list').addEventListener('click', (event) => {
      const item = event.target.closest('li');
      if (!item) return;
      const index = Number(item.dataset.index);
      const move = state.moves.find((m) => m.index === index);
      if (move) selectMove(move);
    });

    root.querySelector('#move-gate').addEventListener('input', updateGateHint);
  }

  async function loadInitialData() {
    const [movesRes, gatesRes, mapsRes] = await Promise.all([
      fetch('/admin/server-editor/api/moves'),
      fetch('/admin/server-editor/api/gates'),
      fetch('/admin/server-editor/api/map-defs')
    ]);
    const movesData = await movesRes.json();
    const gatesData = await gatesRes.json();
    const mapsData = await mapsRes.json();
    if (!movesRes.ok) throw new Error(movesData.error || 'No se pudo cargar moves');
    if (!gatesRes.ok) throw new Error(gatesData.error || 'No se pudo cargar gates');
    if (!mapsRes.ok) throw new Error(mapsData.error || 'No se pudo cargar mapas');
    state.moves = parseMoveTxt(movesData.content || '');
    state.gates = parseGateTxt(gatesData.content || '');
    state.maps = Array.isArray(mapsData.maps) ? mapsData.maps : [];
  }

  async function init() {
    buildLayout();
    bindEvents();
    try {
      await loadInitialData();
      renderMoveList();
      if (state.moves[0]) selectMove(state.moves[0]);
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
