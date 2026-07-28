(function () {
  const root = document.getElementById('cfg-editor');
  if (!root) return;

  const apiGet = root.dataset.apiGet || '/admin/server-editor/api/common';
  const apiPost = root.dataset.apiPost || '/admin/server-editor/api/common';
  const reloadTarget = root.dataset.reload || 'common';

  const KEY_HELP = {
    CheckSpeedHack: 'Controla la velocidad de ataque y magia.',
    CheckSpeedHackTolerance: 'Tolerancia cuando CheckSpeedHack=1.',
    CheckLatencyHack: 'Valida latencia sospechosa.',
    CheckLatencyHackTolerance: 'Tolerancia cuando CheckLatencyHack=1.',
    CheckAutoPotionHack: 'Valida consumo automatico de pociones.',
    CheckAutoPotionHackTolerance: 'Tolerancia cuando CheckAutoPotionHack=1.'
  };

  const state = {
    raw: '',
    lineEnding: '\n',
    entries: [],
    sections: [],
    originalRaw: '',
    visibleSections: []
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

  function translateSectionName(name) {
    const map = {
      GameServerInfo: 'General',
      'Hack Settings': 'Anti-Hack',
      'Common Settings': 'General',
      'Monster Settings': 'Monstruos',
      'PK Settings': 'PK',
      'Guild Settings': 'Guild',
      'Guild War/Soccer Settings': 'Guild War / Soccer',
      'Experience Settings': 'Experiencia',
      'Item Drop Settings': 'Drop de Items',
      'Money Drop Settings': 'Drop de Zen',
      'Shop Settings': 'Shop',
      'Event Settings': 'Eventos',
      'Character Settings': 'Personajes',
      'Inventory Settings': 'Inventario',
      'Connection Settings': 'Conexion',
      'Chaos Item Mix Settings': 'Chaos Mix - Items',
      'Devil Square Mix Settings': 'Devil Square - Entradas',
      '+10/+11 Plus Item Mix Settings': 'Mejora +10/+11',
      'Dinorant Mix Settings': 'Mezcla Dinorant',
      'Fruit Mix Settings': 'Mezcla Frutas',
      'Wing1 Mix Settings': 'Mezcla Ala 1',
      'Blood Castle Mix Settings': 'Blood Castle - Entradas',
      'Wing2 Mix Settings': 'Mezcla Ala 2'
    };
    return map[name] || name;
  }

  function getSectionTitle(text) {
    const trimmed = text.replace(/^;+/, '').trim();
    if (!trimmed) return null;
    const compact = trimmed.replace(/\s+/g, '');
    if (/^=+$/.test(compact) || /^-+$/.test(compact)) return null;
    if (/[=]/.test(trimmed) || /->/.test(trimmed)) return null;
    const lower = trimmed.toLowerCase();
    if (lower.includes('settings') || lower.includes('manager') || lower.includes('config')) {
      return trimmed;
    }
    return null;
  }

  function translateComment(text) {
    const trimmed = text.trim();
    const cleaned = trimmed.replace(/^=+\s*/, '').trim();
    if (!cleaned) return '';
    if (/^=+$/.test(cleaned.replace(/\s+/g, ''))) return '';
    const directMap = {
      'Check Physic and Magic Speed (0 = No / 1 = Yes)':
        'Verifica velocidad de ataque/magia (0 = No / 1 = Si).',
      'The tolerance for the SpeedHack when CheckSpeedHack=1':
        'Tolerancia cuando CheckSpeedHack=1.',
      'Check Latency (0 = No / 1 = Yes)':
        'Verifica latencia (0 = No / 1 = Si).',
      'The tolerance for the Latency when CheckLatencyHack=1':
        'Tolerancia cuando CheckLatencyHack=1.',
      'Check Potions consumption (0 = No / 1 = Yes)':
        'Verifica consumo de pociones (0 = No / 1 = Si).',
      'The tolerance for consuming potions when CheckAutoPotionHack=1':
        'Tolerancia cuando CheckAutoPotionHack=1.',
      'Maximum Jewel of Life additional option':
        'Maximo de opcion adicional para Jewel of Life.',
      'Maximum IPs per computer':
        'Maximo de IPs por computadora.',
      'Disconnect the account when trying to connect to the same account (0 = No / 1 = Yes)':
        'Desconectar al intentar entrar con la misma cuenta (0 = No / 1 = Si).',
      'Check the personal code (0 = No / 1 = Yes)':
        'Verifica el codigo personal (0 = No / 1 = Si).',
      'Rate of life of all Monsters':
        'Multiplicador de vida de todos los monstruos.',
      'Send the monster\'s HP for the client\'s Health Bar (0 = No / 1 = Yes)':
        'Enviar HP del monstruo al cliente (0 = No / 1 = Si).',
      'Disable player vs player? (0 = No / 1 = Yes)':
        'Deshabilitar PVP (0 = No / 1 = Si).',
      'Enable PKs to Move and Talk to NPCs (0 = No / 1 = Yes)':
        'Permite PK para moverse y hablar con NPCs (0 = No / 1 = Si).',
      'Time to decrease the Hero status (In Seconds)':
        'Tiempo para bajar estado Hero (en segundos).',
      'Time to decrease the PK status (In Seconds)':
        'Tiempo para bajar estado PK (en segundos).',
      'Enable Guild Creation (0 = No / 1 = Yes)':
        'Permite crear guild (0 = No / 1 = Si).',
      'Enable Guild Delete (0 = No / 1 = Yes)':
        'Permite borrar guild (0 = No / 1 = Si).',
      'Min Level required to create guild':
        'Nivel minimo para crear guild.',
      'Min Reset required to create guild':
        'Reset minimo para crear guild.',
      'Maximum users to insert in the guild (MAX: 40)':
        'Maximo de usuarios en guild (MAX: 40).',
      'Score needed to win the guild war':
        'Puntaje necesario para ganar guild war.',
      'Score needed to win the battle soccer':
        'Puntaje necesario para ganar battle soccer.',
      'Points gained when enemy guild score is 0':
        'Puntos ganados cuando el enemigo tiene 0.',
      'Points gained when enemy guild score is less than half':
        'Puntos ganados cuando el enemigo tiene menos de la mitad.',
      'Points gained in normal win':
        'Puntos ganados en victoria normal.',
      'Experience rate gained in the server when killing monsters':
        'Experiencia ganada al matar monstruos.',
      'Experience rate gained in events reward (Devil Square, Blood Castle)':
        'Experiencia por recompensas de eventos (DS/BC).',
      'Time of the item on the ground (In Seconds)':
        'Tiempo del item en el suelo (segundos).',
      'Rate of the drop of the items':
        'Tasa de drop de items.',
      'Time of the money on the ground (In Seconds)':
        'Tiempo del zen en el suelo (segundos).',
      'Rate of the amount of money dropped':
        'Tasa de cantidad de zen dropeado.',
      '-1 = (Total SellPrice of every item inside ChaosBox) / 20000':
        '-1 = (Precio total de venta de los items en Chaos Box) / 20000.',
      '-1 = (Total SellPrice of every additional item inside ChaosBox) / 20000':
        '-1 = (Precio total de venta de los items adicionales en Chaos Box) / 20000.',
      '-1 = (Total SellPrice of every jewel inside ChaosBox) / 20000':
        '-1 = (Precio total de venta de las jewels en Chaos Box) / 20000.',
      '-1 = (Total SellPrice of every additional item inside ChaosBox) / 40000':
        '-1 = (Precio total de venta de los items adicionales en Chaos Box) / 40000.',
      '-1 = (Wing SellPrice) / 4000000':
        '-1 = (Precio de venta del ala) / 4000000.',
      '-1 = Rate * 10000':
        '-1 = Tasa * 10000.',
      '0 ~ 100':
        '0 a 100.',
      '0 ~ MAXMONEY':
        '0 a MAXMONEY.',
      'Additional Mix Rate when Item is added':
        'Tasa adicional cuando se agrega un item.',
      'Additional Mix Rate when Jewel is added':
        'Tasa adicional cuando se agrega una jewel.',
      'CustomMixRate: Rate of generating a random Custom Wing':
        'CustomMixRate: probabilidad de generar un ala custom.',
      '0 = Never generate Custom Wings (Always generate S2 Only)':
        '0 = Nunca genera alas custom (siempre genera S2).',
      '100 = Never generate S2 Wings (Always generate Custom Wings Only)':
        '100 = Nunca genera alas S2 (solo genera custom).'
    };
    if (directMap[cleaned]) return directMap[cleaned];
    let t = cleaned;
    t = t.replace(/\(0\s*=\s*No\s*\/\s*1\s*=\s*Yes\)/gi, '(0 = No / 1 = Si)');
    t = t.replace(/In Seconds/gi, 'en segundos');
    t = t.replace(/Maximum/gi, 'Maximo');
    t = t.replace(/Minimum/gi, 'Minimo');
    t = t.replace(/Experience/gi, 'Experiencia');
    t = t.replace(/Rate/gi, 'Tasa');
    t = t.replace(/Time/gi, 'Tiempo');
    t = t.replace(/Money/gi, 'Zen');
    t = t.replace(/Character/gi, 'Personaje');
    t = t.replace(/Level/gi, 'Nivel');
    return t.trim();
  }

  function isBooleanEntry(entry) {
    const val = String(entry.value || '').trim();
    if (val !== '0' && val !== '1') return false;
    if (/^(Check|Enable|Disable|Use|Allow|Switch|Personal|NonPK)/i.test(entry.key)) return true;
    if (/Switch$|Enable$|Disable$/i.test(entry.key)) return true;
    return false;
  }

  function isNumberEntry(entry) {
    return /^-?\d+(\.\d+)?$/.test(String(entry.value || '').trim());
  }

  function getUnit(key) {
    if (/Rate|Percent|Experience/i.test(key)) return '%';
    if (/Time|Seconds|Duration/i.test(key)) return 'seg';
    if (/Money|Zen/i.test(key)) return 'zen';
    if (/Level/i.test(key)) return 'lvl';
    if (/Count|Max|Min|Point/i.test(key)) return '#';
    return '';
  }

  function getVipTag(key) {
    if (/_AL0$/.test(key)) return 'Normal';
    if (/_AL1$/.test(key)) return 'VIP 1';
    if (/_AL2$/.test(key)) return 'VIP 2';
    if (/_AL3$/.test(key)) return 'VIP 3';
    return null;
  }

  function parseCommon(content) {
    const entries = [];
    const sections = [];
    const lines = String(content || '').split(/\r?\n/);
    const lineEnding = content.includes('\r\n') ? '\r\n' : '\n';
    let pendingComments = [];
    let currentSection = 'GameServerInfo';

    for (let i = 0; i < lines.length; i += 1) {
      const rawLine = lines[i];
      const trimmed = rawLine.trim();
      if (!trimmed) {
        pendingComments = [];
        continue;
      }
      if (trimmed.startsWith('[') && trimmed.endsWith(']')) {
        currentSection = trimmed.replace(/[\[\]]/g, '').trim() || 'GameServerInfo';
        if (!sections.includes(currentSection)) sections.push(currentSection);
        pendingComments = [];
        continue;
      }
      if (trimmed.startsWith(';')) {
        const text = trimmed.replace(/^;+\s?/, '').trim();
        const sectionTitle = getSectionTitle(text);
        if (sectionTitle) {
          currentSection = sectionTitle;
          if (!sections.includes(currentSection)) sections.push(currentSection);
          pendingComments = [];
        } else if (text) {
          const translated = translateComment(text);
          if (translated) pendingComments.push(translated);
        }
        continue;
      }

      const match = trimmed.match(/^([A-Za-z0-9_]+)\s*=\s*(.+)$/);
      if (!match) {
        pendingComments = [];
        continue;
      }
      const key = match[1];
      const rawValue = match[2].trim();
      let value = rawValue;
      let inlineComment = '';
      const semiIndex = rawValue.indexOf(';');
      const slashIndex = rawValue.indexOf('//');
      let commentIndex = -1;
      let commentToken = '';
      if (semiIndex >= 0) {
        commentIndex = semiIndex;
        commentToken = ';';
      }
      if (slashIndex >= 0 && (commentIndex === -1 || slashIndex < commentIndex)) {
        commentIndex = slashIndex;
        commentToken = '//';
      }
      if (commentIndex >= 0) {
        inlineComment = rawValue.slice(commentIndex + commentToken.length).trim();
        value = rawValue.slice(0, commentIndex).trim();
      }
      const helpParts = [];
      if (pendingComments.length) helpParts.push(pendingComments.join(' '));
      if (inlineComment) {
        const translatedInline = translateComment(inlineComment);
        if (translatedInline) helpParts.push(translatedInline);
      }
      let help = helpParts.join(' ');
      if (KEY_HELP[key]) help = KEY_HELP[key];
      if (!help && isBooleanEntry({ key, value })) {
        help = '0 = No / 1 = Si';
      } else if (!help && /Rate|Experience/i.test(key)) {
        help = '100 = 100%';
      }
      entries.push({
        key,
        value,
        originalValue: value,
        inlineComment,
        lineIndex: i,
        section: currentSection,
        help,
        vipTag: getVipTag(key)
      });
      if (!sections.includes(currentSection)) sections.push(currentSection);
      pendingComments = [];
    }

    state.lineEnding = lineEnding;
    state.entries = entries;
    state.sections = sections;
  }

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Buscar</label>
          <input type="text" id="cfg-search" placeholder="Buscar config..." />
          <span class="cfg-dirty" id="cfg-dirty">Sin cambios</span>
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="reset-cfg" class="btn-danger">Restaurar valores</button>
          <button type="button" id="save-cfg">Guardar</button>
        </div>
      </div>
      <div class="cfg-layout">
        <aside class="cfg-sidebar">
          <div class="cfg-sidebar-title">Secciones</div>
          <input type="text" id="cfg-nav-search" placeholder="Filtrar secciones..." />
          <ul id="cfg-nav"></ul>
        </aside>
        <div class="cfg-sections" id="cfg-sections"></div>
      </div>
    `;
  }

  function renderSection(sectionName, entries, sectionId) {
    const section = document.createElement('div');
    section.className = 'cfg-section-card';
    section.id = sectionId;
    const header = document.createElement('div');
    header.className = 'cfg-section-title';
    const title = document.createElement('span');
    title.textContent = translateSectionName(sectionName);
    const actions = document.createElement('div');
    actions.className = 'cfg-section-actions';
    const resetBtn = document.createElement('button');
    resetBtn.type = 'button';
    resetBtn.className = 'cfg-reset';
    resetBtn.textContent = 'Restaurar seccion';
    resetBtn.addEventListener('click', () => {
      state.entries
        .filter((entry) => entry.section === sectionName)
        .forEach((entry) => {
          entry.value = entry.originalValue;
        });
      renderList(root.querySelector('#cfg-search').value || '');
      updateDirtyBadge();
      setMessage('Seccion restaurada.', 'success');
    });
    actions.appendChild(resetBtn);
    header.appendChild(title);
    header.appendChild(actions);
    section.appendChild(header);

    entries.forEach((entry) => {
      const item = document.createElement('div');
      item.className = 'cfg-item-row';
      if (entry.value !== entry.originalValue) {
        item.classList.add('dirty');
      }

      const meta = document.createElement('div');
      meta.className = 'cfg-meta';
      const key = document.createElement('div');
      key.className = 'cfg-key';
      key.textContent = entry.key;
      meta.appendChild(key);

      if (entry.help) {
        const help = document.createElement('div');
        help.className = 'cfg-help';
        help.textContent = entry.help;
        meta.appendChild(help);
      }

      if (entry.vipTag) {
        const tag = document.createElement('span');
        tag.className = 'cfg-tag';
        tag.textContent = entry.vipTag;
        meta.appendChild(tag);
      }

      item.appendChild(meta);

      const inputWrap = document.createElement('div');
      inputWrap.className = 'cfg-input';
      let input;
      if (isBooleanEntry(entry)) {
        const toggle = document.createElement('label');
        toggle.className = 'cfg-switch';
        input = document.createElement('input');
        input.type = 'checkbox';
        input.checked = String(entry.value) === '1';
        input.dataset.key = entry.key;
        const slider = document.createElement('span');
        slider.className = 'cfg-slider';
        toggle.appendChild(input);
        toggle.appendChild(slider);
        inputWrap.appendChild(toggle);
      } else {
        input = document.createElement('input');
        input.type = isNumberEntry(entry) ? 'number' : 'text';
        input.value = entry.value;
        input.dataset.key = entry.key;
        inputWrap.appendChild(input);
        const unit = getUnit(entry.key);
        if (unit) {
          const unitEl = document.createElement('span');
          unitEl.className = 'cfg-unit';
          unitEl.textContent = unit;
          inputWrap.appendChild(unitEl);
        }
      }
      input.addEventListener('input', () => {
        entry.value = isBooleanEntry(entry) ? (input.checked ? '1' : '0') : input.value;
        item.classList.toggle('dirty', entry.value !== entry.originalValue);
        updateDirtyBadge();
      });
      const reset = document.createElement('button');
      reset.type = 'button';
      reset.className = 'cfg-reset';
      reset.textContent = 'Restaurar';
      reset.addEventListener('click', () => {
        entry.value = entry.originalValue;
        if (isBooleanEntry(entry)) {
          input.checked = String(entry.value) === '1';
        } else {
          input.value = entry.value;
        }
        item.classList.remove('dirty');
        updateDirtyBadge();
      });
      inputWrap.appendChild(reset);
      item.appendChild(inputWrap);
      section.appendChild(item);
    });

    return section;
  }

  function renderList(filterText) {
    const container = root.querySelector('#cfg-sections');
    if (!container) return;
    container.innerHTML = '';
    const filter = (filterText || '').toLowerCase();
    const bySection = new Map();

    state.entries.forEach((entry) => {
      const matchesFilter =
        !filter ||
        entry.key.toLowerCase().includes(filter) ||
        entry.value.toLowerCase().includes(filter) ||
        (entry.help || '').toLowerCase().includes(filter) ||
        (entry.section || '').toLowerCase().includes(filter);
      if (!matchesFilter) return;
      if (!bySection.has(entry.section)) bySection.set(entry.section, []);
      bySection.get(entry.section).push(entry);
    });

    const frag = document.createDocumentFragment();
    const sectionNames = filter ? Array.from(bySection.keys()) : state.sections.slice();
    state.visibleSections = sectionNames.slice();
    sectionNames.forEach((sectionName, index) => {
      const entries = bySection.get(sectionName) || [];
      if (!entries.length) return;
      const sectionId = `cfg-section-${sectionName.toLowerCase().replace(/[^a-z0-9]+/g, '-')}-${index}`;
      frag.appendChild(renderSection(sectionName, entries, sectionId));
    });
    container.appendChild(frag);
    renderSidebar(sectionNames);
  }

  function renderSidebar(sectionNames) {
    const nav = root.querySelector('#cfg-nav');
    if (!nav) return;
    nav.innerHTML = '';
    const frag = document.createDocumentFragment();
    const filterValue = String(root.querySelector('#cfg-nav-search')?.value || '').toLowerCase();
    sectionNames.forEach((name, index) => {
      if (filterValue && !translateSectionName(name).toLowerCase().includes(filterValue)) {
        return;
      }
      const li = document.createElement('li');
      const button = document.createElement('button');
      button.type = 'button';
      button.textContent = translateSectionName(name);
      button.addEventListener('click', () => {
        const el = document.getElementById(`cfg-section-${name.toLowerCase().replace(/[^a-z0-9]+/g, '-')}-${index}`);
        if (el) el.scrollIntoView({ behavior: 'smooth', block: 'start' });
      });
      li.appendChild(button);
      frag.appendChild(li);
    });
    nav.appendChild(frag);
  }

  function updateDirtyBadge() {
    const badge = root.querySelector('#cfg-dirty');
    if (!badge) return;
    const dirtyCount = state.entries.filter((entry) => entry.value !== entry.originalValue).length;
    badge.textContent = dirtyCount ? `Cambios: ${dirtyCount}` : 'Sin cambios';
  }

  function buildContent() {
    const lines = state.raw.split(/\r?\n/);
    state.entries.forEach((entry) => {
      const suffix = entry.inlineComment ? ` ; ${entry.inlineComment}` : '';
      lines[entry.lineIndex] = `${entry.key}=${String(entry.value).trim()}${suffix}`;
    });
    return lines.join(state.lineEnding);
  }

  function bindEvents() {
    root.querySelector('#cfg-search').addEventListener('input', (event) => {
      renderList(event.target.value || '');
    });

    root.querySelector('#cfg-nav-search').addEventListener('input', () => {
      renderSidebar(state.visibleSections.length ? state.visibleSections : state.sections.slice());
    });

    root.querySelector('#reset-cfg').addEventListener('click', () => {
      if (!state.originalRaw) return;
      const ok = confirm('Restaurar los valores del archivo? Se perderan cambios sin guardar.');
      if (!ok) return;
      state.raw = state.originalRaw;
      parseCommon(state.raw);
      root.querySelector('#cfg-search').value = '';
      renderList('');
      updateDirtyBadge();
      setMessage('Valores restaurados desde el archivo.', 'success');
    });

    root.querySelector('#save-cfg').addEventListener('click', async () => {
      try {
        const content = buildContent();
        const res = await fetch(apiPost, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ content })
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'No se pudo guardar');
        state.raw = content;
        state.originalRaw = content;
        state.entries.forEach((entry) => {
          entry.originalValue = entry.value;
        });
        updateDirtyBadge();
        const reload = await fetch('/admin/server-editor/api/reload', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ target: reloadTarget })
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
    const res = await fetch(apiGet);
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || 'No se pudo cargar');
    state.raw = data.content || '';
    state.originalRaw = state.raw;
    parseCommon(state.raw);
  }

  async function init() {
    buildLayout();
    bindEvents();
    try {
      await loadInitialData();
      renderList('');
      updateDirtyBadge();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
