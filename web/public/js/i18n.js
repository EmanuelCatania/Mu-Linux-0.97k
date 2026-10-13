window.MuText = {
  catalog: {},
  t(value, params = {}) {
    if (value == null) return '';
    const text = String(value);
    const key = text.replace(/\s+/g, ' ').trim();
    const translated = Object.hasOwn(this.catalog, key) ? this.catalog[key] : text;
    return translated.replace(/\{(\w+)\}/g, (match, key) => Object.hasOwn(params, key) ? String(params[key]) : match);
  }
};
// Sólo traduce fragmentos estáticos de las plantillas. Los valores interpolados
// (nombres, mensajes del servidor y archivos editados) conservan su contenido.
window.MuText.html = function (strings, ...values) {
  const escape = (s) => s.replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  return strings.map((part, i) => part
    .replace(/>([^<>]+)</g, (whole, text) => {
      const key = text.replace(/\s+/g, ' ').trim();
      return Object.hasOwn(this.catalog, key) ? '>' + escape(this.catalog[key]) + '<' : whole;
    })
    .replace(/((?:placeholder|title|aria-label)=")([^"]*)(")/g, (whole, a, key, b) => Object.hasOwn(this.catalog, key) ? a + escape(this.catalog[key]) + b : whole)
    + (i < values.length ? values[i] : '')).join('');
};
