'use strict';

// Contraseñas de cuenta del juego (MEMB_INFO.memb__pwd).
//
// Lo nuevo se guarda con bcrypt, el mismo formato que verifica el JoinServer
// (Common/PasswordHash). Lo viejo puede ser MD5 binario (MD5Encryption=2, lo
// que escribía este panel) o texto (MD5Encryption=0); se sigue aceptando y se
// reemplaza por el hash cuando la cuenta entra.
//
// El MD5 de lo viejo lo calcula MySQL (UNHEX(MD5(?))), como hacía el panel
// antes: acá sólo se recibe si coincidió.

const crypto = require('crypto');
const bcrypt = require('bcryptjs');

const HASH_LENGTH = 60;
const COST = 10;

// Expresión SQL que compara memb__pwd contra el MD5 de un parámetro.
const MD5_MATCH_SQL = 'memb__pwd = UNHEX(MD5(?))';

function toBuffer(stored) {
  if (stored == null) return Buffer.alloc(0);
  return Buffer.isBuffer(stored) ? stored : Buffer.from(String(stored), 'latin1');
}

function isHash(stored) {
  const text = toBuffer(stored).toString('latin1');
  return /^\$2[aby]\$\d\d\$[./A-Za-z0-9]{53}$/.test(text);
}

function equalBuffers(a, b) {
  return a.length === b.length && crypto.timingSafeEqual(a, b);
}

// { ok, legacy }: legacy = la contraseña era vieja y conviene reemplazarla.
// md5Match = resultado de MD5_MATCH_SQL para esta contraseña.
async function verify(stored, password, md5Match = false) {
  const value = toBuffer(stored);
  const plain = Buffer.from(String(password ?? ''), 'utf8');
  if (!plain.length) return { ok: false, legacy: false };

  if (isHash(value)) {
    return { ok: await bcrypt.compare(plain.toString('utf8'), value.toString('latin1')), legacy: false };
  }

  if (Number(md5Match) === 1 || md5Match === true || equalBuffers(value, plain)) return { ok: true, legacy: true };
  return { ok: false, legacy: false };
}

// Valor a guardar, como { sql, value }: `sql` es lo que va en el lugar del
// parámetro. Sin la columna de 60 bytes se sigue guardando MD5, como antes:
// un hash cortado dejaría la cuenta sin poder entrar.
async function make(password, wideColumn) {
  const plain = String(password ?? '');
  if (!wideColumn) return { sql: 'UNHEX(MD5(?))', value: plain };
  return { sql: '?', value: Buffer.from(await bcrypt.hash(plain, COST), 'latin1') };
}

module.exports = { HASH_LENGTH, COST, MD5_MATCH_SQL, isHash, verify, make };
