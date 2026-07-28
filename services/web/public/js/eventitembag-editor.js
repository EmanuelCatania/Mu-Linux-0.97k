(function () {
  const root = document.getElementById('event-item-bag-editor');
  if (!root) return;

  const state = {
    bagFiles: [],
    currentFile: '',
    mode: 'normal',
    header: {
      eventName: 'EventName',
      dropZen: '0',
      itemDropRate: '100',
      itemDropCount: '1',
      itemDropType: '1',
      fireworks: '0'
    },
    normalItems: [],
    adv: {
      rates: [],
      sections: [],
      items: []
    },
    manager: [],
    itemDefs: [],
    itemMap: new Map(),
    optionPresets: {
      level: [],
      skill: [],
      luck: [],
      option: [],
      excellent: []
    },
    selected: {
      normalItem: null,
      advRate: null,
      advSection: null,
      advItem: null,
      manager: null
    }
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

  function hasPreset(list, value) {
    const raw = String(value ?? '').trim();
    if (!raw) return true;
    if (!Array.isArray(list) || list.length === 0) return true;
    return list.some((entry) => String(entry.index) === raw);
  }

  function activateTab(tabName) {
    root.querySelectorAll('.tab-btn').forEach((btn) => {
      btn.classList.toggle('active', btn.dataset.tab === tabName);
    });
    root.querySelectorAll('.custom-panel').forEach((panel) => {
      panel.classList.toggle('active', panel.id === `${tabName}-panel`);
    });
  }

  function validateHeader() {
    const nameInput = root.querySelector('#bag-event-name');
    const zenInput = root.querySelector('#bag-drop-zen');
    const rateInput = root.querySelector('#bag-drop-rate');
    const countInput = root.querySelector('#bag-drop-count');
    const typeInput = root.querySelector('#bag-drop-type');
    const fireworksInput = root.querySelector('#bag-fireworks');

    const name = String(nameInput?.value || '').trim();
    const zen = toNumber(zenInput?.value);
    const rate = toNumber(rateInput?.value);
    const count = toNumber(countInput?.value);
    const type = toNumber(typeInput?.value);
    const fireworks = toNumber(fireworksInput?.value);

    const okName = setFieldInvalid(nameInput, !!name);
    const okZen = setFieldInvalid(zenInput, Number.isFinite(zen) && zen >= 0);
    const okRate = setFieldInvalid(rateInput, isInRange(rate, 0, 10000));
    const okCount = setFieldInvalid(countInput, Number.isFinite(count) && count >= 1);
    const okType = setFieldInvalid(typeInput, Number.isFinite(type) && type >= 0);
    const okFireworks = setFieldInvalid(fireworksInput, Number.isFinite(fireworks) && fireworks >= 0);

    return okName && okZen && okRate && okCount && okType && okFireworks;
  }

  function validateNormalItem(item) {
    const typeOk = isInRange(toNumber(item.itemType), 0, 255);
    const indexOk = isInRange(toNumber(item.itemIndex), 0, 512);
    const minOk = isInRange(toNumber(item.minLvl), 0, 15);
    const maxOk = isInRange(toNumber(item.maxLvl), 0, 15);
    const skillOk = isInRange(toNumber(item.skill), 0, 1);
    const luckOk = isInRange(toNumber(item.luck), 0, 1);
    const optOk = isInRange(toNumber(item.opt), 0, 3);
    const exceOk = isInRange(toNumber(item.exce), 0, 1);
    return typeOk && indexOk && minOk && maxOk && minOk <= maxOk && skillOk && luckOk && optOk && exceOk;
  }

  function validateAdvanced() {
    const invalidRate = state.adv.rates.find((row) => {
      const idx = toNumber(row.index);
      const rate = toNumber(row.dropRate);
      return !isInRange(idx, 0, 9999) || !isInRange(rate, 0, 10000);
    });
    if (invalidRate) return { ok: false, type: 'rate' };

    const invalidSection = state.adv.sections.find((row) => {
      const idx = toNumber(row.index);
      const section = toNumber(row.section);
      const rate = toNumber(row.sectionRate);
      const money = toNumber(row.moneyAmount);
      const opt = toNumber(row.optionValue);
      const dw = toNumber(row.dw);
      const dk = toNumber(row.dk);
      const fe = toNumber(row.fe);
      const mg = toNumber(row.mg);
      return (
        !isInRange(idx, 0, 9999) ||
        !isInRange(section, 0, 99) ||
        !isInRange(rate, 0, 10000) ||
        !Number.isFinite(money) ||
        money < 0 ||
        !Number.isFinite(opt) ||
        opt < 0 ||
        !isInRange(dw, 0, 1) ||
        !isInRange(dk, 0, 1) ||
        !isInRange(fe, 0, 1) ||
        !isInRange(mg, 0, 1)
      );
    });
    if (invalidSection) return { ok: false, type: 'section' };

    const invalidItem = state.adv.items.find((row) => {
      const type = toNumber(row.itemType);
      const index = toNumber(row.itemIndex);
      const level = toNumber(row.itemLevel);
      const grade = toNumber(row.itemGrade);
      const levelRateOk = hasPreset(state.optionPresets.level, row.levelRate);
      const skillRateOk = hasPreset(state.optionPresets.skill, row.skillRate);
      const luckRateOk = hasPreset(state.optionPresets.luck, row.luckRate);
      const optionRateOk = hasPreset(state.optionPresets.option, row.optionRate);
      const excRateOk = hasPreset(state.optionPresets.excellent, row.excellentRate);
      return (
        !isInRange(type, 0, 255) ||
        !isInRange(index, 0, 512) ||
        !isInRange(level, 0, 15) ||
        !isInRange(grade, 0, 255) ||
        !levelRateOk ||
        !skillRateOk ||
        !luckRateOk ||
        !optionRateOk ||
        !excRateOk
      );
    });
    if (invalidItem) return { ok: false, type: 'item' };

    return { ok: true };
  }

  function validateManagerRows() {
    const invalid = state.manager.find((row) => {
      const itemIdxOk = isStarOrNumber(row.itemIndex, 0, 512);
      const levelOk = isStarOrNumber(row.itemLevel, 0, 15);
      const monsterOk = isStarOrNumber(row.monsterClass, 0, 9999);
      const specialOk = isStarOrNumber(row.specialValue, 0, 9999);
      return !itemIdxOk || !levelOk || !monsterOk || !specialOk;
    });
    return !invalid;
  }

  function validateBagData() {
    activateTab('bag');
    const headerOk = validateHeader();
    if (!headerOk) return { ok: false, message: 'Completa los campos generales del bag.' };

    const invalidNormal = state.normalItems.findIndex((item) => !validateNormalItem(item));
    if (invalidNormal >= 0) {
      selectNormalItem(invalidNormal);
      return { ok: false, message: 'Hay items normales con valores inválidos.' };
    }

    if (state.mode === 'advanced') {
      const advResult = validateAdvanced();
      if (!advResult.ok) {
        return { ok: false, message: 'Hay datos avanzados inválidos en el bag.' };
      }
    }

    return { ok: true };
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

  function parseItemToken(token) {
    const raw = String(token || '').trim();
    if (!raw) return { itemType: '0', itemIndex: '0' };
    if (raw.includes(',')) {
      const [type, index] = raw.split(',');
      return { itemType: String(type || '0').trim(), itemIndex: String(index || '0').trim() };
    }
    return { itemType: raw, itemIndex: '0' };
  }

  function itemKey(type, index) {
    const t = String(type ?? '').trim();
    const i = String(index ?? '').trim();
    const tKey = /^\d+$/.test(t) ? String(Number(t)) : t;
    const iKey = /^\d+$/.test(i) ? String(Number(i)) : i;
    return `${tKey}:${iKey}`;
  }

  function parseBag(content) {
    const lines = String(content || '').split(/\r?\n/);
    const normalItems = [];
    const adv = { rates: [], sections: [], items: [] };
    const header = { ...state.header };
    let section = null;
    let hasAdvanced = false;

    for (const rawLine of lines) {
      const trimmed = rawLine.trim();
      if (!trimmed) continue;
      if (trimmed.startsWith('//') || trimmed.startsWith(';')) continue;
      if (/^end$/i.test(trimmed)) {
        section = null;
        continue;
      }
      if (/^\d+$/.test(trimmed)) {
        section = Number.parseInt(trimmed, 10);
        if (section >= 2) hasAdvanced = true;
        continue;
      }

      const commentSplit = rawLine.split('//');
      const dataPart = commentSplit[0].trim();
      const comment = commentSplit.slice(1).join('//').trim();
      if (!dataPart) continue;
      const tokens = dataPart.split(/\s+/);

      if (section === 0) {
        const match = dataPart.match(/^"([^"]+)"\s+(.*)$/);
        if (match) {
          header.eventName = match[1] || 'EventName';
          const rest = match[2].split(/\s+/);
          header.dropZen = rest[0] || header.dropZen;
          header.itemDropRate = rest[1] || header.itemDropRate;
          header.itemDropCount = rest[2] || header.itemDropCount;
          header.itemDropType = rest[3] || header.itemDropType;
          header.fireworks = rest[4] || header.fireworks;
        }
        continue;
      }

      if (section === 1) {
        if (tokens.length < 7) continue;
        const { itemType, itemIndex } = parseItemToken(tokens[0]);
        normalItems.push({
          itemType,
          itemIndex,
          minLvl: tokens[1] || '0',
          maxLvl: tokens[2] || '0',
          skill: tokens[3] || '0',
          luck: tokens[4] || '0',
          opt: tokens[5] || '0',
          exce: tokens[6] || '0',
          comment
        });
        continue;
      }

      if (section === 2) {
        if (tokens.length < 2) continue;
        adv.rates.push({ index: tokens[0] || '0', dropRate: tokens[1] || '0' });
        continue;
      }

      if (section === 3) {
        if (tokens.length < 9) continue;
        adv.sections.push({
          index: tokens[0] || '0',
          section: tokens[1] || '0',
          sectionRate: tokens[2] || '0',
          moneyAmount: tokens[3] || '0',
          optionValue: tokens[4] || '0',
          dw: tokens[5] || '0',
          dk: tokens[6] || '0',
          fe: tokens[7] || '0',
          mg: tokens[8] || '0',
          comment
        });
        continue;
      }

      if (section === 4) {
        if (tokens.length < 9) continue;
        adv.items.push({
          itemType: tokens[0] || '0',
          itemIndex: tokens[1] || '0',
          itemLevel: tokens[2] || '0',
          itemGrade: tokens[3] || '0',
          levelRate: tokens[4] || '0',
          skillRate: tokens[5] || '0',
          luckRate: tokens[6] || '0',
          optionRate: tokens[7] || '0',
          excellentRate: tokens[8] || '0',
          comment
        });
      }
    }

    return { header, normalItems, adv, hasAdvanced };
  }

  function buildNormalContent() {
    const lines = [];
    lines.push('//======================================');
    lines.push('// NORMAL DROP');
    lines.push('//======================================');
    lines.push('');
    lines.push('0');
    lines.push('//EventName  DropZen  ItemDropRate  ItemDropCount  ItemDropType  Fireworks');
    lines.push(
      `"${state.header.eventName}"  ${state.header.dropZen}  ${state.header.itemDropRate}  ${state.header.itemDropCount}  ${state.header.itemDropType}  ${state.header.fireworks}`
    );
    lines.push('end');
    lines.push('');
    lines.push('1');
    lines.push('//Item  MinLvl  MaxLvl  Skill  Luck  Opt  Exce  Comment');
    state.normalItems.forEach((item) => {
      const token = formatItemToken(item.itemType, item.itemIndex);
      const comment = String(item.comment || '').trim();
      const suffix = comment ? `  //${comment}` : '';
      lines.push(
        `${token}  ${item.minLvl}  ${item.maxLvl}  ${item.skill}  ${item.luck}  ${item.opt}  ${item.exce}${suffix}`
      );
    });
    lines.push('end');
    return lines.join('\n');
  }

  function buildAdvancedContent() {
    const lines = [];
    lines.push('//======================================');
    lines.push('// NORMAL DROP');
    lines.push('//======================================');
    lines.push('');
    lines.push('0');
    lines.push('//EventName  DropZen  ItemDropRate  ItemDropCount  ItemDropType  Fireworks');
    lines.push(
      `"${state.header.eventName}"  ${state.header.dropZen}  ${state.header.itemDropRate}  ${state.header.itemDropCount}  ${state.header.itemDropType}  ${state.header.fireworks}`
    );
    lines.push('end');
    lines.push('');
    lines.push('1');
    lines.push('//Item  MinLvl  MaxLvl  Skill  Luck  Opt  Exce  Comment');
    state.normalItems.forEach((item) => {
      const token = formatItemToken(item.itemType, item.itemIndex);
      const comment = String(item.comment || '').trim();
      const suffix = comment ? `  //${comment}` : '';
      lines.push(
        `${token}  ${item.minLvl}  ${item.maxLvl}  ${item.skill}  ${item.luck}  ${item.opt}  ${item.exce}${suffix}`
      );
    });
    lines.push('end');
    lines.push('');
    lines.push('//======================================');
    lines.push('//ADVANCED DROP');
    lines.push('//======================================');
    lines.push('');
    lines.push('2');
    lines.push('//Index  DropRate');
    state.adv.rates.forEach((row) => {
      lines.push(`${row.index}  ${row.dropRate}`);
    });
    lines.push('end');
    lines.push('');
    lines.push('3');
    lines.push('//Index  Section  SectionRate  MoneyAmount  OptionValue  DW  DK  FE  MG');
    state.adv.sections.forEach((row) => {
      const comment = String(row.comment || '').trim();
      const suffix = comment ? `  //${comment}` : '';
      lines.push(
        `${row.index}  ${row.section}  ${row.sectionRate}  ${row.moneyAmount}  ${row.optionValue}  ${row.dw}  ${row.dk}  ${row.fe}  ${row.mg}${suffix}`
      );
    });
    lines.push('end');
    lines.push('');
    lines.push('4');
    lines.push('//ItemType  ItemIndex  ItemLevel  ItemGrade  LevelRate  SkillRate  LuckRate  OptionRate  ExcellentRate  Comment');
    state.adv.items.forEach((row) => {
      const comment = String(row.comment || '').trim();
      const suffix = comment ? `  //${comment}` : '';
      lines.push(
        `${row.itemType}  ${row.itemIndex}  ${row.itemLevel}  ${row.itemGrade}  ${row.levelRate}  ${row.skillRate}  ${row.luckRate}  ${row.optionRate}  ${row.excellentRate}${suffix}`
      );
    });
    lines.push('end');
    return lines.join('\n');
  }

  function convertNormalToAdvanced() {
    if (!state.adv.items.length) {
      state.adv.items = state.normalItems.map((item) => {
        const min = String(item.minLvl || '0').trim();
        const max = String(item.maxLvl || '0').trim();
        const level = min === max ? min : '0';
        const skillRate = String(item.skill) === '1' ? '10000' : '0';
        const luckRate = String(item.luck) === '1' ? '10000' : '0';
        const optionRate = Number.parseInt(item.opt, 10) > 0 ? '10000' : '0';
        const excellentRate = String(item.exce) === '1' ? '10000' : '0';
        return {
          itemType: item.itemType,
          itemIndex: item.itemIndex,
          itemLevel: level,
          itemGrade: '0',
          levelRate: '0',
          skillRate,
          luckRate,
          optionRate,
          excellentRate,
          comment: item.comment || ''
        };
      });
    }
    if (!state.adv.rates.length) {
      state.adv.rates = [{ index: '0', dropRate: '10000' }];
    }
    if (!state.adv.sections.length) {
      state.adv.sections = [
        {
          index: '0',
          section: '4',
          sectionRate: '10000',
          moneyAmount: '0',
          optionValue: '0',
          dw: '1',
          dk: '1',
          fe: '1',
          mg: '1',
          comment: ''
        }
      ];
    }
  }

  function convertAdvancedToNormal() {
    state.normalItems = state.adv.items.map((item) => ({
      itemType: item.itemType,
      itemIndex: item.itemIndex,
      minLvl: item.itemLevel,
      maxLvl: item.itemLevel,
      skill: Number.parseInt(item.skillRate, 10) > 0 ? '1' : '0',
      luck: Number.parseInt(item.luckRate, 10) > 0 ? '1' : '0',
      opt: Number.parseInt(item.optionRate, 10) > 0 ? '1' : '0',
      exce: Number.parseInt(item.excellentRate, 10) > 0 ? '1' : '0',
      comment: item.comment || ''
    }));
  }

  function buildManagerContent() {
    const lines = [];
    lines.push('//ItemIndex\tItemLevel\tMonsterClass\tSpecialValue\tComment');
    state.manager.forEach((row) => {
      const comment = String(row.comment || '').trim();
      const suffix = comment ? `\t//${comment}` : '';
      lines.push(
        `${row.itemIndex}\t${row.itemLevel}\t${row.monsterClass}\t${row.specialValue}${suffix}`
      );
    });
    lines.push('end');
    return lines.join('\n');
  }

  function parseManager(content) {
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
      const tokens = dataPart.split(/\s+/);
      if (tokens.length < 4) continue;
      rows.push({
        itemIndex: tokens[0] || '*',
        itemLevel: tokens[1] || '*',
        monsterClass: tokens[2] || '*',
        specialValue: tokens[3] || '*',
        comment
      });
    }
    return rows;
  }

  function parseOptionRate(content) {
    const sections = {
      0: [],
      1: [],
      2: [],
      3: [],
      4: []
    };
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
      const comment = commentSplit.slice(1).join('//').trim();
      const tokens = dataPart.split(/\s+/);
      if (tokens.length < 2) continue;
      const index = tokens[0];
      const rates = tokens.slice(1).map((value) => String(value).trim());
      sections[current].push({ index, rates, comment });
    }
    return sections;
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
    const options = ['<option value="">Sin Option</option>'];
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

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Bag</label>
          <select id="bag-file"></select>
          <button type="button" id="load-bag">Cargar</button>
          <button type="button" id="save-bag">Guardar</button>
        </div>
        <div class="spawn-toolbar-group">
          <span id="bag-mode" class="muted"></span>
          <button type="button" id="convert-mode" class="btn-danger">Convertir</button>
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" class="tab-btn active" data-tab="bag">Bag</button>
          <button type="button" class="tab-btn" data-tab="manager">Manager</button>
        </div>
      </div>
      <div class="custom-note" id="mode-note"></div>
      <div id="bag-panel" class="custom-panel active"></div>
      <div id="manager-panel" class="custom-panel"></div>
    `;
  }

  function renderModeInfo() {
    const modeEl = root.querySelector('#bag-mode');
    const noteEl = root.querySelector('#mode-note');
    const convertBtn = root.querySelector('#convert-mode');
    if (!modeEl || !noteEl || !convertBtn) return;
    modeEl.textContent = `Modo actual: ${state.mode === 'advanced' ? 'Avanzado' : 'Normal'}`;
    if (state.mode === 'advanced') {
      convertBtn.textContent = 'Convertir a Normal';
      noteEl.textContent =
        'Este bag esta en modo avanzado. Al convertir, se simplifican los items y se pierden rates detallados.';
    } else {
      convertBtn.textContent = 'Convertir a Avanzado';
      noteEl.textContent =
        'Este bag esta en modo normal. Al convertir, se generan secciones avanzadas con valores por defecto.';
    }
  }

  function renderBagPanel() {
    const panel = root.querySelector('#bag-panel');
    if (!panel) return;
    const levelOptions = buildPresetOptions(state.optionPresets.level);
    const skillOptions = buildPresetOptions(state.optionPresets.skill);
    const luckOptions = buildPresetOptions(state.optionPresets.luck);
    const optionOptions = buildPresetOptions(state.optionPresets.option);
    const excellentOptions = buildPresetOptions(state.optionPresets.excellent);
    if (state.mode === 'advanced') {
      panel.innerHTML = `
        <div class="spawn-form">
          <h3>Configuracion general</h3>
          <div class="custom-form-grid">
            <div class="field"><label>EventName</label><input id="bag-event-name" type="text"></div>
            <div class="field"><label>DropZen</label><input id="bag-drop-zen" type="number"></div>
            <div class="field"><label>ItemDropRate</label><input id="bag-drop-rate" type="number"></div>
            <div class="field"><label>ItemDropCount</label><input id="bag-drop-count" type="number"></div>
            <div class="field"><label>ItemDropType</label><input id="bag-drop-type" type="number"></div>
            <div class="field"><label>Fireworks</label><input id="bag-fireworks" type="number"></div>
          </div>
        </div>

        <div class="spawn-editor">
          <div class="spawn-list">
            <h3>Section 2 - Rates</h3>
            <ul id="adv-rate-list" class="simple-list"></ul>
            <button type="button" id="add-adv-rate">Agregar rate</button>
          </div>
          <div class="spawn-form">
            <h3>Detalle rate</h3>
            <div class="custom-form-grid">
              <div class="field"><label>Index</label><input id="adv-rate-index" type="number"></div>
              <div class="field"><label>DropRate</label><input id="adv-rate-value" type="number"></div>
            </div>
            <div class="spawn-actions">
              <button type="button" id="update-adv-rate">Actualizar</button>
              <button type="button" id="delete-adv-rate" class="link-button">Eliminar</button>
            </div>
          </div>
        </div>

        <div class="spawn-editor">
          <div class="spawn-list">
            <h3>Section 3 - Reglas</h3>
            <ul id="adv-section-list" class="simple-list"></ul>
            <button type="button" id="add-adv-section">Agregar regla</button>
          </div>
          <div class="spawn-form">
            <h3>Detalle regla</h3>
            <div class="custom-form-grid">
              <div class="field"><label>Index</label><input id="adv-section-index" type="number"></div>
              <div class="field"><label>Section</label><input id="adv-section-number" type="number"></div>
              <div class="field"><label>SectionRate</label><input id="adv-section-rate" type="number"></div>
              <div class="field"><label>MoneyAmount</label><input id="adv-section-money" type="number"></div>
              <div class="field"><label>OptionValue</label><input id="adv-section-option" type="number"></div>
              <div class="field"><label>Comentario</label><input id="adv-section-comment" type="text"></div>
            </div>
            <div class="custom-form-grid">
              <div class="field"><label>DW</label><input id="adv-section-dw" type="number"></div>
              <div class="field"><label>DK</label><input id="adv-section-dk" type="number"></div>
              <div class="field"><label>FE</label><input id="adv-section-fe" type="number"></div>
              <div class="field"><label>MG</label><input id="adv-section-mg" type="number"></div>
            </div>
            <div class="spawn-actions">
              <button type="button" id="update-adv-section">Actualizar</button>
              <button type="button" id="delete-adv-section" class="link-button">Eliminar</button>
            </div>
          </div>
        </div>

        <div class="spawn-editor">
          <div class="spawn-list">
            <h3>Section 4 - Items</h3>
            <ul id="adv-item-list" class="simple-list"></ul>
            <button type="button" id="add-adv-item">Agregar item</button>
          </div>
          <div class="spawn-form">
            <h3>Detalle item</h3>
            <div class="custom-form-grid">
              <div class="field"><label>ItemType</label><input id="adv-item-type" type="number"></div>
              <div class="field"><label>ItemIndex</label><input id="adv-item-index" type="number"></div>
              <div class="field"><label>ItemLevel</label><input id="adv-item-level" type="number"></div>
              <div class="field"><label>ItemGrade</label><input id="adv-item-grade" type="number"></div>
              <div class="field"><label>LevelRate (preset)</label><select id="adv-item-level-rate">${levelOptions}</select></div>
              <div class="field"><label>SkillRate (preset)</label><select id="adv-item-skill-rate">${skillOptions}</select></div>
              <div class="field"><label>LuckRate (preset)</label><select id="adv-item-luck-rate">${luckOptions}</select></div>
              <div class="field"><label>OptionRate (preset)</label><select id="adv-item-option-rate">${optionOptions}</select></div>
              <div class="field"><label>ExcellentRate (preset)</label><select id="adv-item-exc-rate">${excellentOptions}</select></div>
              <div class="field"><label>Comentario</label><input id="adv-item-comment" type="text"></div>
            </div>
            <div class="item-picker">
              <label>Explorador de items</label>
              <input type="text" id="adv-item-search" placeholder="Buscar item..." />
              <ul id="adv-item-picker" class="simple-list"></ul>
            </div>
            <div class="spawn-actions">
              <button type="button" id="update-adv-item">Actualizar</button>
              <button type="button" id="delete-adv-item" class="link-button">Eliminar</button>
            </div>
          </div>
        </div>
      `;
      bindHeaderInputs();
      bindAdvancedEvents();
      renderAdvancedLists();
    } else {
      panel.innerHTML = `
        <div class="spawn-form">
          <h3>Configuracion general</h3>
          <div class="custom-form-grid">
            <div class="field"><label>EventName</label><input id="bag-event-name" type="text"></div>
            <div class="field"><label>DropZen</label><input id="bag-drop-zen" type="number"></div>
            <div class="field"><label>ItemDropRate</label><input id="bag-drop-rate" type="number"></div>
            <div class="field"><label>ItemDropCount</label><input id="bag-drop-count" type="number"></div>
            <div class="field"><label>ItemDropType</label><input id="bag-drop-type" type="number"></div>
            <div class="field"><label>Fireworks</label><input id="bag-fireworks" type="number"></div>
          </div>
        </div>
        <div class="spawn-editor">
          <div class="spawn-list">
            <h3>Items</h3>
            <ul id="normal-item-list" class="simple-list"></ul>
            <button type="button" id="add-normal-item">Agregar item</button>
          </div>
          <div class="spawn-form">
            <h3>Detalle</h3>
            <div class="custom-form-grid">
              <div class="field"><label>ItemType</label><input id="normal-item-type" type="number"></div>
              <div class="field"><label>ItemIndex</label><input id="normal-item-index" type="number"></div>
              <div class="field"><label>MinLvl</label><input id="normal-item-min" type="number"></div>
              <div class="field"><label>MaxLvl</label><input id="normal-item-max" type="number"></div>
              <div class="field"><label>Skill</label><input id="normal-item-skill" type="number"></div>
              <div class="field"><label>Luck</label><input id="normal-item-luck" type="number"></div>
              <div class="field"><label>Opt</label><input id="normal-item-opt" type="number"></div>
              <div class="field"><label>Exce</label><input id="normal-item-exce" type="number"></div>
              <div class="field"><label>Comentario</label><input id="normal-item-comment" type="text"></div>
            </div>
            <div class="item-picker">
              <label>Explorador de items</label>
              <input type="text" id="normal-item-search" placeholder="Buscar item..." />
              <ul id="normal-item-picker" class="simple-list"></ul>
            </div>
            <div class="spawn-actions">
              <button type="button" id="update-normal-item">Actualizar</button>
              <button type="button" id="delete-normal-item" class="link-button">Eliminar</button>
            </div>
          </div>
        </div>
      `;
      bindHeaderInputs();
      bindNormalEvents();
      renderNormalList();
    }

    fillHeaderInputs();
  }

  function renderManagerPanel() {
    const panel = root.querySelector('#manager-panel');
    if (!panel) return;
    panel.innerHTML = `
      <div class="spawn-editor">
        <div class="spawn-list">
          <h3>EventItemBagManager</h3>
          <ul id="manager-list" class="simple-list"></ul>
          <button type="button" id="add-manager">Agregar regla</button>
        </div>
        <div class="spawn-form">
          <h3>Detalle</h3>
          <div class="custom-form-grid">
            <div class="field"><label>ItemIndex</label><input id="manager-item-index" type="text"></div>
            <div class="field"><label>ItemLevel</label><input id="manager-item-level" type="text"></div>
            <div class="field"><label>MonsterClass</label><input id="manager-monster-class" type="text"></div>
            <div class="field"><label>SpecialValue</label><input id="manager-special-value" type="text"></div>
            <div class="field"><label>Comentario</label><input id="manager-comment" type="text"></div>
          </div>
          <div class="spawn-actions">
            <button type="button" id="update-manager">Actualizar</button>
            <button type="button" id="delete-manager" class="link-button">Eliminar</button>
          </div>
          <div class="spawn-actions">
            <button type="button" id="save-manager">Guardar manager</button>
          </div>
        </div>
      </div>
    `;
    bindManagerEvents();
    renderManagerList();
  }

  function fillHeaderInputs() {
    const fields = [
      ['bag-event-name', 'eventName'],
      ['bag-drop-zen', 'dropZen'],
      ['bag-drop-rate', 'itemDropRate'],
      ['bag-drop-count', 'itemDropCount'],
      ['bag-drop-type', 'itemDropType'],
      ['bag-fireworks', 'fireworks']
    ];
    fields.forEach(([id, key]) => {
      const input = root.querySelector(`#${id}`);
      if (input) input.value = state.header[key] ?? '';
    });
  }

  function bindHeaderInputs() {
    const map = {
      'bag-event-name': 'eventName',
      'bag-drop-zen': 'dropZen',
      'bag-drop-rate': 'itemDropRate',
      'bag-drop-count': 'itemDropCount',
      'bag-drop-type': 'itemDropType',
      'bag-fireworks': 'fireworks'
    };
    Object.entries(map).forEach(([id, key]) => {
      const input = root.querySelector(`#${id}`);
      if (!input) return;
      input.addEventListener('input', () => {
        state.header[key] = String(input.value || '').trim();
      });
    });
  }

  function renderNormalList() {
    const list = root.querySelector('#normal-item-list');
    if (!list) return;
    list.innerHTML = '';
    state.normalItems.forEach((item, index) => {
      const itemDef = state.itemMap.get(itemKey(item.itemType, item.itemIndex));
      const name = itemDef ? itemDef.name : `Item ${item.itemType}:${item.itemIndex}`;
      const li = document.createElement('li');
      li.textContent = `${name} (${item.itemType},${item.itemIndex})`;
      li.className = state.selected.normalItem === index ? 'selected' : '';
      li.addEventListener('click', () => selectNormalItem(index));
      list.appendChild(li);
    });
  }

  function selectNormalItem(index) {
    state.selected.normalItem = index;
    renderNormalList();
    const item = state.normalItems[index];
    if (!item) return;
    const set = (id, value) => {
      const input = root.querySelector(`#${id}`);
      if (input) input.value = value ?? '';
    };
    set('normal-item-type', item.itemType);
    set('normal-item-index', item.itemIndex);
    set('normal-item-min', item.minLvl);
    set('normal-item-max', item.maxLvl);
    set('normal-item-skill', item.skill);
    set('normal-item-luck', item.luck);
    set('normal-item-opt', item.opt);
    set('normal-item-exce', item.exce);
    set('normal-item-comment', item.comment);
  }

  function bindNormalEvents() {
    const addBtn = root.querySelector('#add-normal-item');
    const updateBtn = root.querySelector('#update-normal-item');
    const deleteBtn = root.querySelector('#delete-normal-item');
    const pickerInput = root.querySelector('#normal-item-search');
    const pickerList = root.querySelector('#normal-item-picker');

    addBtn?.addEventListener('click', () => {
      const newItem = {
        itemType: '0',
        itemIndex: '0',
        minLvl: '0',
        maxLvl: '0',
        skill: '0',
        luck: '0',
        opt: '0',
        exce: '0',
        comment: ''
      };
      state.normalItems.push(newItem);
      selectNormalItem(state.normalItems.length - 1);
    });

    updateBtn?.addEventListener('click', () => {
      const idx = state.selected.normalItem;
      if (idx == null) return;
      const item = state.normalItems[idx];
      if (!item) return;
      item.itemType = String(root.querySelector('#normal-item-type')?.value || '0').trim();
      item.itemIndex = String(root.querySelector('#normal-item-index')?.value || '0').trim();
      item.minLvl = String(root.querySelector('#normal-item-min')?.value || '0').trim();
      item.maxLvl = String(root.querySelector('#normal-item-max')?.value || '0').trim();
      item.skill = String(root.querySelector('#normal-item-skill')?.value || '0').trim();
      item.luck = String(root.querySelector('#normal-item-luck')?.value || '0').trim();
      item.opt = String(root.querySelector('#normal-item-opt')?.value || '0').trim();
      item.exce = String(root.querySelector('#normal-item-exce')?.value || '0').trim();
      item.comment = String(root.querySelector('#normal-item-comment')?.value || '').trim();
      renderNormalList();
      setMessage('Item actualizado.', 'success');
    });

    deleteBtn?.addEventListener('click', () => {
      const idx = state.selected.normalItem;
      if (idx == null) return;
      state.normalItems.splice(idx, 1);
      state.selected.normalItem = null;
      renderNormalList();
      setMessage('Item eliminado.', 'success');
    });

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
              const typeInput = root.querySelector('#normal-item-type');
              const indexInput = root.querySelector('#normal-item-index');
              if (typeInput) typeInput.value = item.section;
              if (indexInput) indexInput.value = item.index;
            });
            pickerList.appendChild(li);
          });
      };
      pickerInput.addEventListener('input', renderPicker);
      renderPicker();
    }
  }

  function renderAdvancedLists() {
    renderAdvRateList();
    renderAdvSectionList();
    renderAdvItemList();
  }

  function renderAdvRateList() {
    const list = root.querySelector('#adv-rate-list');
    if (!list) return;
    list.innerHTML = '';
    state.adv.rates.forEach((row, index) => {
      const li = document.createElement('li');
      li.textContent = `Index ${row.index} - ${row.dropRate}`;
      li.className = state.selected.advRate === index ? 'selected' : '';
      li.addEventListener('click', () => selectAdvRate(index));
      list.appendChild(li);
    });
  }

  function selectAdvRate(index) {
    state.selected.advRate = index;
    renderAdvRateList();
    const row = state.adv.rates[index];
    if (!row) return;
    const idxInput = root.querySelector('#adv-rate-index');
    const valInput = root.querySelector('#adv-rate-value');
    if (idxInput) idxInput.value = row.index;
    if (valInput) valInput.value = row.dropRate;
  }

  function renderAdvSectionList() {
    const list = root.querySelector('#adv-section-list');
    if (!list) return;
    list.innerHTML = '';
    state.adv.sections.forEach((row, index) => {
      const li = document.createElement('li');
      li.textContent = `Index ${row.index} -> Section ${row.section}`;
      li.className = state.selected.advSection === index ? 'selected' : '';
      li.addEventListener('click', () => selectAdvSection(index));
      list.appendChild(li);
    });
  }

  function selectAdvSection(index) {
    state.selected.advSection = index;
    renderAdvSectionList();
    const row = state.adv.sections[index];
    if (!row) return;
    const set = (id, value) => {
      const input = root.querySelector(`#${id}`);
      if (input) input.value = value ?? '';
    };
    set('adv-section-index', row.index);
    set('adv-section-number', row.section);
    set('adv-section-rate', row.sectionRate);
    set('adv-section-money', row.moneyAmount);
    set('adv-section-option', row.optionValue);
    set('adv-section-dw', row.dw);
    set('adv-section-dk', row.dk);
    set('adv-section-fe', row.fe);
    set('adv-section-mg', row.mg);
    set('adv-section-comment', row.comment);
  }

  function renderAdvItemList() {
    const list = root.querySelector('#adv-item-list');
    if (!list) return;
    list.innerHTML = '';
    state.adv.items.forEach((row, index) => {
      const itemDef = state.itemMap.get(itemKey(row.itemType, row.itemIndex));
      const name = itemDef ? itemDef.name : `Item ${row.itemType}:${row.itemIndex}`;
      const li = document.createElement('li');
      li.textContent = `${name} (Lv ${row.itemLevel})`;
      li.className = state.selected.advItem === index ? 'selected' : '';
      li.addEventListener('click', () => selectAdvItem(index));
      list.appendChild(li);
    });
  }

  function selectAdvItem(index) {
    state.selected.advItem = index;
    renderAdvItemList();
    const row = state.adv.items[index];
    if (!row) return;
    const set = (id, value) => {
      const input = root.querySelector(`#${id}`);
      if (input) input.value = value ?? '';
    };
    set('adv-item-type', row.itemType);
    set('adv-item-index', row.itemIndex);
    set('adv-item-level', row.itemLevel);
    set('adv-item-grade', row.itemGrade);
    ensurePresetValue(root.querySelector('#adv-item-level-rate'), row.levelRate);
    ensurePresetValue(root.querySelector('#adv-item-skill-rate'), row.skillRate);
    ensurePresetValue(root.querySelector('#adv-item-luck-rate'), row.luckRate);
    ensurePresetValue(root.querySelector('#adv-item-option-rate'), row.optionRate);
    ensurePresetValue(root.querySelector('#adv-item-exc-rate'), row.excellentRate);
    set('adv-item-comment', row.comment);
  }

  function bindAdvancedEvents() {
    root.querySelector('#add-adv-rate')?.addEventListener('click', () => {
      state.adv.rates.push({ index: '0', dropRate: '10000' });
      selectAdvRate(state.adv.rates.length - 1);
    });
    root.querySelector('#update-adv-rate')?.addEventListener('click', () => {
      const idx = state.selected.advRate;
      if (idx == null) return;
      const row = state.adv.rates[idx];
      if (!row) return;
      row.index = String(root.querySelector('#adv-rate-index')?.value || '0').trim();
      row.dropRate = String(root.querySelector('#adv-rate-value')?.value || '0').trim();
      renderAdvRateList();
    });
    root.querySelector('#delete-adv-rate')?.addEventListener('click', () => {
      const idx = state.selected.advRate;
      if (idx == null) return;
      state.adv.rates.splice(idx, 1);
      state.selected.advRate = null;
      renderAdvRateList();
    });

    root.querySelector('#add-adv-section')?.addEventListener('click', () => {
      state.adv.sections.push({
        index: '0',
        section: '4',
        sectionRate: '10000',
        moneyAmount: '0',
        optionValue: '0',
        dw: '1',
        dk: '1',
        fe: '1',
        mg: '1',
        comment: ''
      });
      selectAdvSection(state.adv.sections.length - 1);
    });
    root.querySelector('#update-adv-section')?.addEventListener('click', () => {
      const idx = state.selected.advSection;
      if (idx == null) return;
      const row = state.adv.sections[idx];
      if (!row) return;
      row.index = String(root.querySelector('#adv-section-index')?.value || '0').trim();
      row.section = String(root.querySelector('#adv-section-number')?.value || '0').trim();
      row.sectionRate = String(root.querySelector('#adv-section-rate')?.value || '0').trim();
      row.moneyAmount = String(root.querySelector('#adv-section-money')?.value || '0').trim();
      row.optionValue = String(root.querySelector('#adv-section-option')?.value || '0').trim();
      row.dw = String(root.querySelector('#adv-section-dw')?.value || '0').trim();
      row.dk = String(root.querySelector('#adv-section-dk')?.value || '0').trim();
      row.fe = String(root.querySelector('#adv-section-fe')?.value || '0').trim();
      row.mg = String(root.querySelector('#adv-section-mg')?.value || '0').trim();
      row.comment = String(root.querySelector('#adv-section-comment')?.value || '').trim();
      renderAdvSectionList();
    });
    root.querySelector('#delete-adv-section')?.addEventListener('click', () => {
      const idx = state.selected.advSection;
      if (idx == null) return;
      state.adv.sections.splice(idx, 1);
      state.selected.advSection = null;
      renderAdvSectionList();
    });

    root.querySelector('#add-adv-item')?.addEventListener('click', () => {
      state.adv.items.push({
        itemType: '0',
        itemIndex: '0',
        itemLevel: '0',
        itemGrade: '0',
        levelRate: '0',
        skillRate: '0',
        luckRate: '0',
        optionRate: '0',
        excellentRate: '0',
        comment: ''
      });
      selectAdvItem(state.adv.items.length - 1);
    });
    root.querySelector('#update-adv-item')?.addEventListener('click', () => {
      const idx = state.selected.advItem;
      if (idx == null) return;
      const row = state.adv.items[idx];
      if (!row) return;
      row.itemType = String(root.querySelector('#adv-item-type')?.value || '0').trim();
      row.itemIndex = String(root.querySelector('#adv-item-index')?.value || '0').trim();
      row.itemLevel = String(root.querySelector('#adv-item-level')?.value || '0').trim();
      row.itemGrade = String(root.querySelector('#adv-item-grade')?.value || '0').trim();
      row.levelRate = String(root.querySelector('#adv-item-level-rate')?.value || '0').trim();
      row.skillRate = String(root.querySelector('#adv-item-skill-rate')?.value || '0').trim();
      row.luckRate = String(root.querySelector('#adv-item-luck-rate')?.value || '0').trim();
      row.optionRate = String(root.querySelector('#adv-item-option-rate')?.value || '0').trim();
      row.excellentRate = String(root.querySelector('#adv-item-exc-rate')?.value || '0').trim();
      row.comment = String(root.querySelector('#adv-item-comment')?.value || '').trim();
      renderAdvItemList();
    });
    root.querySelector('#delete-adv-item')?.addEventListener('click', () => {
      const idx = state.selected.advItem;
      if (idx == null) return;
      state.adv.items.splice(idx, 1);
      state.selected.advItem = null;
      renderAdvItemList();
    });

    const pickerInput = root.querySelector('#adv-item-search');
    const pickerList = root.querySelector('#adv-item-picker');
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
              const typeInput = root.querySelector('#adv-item-type');
              const indexInput = root.querySelector('#adv-item-index');
              if (typeInput) typeInput.value = item.section;
              if (indexInput) indexInput.value = item.index;
            });
            pickerList.appendChild(li);
          });
      };
      pickerInput.addEventListener('input', renderPicker);
      renderPicker();
    }
  }

  function renderManagerList() {
    const list = root.querySelector('#manager-list');
    if (!list) return;
    list.innerHTML = '';
    state.manager.forEach((row, index) => {
      const li = document.createElement('li');
      const label = row.comment
        ? `${row.itemIndex} -> ${row.comment}`
        : `${row.itemIndex} (Monster ${row.monsterClass})`;
      li.textContent = label;
      li.className = state.selected.manager === index ? 'selected' : '';
      li.addEventListener('click', () => selectManagerRow(index));
      list.appendChild(li);
    });
  }

  function selectManagerRow(index) {
    state.selected.manager = index;
    renderManagerList();
    const row = state.manager[index];
    if (!row) return;
    const set = (id, value) => {
      const input = root.querySelector(`#${id}`);
      if (input) input.value = value ?? '';
    };
    set('manager-item-index', row.itemIndex);
    set('manager-item-level', row.itemLevel);
    set('manager-monster-class', row.monsterClass);
    set('manager-special-value', row.specialValue);
    set('manager-comment', row.comment);
  }

  function bindManagerEvents() {
    root.querySelector('#add-manager')?.addEventListener('click', () => {
      state.manager.push({
        itemIndex: '*',
        itemLevel: '*',
        monsterClass: '*',
        specialValue: '*',
        comment: ''
      });
      selectManagerRow(state.manager.length - 1);
    });

    root.querySelector('#update-manager')?.addEventListener('click', () => {
      const idx = state.selected.manager;
      if (idx == null) return;
      const row = state.manager[idx];
      if (!row) return;
      row.itemIndex = String(root.querySelector('#manager-item-index')?.value || '*').trim();
      row.itemLevel = String(root.querySelector('#manager-item-level')?.value || '*').trim();
      row.monsterClass = String(root.querySelector('#manager-monster-class')?.value || '*').trim();
      row.specialValue = String(root.querySelector('#manager-special-value')?.value || '*').trim();
      row.comment = String(root.querySelector('#manager-comment')?.value || '').trim();
      renderManagerList();
    });

    root.querySelector('#delete-manager')?.addEventListener('click', () => {
      const idx = state.selected.manager;
      if (idx == null) return;
      state.manager.splice(idx, 1);
      state.selected.manager = null;
      renderManagerList();
    });

    root.querySelector('#save-manager')?.addEventListener('click', saveManager);
  }

  async function loadBagList() {
    const res = await fetch('/admin/server-editor/api/event-item-bag/list');
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || 'No se pudo cargar la lista');
    state.bagFiles = Array.isArray(data.files) ? data.files : [];
    const select = root.querySelector('#bag-file');
    if (!select) return;
    select.innerHTML = '';
    state.bagFiles.forEach((name) => {
      const opt = document.createElement('option');
      opt.value = name;
      opt.textContent = name;
      select.appendChild(opt);
    });
    if (state.bagFiles.length) {
      state.currentFile =
        state.currentFile && state.bagFiles.includes(state.currentFile)
          ? state.currentFile
          : state.bagFiles[0];
      select.value = state.currentFile;
    }
  }

  async function loadBagFile(name) {
    if (!name) return;
    const res = await fetch(`/admin/server-editor/api/event-item-bag/file?name=${encodeURIComponent(name)}`);
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || 'No se pudo cargar el bag');
    const parsed = parseBag(data.content || '');
    state.header = parsed.header;
    state.normalItems = parsed.normalItems;
    state.adv = parsed.adv;
    state.mode = parsed.hasAdvanced ? 'advanced' : 'normal';
    state.selected = {
      normalItem: null,
      advRate: null,
      advSection: null,
      advItem: null,
      manager: state.selected.manager
    };
  }

  async function loadManager() {
    const res = await fetch('/admin/server-editor/api/event-item-bag/manager');
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || 'No se pudo cargar el manager');
    state.manager = parseManager(data.content || '');
  }

  async function saveBag() {
    try {
      const validation = validateBagData();
      if (!validation.ok) {
        setMessage(validation.message, 'error');
        return;
      }
      const content = state.mode === 'advanced' ? buildAdvancedContent() : buildNormalContent();
      const res = await fetch('/admin/server-editor/api/event-item-bag/file', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ name: state.currentFile, content })
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudo guardar');
      const reload = await fetch('/admin/server-editor/api/reload', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ target: 'eventitembag' })
      });
      if (!reload.ok) {
        setMessage('Guardado, pero no se pudo recargar en el server.', 'error');
      } else {
        setMessage('Bag guardado y recargado en el servidor.', 'success');
      }
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  async function saveManager() {
    try {
      activateTab('manager');
      if (!validateManagerRows()) {
        setMessage('Hay reglas del manager con valores inválidos.', 'error');
        return;
      }
      const content = buildManagerContent();
      const res = await fetch('/admin/server-editor/api/event-item-bag/manager', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ content })
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudo guardar');
      const reload = await fetch('/admin/server-editor/api/reload', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ target: 'eventitembag' })
      });
      if (!reload.ok) {
        setMessage('Guardado, pero no se pudo recargar en el server.', 'error');
      } else {
        setMessage('Manager guardado y recargado.', 'success');
      }
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  function bindGlobalEvents() {
    root.querySelector('#load-bag')?.addEventListener('click', async () => {
      try {
        const select = root.querySelector('#bag-file');
        const name = select?.value;
        if (!name) return;
        state.currentFile = name;
        await loadBagFile(name);
        renderModeInfo();
        renderBagPanel();
      } catch (err) {
        setMessage(err.message, 'error');
      }
    });

    root.querySelector('#save-bag')?.addEventListener('click', saveBag);

    root.querySelector('#convert-mode')?.addEventListener('click', () => {
      if (state.mode === 'advanced') {
        if (!confirm('Convertir a modo normal? Se pierden rates detallados.')) return;
        convertAdvancedToNormal();
        state.mode = 'normal';
      } else {
        if (!confirm('Convertir a modo avanzado? Se generan valores por defecto.')) return;
        convertNormalToAdvanced();
        state.mode = 'advanced';
      }
      renderModeInfo();
      renderBagPanel();
    });

    root.querySelectorAll('.tab-btn').forEach((btn) => {
      btn.addEventListener('click', () => {
        root.querySelectorAll('.tab-btn').forEach((b) => b.classList.remove('active'));
        btn.classList.add('active');
        const tab = btn.dataset.tab;
        root.querySelectorAll('.custom-panel').forEach((panel) => {
          panel.classList.toggle('active', panel.id === `${tab}-panel`);
        });
      });
    });
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
    bindGlobalEvents();
    await loadItemDefs();
    await loadOptionPresets();
    try {
      await loadBagList();
      if (state.currentFile) {
        await loadBagFile(state.currentFile);
      }
      await loadManager();
      renderModeInfo();
      renderBagPanel();
      renderManagerPanel();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();

})();
