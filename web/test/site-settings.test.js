const test = require('node:test');
const assert = require('node:assert/strict');
const express = require('express');
const session = require('express-session');
const path = require('node:path');
const vm = require('node:vm');
const fs = require('node:fs');
const { validSettings, registerSettings } = require('../lib/site-settings');
const { translator, resolveLanguage } = require('../lib/i18n');

test('sólo admite idiomas y temas de la lista, como strings', () => {
  assert(validSettings({ language: 'en', theme: 'daedalus-dw' }));
  for (const body of [{language:'fr',theme:'classic'}, {language:'es',theme:'../../x'}, {language:['en'],theme:'classic'}, {}]) assert(!validSettings(body));
});

test('traduce la plantilla del editor sin alterar valores interpolados', () => {
  const context = { window: {} }; vm.createContext(context);
  vm.runInContext(fs.readFileSync(path.join(__dirname, '../public/js/i18n.js'), 'utf8'), context);
  const text = context.window.MuText;
  text.catalog = { Guardar: 'Save', Nombre: 'Name', y: 'and' };
  assert.equal(text.html(['<button>Guardar</button><span>', '</span>'], 'Nombre'), '<button>Save</button><span>Nombre</span>');
  assert.equal(text.html(['<input name="y" placeholder="Nombre">']), '<input name="y" placeholder="Name">');
  assert.equal(text.t('constructor'), 'constructor');
  assert.equal(translator('en')('constructor'), 'constructor');
  assert.equal(translator('pt')('Configuración guardada.'), 'Configuração salva.');
});

test('el asistente exige sesión/CSRF y no puede repetirse, incluso con barra final', async (t) => {
  let settings = {language:'es', theme:'classic', setup_complete:0};
  const app = express(); app.use(express.urlencoded({extended:false}));
  app.use(session({secret:'prueba-local-sin-datos-reales', resave:false, saveUninitialized:false}));
  app.use((req,res,next) => {
    res.locals.siteSettings = {...settings};
    res.render = (_view, data) => res.json({...data, preview: res.locals.siteSettings, heading: res.locals.t?.("Configura tu servidor")}); next();
  });
  app.post('/login', (req,res) => { req.session.admin = true; req.session.save(() => res.sendStatus(200)); });
  const requireAdmin = (req,res,next) => req.session.admin ? next() : res.sendStatus(401);
  registerSettings(app, {pool:{query:async(sql,values) => {
    if (sql.includes('AND setup_complete = 0') && settings.setup_complete) return [{affectedRows:0}];
    settings={language:values[0],theme:values[1],setup_complete:1}; return [{affectedRows:1}];
  }},requireAdmin,requirePassword:(_req,_res,next)=>next(),editorEnabled:true,editorRequest:async()=>({enabled:true})});
  const server = app.listen(0,'127.0.0.1'); await new Promise(resolve=>server.once('listening',resolve));
  t.after(()=>{ server.closeAllConnections(); server.close(); });
  const url=`http://127.0.0.1:${server.address().port}`;
  assert.equal((await fetch(url+'/admin/setup')).status,401);
  const login=await fetch(url+'/login',{method:'POST'}); const cookie=login.headers.get('set-cookie').split(';')[0];
  const request=(route,form)=>fetch(url+route,{method:form?'POST':'GET',headers:{cookie,'Content-Type':'application/x-www-form-urlencoded'},body:form?new URLSearchParams(form):undefined,redirect:'manual'});
  const setup=await (await request('/admin/setup')).json(); assert.equal(setup.editorStatus,'connected');
  const preview=await (await request('/admin/setup?previewLanguage=pt&previewTheme=daedalus-elf')).json();
  assert.equal(preview.heading, translator('pt')('Configura tu servidor'));
  assert.equal(preview.preview.language,'pt');
  assert.equal(preview.preview.theme,'daedalus-elf');
  assert.deepEqual(settings,{language:'es',theme:'classic',setup_complete:0});
  assert.equal((await request('/admin/setup',{language:'en',theme:'classic'})).status,403);
  assert.equal((await request('/admin/setup',{csrf:setup.csrf,language:'de',theme:'classic'})).status,400);
  assert.equal((await request('/admin/setup',{csrf:setup.csrf,language:'en',theme:'daedalus-dw'})).status,302);
  for(const route of ['/admin/setup','/admin/setup/']){
    assert.equal((await request(route)).headers.get('location'),'/admin/settings');
    await request(route,{csrf:setup.csrf,language:'pt',theme:'classic'});
    assert.equal(settings.language,'en');
  }
  await request('/admin/settings',{csrf:setup.csrf,language:'pt',theme:'classic'});
  assert.equal(settings.language,'pt');
});

test('el idioma del visitante persiste sin cambiar el default ni el editor', () => {
 const req={path:'/rankings',method:'GET',query:{lang:'pt'},session:{}};
 assert.equal(resolveLanguage(req,'en'),'pt');
 req.query={}; assert.equal(resolveLanguage(req,'en'),'pt');
 req.path='/admin/custom'; assert.equal(resolveLanguage(req,'en'),'en');
 req.path='/'; req.query={lang:'constructor'}; assert.equal(resolveLanguage(req,'en'),'pt');
 req.session={}; req.query={lang:['en']}; assert.equal(resolveLanguage(req,'es'),'es');
});
