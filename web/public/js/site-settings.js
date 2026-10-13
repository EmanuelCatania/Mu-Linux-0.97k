(() => {
  const language = document.getElementById('site-language');
  language?.addEventListener('change', () => {
    const url = new URL(location.href);
    url.searchParams.set('previewLanguage', language.value);
    url.searchParams.set('previewTheme', document.querySelector('input[name="theme"]:checked').value);
    url.searchParams.delete('saved');
    location.assign(url);
  });
  const box = document.querySelector('[data-editor-status]');
  const button = document.getElementById('check-editor');
  if (!box || !button) return;
  button.addEventListener('click', async () => {
    button.disabled = true;
    let status = 'disconnected';
    try {
      const response = await fetch('/admin/settings/editor-status', { cache: 'no-store' });
      if (response.ok) status = (await response.json()).status;
    } catch { /* La falta de respuesta se muestra como desconexión. */ }
    if (!['connected', 'disconnected', 'disabled'].includes(status)) status = 'disconnected';
    box.querySelector('.status-dot').className = `status-dot ${status}`;
    box.querySelector('[data-status-label]').textContent = box.dataset[status];
    button.disabled = false;
  });
})();
