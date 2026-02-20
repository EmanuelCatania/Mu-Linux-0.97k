(function () {
  const root = document.getElementById('items-editor');
  if (!root) return;

  const state = {
    active: 'item',
    itemRaw: '',
    valueRows: [],
    stackRows: [],
    selectedValue: null,
    selectedStack: null
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

  function parseItemValue(content) {
    const rows = [];
    const lines = String(content || '').split(/\r?\n/);
    for (const raw of lines) {
      const line = raw.trim();
      if (!line || line.startsWith('//')) continue;
      if (line.toLowerCase() === 'end') break;
      const [dataPart, commentPart] = raw.split('//');
      const tokens = (dataPart || '').trim().split(/\s+/);
      if (tokens.length < 4) continue;
      rows.push({
        index: tokens[0],
        level: tokens[1],
        buy: tokens[2],
        sell: tokens[3],
        comment: (commentPart || '').trim()
      });
    }
    return rows;
  }

  function buildItemValue() {
    const lines = [];
    lines.push('//Index\t\tLevel\t\tBuyValue\tSellValue\tComment');
    state.valueRows.forEach((row) => {
      const comment = row.comment ? `\t\t// ${row.comment}` : '';
      lines.push(`${row.index}\t\t${row.level}\t\t${row.buy}\t\t${row.sell}${comment}`);
    });
    lines.push('end');
    return lines.join('\n');
  }

  function parseItemStack(content) {
    const rows = [];
    const lines = String(content || '').split(/\r?\n/);
    for (const raw of lines) {
      const line = raw.trim();
      if (!line || line.startsWith('//')) continue;
      if (line.toLowerCase() === 'end') break;
      const [dataPart, commentPart] = raw.split('//');
      const tokens = (dataPart || '').trim().split(/\s+/);
      if (tokens.length < 4) continue;
      rows.push({
        index: tokens[0],
        level: tokens[1],
        maxStack: tokens[2],
        createIndex: tokens[3],
        comment: (commentPart || '').trim()
      });
    }
    return rows;
  }

  function buildItemStack() {
    const lines = [];
    lines.push('//Index\t\tLevel\t\tMaxStack\tCreateIndex\tComment');
    state.stackRows.forEach((row) => {
      const comment = row.comment ? `\t\t// ${row.comment}` : '';
      lines.push(`${row.index}\t\t${row.level}\t\t${row.maxStack}\t\t${row.createIndex}${comment}`);
    });
    lines.push('end');
    return lines.join('\n');
  }

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <button type="button" class="tab-btn" data-tab="item">Item.txt</button>
          <button type="button" class="tab-btn" data-tab="value">ItemValue</button>
          <button type="button" class="tab-btn" data-tab="stack">ItemStack</button>
        </div>
        <div class="spawn-toolbar-group">
          <button type="button" id="save-items">Guardar</button>
        </div>
      </div>
      <div id="items-content"></div>
    `;
  }

  function renderItemRaw() {
    const container = root.querySelector('#items-content');
    container.innerHTML = `
      <div class="alert warning">
        Item.txt es avanzado. Cualquier error puede romper el server.
      </div>
      <textarea id="item-raw" class="raw-editor" rows="24"></textarea>
    `;
    container.querySelector('#item-raw').value = state.itemRaw;
  }

  function renderValueEditor() {
    const container = root.querySelector('#items-content');
    container.innerHTML = `
      <div class="spawn-editor">
        <div class="spawn-list">
          <h3>ItemValue</h3>
          <ul id="value-list" class="simple-list"></ul>
          <button type="button" id="add-value">Agregar</button>
        </div>
        <div class="spawn-form">
          <h3>Detalle</h3>
          <label>Index (grupo,item)</label>
          <input type="text" id="value-index" placeholder="04,007" />
          <label>Level</label>
          <input type="text" id="value-level" />
          <label>BuyValue</label>
          <input type="text" id="value-buy" />
          <label>SellValue</label>
          <input type="text" id="value-sell" />
          <label>Comentario</label>
          <input type="text" id="value-comment" />
          <div class="spawn-actions">
            <button type="button" id="update-value">Actualizar</button>
            <button type="button" id="delete-value" class="link-button">Eliminar</button>
          </div>
        </div>
      </div>
    `;
    renderValueList();
  }

  function renderStackEditor() {
    const container = root.querySelector('#items-content');
    container.innerHTML = `
      <div class="spawn-editor">
        <div class="spawn-list">
          <h3>ItemStack</h3>
          <ul id="stack-list" class="simple-list"></ul>
          <button type="button" id="add-stack">Agregar</button>
        </div>
        <div class="spawn-form">
          <h3>Detalle</h3>
          <label>Index (grupo,item)</label>
          <input type="text" id="stack-index" placeholder="04,007" />
          <label>Level</label>
          <input type="text" id="stack-level" />
          <label>MaxStack</label>
          <input type="text" id="stack-max" />
          <label>CreateIndex</label>
          <input type="text" id="stack-create" />
          <label>Comentario</label>
          <input type="text" id="stack-comment" />
          <div class="spawn-actions">
            <button type="button" id="update-stack">Actualizar</button>
            <button type="button" id="delete-stack" class="link-button">Eliminar</button>
          </div>
        </div>
      </div>
    `;
    renderStackList();
  }

  function renderValueList() {
    const list = root.querySelector('#value-list');
    if (!list) return;
    list.innerHTML = '';
    state.valueRows.forEach((row, index) => {
      const item = document.createElement('li');
      item.textContent = `${row.index} L${row.level} Buy:${row.buy}`;
      item.dataset.index = String(index);
      if (state.selectedValue === index) item.classList.add('selected');
      list.appendChild(item);
    });
  }

  function renderStackList() {
    const list = root.querySelector('#stack-list');
    if (!list) return;
    list.innerHTML = '';
    state.stackRows.forEach((row, index) => {
      const item = document.createElement('li');
      item.textContent = `${row.index} L${row.level} x${row.maxStack}`;
      item.dataset.index = String(index);
      if (state.selectedStack === index) item.classList.add('selected');
      list.appendChild(item);
    });
  }

  function selectValue(index) {
    state.selectedValue = index;
    const row = state.valueRows[index];
    if (!row) return;
    root.querySelector('#value-index').value = row.index;
    root.querySelector('#value-level').value = row.level;
    root.querySelector('#value-buy').value = row.buy;
    root.querySelector('#value-sell').value = row.sell;
    root.querySelector('#value-comment').value = row.comment;
    renderValueList();
  }

  function selectStack(index) {
    state.selectedStack = index;
    const row = state.stackRows[index];
    if (!row) return;
    root.querySelector('#stack-index').value = row.index;
    root.querySelector('#stack-level').value = row.level;
    root.querySelector('#stack-max').value = row.maxStack;
    root.querySelector('#stack-create').value = row.createIndex;
    root.querySelector('#stack-comment').value = row.comment;
    renderStackList();
  }

  function setTab(tab) {
    state.active = tab;
    root.querySelectorAll('.tab-btn').forEach((btn) => {
      btn.classList.toggle('active', btn.dataset.tab === tab);
    });
    if (tab === 'item') {
      renderItemRaw();
    }
    if (tab === 'value') {
      renderValueEditor();
      bindValueEvents();
    }
    if (tab === 'stack') {
      renderStackEditor();
      bindStackEvents();
    }
  }

  function bindTabEvents() {
    root.querySelectorAll('.tab-btn').forEach((btn) => {
      btn.addEventListener('click', () => setTab(btn.dataset.tab));
    });
  }

  function bindValueEvents() {
    const list = root.querySelector('#value-list');
    if (list) {
      list.addEventListener('click', (event) => {
        const item = event.target.closest('li');
        if (!item) return;
        selectValue(Number(item.dataset.index));
      });
    }
    const addBtn = root.querySelector('#add-value');
    if (addBtn) {
      addBtn.addEventListener('click', () => {
        state.valueRows.push({ index: '00,000', level: '0', buy: '0', sell: '*', comment: '' });
        selectValue(state.valueRows.length - 1);
      });
    }
    const updateBtn = root.querySelector('#update-value');
    if (updateBtn) {
      updateBtn.addEventListener('click', () => {
        if (state.selectedValue === null) return;
        const row = state.valueRows[state.selectedValue];
        row.index = root.querySelector('#value-index').value.trim();
        row.level = root.querySelector('#value-level').value.trim();
        row.buy = root.querySelector('#value-buy').value.trim();
        row.sell = root.querySelector('#value-sell').value.trim();
        row.comment = root.querySelector('#value-comment').value.trim();
        renderValueList();
      });
    }
    const deleteBtn = root.querySelector('#delete-value');
    if (deleteBtn) {
      deleteBtn.addEventListener('click', () => {
        if (state.selectedValue === null) return;
        state.valueRows.splice(state.selectedValue, 1);
        state.selectedValue = null;
        renderValueList();
      });
    }
  }

  function bindStackEvents() {
    const list = root.querySelector('#stack-list');
    if (list) {
      list.addEventListener('click', (event) => {
        const item = event.target.closest('li');
        if (!item) return;
        selectStack(Number(item.dataset.index));
      });
    }
    const addBtn = root.querySelector('#add-stack');
    if (addBtn) {
      addBtn.addEventListener('click', () => {
        state.stackRows.push({ index: '00,000', level: '*', maxStack: '255', createIndex: '*', comment: '' });
        selectStack(state.stackRows.length - 1);
      });
    }
    const updateBtn = root.querySelector('#update-stack');
    if (updateBtn) {
      updateBtn.addEventListener('click', () => {
        if (state.selectedStack === null) return;
        const row = state.stackRows[state.selectedStack];
        row.index = root.querySelector('#stack-index').value.trim();
        row.level = root.querySelector('#stack-level').value.trim();
        row.maxStack = root.querySelector('#stack-max').value.trim();
        row.createIndex = root.querySelector('#stack-create').value.trim();
        row.comment = root.querySelector('#stack-comment').value.trim();
        renderStackList();
      });
    }
    const deleteBtn = root.querySelector('#delete-stack');
    if (deleteBtn) {
      deleteBtn.addEventListener('click', () => {
        if (state.selectedStack === null) return;
        state.stackRows.splice(state.selectedStack, 1);
        state.selectedStack = null;
        renderStackList();
      });
    }
  }

  async function saveActive() {
    try {
      let kind = state.active;
      let content = '';
      if (kind === 'item') {
        content = root.querySelector('#item-raw').value || '';
      } else if (kind === 'value') {
        content = buildItemValue();
      } else {
        content = buildItemStack();
        kind = 'stack';
      }
      const res = await fetch(`/admin/server-editor/api/items-base?kind=${kind}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ content })
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudo guardar');
      const reload = await fetch('/admin/server-editor/api/reload', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ target: 'item' })
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

  async function loadAll() {
    const [itemRes, valueRes, stackRes] = await Promise.all([
      fetch('/admin/server-editor/api/items-base?kind=item'),
      fetch('/admin/server-editor/api/items-base?kind=value'),
      fetch('/admin/server-editor/api/items-base?kind=stack')
    ]);
    const itemData = await itemRes.json();
    const valueData = await valueRes.json();
    const stackData = await stackRes.json();
    if (!itemRes.ok) throw new Error(itemData.error || 'No se pudo cargar Item.txt');
    if (!valueRes.ok) throw new Error(valueData.error || 'No se pudo cargar ItemValue.txt');
    if (!stackRes.ok) throw new Error(stackData.error || 'No se pudo cargar ItemStack.txt');
    state.itemRaw = itemData.content || '';
    state.valueRows = parseItemValue(valueData.content || '');
    state.stackRows = parseItemStack(stackData.content || '');
  }

  async function init() {
    buildLayout();
    bindTabEvents();
    root.querySelector('#save-items').addEventListener('click', saveActive);
    try {
      await loadAll();
      setTab(state.active);
      bindValueEvents();
      bindStackEvents();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
