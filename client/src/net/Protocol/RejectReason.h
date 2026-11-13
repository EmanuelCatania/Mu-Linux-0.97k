#pragma once

// Motivos del evento de rechazo C1:F3:FB (Fase 5, A6).  Misma tabla que
// server/Source/MuServer/Common/RejectReason.h: si se agrega un motivo, se
// agrega en los dos lados y su texto en ClientTextId (Client.txt).
enum RejectReason
{
    REJECT_MOVE = 1,
    REJECT_ATTACK_SPEED,
    REJECT_SKILL_STATE,
    REJECT_SKILL_SAFE_ZONE,
    REJECT_SKILL_NOT_LEARNED,
    REJECT_SKILL_DELAY,
    REJECT_SKILL_CLASS,
    REJECT_SKILL_SEQUENCE,
    REJECT_SKILL_NO_ARROWS,
    REJECT_DROP_RANGE,
    REJECT_NPC_MISSING,
    REJECT_NPC_RANGE,
    REJECT_NPC_NO_REPAIR,
    REJECT_TRADE_RANGE,
    REJECT_REPAIR_NO_SHOP,
    REJECT_REASON_COUNT
};

// F3:FB: lo anota en debug.log, deshace lo que haga falta y muestra el motivo
// en el chat (build Debug: siempre; Release: sólo si el server marcó GM).
void Recv_Reject(const unsigned char* msg, int size);
