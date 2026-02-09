const express = require('express');
const session = require('express-session');
const MySQLStore = require('express-mysql-session')(session);
const mysql = require('mysql2/promise');
const bcrypt = require('bcryptjs');
const helmet = require('helmet');
const rateLimit = require('express-rate-limit');

const app = express();

const {
  DB_HOST = 'mysql',
  DB_PORT = '3306',
  DB_USER = 'admin',
  DB_PASS = 'asd123',
  DB_NAME = 'MuOnline97',
  WEB_PORT = '8080',
  SESSION_SECRET = 'change-me',
  TURNSTILE_SITE_KEY = '',
  TURNSTILE_SECRET_KEY = '',
  ADMIN_USER = 'admin',
  ADMIN_PASS = '123456',
  TRUST_PROXY = '0'
} = process.env;

const trustProxyEnabled = String(TRUST_PROXY).toLowerCase() === '1' || String(TRUST_PROXY).toLowerCase() === 'true';

const pool = mysql.createPool({
  host: DB_HOST,
  port: Number(DB_PORT),
  user: DB_USER,
  password: DB_PASS,
  database: DB_NAME,
  waitForConnections: true,
  connectionLimit: 10,
  queueLimit: 0
});

const sessionStore = new MySQLStore({}, pool);

app.use(helmet({
  contentSecurityPolicy: false
}));
app.set('trust proxy', trustProxyEnabled ? 1 : false);
app.use(express.urlencoded({ extended: false }));
app.use(express.static('public'));
app.set('view engine', 'ejs');

app.use(session({
  key: 'mu_web_session',
  secret: SESSION_SECRET,
  store: sessionStore,
  resave: false,
  saveUninitialized: false,
  cookie: {
    httpOnly: true,
    sameSite: 'lax',
    maxAge: 1000 * 60 * 60 * 6
  }
}));

const authLimiter = rateLimit({
  windowMs: 60 * 1000,
  max: 10,
  standardHeaders: true,
  legacyHeaders: false
});

function requireUser(req, res, next) {
  if (!req.session.user) return res.redirect('/login');
  return next();
}

function requireAdmin(req, res, next) {
  if (!req.session.admin) return res.redirect('/admin/login');
  return next();
}

function requireAdminPasswordChange(req, res, next) {
  if (req.session.admin?.mustChange) return res.redirect('/admin/password');
  return next();
}

function randomDigits(len) {
  let out = '';
  for (let i = 0; i < len; i++) {
    out += Math.floor(Math.random() * 10);
  }
  return out;
}

const CLASS_NAMES = {
  0: 'Dark Wizard',
  1: 'Soul Master',
  2: 'Grand Master',
  16: 'Dark Knight',
  17: 'Blade Knight',
  18: 'Blade Master',
  32: 'Fairy Elf',
  33: 'Muse Elf',
  34: 'High Elf',
  48: 'Magic Gladiator',
  49: 'Duel Master',
  64: 'Dark Lord',
  65: 'Lord Emperor',
  80: 'Summoner',
  81: 'Bloody Summoner',
  82: 'Dimension Master',
  96: 'Rage Fighter',
  97: 'Fist Master'
};

function getClassName(value) {
  const id = Number(value);
  if (Number.isNaN(id)) return 'Desconocida';
  return CLASS_NAMES[id] || `Clase ${id}`;
}

function makeExcerpt(text, maxLen = 220) {
  const clean = (text || '').replace(/\r?\n/g, ' ').trim();
  if (clean.length <= maxLen) return clean;
  return `${clean.slice(0, maxLen)}...`;
}

async function fetchPlayersOnline() {
  try {
    const [rows] = await pool.query('SELECT COUNT(*) AS total FROM MEMB_STAT WHERE ConnectStat = 1');
    return rows?.[0]?.total ?? 0;
  } catch {
    return 0;
  }
}

async function ensureSchema() {
  const conn = await pool.getConnection();
  try {
    await conn.query(`
      CREATE TABLE IF NOT EXISTS web_admin (
        id INT UNSIGNED NOT NULL AUTO_INCREMENT,
        username VARCHAR(32) NOT NULL UNIQUE,
        password_hash VARCHAR(255) NOT NULL,
        must_change_password TINYINT(1) NOT NULL DEFAULT 1,
        created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
        PRIMARY KEY (id)
      )
    `);

    await conn.query(`
      CREATE TABLE IF NOT EXISTS web_news (
        id INT UNSIGNED NOT NULL AUTO_INCREMENT,
        title VARCHAR(120) NOT NULL,
        body TEXT NOT NULL,
        published TINYINT(1) NOT NULL DEFAULT 1,
        created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
        PRIMARY KEY (id)
      )
    `);

    const [rows] = await conn.query('SELECT id FROM web_admin WHERE username = ? LIMIT 1', [ADMIN_USER]);
    if (rows.length === 0) {
      const hash = await bcrypt.hash(ADMIN_PASS, 10);
      await conn.query(
        'INSERT INTO web_admin (username, password_hash, must_change_password) VALUES (?, ?, 1)',
        [ADMIN_USER, hash]
      );
    }
  } finally {
    conn.release();
  }
}

async function verifyTurnstile(token, remoteIp) {
  if (!TURNSTILE_SECRET_KEY) return true;
  if (!token) return false;

  const formData = new URLSearchParams();
  formData.append('secret', TURNSTILE_SECRET_KEY);
  formData.append('response', token);
  if (remoteIp) formData.append('remoteip', remoteIp);

  const resp = await fetch('https://challenges.cloudflare.com/turnstile/v0/siteverify', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: formData.toString()
  });
  const data = await resp.json();
  return data.success === true;
}

app.use((req, res, next) => {
  res.locals.turnstileSiteKey = TURNSTILE_SITE_KEY;
  res.locals.session = req.session;
  res.locals.appName = 'MuLinux';
  res.locals.year = new Date().getFullYear();
  next();
});

app.get('/', async (req, res) => {
  const [newsRows] = await pool.query('SELECT id, title, body, created_at FROM web_news WHERE published = 1 ORDER BY created_at DESC LIMIT 5');
  const news = newsRows.map((row) => ({
    ...row,
    excerpt: makeExcerpt(row.body)
  }));
  const [ranking] = await pool.query(
    `SELECT Name, cLevel, ResetCount, GrandResetCount, Class
     FROM \`Character\`
     ORDER BY GrandResetCount DESC, ResetCount DESC, cLevel DESC, Experience DESC
     LIMIT 10`
  );
  const rankingView = ranking.map((row) => ({
    ...row,
    ClassName: getClassName(row.Class)
  }));
  const playersOnline = await fetchPlayersOnline();
  const serverTime = new Date().toLocaleTimeString('es-ES', { hour12: false });
  res.render('home', { news, ranking: rankingView, playersOnline, serverTime, page: 'home', pageTitle: 'MuLinux - Inicio' });
});

app.get('/register', (req, res) => {
  res.render('register', { error: null, page: 'register', pageTitle: 'MuLinux - Registro' });
});

app.post('/register', authLimiter, async (req, res) => {
  const { account, password, confirm, email, name } = req.body;

  if (!account || !password || !confirm || !email) {
    return res.render('register', { error: 'Completa todos los campos.', page: 'register', pageTitle: 'MuLinux - Registro' });
  }

  if (!/^[a-zA-Z0-9]{4,10}$/.test(account)) {
    return res.render('register', { error: 'El usuario debe tener 4-10 caracteres alfanumericos.', page: 'register', pageTitle: 'MuLinux - Registro' });
  }

  if (password.length < 6 || password !== confirm) {
    return res.render('register', { error: 'La contrasena debe tener al menos 6 caracteres y coincidir.', page: 'register', pageTitle: 'MuLinux - Registro' });
  }

  const turnstileToken = req.body['cf-turnstile-response'];
  const turnstileOk = await verifyTurnstile(turnstileToken, req.ip);
  if (!turnstileOk) {
    return res.render('register', { error: 'Captcha invalido. Intenta nuevamente.', page: 'register', pageTitle: 'MuLinux - Registro' });
  }

  const conn = await pool.getConnection();
  try {
    await conn.beginTransaction();
    const [existing] = await conn.query('SELECT memb___id FROM MEMB_INFO WHERE memb___id = ? LIMIT 1', [account]);
    if (existing.length > 0) {
      await conn.rollback();
      return res.render('register', { error: 'La cuenta ya existe.', page: 'register', pageTitle: 'MuLinux - Registro' });
    }

    const membName = name && name.trim().length > 0 ? name.trim().slice(0, 10) : account;
    const sno = randomDigits(18);

    await conn.query(
      `INSERT INTO MEMB_INFO
       (memb___id, memb__pwd, memb_name, mail_addr, sno__numb, AccountLevel, AccountExpireDate, bloc_code, Bloc_Expire)
       VALUES (?, UNHEX(MD5(?)), ?, ?, ?, 0, NOW(), '0', '1900-01-01 00:00:00')`,
      [account, password, membName, email, sno]
    );

    await conn.query(
      'INSERT INTO MEMB_STAT (memb___id, ConnectStat) VALUES (?, 0)',
      [account]
    );

    await conn.query(
      'INSERT INTO AccountCharacter (Id) VALUES (?)',
      [account]
    );

    await conn.commit();
    return res.redirect('/login');
  } catch (err) {
    await conn.rollback();
    return res.render('register', { error: 'Error al crear la cuenta.', page: 'register', pageTitle: 'MuLinux - Registro' });
  } finally {
    conn.release();
  }
});

app.get('/login', (req, res) => {
  res.render('login', { error: null, page: 'login', pageTitle: 'MuLinux - Login' });
});

app.post('/login', authLimiter, async (req, res) => {
  const { account, password } = req.body;
  if (!account || !password) {
    return res.render('login', { error: 'Completa usuario y contrasena.', page: 'login', pageTitle: 'MuLinux - Login' });
  }

  const [rows] = await pool.query(
    'SELECT memb___id FROM MEMB_INFO WHERE memb___id = ? AND memb__pwd = UNHEX(MD5(?)) LIMIT 1',
    [account, password]
  );

  if (rows.length === 0) {
    return res.render('login', { error: 'Credenciales invalidas.', page: 'login', pageTitle: 'MuLinux - Login' });
  }

  req.session.user = { id: account };
  return res.redirect('/account');
});

app.get('/logout', (req, res) => {
  req.session.destroy(() => res.redirect('/'));
});

app.get('/account', requireUser, async (req, res) => {
  const [rows] = await pool.query('SELECT memb___id, memb_name, mail_addr, AccountLevel FROM MEMB_INFO WHERE memb___id = ? LIMIT 1', [req.session.user.id]);
  res.render('account', { account: rows[0], page: 'account', pageTitle: 'MuLinux - Cuenta' });
});

app.get('/rankings', async (req, res) => {
  const [rows] = await pool.query(
    `SELECT Name, cLevel, ResetCount, GrandResetCount, Class
     FROM \`Character\`
     ORDER BY GrandResetCount DESC, ResetCount DESC, cLevel DESC, Experience DESC
     LIMIT 100`
  );
  const viewRows = rows.map((row) => ({
    ...row,
    ClassName: getClassName(row.Class)
  }));
  res.render('rankings', { rows: viewRows, page: 'rankings', pageTitle: 'MuLinux - Rankings' });
});

app.get('/download', (req, res) => {
  res.render('download', { page: 'download', pageTitle: 'MuLinux - Descargas' });
});

app.get('/news', async (req, res) => {
  const [news] = await pool.query('SELECT id, title, body, created_at FROM web_news WHERE published = 1 ORDER BY created_at DESC LIMIT 50');
  res.render('news', { news, page: 'news', pageTitle: 'MuLinux - Noticias' });
});

app.get('/news/:id', async (req, res) => {
  const [rows] = await pool.query('SELECT id, title, body, created_at FROM web_news WHERE published = 1 AND id = ? LIMIT 1', [req.params.id]);
  if (rows.length === 0) return res.redirect('/news');
  res.render('news_detail', { item: rows[0], page: 'news', pageTitle: `MuLinux - ${rows[0].title}` });
});

app.get('/admin/login', (req, res) => {
  res.render('admin_login', { error: null });
});

app.get('/admin', (req, res) => {
  res.redirect('/admin/login');
});

app.post('/admin/login', authLimiter, async (req, res) => {
  const { username, password } = req.body;
  const [rows] = await pool.query('SELECT id, username, password_hash, must_change_password FROM web_admin WHERE username = ? LIMIT 1', [username]);
  if (rows.length === 0) return res.render('admin_login', { error: 'Credenciales invalidas.' });

  const ok = await bcrypt.compare(password || '', rows[0].password_hash);
  if (!ok) return res.render('admin_login', { error: 'Credenciales invalidas.' });

  req.session.admin = {
    id: rows[0].id,
    username: rows[0].username,
    mustChange: rows[0].must_change_password === 1
  };

  if (req.session.admin.mustChange) return res.redirect('/admin/password');
  return res.redirect('/admin/news');
});

app.get('/admin/logout', (req, res) => {
  req.session.admin = null;
  res.redirect('/admin/login');
});

app.get('/admin/password', requireAdmin, (req, res) => {
  res.render('admin_password', { error: null });
});

app.post('/admin/password', requireAdmin, async (req, res) => {
  const { password, confirm } = req.body;
  if (!password || password.length < 8 || password !== confirm) {
    return res.render('admin_password', { error: 'Minimo 8 caracteres y deben coincidir.' });
  }

  const hash = await bcrypt.hash(password, 10);
  await pool.query('UPDATE web_admin SET password_hash = ?, must_change_password = 0 WHERE id = ?', [hash, req.session.admin.id]);
  req.session.admin.mustChange = false;
  return res.redirect('/admin/news');
});

app.get('/admin/news', requireAdmin, requireAdminPasswordChange, async (req, res) => {
  const [news] = await pool.query('SELECT id, title, published, created_at FROM web_news ORDER BY created_at DESC');
  res.render('admin_news', { news });
});

app.get('/admin/news/new', requireAdmin, requireAdminPasswordChange, (req, res) => {
  res.render('admin_news_form', { item: null, error: null });
});

app.post('/admin/news/new', requireAdmin, requireAdminPasswordChange, async (req, res) => {
  const { title, body, published } = req.body;
  if (!title || !body) return res.render('admin_news_form', { item: null, error: 'Titulo y contenido son obligatorios.' });
  await pool.query('INSERT INTO web_news (title, body, published) VALUES (?, ?, ?)', [title, body, published ? 1 : 0]);
  res.redirect('/admin/news');
});

app.get('/admin/news/:id/edit', requireAdmin, requireAdminPasswordChange, async (req, res) => {
  const [rows] = await pool.query('SELECT id, title, body, published FROM web_news WHERE id = ? LIMIT 1', [req.params.id]);
  if (rows.length === 0) return res.redirect('/admin/news');
  res.render('admin_news_form', { item: rows[0], error: null });
});

app.post('/admin/news/:id/edit', requireAdmin, requireAdminPasswordChange, async (req, res) => {
  const { title, body, published } = req.body;
  if (!title || !body) return res.render('admin_news_form', { item: { id: req.params.id, title, body, published }, error: 'Titulo y contenido son obligatorios.' });
  await pool.query('UPDATE web_news SET title = ?, body = ?, published = ? WHERE id = ?', [title, body, published ? 1 : 0, req.params.id]);
  res.redirect('/admin/news');
});

app.post('/admin/news/:id/delete', requireAdmin, requireAdminPasswordChange, async (req, res) => {
  await pool.query('DELETE FROM web_news WHERE id = ?', [req.params.id]);
  res.redirect('/admin/news');
});

ensureSchema()
  .then(() => {
    app.listen(Number(WEB_PORT), () => {
      console.log(`mu-web listening on ${WEB_PORT}`);
    });
  })
  .catch((err) => {
    console.error('Failed to start mu-web', err);
    process.exit(1);
  });
