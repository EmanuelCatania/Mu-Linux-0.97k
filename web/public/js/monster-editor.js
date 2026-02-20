(function () {
  const root = document.getElementById('monster-editor');
  if (!root) return;

  const FIELD_ORDER = [
    { key: 'index', label: 'Index' },
    { key: 'rate', label: 'Rate' },
    { key: 'name', label: 'Nombre' },
    { key: 'level', label: 'Level' },
    { key: 'maxLife', label: 'MaxLife' },
    { key: 'maxMana', label: 'MaxMana' },
    { key: 'dmgMin', label: 'DmgMin' },
    { key: 'dmgMax', label: 'DmgMax' },
    { key: 'defense', label: 'Defense' },
    { key: 'mgcDef', label: 'MgcDef' },
    { key: 'atkRate', label: 'AtkRate' },
    { key: 'defRate', label: 'DefRate' },
    { key: 'mvRange', label: 'MvRange' },
    { key: 'atkType', label: 'AtkType' },
    { key: 'atkRange', label: 'AtkRange' },
    { key: 'view', label: 'View' },
    { key: 'mvSpeed', label: 'MvSpeed' },
    { key: 'atkSpeed', label: 'AtkSpeed' },
    { key: 'regen', label: 'Regen' },
    { key: 'attrib', label: 'Attrib' },
    { key: 'itemRate', label: 'ItemRate' },
    { key: 'money', label: 'Money' },
    { key: 'maxItemLevel', label: 'MaxItemLevel' },
    { key: 'skill', label: 'Skill' },
    { key: 'resis1', label: 'Resis1' },
    { key: 'resis2', label: 'Resis2' },
    { key: 'resis3', label: 'Resis3' },
    { key: 'resis4', label: 'Resis4' }
  ];

  const state = {
    monsters: [],
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

  function parseMonsterTxt(content) {
    const lines = String(content || '').split(/\r?\n/);
    const monsters = [];
    for (const rawLine of lines) {
      const line = rawLine.split('//')[0].trim();
      if (!line) continue;
      if (line.toLowerCase() === 'end') break;
      const match = line.match(/^(\d+)\s+(\d+)\s+"([^"]+)"\s+(.*)$/);
      if (!match) continue;
      const tokens = (match[4] || '').match(/\S+/g) || [];
      const values = tokens.map((t) => Number(t));
      const monster = {
        index: Number(match[1]),
        rate: Number(match[2]),
        name: match[3]
      };
      const numericKeys = FIELD_ORDER.slice(3).map((f) => f.key);
      numericKeys.forEach((key, idx) => {
        monster[key] = Number.isFinite(values[idx]) ? values[idx] : 0;
      });
      monsters.push(monster);
    }
    return monsters;
  }

  function buildMonsterTxt() {
    const lines = [];
    lines.push('//Index\tRate\tName\tLevel\tMaxLife\tMaxMan\tDmgMin\tDmgMax\tDefense\tMgcDef\tAtkRate\tDefRate\tMvRange\tAtkType\tAtkRang\tView\tMvSpeed\tAtkSped\tRegen\tAttrib\tItmRate\tMoney\tMItmLvl\tSkill\tResis1\tResis2\tResis3\tResis4');
    const monsters = state.monsters.slice().sort((a, b) => a.index - b.index);
    for (const m of monsters) {
      const row = [
        m.index,
        m.rate,
        `"${String(m.name || '').replace(/\"/g, '')}"`,
        m.level,
        m.maxLife,
        m.maxMana,
        m.dmgMin,
        m.dmgMax,
        m.defense,
        m.mgcDef,
        m.atkRate,
        m.defRate,
        m.mvRange,
        m.atkType,
        m.atkRange,
        m.view,
        m.mvSpeed,
        m.atkSpeed,
        m.regen,
        m.attrib,
        m.itemRate,
        m.money,
        m.maxItemLevel,
        m.skill,
        m.resis1,
        m.resis2,
        m.resis3,
        m.resis4
      ];
      lines.push(row.join('\t'));
    }
    return lines.join('\n');
  }

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Buscar</label>
          <input type="text" id="monster-search" placeholder="Buscar monster..." />
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="add-monster">Agregar</button>
          <button type="button" id="save-monsters">Guardar</button>
        </div>
      </div>
      <div class="spawn-editor">
        <div class="spawn-list">
          <h3>Monsters</h3>
          <ul id="monster-list" class="simple-list"></ul>
        </div>
        <div class="spawn-form">
          <h3>Detalle</h3>
          <div id="monster-form"></div>
          <div class="spawn-actions">
            <button type="button" id="update-monster">Actualizar</button>
            <button type="button" id="delete-monster" class="link-button">Eliminar</button>
          </div>
        </div>
      </div>
    `;
  }

  function renderList() {
    const list = root.querySelector('#monster-list');
    if (!list) return;
    list.innerHTML = '';
    const filter = state.filter.toLowerCase();
    const monsters = state.monsters.slice().sort((a, b) => a.index - b.index);
    monsters.forEach((monster) => {
      const label = `${monster.index} - ${monster.name}`;
      if (filter && !label.toLowerCase().includes(filter)) return;
      const item = document.createElement('li');
      item.textContent = label;
      item.dataset.index = String(monster.index);
      if (state.selected && state.selected.index === monster.index) {
        item.classList.add('selected');
      }
      list.appendChild(item);
    });
  }

  function renderForm(monster) {
    const form = root.querySelector('#monster-form');
    if (!form) return;
    form.innerHTML = '';
    if (!monster) return;
    FIELD_ORDER.forEach((field) => {
      const label = document.createElement('label');
      label.textContent = field.label;
      let input;
      if (field.key === 'name') {
        input = document.createElement('input');
        input.type = 'text';
        input.value = monster.name;
      } else {
        input = document.createElement('input');
        input.type = 'number';
        input.value = monster[field.key] ?? 0;
      }
      input.dataset.key = field.key;
      form.appendChild(label);
      form.appendChild(input);
    });
  }

  function selectMonster(monster) {
    state.selected = monster;
    renderList();
    renderForm(monster);
  }

  function bindEvents() {
    root.querySelector('#monster-search').addEventListener('input', (event) => {
      state.filter = event.target.value || '';
      renderList();
    });

    root.querySelector('#monster-list').addEventListener('click', (event) => {
      const item = event.target.closest('li');
      if (!item) return;
      const index = Number(item.dataset.index);
      const monster = state.monsters.find((m) => m.index === index);
      if (monster) selectMonster(monster);
    });

    root.querySelector('#add-monster').addEventListener('click', () => {
      const nextIndex = state.monsters.reduce((max, m) => Math.max(max, m.index), 0) + 1;
      const monster = {
        index: nextIndex,
        rate: 1,
        name: 'Nuevo Monster',
        level: 1,
        maxLife: 100,
        maxMana: 0,
        dmgMin: 1,
        dmgMax: 2,
        defense: 0,
        mgcDef: 0,
        atkRate: 1,
        defRate: 1,
        mvRange: 1,
        atkType: 0,
        atkRange: 1,
        view: 3,
        mvSpeed: 400,
        atkSpeed: 1600,
        regen: 5,
        attrib: 0,
        itemRate: 100,
        money: 10,
        maxItemLevel: 0,
        skill: 0,
        resis1: 0,
        resis2: 0,
        resis3: 0,
        resis4: 0
      };
      state.monsters.push(monster);
      selectMonster(monster);
      setMessage('Monster agregado.', 'success');
    });

    root.querySelector('#update-monster').addEventListener('click', () => {
      if (!state.selected) return;
      const form = root.querySelector('#monster-form');
      FIELD_ORDER.forEach((field) => {
        const input = form.querySelector(`[data-key="${field.key}"]`);
        if (!input) return;
        if (field.key === 'name') {
          state.selected.name = input.value.trim();
        } else {
          state.selected[field.key] = Number(input.value);
        }
      });
      renderList();
      setMessage('Monster actualizado.', 'success');
    });

    root.querySelector('#delete-monster').addEventListener('click', () => {
      if (!state.selected) return;
      state.monsters = state.monsters.filter((m) => m.index !== state.selected.index);
      state.selected = null;
      renderList();
      renderForm(null);
      setMessage('Monster eliminado.', 'success');
    });

    root.querySelector('#save-monsters').addEventListener('click', async () => {
      try {
        const content = buildMonsterTxt();
        const res = await fetch('/admin/server-editor/api/monster-file', {
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
  }

  async function loadInitialData() {
    const res = await fetch('/admin/server-editor/api/monster-file');
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || 'No se pudo cargar');
    state.monsters = parseMonsterTxt(data.content || '');
  }

  async function init() {
    buildLayout();
    bindEvents();
    try {
      await loadInitialData();
      renderList();
      if (state.monsters[0]) selectMonster(state.monsters[0]);
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
