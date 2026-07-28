(function () {
  const root = document.getElementById('shop-editor');
  if (!root) return;

  const cols = 8;
  const rows = 15;
  const baseCellSize = 25;
  const cellSize = 25;
  const frameMeta = {
    frameW: 237,
    frameH: 539,
    gridX: 19,
    gridY: 62,
    gridW: 200,
    gridH: 375
  };

  let itemDefs = [];
  let itemDefsMap = new Map();
  let selectedItemId = null;
  let placeMode = false;
  let nextItemId = 1;
  let orderCounter = 1;
  let headerLines = [];
  let currentFile = '';

  const state = {
    items: new Map(),
    occupancy: Array.from({ length: cols * rows }).map(() => null)
  };

  function defaultHeader() {
    return [
      '//==============================================================',
      '// SHOP',
      '//==============================================================',
      '//SLOTS X = 0~7 , Y = 0~14 , * = Orden original',
      '//Type Index Level Dur Skill Luck Option ExcOp SlotX SlotY Comment'
    ];
  }

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

  function resetState() {
    state.items.clear();
    state.occupancy = Array.from({ length: cols * rows }).map(() => null);
    selectedItemId = null;
    placeMode = false;
    nextItemId = 1;
    orderCounter = 1;
  }

  function slotToXY(slotIndex) {
    return { x: slotIndex % cols, y: Math.floor(slotIndex / cols) };
  }

  function canPlace(item, slotIndex, ignoreId) {
    const def = item.def || { width: 1, height: 1 };
    const { x, y } = slotToXY(slotIndex);
    if (x + def.width > cols || y + def.height > rows) return false;
    for (let dy = 0; dy < def.height; dy++) {
      for (let dx = 0; dx < def.width; dx++) {
        const idx = (y + dy) * cols + (x + dx);
        const occ = state.occupancy[idx];
        if (occ && occ !== ignoreId) return false;
      }
    }
    return true;
  }

  function occupy(item, slotIndex) {
    const def = item.def || { width: 1, height: 1 };
    const { x, y } = slotToXY(slotIndex);
    for (let dy = 0; dy < def.height; dy++) {
      for (let dx = 0; dx < def.width; dx++) {
        const idx = (y + dy) * cols + (x + dx);
        state.occupancy[idx] = item.id;
      }
    }
  }

  function clearOccupancy(item) {
    for (let i = 0; i < state.occupancy.length; i++) {
      if (state.occupancy[i] === item.id) state.occupancy[i] = null;
    }
  }

  function findFirstFit(item) {
    for (let y = 0; y < rows; y++) {
      for (let x = 0; x < cols; x++) {
        const slotIndex = y * cols + x;
        if (canPlace(item, slotIndex)) return slotIndex;
      }
    }
    return null;
  }

  function formatLine(item) {
    const name = item.comment || item.def?.name || 'Item';
    return [
      item.section,
      item.index,
      item.level,
      item.durability,
      item.skill,
      item.luck,
      item.option,
      item.excellent,
      item.slotX,
      item.slotY
    ].join(' ') + ` // ${name}`;
  }

  function buildShopText() {
    const lines = headerLines.length ? headerLines.slice() : defaultHeader();
    const items = Array.from(state.items.values()).sort((a, b) => a.order - b.order);
    for (const item of items) {
      lines.push(formatLine(item));
    }
    lines.push('end');
    return lines.join('\n');
  }

  function parseShopText(content) {
    const lines = String(content || '').split(/\r?\n/);
    const items = [];
    const header = [];
    let started = false;
    for (const rawLine of lines) {
      const line = rawLine.trim();
      if (!line) {
        if (!started) header.push(rawLine);
        continue;
      }
      if (!started && line.startsWith('//')) {
        header.push(rawLine);
        continue;
      }
      if (line.toLowerCase() === 'end') break;
      started = true;
      const parts = rawLine.split('//');
      const dataPart = parts[0] || '';
      const commentPart = (parts[1] || '').trim();
      const tokens = dataPart.match(/-?\d+/g);
      if (!tokens || tokens.length < 10) {
        continue;
      }
      const nums = tokens.slice(0, 10).map((token) => Number(token));
      items.push({
        section: nums[0],
        index: nums[1],
        level: nums[2],
        durability: nums[3],
        skill: nums[4],
        luck: nums[5],
        option: nums[6],
        excellent: nums[7],
        slotX: nums[8],
        slotY: nums[9],
        comment: commentPart
      });
    }
    return { items, header };
  }

  function loadFromContent(content, filename) {
    resetState();
    const parsed = parseShopText(content);
    headerLines = parsed.header.length ? parsed.header : defaultHeader();
    let autoPlaced = 0;
    let unplaced = 0;
    for (const raw of parsed.items) {
      const def = itemDefsMap.get(`${raw.section}:${raw.index}`) || null;
      const item = {
        id: nextItemId++,
        order: orderCounter++,
        section: raw.section,
        index: raw.index,
        def,
        level: raw.level,
        durability: raw.durability,
        skill: raw.skill,
        luck: raw.luck,
        option: raw.option,
        excellent: raw.excellent,
        slotX: raw.slotX,
        slotY: raw.slotY,
        slotIndex: 0,
        comment: raw.comment || def?.name || ''
      };

      if (item.slotX < 0 || item.slotY < 0) {
        const slotIndex = findFirstFit(item);
        if (slotIndex !== null) {
          item.slotIndex = slotIndex;
          item.slotX = slotIndex % cols;
          item.slotY = Math.floor(slotIndex / cols);
          occupy(item, slotIndex);
          autoPlaced += 1;
        } else {
          unplaced += 1;
        }
      } else {
        const slotIndex = item.slotY * cols + item.slotX;
        item.slotIndex = slotIndex;
        if (canPlace(item, slotIndex)) {
          occupy(item, slotIndex);
        } else {
          unplaced += 1;
        }
      }

      state.items.set(item.id, item);
    }

    renderItems();
    if (filename) currentFile = filename;
    if (autoPlaced > 0) {
      setMessage(`Se auto-ubicaron ${autoPlaced} items sin slot.`, 'success');
    }
    if (unplaced > 0) {
      setMessage(`Hay ${unplaced} items sin espacio. Ajusta posiciones y guarda.`, 'error');
    }
  }

  function renderItems() {
    const grid = root.querySelector('.inventory-grid');
    if (!grid) return;
    grid.innerHTML = '';
    for (const item of state.items.values()) {
      const def = item.def || { width: 1, height: 1 };
      const x = item.slotX;
      const y = item.slotY;
      if (x < 0 || y < 0) continue;
      const el = document.createElement('div');
      el.className = 'inventory-item';
      if (item.id === selectedItemId) el.classList.add('selected');
      el.dataset.itemId = String(item.id);
      el.style.left = `${x * cellSize}px`;
      el.style.top = `${y * cellSize}px`;
      el.style.width = `${def.width * cellSize}px`;
      el.style.height = `${def.height * cellSize}px`;
      const img = document.createElement('img');
      img.src = `/assets/items/${item.section}/${item.index}.jpg`;
      img.alt = item.def?.name || 'Item';
      img.onerror = () => {
        img.src = '/assets/inventory/unknownItem.jpg';
      };
      el.appendChild(img);
      grid.appendChild(el);
    }
  }

  function populatePicker() {
    const select = root.querySelector('#item-select');
    if (!select) return;
    select.innerHTML = '';
    for (const def of itemDefs) {
      const opt = document.createElement('option');
      opt.value = `${def.section}:${def.index}`;
      opt.textContent = `[${def.section}:${def.index}] ${def.name}`;
      select.appendChild(opt);
    }
  }

  function getSelectedDef() {
    const select = root.querySelector('#item-select');
    if (!select || !select.value) return null;
    return itemDefsMap.get(select.value) || null;
  }

  function createItemFromPicker() {
    const def = getSelectedDef();
    if (!def) return null;
    const level = Number(root.querySelector('#item-level').value || 0);
    const durability = Number(root.querySelector('#item-durability').value || -1);
    const skill = root.querySelector('#item-skill').checked ? 1 : 0;
    const luck = root.querySelector('#item-luck').checked ? 1 : 0;
    const option = Number(root.querySelector('#item-option').value || 0);
    const excellent = Number(root.querySelector('#item-exc').value || 0);
    const commentInput = root.querySelector('#item-comment');
    const comment = commentInput && commentInput.value.trim()
      ? commentInput.value.trim()
      : def.name;
    return {
      id: nextItemId++,
      order: orderCounter++,
      section: def.section,
      index: def.index,
      def,
      level,
      durability,
      skill,
      luck,
      option,
      excellent,
      slotX: 0,
      slotY: 0,
      slotIndex: 0,
      comment
    };
  }

  function selectItem(item) {
    selectedItemId = item ? item.id : null;
    if (!item) {
      renderItems();
      return;
    }
    const select = root.querySelector('#item-select');
    const levelInput = root.querySelector('#item-level');
    const durabilityInput = root.querySelector('#item-durability');
    const skillInput = root.querySelector('#item-skill');
    const luckInput = root.querySelector('#item-luck');
    const optionInput = root.querySelector('#item-option');
    const excInput = root.querySelector('#item-exc');
    const slotXInput = root.querySelector('#item-slotx');
    const slotYInput = root.querySelector('#item-sloty');
    const commentInput = root.querySelector('#item-comment');
    if (select) {
      const value = `${item.section}:${item.index}`;
      if (select.value !== value) {
        select.value = value;
      }
    }
    if (levelInput) levelInput.value = item.level;
    if (durabilityInput) durabilityInput.value = item.durability;
    if (skillInput) skillInput.checked = item.skill === 1;
    if (luckInput) luckInput.checked = item.luck === 1;
    if (optionInput) optionInput.value = item.option;
    if (excInput) excInput.value = item.excellent;
    if (slotXInput) slotXInput.value = item.slotX;
    if (slotYInput) slotYInput.value = item.slotY;
    if (commentInput) commentInput.value = item.comment || item.def?.name || '';
    renderItems();
  }

  function moveItem(item, slotIndex) {
    const prevSlot = item.slotIndex;
    clearOccupancy(item);
    if (!canPlace(item, slotIndex, item.id)) {
      occupy(item, prevSlot);
      return false;
    }
    item.slotIndex = slotIndex;
    item.slotX = slotIndex % cols;
    item.slotY = Math.floor(slotIndex / cols);
    occupy(item, slotIndex);
    return true;
  }

  function buildLayout() {
    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="shop-toolbar">
        <div class="shop-file">
          <label>Shop</label>
          <select id="shop-file"></select>
          <button type="button" id="load-shop">Cargar</button>
          <button type="button" id="save-shop">Guardar</button>
        </div>
        <div class="shop-io">
          <input type="file" id="import-file" accept=".txt" hidden />
          <button type="button" id="import-shop">Importar</button>
          <button type="button" id="export-shop">Exportar</button>
        </div>
      </div>
      <div class="inventory-editor shop-editor">
        <div class="inventory-panels">
          <div class="inventory-frame inventory-frame--shop">
            <div class="frame-overlay frame-overlay--bag"></div>
            <div class="inventory-grid shop-grid" data-grid="shop"></div>
          </div>
        </div>
        <div class="picker">
          <h3>Item</h3>
          <input type="text" id="item-search" placeholder="Buscar item..." />
          <select id="item-select" size="10"></select>
          <div class="picker-fields">
            <label>Nivel</label>
            <input type="number" id="item-level" min="0" max="15" value="0" />
            <label>Durabilidad</label>
            <input type="number" id="item-durability" min="-1" max="255" value="-1" />
            <label class="checkbox"><input type="checkbox" id="item-skill" /> Skill</label>
            <label class="checkbox"><input type="checkbox" id="item-luck" /> Luck</label>
            <label>Option (+4/+8/+12/+16/+20/+24)</label>
            <input type="number" id="item-option" min="0" max="7" value="0" />
            <label>Excellent (0-63)</label>
            <input type="number" id="item-exc" min="0" max="63" value="0" />
            <label>Comentario</label>
            <input type="text" id="item-comment" maxlength="80" placeholder="Nombre del item en el shop" />
            <label>Slot X</label>
            <input type="number" id="item-slotx" min="0" max="7" value="0" />
            <label>Slot Y</label>
            <input type="number" id="item-sloty" min="0" max="14" value="0" />
            <button type="button" id="place-item">Colocar item</button>
            <button type="button" id="update-item">Actualizar seleccionado</button>
            <button type="button" id="remove-item" class="link-button">Eliminar seleccionado</button>
          </div>
        </div>
      </div>
    `;

    const scale = cellSize / baseCellSize;
    const frame = root.querySelector('.inventory-frame');
    if (frame) {
      frame.style.width = `${frameMeta.frameW * scale}px`;
      frame.style.height = `${frameMeta.frameH * scale}px`;
    }

    const grid = root.querySelector('.inventory-grid');
    if (grid) {
      grid.classList.add('warehouse-grid');
      grid.style.setProperty('--cols', String(cols));
      grid.style.setProperty('--rows', String(rows));
      grid.style.setProperty('--cell', `${cellSize}px`);
      grid.style.width = `${cols * cellSize}px`;
      grid.style.height = `${rows * cellSize}px`;
      grid.style.position = 'absolute';
      grid.style.left = `${frameMeta.gridX * scale}px`;
      grid.style.top = `${frameMeta.gridY * scale}px`;
    }

    const overlay = root.querySelector('.frame-overlay--bag');
    if (overlay) {
      overlay.style.left = `${frameMeta.gridX * scale}px`;
      overlay.style.top = `${frameMeta.gridY * scale}px`;
      overlay.style.width = `${frameMeta.gridW * scale}px`;
      overlay.style.height = `${frameMeta.gridH * scale}px`;
      overlay.style.backgroundSize = `${frameMeta.gridW * scale}px ${frameMeta.gridH * scale}px`;
    }
  }

  function bindEvents() {
    const grid = root.querySelector('.inventory-grid');
    const searchInput = root.querySelector('#item-search');
    const select = root.querySelector('#item-select');

    if (searchInput && select) {
      searchInput.addEventListener('input', () => {
        const term = searchInput.value.trim().toLowerCase();
        select.querySelectorAll('option').forEach((opt) => {
          opt.hidden = term && !opt.textContent.toLowerCase().includes(term);
        });
      });
    }

    root.querySelector('#place-item').addEventListener('click', () => {
      placeMode = true;
      setMessage('Selecciona una casilla libre para colocar el item.', 'success');
    });

    root.querySelector('#update-item').addEventListener('click', () => {
      if (!selectedItemId) {
        setMessage('Selecciona un item primero.', 'error');
        return;
      }
      const item = state.items.get(selectedItemId);
      if (!item) return;
      const newDef = getSelectedDef();
      if (!newDef) {
        setMessage('Selecciona un item para aplicar.', 'error');
        return;
      }
      const newSlotX = Number(root.querySelector('#item-slotx').value);
      const newSlotY = Number(root.querySelector('#item-sloty').value);
      if (Number.isNaN(newSlotX) || Number.isNaN(newSlotY)) {
        setMessage('Slot X/Y invalidos.', 'error');
        return;
      }
      if (newSlotX < 0 || newSlotX >= cols || newSlotY < 0 || newSlotY >= rows) {
        setMessage('Slot X/Y fuera de rango.', 'error');
        return;
      }

      const targetSlot = newSlotY * cols + newSlotX;
      const candidate = {
        ...item,
        def: newDef,
        section: newDef.section,
        index: newDef.index
      };
      if (!canPlace(candidate, targetSlot, item.id)) {
        setMessage('No hay espacio para ese item en la posicion.', 'error');
        return;
      }
      clearOccupancy(item);
      item.section = newDef.section;
      item.index = newDef.index;
      item.def = newDef;
      item.slotIndex = targetSlot;
      item.slotX = newSlotX;
      item.slotY = newSlotY;
      occupy(item, targetSlot);

      item.level = Number(root.querySelector('#item-level').value || 0);
      item.durability = Number(root.querySelector('#item-durability').value || -1);
      item.skill = root.querySelector('#item-skill').checked ? 1 : 0;
      item.luck = root.querySelector('#item-luck').checked ? 1 : 0;
      item.option = Number(root.querySelector('#item-option').value || 0);
      item.excellent = Number(root.querySelector('#item-exc').value || 0);
      const commentInput = root.querySelector('#item-comment');
      item.comment = commentInput && commentInput.value.trim()
        ? commentInput.value.trim()
        : newDef.name;
      renderItems();
      setMessage('Item actualizado.', 'success');
    });

    root.querySelector('#remove-item').addEventListener('click', () => {
      if (!selectedItemId) {
        setMessage('Selecciona un item primero.', 'error');
        return;
      }
      const item = state.items.get(selectedItemId);
      if (!item) return;
      clearOccupancy(item);
      state.items.delete(item.id);
      selectedItemId = null;
      renderItems();
    });

    if (grid) {
      grid.addEventListener('click', (event) => {
        const rect = grid.getBoundingClientRect();
        const x = Math.floor((event.clientX - rect.left) / cellSize);
        const y = Math.floor((event.clientY - rect.top) / cellSize);
        if (x < 0 || y < 0 || x >= cols || y >= rows) return;
        const slotIndex = y * cols + x;
      if (placeMode) {
        const newItem = createItemFromPicker();
        if (!newItem) {
          setMessage('Selecciona un item para colocar.', 'error');
          return;
          }
          if (!canPlace(newItem, slotIndex)) {
            setMessage('No hay espacio para el item en esa posicion.', 'error');
            return;
          }
        newItem.slotIndex = slotIndex;
        newItem.slotX = x;
        newItem.slotY = y;
        state.items.set(newItem.id, newItem);
        occupy(newItem, slotIndex);
          placeMode = false;
          selectedItemId = newItem.id;
          renderItems();
          setMessage('Item colocado.', 'success');
          selectItem(newItem);
          return;
        }

        if (selectedItemId) {
          const item = state.items.get(selectedItemId);
          if (item && moveItem(item, slotIndex)) {
            renderItems();
            selectItem(item);
            return;
          }
        }
      });
    }

    root.addEventListener('click', (event) => {
      const itemEl = event.target.closest('.inventory-item');
      if (!itemEl) return;
      const itemId = Number(itemEl.dataset.itemId);
      const item = state.items.get(itemId);
      if (item) selectItem(item);
    });

    root.querySelector('#load-shop')?.addEventListener('click', async () => {
      const fileName = root.querySelector('#shop-file').value;
      if (!fileName) return;
      await loadShopFile(fileName);
    });

    root.querySelector('#save-shop')?.addEventListener('click', async () => {
      const fileName = root.querySelector('#shop-file').value;
      if (!fileName) {
        setMessage('Selecciona un shop para guardar.', 'error');
        return;
      }
      await saveShopFile(fileName);
    });

    root.querySelector('#import-shop')?.addEventListener('click', () => {
      root.querySelector('#import-file')?.click();
    });

    root.querySelector('#import-file')?.addEventListener('change', (event) => {
      const file = event.target.files?.[0];
      if (!file) return;
      const reader = new FileReader();
      reader.onload = () => {
        loadFromContent(reader.result, file.name);
        const selectEl = root.querySelector('#shop-file');
        if (selectEl) {
          const option = Array.from(selectEl.options).find((opt) => opt.value === file.name);
          if (option) selectEl.value = file.name;
        }
        setMessage('Shop importado. Guardalo para subirlo al servidor.', 'success');
      };
      reader.readAsText(file);
      event.target.value = '';
    });

    root.querySelector('#export-shop')?.addEventListener('click', () => {
      const fileName = root.querySelector('#shop-file').value || currentFile || 'shop.txt';
      const content = buildShopText();
      const blob = new Blob([content], { type: 'text/plain' });
      const link = document.createElement('a');
      link.href = URL.createObjectURL(blob);
      link.download = fileName;
      document.body.appendChild(link);
      link.click();
      URL.revokeObjectURL(link.href);
      link.remove();
    });
  }

  async function loadShopList() {
    const selectEl = root.querySelector('#shop-file');
    if (!selectEl) return;
    try {
      const res = await fetch('/admin/server-editor/api/shops');
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudo cargar');
      const files = Array.isArray(data.files) ? data.files : [];
      selectEl.innerHTML = '';
      files.forEach((name) => {
        const opt = document.createElement('option');
        opt.value = name;
        opt.textContent = name;
        selectEl.appendChild(opt);
      });
      if (files.length > 0) {
        selectEl.value = currentFile && files.includes(currentFile) ? currentFile : files[0];
      }
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  async function loadShopFile(name) {
    if (!name) return;
    setMessage('Cargando shop...', 'success');
    try {
      const res = await fetch(`/admin/server-editor/api/shops/file?name=${encodeURIComponent(name)}`);
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudo cargar');
      loadFromContent(data.content || '', name);
      setMessage('Shop cargado.', 'success');
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  async function saveShopFile(name) {
    setMessage('Guardando shop...', 'success');
    try {
      const content = buildShopText();
      const res = await fetch('/admin/server-editor/api/shops/file', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ name, content })
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudo guardar');
      const reloaded = await triggerReload();
      if (reloaded) {
        setMessage('Shop guardado y recargado en el servidor.', 'success');
      } else {
        setMessage('Shop guardado. No se pudo recargar automaticamente.', 'error');
      }
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  async function triggerReload() {
    try {
      const res = await fetch('/admin/server-editor/api/reload', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ target: 'shop' })
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || 'No se pudo recargar');
      return true;
    } catch {
      return false;
    }
  }

  async function init() {
    buildLayout();
    bindEvents();
    try {
      const res = await fetch('/admin/item-defs.json');
      const data = await res.json();
      itemDefs = Array.isArray(data.items) ? data.items : [];
      itemDefsMap = new Map(itemDefs.map((def) => [`${def.section}:${def.index}`, def]));
      populatePicker();
    } catch {
      setMessage('No se pudo cargar la lista de items.', 'error');
    }
    await loadShopList();
    const selectEl = root.querySelector('#shop-file');
    if (selectEl?.value) {
      await loadShopFile(selectEl.value);
    }
  }

  init();
})();
