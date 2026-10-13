const crypto = require('crypto');
const { translator, locales } = require('./i18n');

const languages = Object.freeze({ es: 'Español', en: 'English', pt: 'Português' });
const themes = Object.freeze({ classic: 'Clásico', 'daedalus-elf': 'Daedalus · Elf', 'daedalus-dk': 'Daedalus · Dark Knight', 'daedalus-dw': 'Daedalus · Dark Wizard' });

function validSettings(body) {
  return typeof body.language === 'string' && typeof body.theme === 'string' && Object.hasOwn(languages, body.language) && Object.hasOwn(themes, body.theme);
}

// Estado compartido en MySQL: persiste al recrear la web y entre sus instancias.
async function ensureSettings(pool) {
  await pool.query(`CREATE TABLE IF NOT EXISTS web_site_settings (
    id TINYINT UNSIGNED NOT NULL PRIMARY KEY,
    language VARCHAR(2) NOT NULL DEFAULT 'es',
    theme VARCHAR(32) NOT NULL DEFAULT 'classic',
    setup_complete TINYINT(1) NOT NULL DEFAULT 0
  )`);
  await pool.query('INSERT IGNORE INTO web_site_settings (id) VALUES (1)');
}

async function readSettings(pool) {
  const [rows] = await pool.query('SELECT language, theme, setup_complete FROM web_site_settings WHERE id = 1');
  if (!rows[0] || !validSettings(rows[0])) throw new Error('Invalid site settings');
  return rows[0];
}

function csrf(req) {
  if (!req.session.settingsCsrf) req.session.settingsCsrf = crypto.randomBytes(32).toString('hex');
  return req.session.settingsCsrf;
}

function registerSettings(app, { pool, requireAdmin, requirePassword, editorEnabled, editorRequest }) {
  const status = async () => {
    if (!editorEnabled) return 'disabled';
    try {
      const result = await editorRequest('GET', '/api/status');
      return result.enabled === true ? 'connected' : 'disconnected';
    } catch { return 'disconnected'; }
  };
  const render = async (req, res, next) => {
    try {
      const setup = req.route.path === '/admin/setup';
      if (setup && res.locals.siteSettings.setup_complete) return res.redirect('/admin/settings');
      // Vista previa del formulario: no guarda opciones ni completa el asistente.
      if (typeof req.query.previewLanguage === 'string' && Object.hasOwn(languages, req.query.previewLanguage)) {
        const language = req.query.previewLanguage;
        res.locals.language = language;
        res.locals.locale = locales[language];
        res.locals.t = translator(language);
        res.locals.siteSettings = { ...res.locals.siteSettings, language };
      }
      if (typeof req.query.previewTheme === 'string' && Object.hasOwn(themes, req.query.previewTheme)) res.locals.siteSettings = { ...res.locals.siteSettings, theme: req.query.previewTheme };
      res.render('admin_settings', { setup, languages, themes, editorStatus: await status(), csrf: csrf(req), error: null, saved: req.query.saved === '1' });
    } catch (err) { next(err); }
  };
  const save = async (req, res, next) => {
    try {
      if (!req.session.settingsCsrf || req.body.csrf !== req.session.settingsCsrf) return res.sendStatus(403);
      const setup = req.route.path === '/admin/setup';
      if (!validSettings(req.body)) return res.status(400).render('admin_settings', { setup, languages, themes, editorStatus: await status(), csrf: csrf(req), error: 'Configuración inválida.', saved: false });
      // Un segundo POST del asistente no puede modificar una instalación ya configurada.
      const [result] = await pool.query(`UPDATE web_site_settings SET language = ?, theme = ?, setup_complete = 1 WHERE id = 1${setup ? ' AND setup_complete = 0' : ''}`, [req.body.language, req.body.theme]);
      if (setup && result.affectedRows !== 1) return res.redirect('/admin/settings');
      res.redirect('/admin/settings?saved=1');
    } catch (err) { next(err); }
  };
  for (const route of ['/admin/setup', '/admin/settings']) {
    app.get(route, requireAdmin, requirePassword, render);
    app.post(route, requireAdmin, requirePassword, save);
  }
  app.get('/admin/settings/editor-status', requireAdmin, requirePassword, async (req, res) => {
    res.set('Cache-Control', 'no-store').json({ status: await status() });
  });
}
module.exports = { languages, themes, validSettings, ensureSettings, readSettings, registerSettings };
