'use strict';

// Contraseñas de cuenta del juego (MEMB_INFO.memb__pwd).
//
// Lo nuevo se guarda con bcrypt, el mismo formato que verifica el JoinServer
// (Common/PasswordHash). Lo viejo puede ser MD5 binario (MD5Encryption=2, lo
// que escribía este panel) o texto (MD5Encryption=0); se sigue aceptando y se
// reemplaza por el hash cuando la cuenta entra.

const crypto = require('crypto');
const bcrypt = require('bcryptjs');

const HASH_LENGTH = 60;
const COST = 10;

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
async function verify(stored, password) {
  const value = toBuffer(stored);
  const plain = Buffer.from(String(password ?? ''), 'utf8');
  if (!plain.length) return { ok: false, legacy: false };

  if (isHash(value)) {
    return { ok: await bcrypt.compare(plain.toString('utf8'), value.toString('latin1')), legacy: false };
  }

  const md5 = crypto.createHash('md5').update(plain).digest();
  if (equalBuffers(value, md5) || equalBuffers(value, plain)) return { ok: true, legacy: true };
  return { ok: false, legacy: false };
}

// Valor a guardar. Sin la columna de 60 bytes se sigue guardando MD5, como
// antes: un hash cortado dejaría la cuenta sin poder entrar.
async function make(password, wideColumn) {
  const plain = String(password ?? '');
  if (!wideColumn) return crypto.createHash('md5').update(plain, 'utf8').digest();
  return Buffer.from(await bcrypt.hash(plain, COST), 'latin1');
}

module.exports = { HASH_LENGTH, COST, isHash, verify, make };
