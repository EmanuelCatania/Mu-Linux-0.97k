// Editor de spawns de todos los mapas (MonsterSetBase.txt o, ya convertidos,
// los Spawns.toml de Data/Maps más las invasiones). La parte visual es
// spawn-visual.js; un solo mapa se edita también desde el editor por mapa.
(function () {
  const root = document.getElementById('monsterset-editor');
  if (!root) return;

  const TYPE_META = window.MuSpawnVisual.TYPE_META;

  const state = {
    maps: [],
    monsters: [],
    entries: [],
    format: 'txt'
  };

  let view = null;

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
    root.innerHTML = window.MuText.html`
      <div class="editor-message alert hidden"></div>
      <div class="spawn-format">
        <span id="spawn-format-label"></span>
        <button type="button" id="convert-spawns" class="hidden">Pasar a archivos por mapa</button>
      </div>
      <div id="spawn-visual"></div>
    `;
  }

  // 'spawns' sólo recrea los monstruos de los mapas que cambiaron; al pasar del
  // .txt a los archivos por mapa se recarga todo ('monster').
  async function reloadSpawns(target = 'spawns') {
    const res = await fetch('/admin/server-editor/api/reload', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ target })
    });
    return res.ok;
  }

  async function convert() {
    if (!window.confirm(window.MuText.t('Los spawns pasan a un archivo por mapa (Data/Maps) y las invasiones a Data/Events/Invasions. MonsterSetBase.txt queda como respaldo y el servidor deja de leerlo. ¿Seguir?'))) {
      return;
    }
    try {
      const res = await fetch('/admin/server-editor/api/monster-set/convert', { method: 'POST' });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || window.MuText.t('No se pudo convertir'));
      state.format = 'toml';
      renderFormat();
      const ok = await reloadSpawns('monster');
      setMessage(window.MuText.t(ok ? 'Convertido y recargado en el servidor.' : 'Convertido, pero no se pudo recargar en el server.'), ok ? 'success' : 'error');
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  async function save() {
    try {
      if (view.findInvalid()) return;
      state.entries = view.getEntries();
      const res = await fetch('/admin/server-editor/api/monster-set', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ content: buildMonsterSet() })
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || window.MuText.t('No se pudo guardar'));
      if (await reloadSpawns()) {
        setMessage(window.MuText.t('Guardado y recargado en el servidor.'), 'success');
      } else {
        setMessage(window.MuText.t('Guardado, pero no se pudo recargar en el server.'), 'error');
      }
    } catch (err) {
      setMessage(err.message, 'error');
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

    if (!mapsRes.ok) throw new Error(mapsData.error || window.MuText.t('No se pudo cargar mapas'));
    if (!monstersRes.ok) throw new Error(monstersData.error || window.MuText.t('No se pudo cargar monstruos'));
    if (!setRes.ok) throw new Error(setData.error || window.MuText.t('No se pudo cargar MonsterSetBase'));

    state.maps = Array.isArray(mapsData.maps) ? mapsData.maps : [];
    state.monsters = Array.isArray(monstersData.monsters) ? monstersData.monsters : [];
    state.entries = parseMonsterSet(setData.content || '');
    state.format = setData.format === 'toml' ? 'toml' : 'txt';
  }

  // De dónde lee los spawns el servidor. Con los archivos por mapa, el panel
  // los convierte al formato de la grilla para editarlos y de vuelta al guardar.
  function renderFormat() {
    const label = root.querySelector('#spawn-format-label');
    const button = root.querySelector('#convert-spawns');
    label.textContent = state.format === 'toml'
      ? window.MuText.t('Formato: un archivo por mapa (Data/Maps) y las invasiones en Data/Events/Invasions.')
      : window.MuText.t('Formato: MonsterSetBase.txt.');
    button.classList.toggle('hidden', state.format === 'toml');
  }

  async function init() {
    buildLayout();
    root.querySelector('#convert-spawns').addEventListener('click', convert);
    try {
      await loadInitialData();
      renderFormat();
      view = window.MuSpawnVisual.create(root.querySelector('#spawn-visual'), {
        maps: state.maps,
        monsters: state.monsters,
        entries: state.entries,
        map: state.maps[0]?.id ?? 0,
        setMessage
      });
      const saveButton = document.createElement('button');
      saveButton.type = 'button';
      saveButton.id = 'save-spawns';
      saveButton.textContent = window.MuText.t('Guardar');
      saveButton.addEventListener('click', save);
      view.toolbar.appendChild(saveButton);
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
