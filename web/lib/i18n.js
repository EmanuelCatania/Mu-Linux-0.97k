// Se comparte el catálogo entre las vistas del servidor y los controles del editor.
const catalogs = { es: require('../locales/es.json'), en: require('../locales/en.json'), pt: require('../locales/pt.json') };
const locales = { es: 'es-ES', en: 'en-US', pt: 'pt-BR' };
function translator(language) {
  const catalog = catalogs[language] || catalogs.es;
  return (value) => {
    if (value == null) return '';
    const text = String(value);
    const key = text.replace(/\s+/g, ' ').trim();
    return Object.hasOwn(catalog, key) ? catalog[key] : text;
  };
}
function resolveLanguage(req, defaultLanguage) {
  if (req.path.startsWith('/admin')) return defaultLanguage;
  if (req.method === 'GET' && typeof req.query.lang === 'string' && Object.hasOwn(locales, req.query.lang)) req.session.language = req.query.lang;
  return Object.hasOwn(locales, req.session.language) ? req.session.language : defaultLanguage;
}
function registerCatalogs(app) {
  app.get('/locales/:language.js', (req, res) => {
    if (!Object.hasOwn(catalogs, req.params.language)) return res.sendStatus(404);
    res.type('application/javascript').set('Cache-Control', 'no-cache').send(`window.MuText.catalog = ${JSON.stringify(catalogs[req.params.language]).replace(/</g, '\\u003c')};`);
  });
}
module.exports = { translator, locales, registerCatalogs, resolveLanguage };
