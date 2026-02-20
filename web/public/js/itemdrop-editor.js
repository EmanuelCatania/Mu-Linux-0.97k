(function () {
  const root = document.getElementById('item-drop-editor');
  if (!root) return;

  const apiGet = root.dataset.apiGet || '/admin/server-editor/api/drop';
  const apiPost = root.dataset.apiPost || '/admin/server-editor/api/drop';
  const reloadTarget = root.dataset.reload || 'item';

  const state = {
    rows: [],
    itemDefs: [],
    itemMap: new Map(),
    optionPresets: {
      level: [],
      skill: [],
      luck: [],
      option: [],
      excellent: []
    },
    selected: null,
    filter: '',
    mapFilter: '',
    monsterFilter: ''
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

  function padIndex(value) {
    const raw = String(value ?? '').trim();
    if (!raw || raw === '*') return raw;
    if (!/^\d+$/.test(raw)) return raw;
    return raw.padStart(3, '0');
  }

  function formatItemToken(type, index) {
    const t = String(type ?? '').trim() || '0';
    const i = padIndex(index);
    return `${t},${i || '0'}`;
  }

  function itemKey(type, index) {
    const t = String(type ?? '').trim();
    const i = String(index ?? '').trim();
    const tKey = /^\d+$/.test(t) ? String(Number(t)) : t;
    const iKey = /^\d+$/.test(i) ? String(Number(i)) : i;
    return `${tKey}:${iKey}`;
  }

  function formatPercent(value, total) {
    if (!total) return '0%';
    const percent = (value / total) * 100;
    const rounded = Math.round(percent * 10) / 10;
    return Number.isInteger(rounded) ? `${rounded}%` : `${rounded.toFixed(1)}%`;
  }

  function presetLabel(entry) {
    const rates = Array.isArray(entry.rates) ? entry.rates : [];
    const values = rates.map((rate) => Number.parseFloat(rate) || 0);
    const total = values.reduce((sum, value) => sum + value, 0);
    const percentText = values.map((value) => formatPercent(value, total)).join(' / ');
    return `${entry.index} (${percentText})`;
  }

  function buildPresetOptions(list) {
    const options = ['<option value="*">Sin Option</option>'];
    if (!list || !list.length) return options.join('');
    list.forEach((entry) => {
      options.push(`<option value="${entry.index}">${presetLabel(entry)}</option>`);
    });
    return options.join('');
  }

  function ensurePresetValue(select, value) {
    if (!select) return;
    const raw = String(value ?? '').trim();
    if (!raw) {
      select.value = '';
      return;
    }
    const exists = Array.from(select.options).some((opt) => opt.value === raw);
    if (!exists) {
      const opt = document.createElement('option');
      opt.value = raw;
      opt.textContent = `Preset ${raw} (no encontrado)`;
      select.appendChild(opt);
    }
    select.value = raw;
  }

  function parseOptionRate(content) {
    const sections = { 0: [], 1: [], 2: [], 3: [], 4: [] };
    let current = null;
    const lines = String(content || '').split(/\r?\n/);
    for (const rawLine of lines) {
      const trimmed = rawLine.trim();
      if (!trimmed) continue;
      if (trimmed.startsWith('//') || trimmed.startsWith(';')) continue;
      if (/^end$/i.test(trimmed)) {
        current = null;
        continue;
      }
      if (/^\d+$/.test(trimmed)) {
        current = Number.parseInt(trimmed, 10);
        continue;
      }
      if (current === null || current > 4) continue;

      const commentSplit = rawLine.split('//');
      const dataPart = commentSplit[0].trim();
      const tokens = dataPart.split(/\s+/);
      if (tokens.length < 2) continue;
      const index = tokens[0];
      const rates = tokens.slice(1).map((value) => String(value).trim());
      sections[current].push({ index, rates });
    }
    return sections;
  }

  function parseDrop(content) {
    const rows = [];
    const lines = String(content || '').split(/\r?\n/);
    for (const rawLine of lines) {
      const trimmed = rawLine.trim();
      if (!trimmed) continue;
      if (trimmed.startsWith('//') || trimmed.startsWith(';')) continue;
      if (/^end$/i.test(trimmed)) break;
      const commentSplit = rawLine.split('//');
      const dataPart = commentSplit[0].trim();
      const comment = commentSplit.slice(1).join('//').trim();
      if (!dataPart) continue;
      const tokens = dataPart.split(/\s+/);
      if (!tokens.length) continue;

      let itemType = '0';
      let itemIndex = '0';
      let offset = 0;
      if (tokens[0].includes(',')) {
        const [type, index] = tokens[0].split(',');
        itemType = String(type || '0').trim();
        itemIndex = String(index || '0').trim();
        offset = 1;
      } else {
        if (tokens.length < 2) continue;
        itemType = String(tokens[0] || '0').trim();
        itemIndex = String(tokens[1] || '0').trim();
        offset = 2;
      }
      const fields = tokens.slice(offset);
      if (fields.length < 12) continue;
      rows.push({
        itemType,
        itemIndex,
        itemLevel: fields[0] || '0',
        itemGrade: fields[1] || '0',
        levelRate: fields[2] || '0',
        skillRate: fields[3] || '0',
        luckRate: fields[4] || '0',
        optionRate: fields[5] || '0',
        excellentRate: fields[6] || '0',
        mapNumber: fields[7] || '*',
        monsterClass: fields[8] || '*',
        monsterLevelMin: fields[9] || '*',
        monsterLevelMax: fields[10] || '*',
        dropRate: fields[11] || '0',
        comment
      });
    }
    return rows;
  }

  function buildContent() {
    const lines = [];
    lines.push('// ItemGrade works when ExcellentRate = 0');
    lines.push('// Level, Skill, Luck, Option and Exe');
    lines.push('// Uses ItemOptionRate Indexes');
    lines.push('');
    lines.push(
      '//Index\tLevel\tGrade\tLevel\tSkill\tLuck\tOption\tExe\tMapNum\tMonster\tMonsterLvlMin\tMonsterLvlMax\tRate\t\tComment'
    );
    state.rows.forEach((row) => {
      const token = formatItemToken(row.itemType, row.itemIndex);
      const comment = String(row.comment || '').trim();
      const suffix = comment ? `\t\t//${comment}` : '';
      lines.push(
        `${token}\t${row.itemLevel}\t${row.itemGrade}\t${row.levelRate}\t${row.skillRate}\t${row.luckRate}\t${row.optionRate}\t${row.excellentRate}\t${row.mapNumber}\t${row.monsterClass}\t${row.monsterLevelMin}\t${row.monsterLevelMax}\t${row.dropRate}${suffix}`
      );
    });
    lines.push('end');
    return lines.join('\n');
  }

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Buscar</label>
          <input id="drop-search" type="text" placeholder="Buscar item, mapa o monster..." />
          <span class="muted" id="drop-count"></span>
        </div>
        <div class="spawn-toolbar-group">
          <label>Mapa</label>
          <input id="drop-filter-map" type="text" placeholder="*">
          <label>Monster</label>
          <input id="drop-filter-monster" type="text" placeholder="*">
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="save-drop">Guardar</button>
        </div>
      </div>
      <div class="custom-note">
        Usa <code>*</code> para “cualquier” mapa/monster/nivel. El <code>DropRate</code> usa escala
        <strong>1,000,000</strong> (1,000,000 = 100%).
      </div>
      <div class="custom-note">
        <strong>ItemGrade</strong> solo aplica cuando <strong>ExcellentRate</strong> = 0.
      </div>
      <div class="spawn-editor">
        <div class="spawn-list">
          <h3>Drops</h3>
          <ul id="drop-list" class="simple-list"></ul>
          <button type="button" id="add-drop">Agregar drop</button>
        </div>
        <div class="spawn-form">
          <h3>Detalle</h3>
          <div class="custom-form-grid">
            <div class="field"><label>ItemType</label><input id="drop-item-type" type="number"></div>
            <div class="field"><label>ItemIndex</label><input id="drop-item-index" type="number"></div>
            <div class="field"><label>ItemLevel</label><input id="drop-item-level" type="text"></div>
            <div class="field"><label>ItemGrade</label><input id="drop-item-grade" type="number"></div>
            <div class="field"><label>LevelRate (preset)</label><select id="drop-level-rate"></select></div>
            <div class="field"><label>SkillRate (preset)</label><select id="drop-skill-rate"></select></div>
            <div class="field"><label>LuckRate (preset)</label><select id="drop-luck-rate"></select></div>
            <div class="field"><label>OptionRate (preset)</label><select id="drop-option-rate"></select></div>
            <div class="field"><label>ExcellentRate (preset)</label><select id="drop-exc-rate"></select></div>
            <div class="field"><label>MapNum</label><input id="drop-map" type="text" placeholder="*"></div>
            <div class="field"><label>Monster</label><input id="drop-monster" type="text" placeholder="*"></div>
            <div class="field"><label>MonsterLvlMin</label><input id="drop-monster-min" type="text" placeholder="*"></div>
            <div class="field"><label>MonsterLvlMax</label><input id="drop-monster-max" type="text" placeholder="*"></div>
            <div class="field"><label>DropRate</label><input id="drop-rate" type="text"></div>
            <div class="field"><label>DropRate %</label><input id="drop-rate-percent" type="text" readonly></div>
            <div class="field"><label>Comentario</label><input id="drop-comment" type="text"></div>
          </div>
          <div class="item-picker">
            <label>Explorador de items</label>
            <input type="text" id="drop-item-search" placeholder="Buscar item..." />
            <ul id="drop-item-picker" class="simple-list"></ul>
          </div>
          <div class="spawn-actions">
            <button type="button" id="update-drop">Actualizar</button>
            <button type="button" id="delete-drop" class="link-button">Eliminar</button>
          </div>
        </div>
      </div>
    `;
  }

  function renderList() {
    const list = root.querySelector('#drop-list');
    const count = root.querySelector('#drop-count');
    if (!list) return;
    list.innerHTML = '';
    const filter = String(state.filter || '').toLowerCase();
    const mapFilter = String(state.mapFilter || '').trim();
    const monsterFilter = String(state.monsterFilter || '').trim();
    let visible = 0;
    state.rows.forEach((row, index) => {
      const itemDef = state.itemMap.get(itemKey(row.itemType, row.itemIndex));
      const name = itemDef ? itemDef.name : `Item ${row.itemType},${row.itemIndex}`;
      const mapLabel = row.mapNumber === '*' ? 'Todos' : row.mapNumber;
      const monsterLabel = row.monsterClass === '*' ? 'Todos' : row.monsterClass;
      const title = `${name} | Map ${mapLabel} | Mob ${monsterLabel} | ${formatDropRateDisplay(row.dropRate)}`;
      const searchable = `${name} ${row.comment || ''} ${row.mapNumber} ${row.monsterClass}`.toLowerCase();
      if (filter && !searchable.includes(filter)) return;
      if (!matchFilter(mapFilter, row.mapNumber)) return;
      if (!matchFilter(monsterFilter, row.monsterClass)) return;
      visible += 1;
      const li = document.createElement('li');
      li.textContent = title;
      li.className = state.selected === index ? 'selected' : '';
      li.addEventListener('click', () => selectRow(index));
      list.appendChild(li);
    });
    if (count) {
      count.textContent = `${visible}/${state.rows.length}`;
    }
  }

  function selectRow(index) {
    state.selected = index;
    renderList();
    const row = state.rows[index];
    if (!row) return;
    const set = (id, value) => {
      const input = root.querySelector(`#${id}`);
      if (input) input.value = value ?? '';
    };
    set('drop-item-type', row.itemType);
    set('drop-item-index', row.itemIndex);
    set('drop-item-level', row.itemLevel);
    set('drop-item-grade', row.itemGrade);
    ensurePresetValue(root.querySelector('#drop-level-rate'), row.levelRate);
    ensurePresetValue(root.querySelector('#drop-skill-rate'), row.skillRate);
    ensurePresetValue(root.querySelector('#drop-luck-rate'), row.luckRate);
    ensurePresetValue(root.querySelector('#drop-option-rate'), row.optionRate);
    ensurePresetValue(root.querySelector('#drop-exc-rate'), row.excellentRate);
    set('drop-map', row.mapNumber);
    set('drop-monster', row.monsterClass);
    set('drop-monster-min', row.monsterLevelMin);
    set('drop-monster-max', row.monsterLevelMax);
    set('drop-rate', row.dropRate);
    set('drop-rate-percent', formatDropRatePercent(row.dropRate));
    set('drop-comment', row.comment);
    validateInputs();
  }

  function readValue(id, fallback) {
    const raw = String(root.querySelector(`#${id}`)?.value || '').trim();
    return raw === '' ? fallback : raw;
  }

  function matchFilter(filterValue, rowValue) {
    if (!filterValue || filterValue === '') return true;
    if (filterValue === '*') return String(rowValue || '').trim() === '*';
    const rowStr = String(rowValue || '').trim();
    return rowStr === filterValue || rowStr === '*';
  }

  function formatDropRatePercent(rate) {
    const value = Number.parseFloat(rate);
    if (!Number.isFinite(value)) return 'N/A';
    return formatPercent(value, 1000000);
  }

  function formatDropRateDisplay(rate) {
    const value = String(rate ?? '').trim();
    if (!value) return 'Rate 0 (0%)';
    const percent = formatDropRatePercent(value);
    return `Rate ${value} (${percent})`;
  }

  function isWildcard(value) {
    return String(value || '').trim() === '*';
  }

  function isInteger(value) {
    return /^-?\d+$/.test(String(value || '').trim());
  }

  function validateRow(row) {
    if (!row) return { ok: false, reason: 'Fila vacia.' };
    const itemType = String(row.itemType ?? '').trim();
    const itemIndex = String(row.itemIndex ?? '').trim();
    const level = String(row.itemLevel ?? '').trim();
    const grade = String(row.itemGrade ?? '').trim();
    const map = String(row.mapNumber ?? '').trim();
    const monster = String(row.monsterClass ?? '').trim();
    const minLvl = String(row.monsterLevelMin ?? '').trim();
    const maxLvl = String(row.monsterLevelMax ?? '').trim();
    const dropRate = String(row.dropRate ?? '').trim();

    const typeOk = isInteger(itemType) && Number(itemType) >= 0;
    if (!typeOk) return { ok: false, reason: 'ItemType invalido.' };
    const indexOk = isInteger(itemIndex) && Number(itemIndex) >= 0;
    if (!indexOk) return { ok: false, reason: 'ItemIndex invalido.' };
    const levelOk = isWildcard(level) || (isInteger(level) && Number(level) >= 0 && Number(level) <= 15);
    if (!levelOk) return { ok: false, reason: 'ItemLevel invalido.' };
    const gradeOk = isInteger(grade) && Number(grade) >= 0 && Number(grade) <= 15;
    if (!gradeOk) return { ok: false, reason: 'ItemGrade invalido.' };
    const mapOk = isWildcard(map) || (isInteger(map) && Number(map) >= 0 && Number(map) <= 255);
    if (!mapOk) return { ok: false, reason: 'MapNum invalido.' };
    const monsterOk = isWildcard(monster) || (isInteger(monster) && Number(monster) >= 0);
    if (!monsterOk) return { ok: false, reason: 'Monster invalido.' };
    const minOk = isWildcard(minLvl) || (isInteger(minLvl) && Number(minLvl) >= 0);
    const maxOk = isWildcard(maxLvl) || (isInteger(maxLvl) && Number(maxLvl) >= 0);
    if (!minOk || !maxOk) return { ok: false, reason: 'Rango de nivel invalido.' };
    if (isInteger(minLvl) && isInteger(maxLvl) && Number(minLvl) > Number(maxLvl)) {
      return { ok: false, reason: 'MonsterLvlMin > MonsterLvlMax.' };
    }
    const rateOk = isInteger(dropRate) && Number(dropRate) >= 0 && Number(dropRate) <= 1000000;
    if (!rateOk) return { ok: false, reason: 'DropRate invalido.' };
    return { ok: true };
  }

  function markField(id, ok, hint) {
    const input = root.querySelector(`#${id}`);
    if (!input) return;
    const field = input.closest('.field');
    if (!field) return;
    field.classList.toggle('invalid', !ok);
    if (hint) {
      let hintEl = field.querySelector('.field-hint');
      if (!hintEl) {
        hintEl = document.createElement('div');
        hintEl.className = 'field-hint';
        field.appendChild(hintEl);
      }
      hintEl.textContent = ok ? '' : hint;
    }
  }

  function validateInputs() {
    const level = readValue('drop-item-level', '0');
    const grade = readValue('drop-item-grade', '0');
    const itemType = readValue('drop-item-type', '0');
    const itemIndex = readValue('drop-item-index', '0');
    const map = readValue('drop-map', '*');
    const monster = readValue('drop-monster', '*');
    const minLvl = readValue('drop-monster-min', '*');
    const maxLvl = readValue('drop-monster-max', '*');
    const dropRate = readValue('drop-rate', '0');

    const typeOk = isInteger(itemType) && Number(itemType) >= 0;
    markField('drop-item-type', typeOk, 'Usa un numero >= 0.');

    const indexOk = isInteger(itemIndex) && Number(itemIndex) >= 0;
    markField('drop-item-index', indexOk, 'Usa un numero >= 0.');

    const levelOk = isWildcard(level) || (isInteger(level) && Number(level) >= 0 && Number(level) <= 15);
    markField('drop-item-level', levelOk, 'Usa * o un numero entre 0 y 15.');

    const gradeOk = isInteger(grade) && Number(grade) >= 0 && Number(grade) <= 15;
    markField('drop-item-grade', gradeOk, 'Usa un numero entre 0 y 15.');

    const mapOk = isWildcard(map) || (isInteger(map) && Number(map) >= 0 && Number(map) <= 255);
    markField('drop-map', mapOk, 'Usa * o un numero entre 0 y 255.');

    const monsterOk = isWildcard(monster) || (isInteger(monster) && Number(monster) >= 0);
    markField('drop-monster', monsterOk, 'Usa * o un numero >= 0.');

    const minOk = isWildcard(minLvl) || (isInteger(minLvl) && Number(minLvl) >= 0);
    const maxOk = isWildcard(maxLvl) || (isInteger(maxLvl) && Number(maxLvl) >= 0);
    const rangeOk =
      minOk &&
      maxOk &&
      (!isInteger(minLvl) || !isInteger(maxLvl) || Number(minLvl) <= Number(maxLvl));
    markField('drop-monster-min', minOk && rangeOk, 'Usa * o un numero >= 0.');
    markField('drop-monster-max', maxOk && rangeOk, 'Usa * o un numero >= 0.');

    const rateOk = isInteger(dropRate) && Number(dropRate) >= 0 && Number(dropRate) <= 1000000;
    markField('drop-rate', rateOk, 'Usa un numero entre 0 y 1000000.');

    const percentInput = root.querySelector('#drop-rate-percent');
    if (percentInput) {
      percentInput.value = formatDropRatePercent(dropRate);
    }

    return typeOk && indexOk && levelOk && gradeOk && mapOk && monsterOk && rangeOk && rateOk;
  }

  function bindEvents() {
    root.querySelector('#drop-search')?.addEventListener('input', (event) => {
      state.filter = event.target.value || '';
      renderList();
    });
    root.querySelector('#drop-filter-map')?.addEventListener('input', (event) => {
      state.mapFilter = event.target.value || '';
      renderList();
    });
    root.querySelector('#drop-filter-monster')?.addEventListener('input', (event) => {
      state.monsterFilter = event.target.value || '';
      renderList();
    });

    root.querySelector('#add-drop')?.addEventListener('click', () => {
      state.rows.push({
        itemType: '0',
        itemIndex: '0',
        itemLevel: '0',
        itemGrade: '0',
        levelRate: '0',
        skillRate: '0',
        luckRate: '0',
        optionRate: '0',
        excellentRate: '0',
        mapNumber: '*',
        monsterClass: '*',
        monsterLevelMin: '*',
        monsterLevelMax: '*',
        dropRate: '100',
        comment: ''
      });
      selectRow(state.rows.length - 1);
    });

    root.querySelector('#update-drop')?.addEventListener('click', () => {
      if (!validateInputs()) {
        setMessage('Hay campos invalidos. Revisar resaltados.', 'error');
        return;
      }
      const idx = state.selected;
      if (idx == null) return;
      const row = state.rows[idx];
      if (!row) return;
      row.itemType = readValue('drop-item-type', '0');
      row.itemIndex = readValue('drop-item-index', '0');
      row.itemLevel = readValue('drop-item-level', '0');
      row.itemGrade = readValue('drop-item-grade', '0');
      row.levelRate = readValue('drop-level-rate', '*');
      row.skillRate = readValue('drop-skill-rate', '*');
      row.luckRate = readValue('drop-luck-rate', '*');
      row.optionRate = readValue('drop-option-rate', '*');
      row.excellentRate = readValue('drop-exc-rate', '*');
      row.mapNumber = readValue('drop-map', '*');
      row.monsterClass = readValue('drop-monster', '*');
      row.monsterLevelMin = readValue('drop-monster-min', '*');
      row.monsterLevelMax = readValue('drop-monster-max', '*');
      row.dropRate = readValue('drop-rate', '0');
      row.comment = readValue('drop-comment', '');
      renderList();
      setMessage('Drop actualizado.', 'success');
    });

    root.querySelector('#delete-drop')?.addEventListener('click', () => {
      const idx = state.selected;
      if (idx == null) return;
      state.rows.splice(idx, 1);
      state.selected = null;
      renderList();
      setMessage('Drop eliminado.', 'success');
    });

    root.querySelector('#save-drop')?.addEventListener('click', saveDrop);

    const pickerInput = root.querySelector('#drop-item-search');
    const pickerList = root.querySelector('#drop-item-picker');
    if (pickerInput && pickerList) {
      const renderPicker = () => {
        const filter = String(pickerInput.value || '').toLowerCase();
        pickerList.innerHTML = '';
        state.itemDefs
          .filter((item) => !filter || item.name.toLowerCase().includes(filter))
          .slice(0, 200)
          .forEach((item) => {
            const li = document.createElement('li');
            li.textContent = `${item.name} (${item.section},${item.index})`;
            li.addEventListener('click', () => {
              const typeInput = root.querySelector('#drop-item-type');
              const indexInput = root.querySelector('#drop-item-index');
              if (typeInput) typeInput.value = item.section;
              if (indexInput) indexInput.value = item.index;
            });
            pickerList.appendChild(li);
          });
      };
      pickerInput.addEventListener('input', renderPicker);
      renderPicker();
    }

    const validateOn = [
      'drop-item-level',
      'drop-item-grade',
      'drop-map',
      'drop-monster',
      'drop-monster-min',
      'drop-monster-max',
      'drop-rate'
    ];
    validateOn.forEach((id) => {
      root.querySelector(`#${id}`)?.addEventListener('input', validateInputs);
    });
  }

  function fillPresetOptions() {
    const levelSelect = root.querySelector('#drop-level-rate');
    const skillSelect = root.querySelector('#drop-skill-rate');
    const luckSelect = root.querySelector('#drop-luck-rate');
    const optionSelect = root.querySelector('#drop-option-rate');
    const excSelect = root.querySelector('#drop-exc-rate');
    if (levelSelect) levelSelect.innerHTML = buildPresetOptions(state.optionPresets.level);
    if (skillSelect) skillSelect.innerHTML = buildPresetOptions(state.optionPresets.skill);
    if (luckSelect) luckSelect.innerHTML = buildPresetOptions(state.optionPresets.luck);
    if (optionSelect) optionSelect.innerHTML = buildPresetOptions(state.optionPresets.option);
    if (excSelect) excSelect.innerHTML = buildPresetOptions(state.optionPresets.excellent);
  }

  async function loadDrop() {
    const res = await fetch(apiGet);
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || 'No se pudo cargar el drop');
    state.rows = parseDrop(data.content || '');
    state.selected = null;
  }

  async function saveDrop() {
    try {
      if (state.selected != null && !validateInputs()) {
        setMessage('Hay campos invalidos. Revisar resaltados.', 'error');
        return;
      }
      for (let i = 0; i < state.rows.length; i += 1) {
        const result = validateRow(state.rows[i]);
        if (!result.ok) {
          selectRow(i);
          setMessage(`No se pudo guardar: ${result.reason}`, 'error');
          return;
        }
      }
      const content = buildContent();
      const res = await fetch(apiPost, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ content })
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudo guardar');
      const reload = await fetch('/admin/server-editor/api/reload', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ target: reloadTarget })
      });
      if (!reload.ok) {
        setMessage('Guardado, pero no se pudo recargar en el server.', 'error');
      } else {
        setMessage('Drop guardado y recargado en el servidor.', 'success');
      }
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  async function loadItemDefs() {
    try {
      const res = await fetch('/admin/item-defs.json');
      const data = await res.json();
      state.itemDefs = Array.isArray(data.items) ? data.items : [];
      state.itemMap = new Map(state.itemDefs.map((item) => [itemKey(item.section, item.index), item]));
    } catch {
      setMessage('No se pudo cargar la lista de items.', 'error');
    }
  }

  async function loadOptionPresets() {
    try {
      const res = await fetch('/admin/server-editor/api/item-option-rate');
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudo cargar ItemOptionRate');
      const sections = parseOptionRate(data.content || '');
      state.optionPresets.level = sections[0] || [];
      state.optionPresets.skill = sections[1] || [];
      state.optionPresets.luck = sections[2] || [];
      state.optionPresets.option = sections[3] || [];
      state.optionPresets.excellent = sections[4] || [];
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  async function init() {
    buildLayout();
    bindEvents();
    await loadItemDefs();
    await loadOptionPresets();
    fillPresetOptions();
    try {
      await loadDrop();
      renderList();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
