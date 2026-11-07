'use strict';

// Límites de Data/Util/Notice.txt, los del server (GameServer/Notice.h):
//   - NOTICE_INFO::Message[128]: el texto se copia con strcpy_s, así que entran 127
//     bytes de UTF-8 (un acento ocupa 2 y un emoji 4). Uno más largo hace fallar la
//     carga del archivo.
//   - MAX_NOTICE = 100: los avisos de más los descarta CNotice::SetInfo sin avisar.

const MAX_NOTICE_BYTES = 127;
const MAX_NOTICES = 100;

// Mensajes entre comillas de las líneas de datos (las que no son comentario).
function noticeMessages(content) {
  const out = [];
  for (const raw of String(content ?? '').replace(/^﻿/, '').split(/\r?\n/)) {
    const line = raw.trim();
    if (line === '' || line.startsWith('//')) continue;
    if (line.toLowerCase() === 'end') break;
    const match = line.match(/^"([^"]*)"/);
    if (match) out.push(match[1]);
  }
  return out;
}

// Lista de errores del contenido (vacía si el server lo va a leer entero).
function validateNoticeContent(content) {
  const errors = [];
  const messages = noticeMessages(content);
  if (messages.length > MAX_NOTICES) {
    errors.push(`Hay ${messages.length} avisos y el server lee hasta ${MAX_NOTICES}: el resto se descarta`);
  }
  messages.forEach((message, n) => {
    const bytes = Buffer.byteLength(message, 'utf8');
    if (bytes > MAX_NOTICE_BYTES) errors.push(`Aviso ${n + 1}: mide ${bytes} bytes y el máximo es ${MAX_NOTICE_BYTES}`);
  });
  return errors;
}

module.exports = { MAX_NOTICE_BYTES, MAX_NOTICES, noticeMessages, validateNoticeContent };
