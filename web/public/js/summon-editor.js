(function () {
  const root = document.getElementById('summon-editor');
  if (!root) return;
  const T = (s, p) => window.MuText.t(s, p);

  const state = { slots: [], constants: {}, monsters: [] };

  function setMessage(text, type) {
    const box = root.querySelector('.editor-message');
    box.textContent = text || '';
    box.classList.toggle('hidden', !text);
    box.classList.toggle('success', type === 'success');
    box.classList.toggle('error', type === 'error');
  }

  // Misma cuenta que CSkillManager::SkillSummon en el GameServer.
  function rate(energy, level, resets) {
    const c = state.constants;
    let value = Number(c.SummonConstA) || 0;
    if (c.SummonConstB > 0) value += Math.floor(energy / c.SummonConstB);
    if (c.SummonConstC > 0) value += Math.floor(level / c.SummonConstC);
    if (c.SummonResetRate > 0 && resets > 0) value += resets * c.SummonResetRate;
    return Math.min(value, Number(c.SummonMaxRate) || 0);
  }

  function monsterOptions(selected) {
    const sorted = [...state.monsters].sort((a, b) => (a.level - b.level) || (a.id - b.id));
    const none = `<option value="0"${selected === 0 ? ' selected' : ''}>${T('(sin invocación)')}</option>`;
    return none + sorted.map((m) => {
      const label = `${m.name} — ${T('nivel')} ${m.level} (#${m.id})`;
      return `<option value="${m.id}"${m.id === selected ? ' selected' : ''}>${label.replace(/</g, '&lt;')}</option>`;
    }).join('');
  }

  function render() {
    const rows = state.slots.map((id, n) => `
      <tr>
        <td>${T('Orbe')} +${n}</td>
        <td>${30 + n}</td>
        <td><select data-slot="${n}">${monsterOptions(id)}</select></td>
      </tr>`).join('');
    const c = state.constants;
    root.innerHTML = window.MuText.html`
      <div class="editor-message alert hidden"></div>
      <h3>Monstruos</h3>
      <table class="table">
        <thead><tr><th>Orbe</th><th>Skill</th><th>Monstruo</th></tr></thead>
        <tbody>${rows}</tbody>
      </table>
      <h3>Escalado con la elfa</h3>
      <p class="muted">Porcentaje = A + Energía / B + Nivel / C + Resets × R, con tope. Sube vida, daño, defensas y tasas de ataque y defensa. 0 en B o C desactiva ese término.</p>
      <div class="form-grid">
        <label class="field">A <input type="number" min="0" data-const="SummonConstA" value="${c.SummonConstA}"></label>
        <label class="field">B <input type="number" min="0" data-const="SummonConstB" value="${c.SummonConstB}"></label>
        <label class="field">C <input type="number" min="0" data-const="SummonConstC" value="${c.SummonConstC}"></label>
        <label class="field">R (% por reset) <input type="number" min="0" data-const="SummonResetRate" value="${c.SummonResetRate}"></label>
        <label class="field">Tope (%) <input type="number" min="0" data-const="SummonMaxRate" value="${c.SummonMaxRate}"></label>
      </div>
      <p class="muted">Ejemplo: energía <input type="number" min="0" id="sample-energy" value="1000" style="width:6em"> nivel <input type="number" min="1" id="sample-level" value="200" style="width:5em"> resets <input type="number" min="0" id="sample-resets" value="0" style="width:5em"> → <strong id="sample-rate"></strong></p>
      <div class="editor-actions">
        <button type="button" id="save-summons">Guardar</button>
      </div>`;
    updateSample();
  }

  function updateSample() {
    const energy = Number(root.querySelector('#sample-energy')?.value) || 0;
    const level = Number(root.querySelector('#sample-level')?.value) || 0;
    const resets = Number(root.querySelector('#sample-resets')?.value) || 0;
    const out = root.querySelector('#sample-rate');
    if (out) out.textContent = `+${rate(energy, level, resets)}%`;
  }

  root.addEventListener('change', (ev) => {
    const el = ev.target;
    if (el.dataset.slot !== undefined) state.slots[Number(el.dataset.slot)] = Number(el.value);
    if (el.dataset.const) state.constants[el.dataset.const] = Number(el.value);
    updateSample();
  });
  root.addEventListener('input', (ev) => {
    if (ev.target.dataset.const) state.constants[ev.target.dataset.const] = Number(ev.target.value);
    updateSample();
  });

  root.addEventListener('click', async (ev) => {
    if (ev.target.id !== 'save-summons') return;
    try {
      const res = await fetch('/admin/server-editor/api/summons', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ slots: state.slots, constants: state.constants })
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || T('No se pudo guardar'));
      const reload = await fetch('/admin/server-editor/api/reload', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ target: 'skill' })
      });
      setMessage(T(reload.ok ? 'Guardado y recargado en el servidor.' : 'Guardado, pero no se pudo recargar en el server.'),
        reload.ok ? 'success' : 'error');
    } catch (err) {
      setMessage(err.message, 'error');
    }
  });

  async function init() {
    root.innerHTML = '<div class="editor-message alert hidden"></div>';
    try {
      const res = await fetch('/admin/server-editor/api/summons');
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || T('No se pudo cargar'));
      state.slots = data.slots;
      state.constants = data.constants;
      state.monsters = data.monsters;
      render();
    } catch (err) {
      setMessage(err.message, 'error');
    }
  }

  init();
})();
