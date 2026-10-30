(function () {
  const root = document.getElementById('custom-items-editor');
  if (!root) return;

  const t = (text, params) => window.MuText.t(text, params);
  const MAX_ROWS = 300;

  const state = {
    items: [],
    files: { items: new Set(), pets: new Set() },
    colors: [],
    maxTooltipLines: 6,
    filter: '',
    onlyCustom: false,
    selected: null,       // { key, name, slot }
    // Por tipo: null = no tiene archivo; { data, header, dirty, hasInnerComments }
    docs: { items: null, pets: null },
    // Fila de Item.txt: { mode: 'edit' | 'add', model, dirty }
    row: null,
    layouts: [],          // columnas de datos de cada sección
    prefix: [],
    maxNameBytes: 31
  };

  const keyId = (key) => `${key[0]},${key[1]}`;
  const keyLabel = (key) => `${key[0]}/${key[1]}`;
  const hasFile = (kind, key) => state.files[kind].has(keyId(key));

  function setMessage(text, type) {
    const box = root.querySelector('.editor-message');
    if (!box) return;
    box.textContent = text || '';
    box.classList.toggle('hidden', !text);
    box.classList.toggle('success', type === 'success');
    box.classList.toggle('error', type === 'error');
  }

  function showErrors(error, errors) {
    const box = root.querySelector('.editor-message');
    box.classList.remove('hidden', 'success');
    box.classList.add('error');
    box.textContent = '';
    box.append(document.createTextNode(error));
    if (Array.isArray(errors) && errors.length) {
      const list = document.createElement('ul');
      errors.forEach((e) => { const li = document.createElement('li'); li.textContent = e; list.appendChild(li); });
      box.appendChild(list);
    }
  }

  async function api(url, options) {
    const res = await fetch(url, options);
    const data = await res.json().catch(() => ({}));
    if (!res.ok) {
      const err = new Error(data.error || t('No se pudo completar'));
      err.errors = data.errors;
      throw err;
    }
    return data;
  }

  const post = (url, body) => api(url, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) });

  // ── Helpers de DOM ─────────────────────────────────────────────────────────
  function el(tag, props, ...children) {
    const node = document.createElement(tag);
    Object.entries(props || {}).forEach(([k, v]) => {
      if (k === 'class') node.className = v;
      else if (k === 'text') node.textContent = v;
      else if (k.startsWith('on')) node.addEventListener(k.slice(2), v);
      else if (v !== undefined && v !== null && v !== false) node.setAttribute(k, v === true ? '' : v);
    });
    children.flat().forEach((c) => { if (c) node.append(c); });
    return node;
  }

  function field(label, input, hint) {
    const wrap = el('label', { class: 'ci-field' }, el('span', { text: label }), input);
    if (hint) wrap.title = hint;
    return wrap;
  }

  // Campo numérico: vacío = se quita la clave.
  function numberInput(obj, key, step, onChange) {
    const input = el('input', { type: 'number', step: step || 'any' });
    if (obj[key] !== undefined) input.value = obj[key];
    input.addEventListener('input', () => {
      if (input.value === '') delete obj[key]; else obj[key] = Number(input.value);
      onChange();
    });
    return input;
  }

  function vectorInputs(obj, key, size, onChange) {
    const wrap = el('span', { class: 'ci-vector' });
    for (let i = 0; i < size; i += 1) {
      const input = el('input', { type: 'number', step: 'any' });
      if (Array.isArray(obj[key]) && obj[key][i] !== undefined) input.value = obj[key][i];
      input.addEventListener('input', () => {
        const values = Array.from(wrap.querySelectorAll('input')).map((x) => (x.value === '' ? null : Number(x.value)));
        if (values.every((v) => v === null)) delete obj[key];
        else obj[key] = values.map((v) => (v === null ? 0 : v));
        onChange();
      });
      wrap.appendChild(input);
    }
    return wrap;
  }

  // ── Lista ──────────────────────────────────────────────────────────────────
  function visibleItems() {
    const f = state.filter.toLowerCase();
    return state.items.filter((item) => {
      if (state.onlyCustom && !hasFile('items', item.key) && !hasFile('pets', item.key)) return false;
      if (!f) return true;
      return item.name.toLowerCase().includes(f) || keyId(item.key).includes(f) || keyLabel(item.key).includes(f);
    });
  }

  function renderList() {
    const list = root.querySelector('#ci-list');
    list.textContent = '';
    const shown = visibleItems();
    shown.slice(0, MAX_ROWS).forEach((item) => {
      const li = el('li', { class: state.selected && keyId(state.selected.key) === keyId(item.key) ? 'selected' : '' },
        el('span', { class: 'ci-key', text: keyLabel(item.key) }),
        el('span', { text: item.name }),
        hasFile('items', item.key) ? el('span', { class: 'ci-badge', title: t('Tiene archivo de efectos, pose y tooltip'), text: 'JSON' }) : null,
        hasFile('pets', item.key) ? el('span', { class: 'ci-badge ci-badge-pet', title: t('Tiene archivo de pet'), text: 'PET' }) : null);
      li.addEventListener('click', () => selectItem(item));
      list.appendChild(li);
    });
    root.querySelector('#ci-more').textContent = shown.length > MAX_ROWS
      ? t('Mostrando {shown} de {total}. Afiná la búsqueda para ver el resto.', { shown: MAX_ROWS, total: shown.length })
      : '';
  }

  // ── Detalle ────────────────────────────────────────────────────────────────
  async function selectItem(item, force) {
    if (!force && anyDirty() && !window.confirm(t('Hay cambios sin guardar. ¿Descartarlos?'))) return;
    state.selected = item;
    state.docs = { items: null, pets: null };
    state.row = null;
    renderList();
    try {
      const rowData = await api(`/admin/server-editor/api/custom-items/row?key=${keyId(item.key)}`);
      state.row = { mode: 'edit', model: rowData.row, dirty: false, nextFree: rowData.nextFree };
      for (const kind of ['items', 'pets']) {
        const data = await api(`/admin/server-editor/api/custom-items/file?kind=${kind}&key=${keyId(item.key)}`);
        if (data.exists && data.error) {
          setMessage(t('El archivo existente no se pudo leer: {error}', { error: data.error }), 'error');
        } else if (data.exists) {
          state.docs[kind] = { data: data.data, header: data.header || [], dirty: false, hasInnerComments: !!data.hasInnerComments };
        }
      }
    } catch (err) {
      setMessage(err.message, 'error');
    }
    renderDetail();
  }

  const anyDirty = () => Object.values(state.docs).some((d) => d && d.dirty) || !!(state.row && state.row.dirty);

  function markDirty(kind) {
    state.docs[kind].dirty = true;
    const preview = root.querySelector(`#ci-preview-${kind}`);
    if (preview) preview.textContent = JSON.stringify(state.docs[kind].data, null, 2);
    const save = root.querySelector(`#ci-save-${kind}`);
    if (save) save.classList.add('ci-pending');
  }

  function renderDetail() {
    const box = root.querySelector('#ci-detail');
    box.textContent = '';
    if (!state.selected) {
      box.appendChild(el('p', { class: 'muted', text: t('Elegí un item de la lista.') }));
      return;
    }
    const item = state.selected;
    if (state.row && state.row.mode === 'add') {
      box.appendChild(renderRowSection());
      return;
    }
    box.appendChild(el('h3', { text: `${item.name}  (${keyId(item.key)})` }));
    if (state.row) box.appendChild(renderRowSection());
    box.appendChild(renderItemSection(item));
    box.appendChild(renderPetSection(item));
  }

  // ── Fila de Item.txt: datos, columnas de la sección y columnas custom ──────
  const intValue = (input) => (input.value === '' ? 0 : Math.trunc(Number(input.value)));

  function rowPreview(model) {
    const q = (s) => `"${s}"`;
    const tail = model.tail || {};
    const pad = (n, w) => String(n).padStart(w, '0');
    const opt = [
      tail.behavior ? `${pad(tail.behavior[0], 2)},${pad(tail.behavior[1], 3)}` : '*',
      tail.folder ? q(tail.folder) : '*',
      tail.model ? q(tail.model) : '*',
      ...(tail.glow ? tail.glow : ['*']),
      tail.equippedFolder ? q(tail.equippedFolder) : '*',
      tail.equippedModel ? q(tail.equippedModel) : '*',
      tail.gate !== null && tail.gate !== undefined ? tail.gate : '*'
    ];
    while (opt.length && opt[opt.length - 1] === '*') opt.pop();
    return [model.key[1], model.slot, model.skill, model.width, model.height, model.serial, model.option, model.drop, q(model.name), ...model.stats, ...opt].join('\t');
  }

  function renderRowSection() {
    const draft = state.row;
    const model = draft.model;
    const add = draft.mode === 'add';
    model.tail = model.tail || {};
    const tail = model.tail;
    const preview = el('pre', { class: 'ci-preview' });
    const refresh = () => { preview.textContent = rowPreview(model); };
    const dirty = () => {
      draft.dirty = true;
      refresh();
      const btn = root.querySelector('#ci-save-row');
      if (btn) btn.classList.add('ci-pending');
    };

    const frame = el('div', { class: 'ci-section' },
      el('h4', { text: add ? t('Item nuevo') : t('Datos en Item.txt') }),
      el('p', { class: 'muted', text: add
        ? t('Se agrega una fila a Item.txt copiando los datos del item elegido. Cambiá lo que haga falta.')
        : t('La fila del item en Data/Item/Item.txt. Las columnas custom del final son opcionales; vacío es "la original".') }));

    // Identidad
    const name = el('input', { type: 'text', value: model.name, maxlength: 60 });
    name.addEventListener('input', () => { model.name = name.value; dirty(); });
    const index = el('input', { type: 'number', min: 0, max: 511, value: model.key[1] });
    index.disabled = !add;
    index.addEventListener('input', () => { model.key[1] = intValue(index); dirty(); });
    const ident = el('div', { class: 'ci-grid' },
      field(t('Nombre (hasta {max} bytes)', { max: state.maxNameBytes }), name),
      field(t('Sección'), el('input', { type: 'number', value: model.key[0], disabled: true })),
      field(t('Índice (0 a 511; los agregados desde 32)'), index));
    frame.appendChild(ident);

    // Datos comunes y de la sección
    const common = el('details', { open: true }, el('summary', { text: t('Datos del item') }));
    const grid = el('div', { class: 'ci-grid' });
    const commonFields = [['slot', 'Slot'], ['skill', 'Skill'], ['width', t('Ancho')], ['height', t('Alto')], ['serial', 'Serial'], ['option', t('Opción')], ['drop', 'Drop']];
    commonFields.forEach(([key, label]) => {
      const input = el('input', { type: 'number', step: 1, value: model[key] });
      input.addEventListener('input', () => { model[key] = intValue(input); dirty(); });
      grid.appendChild(field(label, input));
    });
    (state.layouts[model.key[0]] || []).forEach((label, n) => {
      const input = el('input', { type: 'number', step: 1, value: model.stats[n] ?? 0 });
      input.addEventListener('input', () => { model.stats[n] = intValue(input); dirty(); });
      grid.appendChild(field(label, input));
    });
    common.appendChild(grid);
    frame.appendChild(common);

    // Columnas custom
    const custom = el('fieldset', {}, el('legend', { text: t('Columnas custom (opcionales)') }));
    const behaviorSec = el('input', { type: 'number', min: 0, max: 15, value: tail.behavior ? tail.behavior[0] : '' });
    const behaviorIdx = el('input', { type: 'number', min: 0, max: 511, value: tail.behavior ? tail.behavior[1] : '' });
    const setBehavior = () => {
      tail.behavior = behaviorSec.value === '' || behaviorIdx.value === '' ? null : [intValue(behaviorSec), intValue(behaviorIdx)];
      dirty();
    };
    behaviorSec.addEventListener('input', setBehavior);
    behaviorIdx.addEventListener('input', setBehavior);
    const textField = (key, placeholder) => {
      const input = el('input', { type: 'text', value: tail[key] || '', placeholder });
      input.addEventListener('input', () => { tail[key] = input.value === '' ? null : input.value; dirty(); });
      return input;
    };
    const glowBox = el('span', { class: 'ci-vector' });
    for (let i = 0; i < 3; i += 1) {
      const input = el('input', { type: 'number', min: 0, max: 255, value: tail.glow ? tail.glow[i] : '' });
      input.addEventListener('input', () => {
        const values = Array.from(glowBox.querySelectorAll('input')).map((x) => (x.value === '' ? null : intValue(x)));
        tail.glow = values.every((v) => v === null) ? null : values.map((v) => (v === null ? 0 : v));
        dirty();
      });
      glowBox.appendChild(input);
    }
    const gate = el('input', { type: 'number', min: 0, value: tail.gate ?? '' });
    gate.addEventListener('input', () => { tail.gate = gate.value === '' ? null : intValue(gate); dirty(); });
    custom.append(el('div', { class: 'ci-grid' },
      field(t('Comportamiento: sección'), behaviorSec, t('Item original que imita en la lógica. Obligatorio en los agregados (índice 32 o más).')),
      field(t('Comportamiento: índice'), behaviorIdx),
      field(t('Carpeta del modelo'), textField('folder', 'Item\\Custom\\20\\')),
      field(t('Nombre del modelo'), textField('model', 'Sword21')),
      field(t('Brillo R, G, B (0 a 255)'), glowBox),
      field(t('Carpeta del modelo puesto'), textField('equippedFolder', ''), t('Solo si el item se ve distinto puesto, como una montura.')),
      field(t('Nombre del modelo puesto'), textField('equippedModel', '')),
      field(t('Gate de destino'), gate, t('Para pergaminos de teleport a un mapa fijo.'))));
    frame.appendChild(custom);

    frame.appendChild(el('details', {}, el('summary', { text: t('Ver la fila tal como queda') }), preview));
    refresh();

    frame.appendChild(el('div', { class: 'spawn-actions' },
      el('button', { type: 'button', id: 'ci-save-row', text: add ? t('Agregar item') : t('Guardar fila'), onclick: () => saveRow() }),
      add
        ? el('button', { type: 'button', class: 'link-button', text: t('Cancelar'), onclick: () => cancelAdd() })
        : el('button', { type: 'button', class: 'link-button', text: t('Crear un item nuevo a partir de este'), onclick: () => startAdd() })));
    return frame;
  }

  function startAdd() {
    if (anyDirty() && !window.confirm(t('Hay cambios sin guardar. ¿Descartarlos?'))) return;
    const source = state.row.model;
    const copy = JSON.parse(JSON.stringify(source));
    copy.key[1] = state.row.nextFree > 0 ? state.row.nextFree : 32;
    copy.name = `${source.name} (copia)`.slice(0, state.maxNameBytes);
    copy.tail = copy.tail || {};
    // Un agregado imita a un original: el original del que parte, o el que ese ya imitaba.
    if (!copy.tail.behavior) copy.tail.behavior = [source.key[0], source.key[1]];
    state.row = { mode: 'add', model: copy, dirty: true, nextFree: state.row.nextFree, source: state.selected };
    renderDetail();
  }

  function cancelAdd() {
    const source = state.row.source;
    state.row = null;
    selectItem(source, true);
  }

  async function saveRow() {
    const draft = state.row;
    try {
      await post('/admin/server-editor/api/custom-items/row', { mode: draft.mode, row: draft.model });
      draft.dirty = false;
      const key = draft.model.key.slice();
      await loadList();
      const saved = state.items.find((item) => keyId(item.key) === keyId(key));
      const reloaded = await reloadServer();
      setMessage(reloaded
        ? t('Guardado y recargado en el servidor. Los jugadores lo ven al volver a entrar.')
        : t('Guardado, pero no se pudo recargar en el server.'), reloaded ? 'success' : 'error');
      if (saved) await selectItem(saved, true);
    } catch (err) {
      showErrors(err.message, err.errors);
    }
  }

  function sectionFrame(kind, title, intro) {
    const doc = state.docs[kind];
    const frame = el('div', { class: 'ci-section' }, el('h4', { text: title }), el('p', { class: 'muted', text: intro }));
    if (!doc) {
      frame.appendChild(el('button', { type: 'button', text: t('Crear archivo'), onclick: () => createDoc(kind) }));
      return { frame, doc: null };
    }
    if (doc.hasInnerComments) {
      frame.appendChild(el('p', { class: 'alert', text: t('Este archivo tiene comentarios dentro del JSON: al guardar se pierden (los del principio se conservan).') }));
    }
    return { frame, doc };
  }

  function sectionFooter(kind, frame) {
    frame.appendChild(el('details', {}, el('summary', { text: t('Ver JSON') }),
      el('pre', { id: `ci-preview-${kind}`, class: 'ci-preview', text: JSON.stringify(state.docs[kind].data, null, 2) })));
    frame.appendChild(el('div', { class: 'spawn-actions' },
      el('button', { type: 'button', id: `ci-save-${kind}`, text: t('Guardar'), onclick: () => save(kind) }),
      el('button', { type: 'button', class: 'link-button', text: t('Eliminar archivo'), onclick: () => remove(kind) })));
  }

  function createDoc(kind) {
    const item = state.selected;
    const data = { item: keyId(item.key) };
    if (kind === 'pets') data.movement = { type: 'follow', radius: 50, period: 4000, height: 20 };
    state.docs[kind] = { data, header: [], dirty: true, hasInnerComments: false };
    renderDetail();
  }

  // ── Items: pose, tooltip y efectos ─────────────────────────────────────────
  function renderItemSection(item) {
    const { frame, doc } = sectionFrame('items', t('Efectos, pose y tooltip'),
      t('Archivo Data/Custom/Items/{name}. Sirve para items custom y para los originales.', { name: `${String(item.key[0]).padStart(2, '0')}_${String(item.key[1]).padStart(3, '0')}.json` }));
    if (!doc) return frame;
    const data = doc.data;
    const dirty = () => markDirty('items');

    // Pose en el inventario
    const pose = el('fieldset', {}, el('legend', { text: t('Pose en el inventario') }));
    const usePose = el('input', { type: 'checkbox' });
    usePose.checked = !!data.inventory;
    const poseBody = el('div', { class: 'ci-grid' });
    const drawPose = () => {
      poseBody.textContent = '';
      if (!data.inventory) return;
      poseBody.append(
        field(t('Posición X, Y'), vectorInputs(data.inventory, 'position', 2, dirty)),
        field(t('Rotación X, Y, Z'), vectorInputs(data.inventory, 'rotation', 3, dirty)),
        field(t('Escala (0 = la original)'), numberInput(data.inventory, 'scale', 'any', dirty)));
    };
    usePose.addEventListener('change', () => {
      if (usePose.checked) data.inventory = { position: [0, 0], rotation: [0, 0, 0] }; else delete data.inventory;
      drawPose(); dirty();
    });
    pose.append(el('label', {}, usePose, ' ', t('Definir la pose')), poseBody);
    drawPose();

    // Tooltip
    const tip = el('fieldset', {}, el('legend', { text: t('Líneas del tooltip') }));
    const tipBody = el('div', {});
    const asLine = (line) => (typeof line === 'string' ? { text: line, color: 'white' } : line);
    const drawTip = () => {
      tipBody.textContent = '';
      (data.tooltip || []).forEach((raw, index) => {
        const line = asLine(raw);
        data.tooltip[index] = line;
        const text = el('input', { type: 'text', value: line.text || '', placeholder: t('Texto (hasta 59 bytes)') });
        text.addEventListener('input', () => { line.text = text.value; dirty(); });
        const color = el('select', {}, ...state.colors.map((c) => el('option', { value: c, text: c, selected: (line.color || 'white') === c })));
        color.addEventListener('change', () => { line.color = color.value; dirty(); });
        const del = el('button', { type: 'button', class: 'link-button', text: t('Quitar'), onclick: () => { data.tooltip.splice(index, 1); if (!data.tooltip.length) delete data.tooltip; drawTip(); dirty(); } });
        tipBody.appendChild(el('div', { class: 'ci-row' }, text, color, del));
      });
    };
    const addTip = el('button', { type: 'button', text: t('Agregar línea'), onclick: () => {
      data.tooltip = data.tooltip || [];
      if (data.tooltip.length >= state.maxTooltipLines) { setMessage(t('El tooltip admite hasta {max} líneas.', { max: state.maxTooltipLines }), 'error'); return; }
      data.tooltip.push({ text: '', color: 'white' }); drawTip(); dirty();
    } });
    tip.append(tipBody, addTip);
    drawTip();

    // Efectos
    const fx = el('fieldset', {}, el('legend', { text: t('Efectos sobre los huesos del modelo') }));
    const fxBody = el('div', {});
    const drawFx = () => {
      fxBody.textContent = '';
      (data.effects || []).forEach((effect, index) => fxBody.appendChild(effectCard(data, effect, index, drawFx, dirty)));
    };
    const addFx = el('button', { type: 'button', text: t('Agregar efecto'), onclick: () => {
      data.effects = data.effects || [];
      data.effects.push({ on: 'equipped', type: 'sprite', texture: 'Effect/Flare.jpg', bones: [0], color: [1, 1, 1], scale: 1 });
      drawFx(); dirty();
    } });
    fx.append(fxBody, addFx);
    drawFx();

    frame.append(pose, tip, fx);
    sectionFooter('items', frame);
    return frame;
  }

  function effectCard(data, effect, index, redraw, dirty) {
    const typeSel = el('select', {}, el('option', { value: 'sprite', text: 'sprite', selected: effect.type === 'sprite' }), el('option', { value: 'particle', text: 'particle', selected: effect.type === 'particle' }));
    typeSel.addEventListener('change', () => { effect.type = typeSel.value; if (effect.type === 'particle' && effect.particle === undefined) effect.particle = 0; redraw(); dirty(); });

    const texture = el('input', { type: 'text', value: effect.texture || '', placeholder: 'Effect/Flare.jpg' });
    texture.addEventListener('input', () => { if (texture.value === '') delete effect.texture; else effect.texture = texture.value; dirty(); });

    const bones = el('input', { type: 'text', value: (effect.bones || []).join(', '), placeholder: '5, 6, 7' });
    bones.addEventListener('input', () => {
      effect.bones = bones.value.split(/[,\s]+/).filter(Boolean).map(Number);
      dirty();
    });

    const del = el('button', { type: 'button', class: 'link-button', text: t('Quitar efecto'), onclick: () => { data.effects.splice(index, 1); if (!data.effects.length) delete data.effects; redraw(); dirty(); } });

    const usePulse = el('input', { type: 'checkbox' });
    usePulse.checked = !!effect.pulse;
    const pulseBody = el('span', { class: 'ci-vector' });
    const drawPulse = () => {
      pulseBody.textContent = '';
      if (!effect.pulse) return;
      pulseBody.append(
        field(t('Velocidad'), numberInput(effect.pulse, 'speed', 'any', dirty)),
        field(t('Escala'), numberInput(effect.pulse, 'scale', 'any', dirty)),
        field(t('Color'), numberInput(effect.pulse, 'color', 'any', dirty)));
    };
    usePulse.addEventListener('change', () => { if (usePulse.checked) effect.pulse = { speed: 0.002, scale: 0.2, color: 0.4 }; else delete effect.pulse; drawPulse(); dirty(); });
    drawPulse();

    return el('div', { class: 'ci-effect' },
      el('div', { class: 'ci-row' }, el('strong', { text: `${t('Efecto')} ${index + 1}` }), del),
      el('div', { class: 'ci-grid' },
        field(t('Tipo'), typeSel),
        effect.type === 'particle' ? field(t('Partícula (tipo del juego)'), numberInput(effect, 'particle', '1', dirty)) : null,
        effect.type === 'particle' ? field(t('Subtipo'), numberInput(effect, 'subtype', '1', dirty)) : null,
        field(t('Textura (bajo Data\\)'), texture),
        field(t('o bitmap'), numberInput(effect, 'bitmap', '1', dirty)),
        field(t('Huesos (0 a 199, hasta 32)'), bones),
        field(t('Color R, G, B (0 a 1)'), vectorInputs(effect, 'color', 3, dirty)),
        field(t('Desplazamiento X, Y, Z'), vectorInputs(effect, 'offset', 3, dirty)),
        field(t('Escala'), numberInput(effect, 'scale', 'any', dirty)),
        field(t('Probabilidad (1 = siempre)'), numberInput(effect, 'chance', '1', dirty))),
      el('label', {}, usePulse, ' ', t('Pulso')), pulseBody);
  }

  // ── Pets ───────────────────────────────────────────────────────────────────
  function renderPetSection(item) {
    if (item.slot !== 8 && !state.docs.pets) {
      return el('div', { class: 'ci-section' }, el('h4', { text: t('Pet') }),
        el('p', { class: 'muted', text: t('Este item no va en el slot del helper (Slot 8 en Item.txt): no puede ser un pet.') }));
    }
    const { frame, doc } = sectionFrame('pets', t('Pet'),
      t('Archivo Data/Custom/Pets/{name}. El cliente dibuja el movimiento y el server ejecuta las habilidades.', { name: `${String(item.key[0]).padStart(2, '0')}_${String(item.key[1]).padStart(3, '0')}.json` }));
    if (!doc) return frame;
    const data = doc.data;
    const dirty = () => markDirty('pets');
    data.movement = data.movement || { type: 'follow' };

    const type = el('select', {}, el('option', { value: 'follow', text: t('follow: vuela detrás del dueño'), selected: data.movement.type === 'follow' }), el('option', { value: 'orbit', text: t('orbit: da vueltas alrededor'), selected: data.movement.type === 'orbit' }));
    type.addEventListener('change', () => { data.movement.type = type.value; dirty(); });

    const pickup = (data.abilities || []).find((a) => a.type === 'pickup');
    const usePickup = el('input', { type: 'checkbox' });
    usePickup.checked = !!pickup;
    const pickupBody = el('div', { class: 'ci-grid' });
    const drawPickup = () => {
      pickupBody.textContent = '';
      const current = (data.abilities || []).find((a) => a.type === 'pickup');
      if (!current) return;
      pickupBody.append(
        field(t('Alcance (1 a 8 casillas)'), numberInput(current, 'range', '1', dirty)),
        field(t('Intervalo (ms)'), numberInput(current, 'interval', '1', dirty)),
        field(t('Demora (ms)'), numberInput(current, 'delay', '1', dirty)));
    };
    usePickup.addEventListener('change', () => {
      if (usePickup.checked) data.abilities = [{ type: 'pickup', what: 'zen', range: 3, interval: 1000, delay: 1500 }]; else delete data.abilities;
      drawPickup(); dirty();
    });
    drawPickup();

    frame.append(
      el('div', { class: 'ci-grid' },
        field(t('Movimiento'), type),
        field(t('Radio'), numberInput(data.movement, 'radius', 'any', dirty)),
        field(t('Período (ms, mínimo 100)'), numberInput(data.movement, 'period', '1', dirty)),
        field(t('Altura'), numberInput(data.movement, 'height', 'any', dirty)),
        field(t('Escala del modelo'), numberInput(data, 'scale', 'any', dirty)),
        field(t('Malla aditiva (blendMesh)'), numberInput(data, 'blendMesh', '1', dirty)),
        field(t('Animación'), numberInput(data, 'action', '1', dirty))),
      el('label', {}, usePickup, ' ', t('Levanta zen cercano')), pickupBody);
    sectionFooter('pets', frame);
    return frame;
  }

  // ── Guardar y borrar ───────────────────────────────────────────────────────
  async function reloadServer() {
    // 'item' vuelve a leer Item.txt, los JSON de items y pets, y los textos.
    const res = await fetch('/admin/server-editor/api/reload', {
      method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ target: 'item' })
    });
    return res.ok;
  }

  async function save(kind) {
    const doc = state.docs[kind];
    try {
      await post('/admin/server-editor/api/custom-items/file', { kind, key: state.selected.key, data: doc.data, header: doc.header });
      doc.dirty = false;
      state.files[kind].add(keyId(state.selected.key));
      renderList();
      renderDetail();
      setMessage((await reloadServer())
        ? t('Guardado y recargado en el servidor. Los jugadores lo ven al volver a entrar.')
        : t('Guardado, pero no se pudo recargar en el server.'), 'success');
    } catch (err) {
      showErrors(err.message, err.errors);
    }
  }

  async function remove(kind) {
    if (!window.confirm(t('¿Eliminar este archivo? Queda una copia en los backups.'))) return;
    try {
      if (hasFile(kind, state.selected.key)) {
        await post('/admin/server-editor/api/custom-items/delete', { kind, key: state.selected.key });
        state.files[kind].delete(keyId(state.selected.key));
        await reloadServer();
      }
      state.docs[kind] = null;
      renderList();
      renderDetail();
      setMessage(t('Archivo eliminado.'), 'success');
    } catch (err) {
      showErrors(err.message, err.errors);
    }
  }

  // ── Arranque ───────────────────────────────────────────────────────────────
  function buildLayout() {
    root.innerHTML = window.MuText.html`
      <div class="editor-message alert hidden"></div>
      <div class="spawn-toolbar">
        <div class="spawn-toolbar-group">
          <label>Buscar</label>
          <input type="text" id="ci-search" placeholder="Buscar por nombre o número..." />
        </div>
        <div class="spawn-toolbar-group">
          <label><input type="checkbox" id="ci-only" /> Solo con archivo custom</label>
        </div>
      </div>
      <div class="ci-layout">
        <div class="spawn-list">
          <ul id="ci-list" class="simple-list"></ul>
          <p class="muted" id="ci-more"></p>
        </div>
        <div id="ci-detail" class="spawn-form"></div>
      </div>
    `;
    root.querySelector('#ci-search').addEventListener('input', (e) => { state.filter = e.target.value.trim(); renderList(); });
    root.querySelector('#ci-only').addEventListener('change', (e) => { state.onlyCustom = e.target.checked; renderList(); });
    window.addEventListener('beforeunload', (e) => { if (anyDirty()) { e.preventDefault(); e.returnValue = ''; } });
  }

  // Lista de items y archivos; se repite al agregar un item para que aparezca.
  async function loadList() {
    const data = await api('/admin/server-editor/api/custom-items');
    state.items = data.items;
    state.colors = data.colors;
    state.maxTooltipLines = data.maxTooltipLines || 6;
    state.layouts = data.layouts || [];
    state.prefix = data.prefix || [];
    state.maxNameBytes = data.maxNameBytes || 31;
    state.files.items = new Set(data.files.items.map(keyId));
    state.files.pets = new Set(data.files.pets.map(keyId));
    renderList();
  }

  async function init() {
    buildLayout();
    try {
      await loadList();
      renderDetail();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
