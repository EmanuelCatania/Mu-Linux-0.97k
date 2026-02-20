(function () {
  const root = document.getElementById('inventory-editor');
  if (!root) return;

  const mode = root.dataset.mode || 'inventory';
  const initialHex = root.dataset.hex || '';
  const accountId = root.dataset.account || '';
  const charName = root.dataset.character || '';
  const baseCellSize = 25;
  const cellSize = 25;
  const cols = 8;
  const rows = mode === 'warehouse' ? 15 : 8;
  const equipSlots = mode === 'inventory' ? 12 : 0;
  const hexInput = document.getElementById('inventory-hex');
  const equipCols = 4;
  const equipRows = 3;
  const frameMeta = {
    inventory: {
      frameW: 237,
      frameH: 539,
      gridX: 19,
      gridY: 249,
      equipOverlayX: 18,
      equipOverlayY: 55,
      equipGridX: 18,
      equipGridY: 55,
      gridW: 200,
      gridH: 200,
      equipW: 200,
      equipH: 186
    },
    warehouse: {
      frameW: 237,
      frameH: 539,
      gridX: 19,
      gridY: 62,
      gridW: 200,
      gridH: 375
    }
  };

  const EQUIP_META = [
    { index: 0, label: 'Arma' },
    { index: 1, label: 'Escudo' },
    { index: 2, label: 'Casco' },
    { index: 3, label: 'Armadura' },
    { index: 4, label: 'Pantalon' },
    { index: 5, label: 'Guantes' },
    { index: 6, label: 'Botas' },
    { index: 7, label: 'Alas' },
    { index: 8, label: 'Pet' },
    { index: 9, label: 'Pendiente' },
    { index: 10, label: 'Anillo 1' },
    { index: 11, label: 'Anillo 2' }
  ];

  const EQUIP_RECTS = [
    { x: 3, y: 59, w: 45, h: 70 },  // Weapon
    { x: 152, y: 59, w: 45, h: 70 }, // Shield
    { x: 78, y: 5, w: 45, h: 45 },   // Helm
    { x: 78, y: 59, w: 45, h: 70 },  // Armor
    { x: 78, y: 137, w: 45, h: 45 }, // Pants
    { x: 3, y: 137, w: 45, h: 45 },  // Gloves
    { x: 152, y: 137, w: 45, h: 45 }, // Boots
    { x: 128, y: 5, w: 70, h: 45 },  // Wings
    { x: 3, y: 5, w: 45, h: 45 },    // Pet
    { x: 53, y: 59, w: 20, h: 20 },  // Pendant
    { x: 53, y: 137, w: 20, h: 20 }, // Ring1
    { x: 128, y: 137, w: 20, h: 20 } // Ring2
  ];

  let itemDefs = [];
  let itemDefsMap = new Map();
  let selectedItemId = null;
  let placeMode = false;
  let nextItemId = 1;

  const state = {
    equip: Array.from({ length: equipSlots }).map(() => null),
    items: new Map(),
    occupancy: Array.from({ length: cols * rows }).map(() => null)
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

  function hexToBytes(hex) {
    let value = String(hex || '').trim();
    if (value.startsWith('0x') || value.startsWith('0X')) value = value.slice(2);
    if (!value) return new Uint8Array(0);
    const out = new Uint8Array(value.length / 2);
    for (let i = 0; i < out.length; i++) {
      out[i] = parseInt(value.substr(i * 2, 2), 16);
    }
    return out;
  }

  function bytesToHex(bytes) {
    return Array.from(bytes)
      .map((b) => b.toString(16).padStart(2, '0'))
      .join('')
      .toUpperCase();
  }

  function isEmptyItem(bytes) {
    return bytes[0] === 0xff && (bytes[7] & 0x80) === 0x80 && (bytes[9] & 0xf0) === 0xf0;
  }

  function decodeItem(bytes) {
    if (!bytes || bytes.length < 10 || isEmptyItem(bytes)) return null;
    const itemIndex = bytes[0] | ((bytes[9] & 0xf0) * 32) | ((bytes[7] & 0x80) * 2);
    const section = Math.floor(itemIndex / 32);
    const index = itemIndex % 32;
    const def = itemDefsMap.get(`${section}:${index}`);
    const level = (bytes[1] / 8) & 15;
    const durability = bytes[2];
    const skill = (bytes[1] / 128) & 1;
    const luck = (bytes[1] / 4) & 1;
    const add = (bytes[1] & 3) + ((bytes[7] & 64) / 16);
    const serial = ((bytes[3] << 24) | (bytes[4] << 16) | (bytes[5] << 8) | bytes[6]) >>> 0;
    const excellent = bytes[7] & 63;
    return {
      id: nextItemId++,
      section,
      index,
      def,
      level,
      durability,
      skill,
      luck,
      add,
      excellent,
      serial,
      bytes: Uint8Array.from(bytes)
    };
  }

  function encodeItem(item) {
    const itemIndex = item.section * 32 + item.index;
    const bytes = new Uint8Array(10).fill(0xff);
    bytes[0] = itemIndex & 0xff;
    bytes[1] = (item.level & 0x0f) * 8;
    if (item.skill) bytes[1] |= 0x80;
    if (item.luck) bytes[1] |= 0x04;
    bytes[1] |= item.add & 3;
    bytes[2] = item.durability & 0xff;
    const serial = item.serial >>> 0;
    bytes[3] = (serial >>> 24) & 0xff;
    bytes[4] = (serial >>> 16) & 0xff;
    bytes[5] = (serial >>> 8) & 0xff;
    bytes[6] = serial & 0xff;
    bytes[7] = 0;
    if (itemIndex & 0x100) bytes[7] |= 0x80;
    if (item.add > 3) bytes[7] |= 0x40;
    bytes[7] |= item.excellent & 0x3f;
    bytes[8] = 0;
    bytes[9] = ((itemIndex & 0x1e0) >> 5) & 0xff;
    return bytes;
  }

  function canEquip(item, slotIndex) {
    const def = item.def;
    if (!def || def.slot === -1) return false;
    if ([0, 1, 2, 3].includes(item.section)) {
      return slotIndex === 0 || slotIndex === 1;
    }
    if (def.slot === 10) {
      return slotIndex === 10 || slotIndex === 11;
    }
    return def.slot === slotIndex;
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

  function render() {
    const equipHtml = mode === 'inventory'
      ? `<div class="equip-grid">
          ${EQUIP_META.map((slot) => {
            const rect = EQUIP_RECTS[slot.index] || { x: 0, y: 0, w: 0, h: 0 };
            return `
            <div class="equip-slot"
                 data-equip="${slot.index}"
                 data-x="${rect.x}"
                 data-y="${rect.y}"
                 data-w="${rect.w}"
                 data-h="${rect.h}">
              <span class="slot-label">${slot.label}</span>
            </div>`;
          }).join('')}
        </div>`
      : '';

    const frameClass = mode === 'inventory'
      ? 'inventory-frame inventory-frame--inventory'
      : 'inventory-frame inventory-frame--warehouse';

    const overlayHtml = mode === 'inventory'
      ? `<div class="frame-overlay frame-overlay--equip"></div>
         <div class="frame-overlay frame-overlay--bag"></div>`
      : `<div class="frame-overlay frame-overlay--bag"></div>`;

    const frameContent = mode === 'inventory'
      ? `${overlayHtml}${equipHtml}<div class="inventory-grid" data-grid="bag"></div>`
      : `${overlayHtml}<div class="inventory-grid" data-grid="bag"></div>`;

    const pickerHtml = `
      <div class="picker">
        <h3>Item</h3>
        <input type="text" id="item-search" placeholder="Buscar item..." />
        <select id="item-select" size="10"></select>
        <div class="picker-fields">
          <label>Nivel</label>
          <input type="number" id="item-level" min="0" max="15" value="0" />
          <label>Durabilidad</label>
          <input type="number" id="item-durability" min="0" max="255" value="255" />
          <label class="checkbox"><input type="checkbox" id="item-skill" /> Skill</label>
          <label class="checkbox"><input type="checkbox" id="item-luck" /> Luck</label>
          <label>Option (+4/+8/+12/+16/+20/+24)</label>
          <input type="number" id="item-add" min="0" max="7" value="0" />
          <label>Excellent (0-63)</label>
          <input type="number" id="item-exc" min="0" max="63" value="0" />
          <button type="button" id="place-item">Colocar item</button>
          <button type="button" id="update-item">Actualizar seleccionado</button>
          <button type="button" id="remove-item" class="link-button">Eliminar seleccionado</button>
        </div>
      </div>
    `;

    root.innerHTML = `
      <div class="editor-message alert hidden"></div>
      <div class="inventory-editor">
        <div class="inventory-panels">
          <div class="${frameClass}">
            ${frameContent}
          </div>
        </div>
        ${pickerHtml}
      </div>
    `;

    const scale = cellSize / baseCellSize;
    const meta = frameMeta[mode];
    const frame = root.querySelector('.inventory-frame');
    if (frame && meta) {
      frame.style.width = `${meta.frameW * scale}px`;
      frame.style.height = `${meta.frameH * scale}px`;
    }

    const grid = root.querySelector('.inventory-grid');
    if (grid) {
      grid.classList.add(mode === 'warehouse' ? 'warehouse-grid' : 'bag-grid');
      grid.style.setProperty('--cols', String(cols));
      grid.style.setProperty('--rows', String(rows));
      grid.style.setProperty('--cell', `${cellSize}px`);
      grid.style.width = `${cols * cellSize}px`;
      grid.style.height = `${rows * cellSize}px`;
      if (meta) {
        grid.style.position = 'absolute';
        grid.style.left = `${meta.gridX * scale}px`;
        grid.style.top = `${meta.gridY * scale}px`;
      }
    }

    if (meta) {
      const bagOverlay = root.querySelector('.frame-overlay--bag');
      if (bagOverlay) {
        if (mode === 'inventory') {
          bagOverlay.style.backgroundImage = 'url(/assets/inventory/Item_Boxs_8x8.jpg)';
        } else {
          bagOverlay.style.backgroundImage = 'url(/assets/inventory/Item_Boxs.jpg)';
        }
        bagOverlay.style.left = `${meta.gridX * scale}px`;
        bagOverlay.style.top = `${meta.gridY * scale}px`;
        bagOverlay.style.width = `${meta.gridW * scale}px`;
        bagOverlay.style.height = `${meta.gridH * scale}px`;
        bagOverlay.style.backgroundSize = `${meta.gridW * scale}px ${meta.gridH * scale}px`;
      }
      const equipOverlay = root.querySelector('.frame-overlay--equip');
      if (equipOverlay) {
        const overlayX = meta.equipOverlayX ?? meta.equipX ?? 0;
        const overlayY = meta.equipOverlayY ?? meta.equipY ?? 0;
        equipOverlay.style.left = `${overlayX * scale}px`;
        equipOverlay.style.top = `${overlayY * scale}px`;
        equipOverlay.style.width = `${meta.equipW * scale}px`;
        equipOverlay.style.height = `${meta.equipH * scale}px`;
        equipOverlay.style.backgroundSize = `${meta.equipW * scale}px ${meta.equipH * scale}px`;
      }
    }

    const equipGrid = root.querySelector('.equip-grid');
    if (equipGrid) {
      equipGrid.style.display = 'block';
      if (meta) {
        const gridX = meta.equipGridX ?? meta.equipOverlayX ?? meta.equipX ?? 0;
        const gridY = meta.equipGridY ?? meta.equipOverlayY ?? meta.equipY ?? 0;
        equipGrid.style.position = 'absolute';
        equipGrid.style.left = `${gridX * scale}px`;
        equipGrid.style.top = `${gridY * scale}px`;
        equipGrid.style.width = `${meta.equipW * scale}px`;
        equipGrid.style.height = `${meta.equipH * scale}px`;
      }
    }

    root.querySelectorAll('.equip-slot').forEach((slotEl) => {
      const x = Number(slotEl.dataset.x || 0) * scale;
      const y = Number(slotEl.dataset.y || 0) * scale;
      const w = Number(slotEl.dataset.w || 0) * scale;
      const h = Number(slotEl.dataset.h || 0) * scale;
      slotEl.style.left = `${x}px`;
      slotEl.style.top = `${y}px`;
      slotEl.style.width = `${w}px`;
      slotEl.style.height = `${h}px`;
    });

    renderItems();
    bindEvents();
    populatePicker();
  }

  function renderItems() {
    const grid = root.querySelector('.inventory-grid');
    if (!grid) return;
    grid.innerHTML = '';
    for (const item of state.items.values()) {
      if (item.area !== 'bag') continue;
      const def = item.def || { width: 1, height: 1 };
      const { x, y } = slotToXY(item.slotIndex);
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

    if (mode === 'inventory') {
      root.querySelectorAll('.equip-slot').forEach((slotEl) => {
        const idx = Number(slotEl.dataset.equip);
        const item = state.equip[idx];
        slotEl.querySelectorAll('.equip-item').forEach((node) => node.remove());
        if (!item) return;
        const el = document.createElement('div');
        el.className = 'equip-item';
        if (item.id === selectedItemId) el.classList.add('selected');
        el.dataset.itemId = String(item.id);
        const img = document.createElement('img');
        img.src = `/assets/items/${item.section}/${item.index}.jpg`;
        img.alt = item.def?.name || 'Item';
        img.onerror = () => {
          img.src = '/assets/inventory/unknownItem.jpg';
        };
        el.appendChild(img);
        slotEl.appendChild(el);
      });
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
    const durability = Number(root.querySelector('#item-durability').value || 255);
    const skill = root.querySelector('#item-skill').checked ? 1 : 0;
    const luck = root.querySelector('#item-luck').checked ? 1 : 0;
    const add = Number(root.querySelector('#item-add').value || 0);
    const exc = Number(root.querySelector('#item-exc').value || 0);
    return {
      id: nextItemId++,
      section: def.section,
      index: def.index,
      def,
      level,
      durability,
      skill,
      luck,
      add,
      excellent: exc,
      serial: 0,
      bytes: null,
      area: 'bag',
      slotIndex: 0
    };
  }

  function selectItem(item) {
    selectedItemId = item ? item.id : null;
    if (!item) {
      renderItems();
      return;
    }
    const levelInput = root.querySelector('#item-level');
    const durabilityInput = root.querySelector('#item-durability');
    const skillInput = root.querySelector('#item-skill');
    const luckInput = root.querySelector('#item-luck');
    const addInput = root.querySelector('#item-add');
    const excInput = root.querySelector('#item-exc');
    if (levelInput) levelInput.value = item.level;
    if (durabilityInput) durabilityInput.value = item.durability;
    if (skillInput) skillInput.checked = item.skill === 1;
    if (luckInput) luckInput.checked = item.luck === 1;
    if (addInput) addInput.value = item.add;
    if (excInput) excInput.value = item.excellent;
    renderItems();
  }

  function moveItemToBag(item, slotIndex) {
    const prevArea = item.area;
    const prevSlot = item.slotIndex;
    if (prevArea === 'bag') {
      clearOccupancy(item);
    }
    if (!canPlace(item, slotIndex, item.id)) {
      if (prevArea === 'bag') occupy(item, prevSlot);
      return false;
    }
    if (mode === 'inventory' && prevArea === 'equip' && state.equip[prevSlot]?.id === item.id) {
      state.equip[prevSlot] = null;
    }
    item.area = 'bag';
    item.slotIndex = slotIndex;
    occupy(item, slotIndex);
    return true;
  }

  function moveItemToEquip(item, slotIndex) {
    if (!canEquip(item, slotIndex)) return false;
    if (state.equip[slotIndex] && state.equip[slotIndex].id !== item.id) return false;
    const prevArea = item.area;
    const prevSlot = item.slotIndex;
    if (prevArea === 'bag') {
      clearOccupancy(item);
    }
    if (prevArea === 'equip' && state.equip[prevSlot]?.id === item.id) {
      state.equip[prevSlot] = null;
    }
    item.area = 'equip';
    item.slotIndex = slotIndex;
    state.equip[slotIndex] = item;
    return true;
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
      const item = state.items.get(selectedItemId) || state.equip.find((it) => it && it.id === selectedItemId);
      if (!item) return;
      item.level = Number(root.querySelector('#item-level').value || 0);
      item.durability = Number(root.querySelector('#item-durability').value || 255);
      item.skill = root.querySelector('#item-skill').checked ? 1 : 0;
      item.luck = root.querySelector('#item-luck').checked ? 1 : 0;
      item.add = Number(root.querySelector('#item-add').value || 0);
      item.excellent = Number(root.querySelector('#item-exc').value || 0);
      item.bytes = encodeItem(item);
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
      if (item.area === 'bag') {
        clearOccupancy(item);
      }
      if (mode === 'inventory' && item.area === 'equip' && state.equip[item.slotIndex]?.id === item.id) {
        state.equip[item.slotIndex] = null;
      }
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
          newItem.area = 'bag';
          newItem.slotIndex = slotIndex;
          newItem.bytes = encodeItem(newItem);
          state.items.set(newItem.id, newItem);
          occupy(newItem, slotIndex);
          placeMode = false;
          selectedItemId = newItem.id;
          renderItems();
          setMessage('Item colocado.', 'success');
          return;
        }

        if (selectedItemId) {
          const item = state.items.get(selectedItemId);
          if (item) {
            if (moveItemToBag(item, slotIndex)) {
              renderItems();
            } else {
              setMessage('No se puede mover a esa posicion.', 'error');
            }
            return;
          }
        }
      });
    }

    root.querySelectorAll('.equip-slot').forEach((slotEl) => {
      slotEl.addEventListener('click', () => {
        const slotIndex = Number(slotEl.dataset.equip);
        if (placeMode) {
          const newItem = createItemFromPicker();
          if (!newItem) {
            setMessage('Selecciona un item para colocar.', 'error');
            return;
          }
          if (!canEquip(newItem, slotIndex)) {
            setMessage('Ese item no puede equiparse ahi.', 'error');
            return;
          }
          if (state.equip[slotIndex]) {
            setMessage('Ese slot esta ocupado.', 'error');
            return;
          }
          newItem.area = 'equip';
          newItem.slotIndex = slotIndex;
          newItem.bytes = encodeItem(newItem);
          state.items.set(newItem.id, newItem);
          state.equip[slotIndex] = newItem;
          placeMode = false;
          selectedItemId = newItem.id;
          renderItems();
          setMessage('Item equipado.', 'success');
          return;
        }

        if (selectedItemId) {
          const selected = state.items.get(selectedItemId);
          if (selected && selected.id !== state.equip[slotIndex]?.id) {
            if (moveItemToEquip(selected, slotIndex)) {
              renderItems();
              return;
            }
            setMessage('No se puede equipar en ese slot.', 'error');
            return;
          }
        }

        const item = state.equip[slotIndex];
        if (item) {
          selectItem(item);
        }
      });
    });

    root.addEventListener('click', (event) => {
      const itemEl = event.target.closest('.inventory-item, .equip-item');
      if (!itemEl) return;
      const itemId = Number(itemEl.dataset.itemId);
      const item = state.items.get(itemId) || state.equip.find((it) => it && it.id === itemId);
      if (item) selectItem(item);
    });

    if (hexInput) {
      const form = hexInput.closest('form');
      if (form) {
        form.addEventListener('submit', () => {
          const totalSlots = mode === 'inventory' ? 76 : 120;
          const buffer = new Uint8Array(totalSlots * 10).fill(0xff);
          for (const item of state.items.values()) {
            const slot = item.area === 'equip' ? item.slotIndex : item.slotIndex + equipSlots;
            const bytes = encodeItem(item);
            buffer.set(bytes, slot * 10);
          }
          if (mode === 'inventory') {
            for (let i = 0; i < state.equip.length; i++) {
              const item = state.equip[i];
              if (!item) continue;
              const bytes = encodeItem(item);
              buffer.set(bytes, i * 10);
            }
          }
          hexInput.value = bytesToHex(buffer);
        });
      }
    }
  }

  function loadItemsFromHex() {
    const bytes = hexToBytes(initialHex);
    const totalSlots = mode === 'inventory' ? 76 : 120;
    for (let slot = 0; slot < totalSlots; slot++) {
      const offset = slot * 10;
      const chunk = bytes.slice(offset, offset + 10);
      const item = decodeItem(chunk);
      if (!item) continue;
      item.bytes = chunk;
      if (mode === 'inventory' && slot < equipSlots) {
        item.area = 'equip';
        item.slotIndex = slot;
        state.items.set(item.id, item);
        state.equip[slot] = item;
      } else {
        const bagSlot = mode === 'inventory' ? slot - equipSlots : slot;
        item.area = 'bag';
        item.slotIndex = bagSlot;
        state.items.set(item.id, item);
        occupy(item, bagSlot);
      }
    }
  }

  fetch('/admin/item-defs.json')
    .then((resp) => resp.json())
    .then((data) => {
      itemDefs = Array.isArray(data.items) ? data.items : [];
      itemDefsMap = new Map(itemDefs.map((def) => [`${def.section}:${def.index}`, def]));
      loadItemsFromHex();
      render();
    })
    .catch(() => {
      setMessage('No se pudo cargar la lista de items.', 'error');
    });
})();
