(function () {
  const root = document.getElementById('notice-editor');
  if (!root) return;

  const HEADER = `//=============================================================================================================================================\n// Message:\n//\tMax 127 bytes in UTF-8 (spaces included; an accent takes 2 bytes)\n// Type:\n//\t0 -> Global Gold Center\n//\t1 -> Global Green Center\n// Repeat Time:\n//\tIn seconds\n//=============================================================================================================================================\n\n//Message\t\t\t\t\tType\tRepeatTime`;

  // Límites del server (GameServer/Notice.h): el mensaje entra en 128 bytes con el 0
  // final y se leen hasta 100 avisos. Se cuentan bytes de UTF-8, no caracteres.
  const MAX_BYTES = 127;
  const MAX_NOTICES = 100;
  const byteLength = (text) => new TextEncoder().encode(String(text)).length;

  const state = {
    notices: [],
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

  function parseNotice(content) {
    const lines = String(content || '').split(/\r?\n/);
    const notices = [];
    for (const rawLine of lines) {
      const line = rawLine.split('//')[0].trim();
      if (!line) continue;
      if (line.toLowerCase() === 'end') break;
      const match = line.match(/"([^"]+)"\s+(\d+)\s+(\d+)/);
      if (!match) continue;
      notices.push({
        message: match[1],
        type: Number(match[2]),
        repeat: Number(match[3])
      });
    }
    return notices;
  }

  function buildNotice() {
    const lines = [];
    lines.push(HEADER);
    const notices = state.notices.slice();
    for (const notice of notices) {
      const msg = String(notice.message || '').replace(/"/g, '').trim();
      lines.push(`"${msg}"\t\t\t\t${notice.type}\t${notice.repeat}`);
    }
    lines.push('end');
    return lines.join('\n');
  }

  function buildLayout() {
    root.innerHTML = window.MuText.html`
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Buscar</label>
          <input type="text" id="notice-search" placeholder="Buscar mensaje..." />
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="add-notice">Agregar notice</button>
          <button type="button" id="save-notice">Guardar</button>
        </div>
      </div>
      <div class="spawn-editor">
        <div class="spawn-list">
          <h3>Notices</h3>
          <ul id="notice-list" class="simple-list"></ul>
        </div>
        <div class="spawn-form">
          <h3>Detalle</h3>
          <label>Mensaje</label>
          <input type="text" id="notice-message" />
          <small id="notice-bytes" class="muted"></small>
          <label>Tipo</label>
          <select id="notice-type">
            <option value="0">0 - Global Gold Center</option>
            <option value="1">1 - Global Green Center</option>
          </select>
          <label>Repeat (seg)</label>
          <input type="number" id="notice-repeat" min="5" />
          <div class="spawn-actions">
            <button type="button" id="update-notice">Actualizar</button>
            <button type="button" id="delete-notice" class="link-button">Eliminar</button>
          </div>
        </div>
      </div>
    `;
  }

  function renderList() {
    const list = root.querySelector('#notice-list');
    if (!list) return;
    list.innerHTML = '';
    const filter = state.filter.toLowerCase();
    state.notices.forEach((notice, index) => {
      const label = `${notice.message} (Tipo ${notice.type}, ${notice.repeat}s)`;
      if (filter && !label.toLowerCase().includes(filter)) return;
      const item = document.createElement('li');
      item.textContent = label;
      item.dataset.index = String(index);
      if (state.selected && state.selected.index === index) {
        item.classList.add('selected');
      }
      list.appendChild(item);
    });
  }

  // Contador de bytes del mensaje, en rojo si pasa el máximo.
  function updateByteCounter() {
    const input = root.querySelector('#notice-message');
    const counter = root.querySelector('#notice-bytes');
    if (!input || !counter) return;
    const bytes = byteLength(input.value.trim());
    counter.textContent = `${bytes}/${MAX_BYTES} bytes`;
    counter.style.color = bytes > MAX_BYTES ? '#BD382D' : '';
  }

  function selectNotice(notice, index) {
    state.selected = notice ? { ...notice, index } : null;
    if (!notice) {
      renderList();
      return;
    }
    root.querySelector('#notice-message').value = notice.message;
    updateByteCounter();
    root.querySelector('#notice-type').value = notice.type;
    root.querySelector('#notice-repeat').value = notice.repeat;
    renderList();
  }

  function bindEvents() {
    root.querySelector('#notice-search').addEventListener('input', (event) => {
      state.filter = event.target.value || '';
      renderList();
    });

    root.querySelector('#notice-list').addEventListener('click', (event) => {
      const item = event.target.closest('li');
      if (!item) return;
      const index = Number(item.dataset.index);
      const notice = state.notices[index];
      if (notice) selectNotice(notice, index);
    });

    root.querySelector('#add-notice').addEventListener('click', () => {
      if (state.notices.length >= MAX_NOTICES) {
        setMessage(window.MuText.t('El server lee hasta {max} avisos: no se pueden agregar más.', { max: MAX_NOTICES }), 'error');
        return;
      }
      const notice = {
        message: 'Nuevo notice',
        type: 0,
        repeat: 600
      };
      state.notices.push(notice);
      selectNotice(notice, state.notices.length - 1);
      setMessage(window.MuText.t('Notice agregado.'), 'success');
    });

    root.querySelector('#notice-message').addEventListener('input', updateByteCounter);

    root.querySelector('#update-notice').addEventListener('click', () => {
      if (!state.selected) return;
      const index = state.selected.index;
      const message = String(root.querySelector('#notice-message').value || '').trim();
      const bytes = byteLength(message);
      if (bytes > MAX_BYTES) {
        setMessage(window.MuText.t('El mensaje mide {bytes} bytes y el máximo es {max}.', { bytes, max: MAX_BYTES }), 'error');
        return;
      }
      state.notices[index] = {
        message,
        type: Number(root.querySelector('#notice-type').value),
        repeat: Number(root.querySelector('#notice-repeat').value)
      };
      selectNotice(state.notices[index], index);
      setMessage(window.MuText.t('Notice actualizado.'), 'success');
    });

    root.querySelector('#delete-notice').addEventListener('click', () => {
      if (!state.selected) return;
      state.notices.splice(state.selected.index, 1);
      state.selected = null;
      renderList();
      setMessage(window.MuText.t('Notice eliminado.'), 'success');
    });

    root.querySelector('#save-notice').addEventListener('click', async () => {
      try {
        const content = buildNotice();
        const res = await fetch('/admin/server-editor/api/notices', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ content })
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || window.MuText.t('No se pudo guardar'));
        const reload = await fetch('/admin/server-editor/api/reload', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ target: 'util' })
        });
        if (!reload.ok) {
          setMessage(window.MuText.t('Guardado, pero no se pudo recargar en el server.'), 'error');
        } else {
          setMessage(window.MuText.t('Guardado y recargado en el servidor.'), 'success');
        }
      } catch (err) {
        setMessage(err.message, 'error');
      }
    });
  }

  async function loadInitialData() {
    const res = await fetch('/admin/server-editor/api/notices');
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || window.MuText.t('No se pudo cargar'));
    state.notices = parseNotice(data.content || '');
  }

  async function init() {
    buildLayout();
    bindEvents();
    try {
      await loadInitialData();
      renderList();
      if (state.notices[0]) selectNotice(state.notices[0], 0);
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
