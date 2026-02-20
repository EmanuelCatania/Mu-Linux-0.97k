(function () {
  const root = document.getElementById('custom-editor');
  if (!root) return;

  const TAB_CONFIG = {
    safezone: {
      label: 'Safe Zone',
      title: 'Zonas seguras',
      fields: [
        { key: 'map', label: 'Mapa', type: 'number' },
        { key: 'x', label: 'X', type: 'number' },
        { key: 'y', label: 'Y', type: 'number' },
        { key: 'tx', label: 'TX', type: 'number' },
        { key: 'ty', label: 'TY', type: 'number' },
        { key: 'comment', label: 'Comentario', type: 'text' }
      ],
      listLabel: (row) => `Mapa ${row.map} (${row.x},${row.y}) -> (${row.tx},${row.ty})`
    },
    pkfree: {
      label: 'PK Free',
      title: 'Zonas PK Free',
      fields: [
        { key: 'map', label: 'Mapa', type: 'number' },
        { key: 'x', label: 'X', type: 'number' },
        { key: 'y', label: 'Y', type: 'number' },
        { key: 'tx', label: 'TX', type: 'number' },
        { key: 'ty', label: 'TY', type: 'number' },
        { key: 'comment', label: 'Comentario', type: 'text' }
      ],
      listLabel: (row) => `Mapa ${row.map} (${row.x},${row.y}) -> (${row.tx},${row.ty})`
    },
    npcmove: {
      label: 'NPC Move',
      title: 'NPC Move',
      fields: [
        { key: 'npc', label: 'Npc', type: 'number' },
        { key: 'npcMap', label: 'NpcMap', type: 'number' },
        { key: 'npcX', label: 'NpcX', type: 'number' },
        { key: 'npcY', label: 'NpcY', type: 'number' },
        { key: 'moveMap', label: 'MoveMap', type: 'number' },
        { key: 'moveX', label: 'MoveX', type: 'number' },
        { key: 'moveY', label: 'MoveY', type: 'number' },
        { key: 'minLvl', label: 'Min Lvl', type: 'text', placeholder: '*' },
        { key: 'maxLvl', label: 'Max Lvl', type: 'text', placeholder: '*' },
        { key: 'minRes', label: 'Min Reset', type: 'text', placeholder: '*' },
        { key: 'maxRes', label: 'Max Reset', type: 'text', placeholder: '*' },
        { key: 'vipLvl', label: 'VIP', type: 'number' },
        { key: 'pkMove', label: 'PK Move', type: 'number' },
        { key: 'comment', label: 'Comentario', type: 'text' }
      ],
      listLabel: (row) =>
        `Npc ${row.npc} (${row.npcMap}:${row.npcX},${row.npcY}) -> ${row.moveMap}:${row.moveX},${row.moveY}`
    }
  };

  const DEFAULTS = {
    safezone: { map: '0', x: '0', y: '0', tx: '0', ty: '0', comment: '' },
    pkfree: { map: '0', x: '0', y: '0', tx: '0', ty: '0', comment: '' },
    npcmove: {
      npc: '0',
      npcMap: '0',
      npcX: '0',
      npcY: '0',
      moveMap: '0',
      moveX: '0',
      moveY: '0',
      minLvl: '*',
      maxLvl: '*',
      minRes: '*',
      maxRes: '*',
      vipLvl: '0',
      pkMove: '1',
      comment: ''
    }
  };

  const state = {
    activeTab: 'safezone',
    safezone: { rows: [], selected: null },
    pkfree: { rows: [], selected: null },
    npcmove: { rows: [], selected: null }
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

  function parseZone(content) {
    const rows = [];
    const lines = String(content || '').split(/\r?\n/);
    for (const rawLine of lines) {
      const trimmed = rawLine.trim();
      if (!trimmed) continue;
      if (/^end$/i.test(trimmed)) continue;
      if (trimmed.startsWith('//') || trimmed.startsWith(';')) continue;
      const parts = trimmed.split('//');
      const dataPart = parts[0].trim();
      if (!dataPart) continue;
      const comment = parts.slice(1).join('//').trim();
      const tokens = dataPart.split(/\s+/);
      if (tokens.length < 5) continue;
      rows.push({
        map: tokens[0],
        x: tokens[1],
        y: tokens[2],
        tx: tokens[3],
        ty: tokens[4],
        comment
      });
    }
    return rows;
  }

  function parseNpcMove(content) {
    const rows = [];
    const lines = String(content || '').split(/\r?\n/);
    for (const rawLine of lines) {
      const trimmed = rawLine.trim();
      if (!trimmed) continue;
      if (/^end$/i.test(trimmed)) continue;
      if (trimmed.startsWith('//') || trimmed.startsWith(';')) continue;
      const parts = trimmed.split('//');
      const dataPart = parts[0].trim();
      if (!dataPart) continue;
      const comment = parts.slice(1).join('//').trim();
      const tokens = dataPart.split(/\s+/);
      if (tokens.length < 13) continue;
      rows.push({
        npc: tokens[0],
        npcMap: tokens[1],
        npcX: tokens[2],
        npcY: tokens[3],
        moveMap: tokens[4],
        moveX: tokens[5],
        moveY: tokens[6],
        minLvl: tokens[7],
        maxLvl: tokens[8],
        minRes: tokens[9],
        maxRes: tokens[10],
        vipLvl: tokens[11],
        pkMove: tokens[12],
        comment
      });
    }
    return rows;
  }

  function buildZoneFile(rows) {
    const lines = [];
    lines.push(`//Map\tX\tY\tTX\tTY\tComment`);
    rows.forEach((row) => {
      const map = String(row.map || '0').trim();
      const x = String(row.x || '0').trim();
      const y = String(row.y || '0').trim();
      const tx = String(row.tx || '0').trim();
      const ty = String(row.ty || '0').trim();
      const comment = String(row.comment || '').trim();
      const suffix = comment ? `\t//${comment}` : '';
      lines.push(`${map}\t${x}\t${y}\t${tx}\t${ty}${suffix}`);
    });
    lines.push('end');
    return lines.join('\n');
  }

  function buildNpcMoveFile(rows) {
    const lines = [];
    lines.push(
      '//Npc\tNpcMap\tNpcX\tNpcY\tMoveMap\tMoveX\tMoveY\tMinLvl\tMaxLvl\tMinRes\tMaxRes\tVIPLvl\tPKMove\tComment'
    );
    rows.forEach((row) => {
      const values = [
        row.npc,
        row.npcMap,
        row.npcX,
        row.npcY,
        row.moveMap,
        row.moveX,
        row.moveY,
        row.minLvl,
        row.maxLvl,
        row.minRes,
        row.maxRes,
        row.vipLvl,
        row.pkMove
      ].map((value) => String(value ?? '').trim() || '0');
      const comment = String(row.comment || '').trim();
      const suffix = comment ? `\t//${comment}` : '';
      lines.push(`${values.join('\t')}${suffix}`);
    });
    lines.push('end');
    return lines.join('\n');
  }

  function buildLayout() {
    const buttons = Object.entries(TAB_CONFIG)
      .map(([key, config], index) => {
        const active = index === 0 ? 'active' : '';
        return `<button type="button" class="tab-btn ${active}" data-tab="${key}">${config.label}</button>`;
      })
      .join('');

    const panels = Object.entries(TAB_CONFIG)
      .map(([key, config], index) => {
        const active = index === 0 ? 'active' : '';
        return `
        <div class="custom-panel ${active}" data-tab="${key}">
          <div class="spawn-editor">
            <div class="spawn-list">
              <h3>${config.title}</h3>
              <ul id="${key}-list" class="simple-list"></ul>
            </div>
            <div class="spawn-form" id="${key}-form"></div>
          </div>
        </div>`;
      })
      .join('');

    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          ${buttons}
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="add-row">Agregar</button>
          <button type="button" id="save-customs">Guardar</button>
        </div>
      </div>
      <div class="muted custom-note">
        Estos archivos se aplican solo del lado servidor. Recorda recargar <code>custom</code> luego de guardar.
      </div>
      ${panels}
    `;
  }

  function buildForm(tabKey) {
    const config = TAB_CONFIG[tabKey];
    const form = root.querySelector(`#${tabKey}-form`);
    if (!form) return;
    const fields = config.fields
      .map((field) => {
        const placeholder = field.placeholder ? `placeholder="${field.placeholder}"` : '';
        return `
          <div class="field">
            <label>${field.label}</label>
            <input type="${field.type}" data-field="${field.key}" ${placeholder} />
          </div>
        `;
      })
      .join('');

    form.innerHTML = `
      <h3>Detalle</h3>
      <div class="custom-form-grid">${fields}</div>
      <div class="spawn-actions">
        <button type="button" data-action="update">Actualizar</button>
        <button type="button" class="link-button" data-action="delete">Eliminar</button>
      </div>
    `;

    form.querySelector('[data-action="update"]').addEventListener('click', () => {
      updateSelected(tabKey);
    });
    form.querySelector('[data-action="delete"]').addEventListener('click', () => {
      deleteSelected(tabKey);
    });
  }

  function setActiveTab(tabKey) {
    state.activeTab = tabKey;
    root.querySelectorAll('.tab-btn').forEach((btn) => {
      btn.classList.toggle('active', btn.dataset.tab === tabKey);
    });
    root.querySelectorAll('.custom-panel').forEach((panel) => {
      panel.classList.toggle('active', panel.dataset.tab === tabKey);
    });
    if (state[tabKey].rows.length && state[tabKey].selected == null) {
      selectRow(tabKey, 0);
    } else {
      renderList(tabKey);
      fillForm(tabKey, getSelectedRow(tabKey));
    }
  }

  function renderList(tabKey) {
    const list = root.querySelector(`#${tabKey}-list`);
    if (!list) return;
    const rows = state[tabKey].rows;
    list.innerHTML = '';
    rows.forEach((row, index) => {
      const item = document.createElement('li');
      item.textContent = TAB_CONFIG[tabKey].listLabel(row);
      item.className = index === state[tabKey].selected ? 'selected' : '';
      item.addEventListener('click', () => selectRow(tabKey, index));
      list.appendChild(item);
    });
  }

  function selectRow(tabKey, index) {
    state[tabKey].selected = index;
    renderList(tabKey);
    fillForm(tabKey, getSelectedRow(tabKey));
  }

  function getSelectedRow(tabKey) {
    const idx = state[tabKey].selected;
    if (idx == null) return null;
    return state[tabKey].rows[idx] || null;
  }

  function fillForm(tabKey, row) {
    const form = root.querySelector(`#${tabKey}-form`);
    if (!form) return;
    const inputs = form.querySelectorAll('[data-field]');
    inputs.forEach((input) => {
      const key = input.dataset.field;
      input.value = row ? String(row[key] ?? '') : '';
    });
  }

  function readForm(tabKey) {
    const config = TAB_CONFIG[tabKey];
    const form = root.querySelector(`#${tabKey}-form`);
    if (!form) return null;
    const result = {};
    config.fields.forEach((field) => {
      const input = form.querySelector(`[data-field="${field.key}"]`);
      if (!input) return;
      let value = String(input.value || '').trim();
      if (!value) {
        if (field.placeholder === '*') {
          value = '*';
        } else if (field.type === 'number') {
          value = '0';
        }
      }
      result[field.key] = value;
    });
    return result;
  }

  function addRow(tabKey) {
    const defaults = DEFAULTS[tabKey];
    state[tabKey].rows.push({ ...defaults });
    selectRow(tabKey, state[tabKey].rows.length - 1);
  }

  function updateSelected(tabKey) {
    const idx = state[tabKey].selected;
    if (idx == null) {
      setMessage('Selecciona un registro para actualizar.', 'error');
      return;
    }
    const data = readForm(tabKey);
    if (!data) return;
    state[tabKey].rows[idx] = data;
    renderList(tabKey);
    setMessage('Registro actualizado.', 'success');
  }

  function deleteSelected(tabKey) {
    const idx = state[tabKey].selected;
    if (idx == null) {
      setMessage('Selecciona un registro para eliminar.', 'error');
      return;
    }
    state[tabKey].rows.splice(idx, 1);
    state[tabKey].selected = null;
    renderList(tabKey);
    fillForm(tabKey, null);
    setMessage('Registro eliminado.', 'success');
  }

  async function saveAll() {
    try {
      const safeContent = buildZoneFile(state.safezone.rows);
      const pkContent = buildZoneFile(state.pkfree.rows);
      const npcContent = buildNpcMoveFile(state.npcmove.rows);

      const requests = [
        fetch('/admin/server-editor/api/custom?kind=safezone', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ content: safeContent })
        }),
        fetch('/admin/server-editor/api/custom?kind=pkfree', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ content: pkContent })
        }),
        fetch('/admin/server-editor/api/custom?kind=npcmove', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ content: npcContent })
        })
      ];

      const responses = await Promise.all(requests);
      for (const res of responses) {
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'No se pudo guardar');
      }

      const reload = await fetch('/admin/server-editor/api/reload', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ target: 'custom' })
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

  function bindEvents() {
    root.querySelectorAll('.tab-btn').forEach((btn) => {
      btn.addEventListener('click', () => setActiveTab(btn.dataset.tab));
    });
    root.querySelector('#add-row').addEventListener('click', () => addRow(state.activeTab));
    root.querySelector('#save-customs').addEventListener('click', saveAll);
  }

  async function loadInitialData() {
    const [safeRes, pkRes, npcRes] = await Promise.all([
      fetch('/admin/server-editor/api/custom?kind=safezone'),
      fetch('/admin/server-editor/api/custom?kind=pkfree'),
      fetch('/admin/server-editor/api/custom?kind=npcmove')
    ]);
    const safeData = await safeRes.json();
    const pkData = await pkRes.json();
    const npcData = await npcRes.json();
    if (!safeRes.ok) throw new Error(safeData.error || 'No se pudo cargar SafeZone');
    if (!pkRes.ok) throw new Error(pkData.error || 'No se pudo cargar PKFree');
    if (!npcRes.ok) throw new Error(npcData.error || 'No se pudo cargar NpcMove');
    state.safezone.rows = parseZone(safeData.content || '');
    state.pkfree.rows = parseZone(pkData.content || '');
    state.npcmove.rows = parseNpcMove(npcData.content || '');
  }

  async function init() {
    buildLayout();
    Object.keys(TAB_CONFIG).forEach((key) => buildForm(key));
    bindEvents();
    try {
      await loadInitialData();
      setActiveTab(state.activeTab);
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
