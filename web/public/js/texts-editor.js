(function () {
  const root = document.getElementById('texts-editor');
  if (!root) return;

  const MAX_ROWS = 300;
  const t = (text, params) => window.MuText.t(text, params);

  const state = {
    lang: 'Spn',
    table: 'items',
    maxBytes: 63,
    rows: [],          // { key, id, reference, label, saved, value }
    filter: '',
    onlyTranslated: false,
    parseErrors: []
  };

  // Los %d/%s/%u/%.0f de un texto del cliente tienen que ser los del inglés, en el
  // mismo orden: el cliente los usa como formato de printf. Misma regla que
  // lib/translations.js (sameFormatSpecifiers) y que ClientText::SetServerText.
  function formatSpecifiers(text) {
    const specs = [];
    const flags = '-+ #0123456789.*hlLzjt';
    for (let i = 0; i < text.length; i += 1) {
      if (text[i] !== '%') continue;
      if (text[i + 1] === '%') { i += 1; continue; }
      let j = i + 1;
      while (j < text.length && flags.includes(text[j])) j += 1;
      if (j < text.length) j += 1;
      specs.push(text.slice(i, j));
      i = j - 1;
    }
    return specs;
  }

  function sameFormat(a, b) {
    const sa = formatSpecifiers(a);
    const sb = formatSpecifiers(b);
    return sa.length === sb.length && sa.every((s, i) => s === sb[i]);
  }

  const byteLength = (text) => new TextEncoder().encode(text).length;

  function problem(row) {
    const value = row.value.trim().normalize('NFC');
    if (value === '') return '';
    if (/["\r\n\t]/.test(value)) return t('No puede llevar comillas, tabulaciones ni saltos de línea');
    if (/[\p{Cc}\p{Cs}\p{Co}\p{Cn}\p{Zl}\p{Zp}]/u.test(value)) return t('Lleva caracteres de control');
    const bytes = byteLength(value);
    if (bytes > state.maxBytes) return t('Mide {bytes} bytes y el máximo es {max}', { bytes, max: state.maxBytes });
    if (state.table === 'client' && state.lang !== 'Eng' && !sameFormat(value, row.reference)) {
      return t('Los %d/%s del texto no coinciden con los del inglés');
    }
    return '';
  }

  const isDirty = (row) => row.value.trim().normalize('NFC') !== row.saved;
  const idLabel = (key) => key.join(' / ');

  function setMessage(text, type) {
    const box = root.querySelector('.editor-message');
    if (!box) return;
    box.textContent = text || '';
    box.classList.toggle('hidden', !text);
    box.classList.toggle('success', type === 'success');
    box.classList.toggle('error', type === 'error');
  }

  function buildLayout() {
    root.innerHTML = window.MuText.html`
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Idioma</label>
          <select id="texts-lang">
            <option value="Eng">English</option>
            <option value="Spn">Español</option>
            <option value="Por">Português</option>
          </select>
        </div>
        <div class="spawn-toolbar-group">
          <label>Tabla</label>
          <select id="texts-table">
            <option value="items">Items</option>
            <option value="monsters">Monstruos y NPC</option>
            <option value="skills">Skills</option>
            <option value="client">Textos del cliente</option>
            <option value="tooltips">Tooltips de items custom</option>
          </select>
        </div>
        <div class="spawn-toolbar-group">
          <label>Buscar</label>
          <input type="text" id="texts-search" placeholder="Buscar por nombre o número..." />
        </div>
        <div class="spawn-toolbar-group">
          <label><input type="checkbox" id="texts-only" /> Solo traducidos</label>
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="texts-save">Guardar</button>
        </div>
      </div>
      <p class="muted" id="texts-hint"></p>
      <table class="texts-table">
        <thead>
          <tr><th>Id</th><th>Referencia</th><th>Traducción</th><th>Bytes</th></tr>
        </thead>
        <tbody id="texts-body"></tbody>
      </table>
      <p class="muted" id="texts-more"></p>
    `;
    root.querySelector('#texts-lang').value = state.lang;
    root.querySelector('#texts-table').value = state.table;
  }

  function matches(row) {
    if (state.onlyTranslated && row.value.trim() === '') return false;
    if (!state.filter) return true;
    const f = state.filter.toLowerCase();
    return row.reference.toLowerCase().includes(f)
      || row.value.toLowerCase().includes(f)
      || row.label.toLowerCase().includes(f)
      || idLabel(row.key).includes(f);
  }

  function updateRowState(row, tr) {
    const msg = problem(row);
    const bytes = byteLength(row.value.trim().normalize('NFC'));
    tr.classList.toggle('texts-dirty', isDirty(row));
    tr.classList.toggle('texts-invalid', msg !== '');
    const counter = tr.querySelector('.texts-bytes');
    counter.textContent = row.value.trim() === '' ? '' : `${bytes}/${state.maxBytes}`;
    counter.title = msg;
    tr.querySelector('input').title = msg;
  }

  function renderRows() {
    const body = root.querySelector('#texts-body');
    body.textContent = '';
    const shown = state.rows.filter(matches);
    shown.slice(0, MAX_ROWS).forEach((row) => {
      const tr = document.createElement('tr');
      const id = document.createElement('td');
      id.textContent = idLabel(row.key);
      const ref = document.createElement('td');
      ref.textContent = row.reference || '—';
      if (row.label) ref.title = row.label;
      const cell = document.createElement('td');
      const input = document.createElement('input');
      input.type = 'text';
      input.value = row.value;
      input.autocomplete = 'off';
      input.spellcheck = false;
      input.addEventListener('input', () => {
        row.value = input.value;
        updateRowState(row, tr);
      });
      cell.appendChild(input);
      const bytes = document.createElement('td');
      bytes.className = 'texts-bytes';
      tr.append(id, ref, cell, bytes);
      updateRowState(row, tr);
      body.appendChild(tr);
    });
    const more = root.querySelector('#texts-more');
    more.textContent = shown.length > MAX_ROWS
      ? t('Mostrando {shown} de {total}. Afiná la búsqueda para ver el resto.', { shown: MAX_ROWS, total: shown.length })
      : '';
    const hint = root.querySelector('#texts-hint');
    hint.textContent = state.table === 'client'
      ? t('Los %d, %s y similares tienen que ser los mismos que en inglés. Si no, el cliente descarta el texto.')
      : t('Dejá vacío para usar el nombre que trae el cliente. Máximo {max} bytes (un acento o una letra no latina ocupan más de uno).', { max: state.maxBytes });
    if (state.parseErrors.length > 0) {
      hint.textContent += ' ' + t('Hay {n} líneas del archivo que no se entienden y no se van a reescribir (la primera es la {line}).',
        { n: state.parseErrors.length, line: state.parseErrors[0].line });
    }
  }

  async function load() {
    setMessage('');
    const res = await fetch(`/admin/server-editor/api/texts?lang=${encodeURIComponent(state.lang)}&table=${encodeURIComponent(state.table)}`);
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || t('No se pudo cargar'));
    state.maxBytes = data.maxBytes || 63;
    state.parseErrors = data.errors || [];
    const saved = new Map(data.translations.map((entry) => [entry.key.join('\t'), entry.name]));
    const rows = data.reference.map((entry) => ({
      key: entry.key,
      reference: entry.name,
      label: entry.label || '',
      saved: saved.get(entry.key.join('\t')) || '',
      value: saved.get(entry.key.join('\t')) || ''
    }));
    // Traducciones de algo que ya no está en los datos del server: se muestran para
    // poder quitarlas.
    const known = new Set(rows.map((row) => row.key.join('\t')));
    data.translations.forEach((entry) => {
      if (known.has(entry.key.join('\t'))) return;
      rows.push({ key: entry.key, reference: '', label: '', saved: entry.name, value: entry.name });
    });
    state.rows = rows;
    renderRows();
  }

  function confirmDiscard() {
    return !state.rows.some(isDirty) || window.confirm(t('Hay cambios sin guardar. ¿Descartarlos?'));
  }

  async function save() {
    const dirty = state.rows.filter(isDirty);
    if (dirty.length === 0) {
      setMessage(t('No hay cambios para guardar.'), 'success');
      return;
    }
    const invalid = dirty.find((row) => problem(row) !== '');
    if (invalid) {
      setMessage(t('Hay textos inválidos: {id}. Corregilos antes de guardar.', { id: idLabel(invalid.key) }), 'error');
      return;
    }
    const edits = dirty.map((row) => ({ key: row.key, name: row.value.trim().normalize('NFC') }));
    const res = await fetch('/admin/server-editor/api/texts', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ lang: state.lang, table: state.table, edits })
    });
    const data = await res.json();
    if (!res.ok) {
      const detail = Array.isArray(data.errors) && data.errors.length > 0
        ? ` ${data.errors.slice(0, 3).map((e) => `${e.key ? idLabel(e.key) : '?'}: ${e.error}`).join('; ')}`
        : '';
      throw new Error((data.error || t('No se pudo guardar')) + detail);
    }
    dirty.forEach((row) => { row.saved = row.value.trim().normalize('NFC'); row.value = row.saved; });
    renderRows();
    // Los textos viajan al cliente junto con el resto del contenido ('item').
    const reload = await fetch('/admin/server-editor/api/reload', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ target: 'item' })
    });
    setMessage(reload.ok
      ? t('Guardado y recargado en el servidor. Los jugadores lo ven al volver a entrar.')
      : t('Guardado, pero no se pudo recargar en el server.'), reload.ok ? 'success' : 'error');
  }

  function bindEvents() {
    const reloadList = async (apply) => {
      try {
        if (!confirmDiscard()) { buildSelectors(); return; }
        apply();
        await load();
      } catch (err) {
        setMessage(err.message, 'error');
      }
    };
    const buildSelectors = () => {
      root.querySelector('#texts-lang').value = state.lang;
      root.querySelector('#texts-table').value = state.table;
    };
    root.querySelector('#texts-lang').addEventListener('change', (e) => reloadList(() => { state.lang = e.target.value; }));
    root.querySelector('#texts-table').addEventListener('change', (e) => reloadList(() => { state.table = e.target.value; }));
    root.querySelector('#texts-search').addEventListener('input', (e) => { state.filter = e.target.value.trim(); renderRows(); });
    root.querySelector('#texts-only').addEventListener('change', (e) => { state.onlyTranslated = e.target.checked; renderRows(); });
    root.querySelector('#texts-save').addEventListener('click', () => save().catch((err) => setMessage(err.message, 'error')));
    window.addEventListener('beforeunload', (e) => {
      if (state.rows.some(isDirty)) { e.preventDefault(); e.returnValue = ''; }
    });
  }

  async function init() {
    buildLayout();
    bindEvents();
    try {
      await load();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
