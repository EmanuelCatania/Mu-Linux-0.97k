'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { MAX_NOTICE_BYTES, MAX_NOTICES, noticeMessages, validateNoticeContent } = require('../lib/notices');

const header = '//Message\t\tType\tRepeatTime\r\n';
const file = (...messages) => header + messages.map((m) => `"${m}"\t\t\t\t0\t600`).join('\r\n') + '\r\nend\r\n';

test('lee los mensajes entre comillas y se detiene en end', () => {
  assert.deepEqual(noticeMessages(file('Hola', 'Evento ñandú')), ['Hola', 'Evento ñandú']);
  assert.deepEqual(noticeMessages('"a"\t0\t5\nend\n"b"\t0\t5\n'), ['a']);
  assert.deepEqual(noticeMessages('//"comentado" 0 5\n"a" 0 5\n'), ['a']);
});

test('el límite son 127 bytes de UTF-8, no caracteres', () => {
  assert.deepEqual(validateNoticeContent(file('a'.repeat(127))), []);
  assert.match(validateNoticeContent(file('a'.repeat(128)))[0], /128 bytes y el máximo es 127/);
  // 64 letras de dos bytes = 128 bytes: son 64 caracteres y ya no entra
  assert.ok(validateNoticeContent(file('ñ'.repeat(64))).length > 0);
  assert.deepEqual(validateNoticeContent(file('ñ'.repeat(63))), []);
  // un emoji ocupa 4 bytes
  assert.ok(validateNoticeContent(file('🗡️'.repeat(20))).length > 0);
});

test('el server lee hasta 100 avisos', () => {
  const ok = file(...Array(MAX_NOTICES).fill('x'));
  assert.deepEqual(validateNoticeContent(ok), []);
  const demasiados = file(...Array(MAX_NOTICES + 1).fill('x'));
  assert.match(validateNoticeContent(demasiados)[0], /101 avisos/);
});

test('el Notice.txt del repo cumple los límites', () => {
  const content = fs.readFileSync(path.join(__dirname, '..', '..', 'MuServer', 'Data', 'Util', 'Notice.txt'), 'utf8');
  assert.deepEqual(validateNoticeContent(content), []);
  assert.ok(MAX_NOTICE_BYTES === 127);
});
