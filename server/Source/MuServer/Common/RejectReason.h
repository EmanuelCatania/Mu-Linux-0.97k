#pragma once

// Motivos del evento de rechazo C1:F3:FB (Fase 5, A6).
//
// Cuando el server descarta un pedido del cliente que el cliente ya dio por
// hecho, le avisa con el opcode del pedido y uno de estos motivos.  El cliente
// lo anota en su debug.log, deshace lo que haga falta (por ejemplo, cierra la
// tienda si el NPC ya no la atiende) y muestra el motivo en el chat: siempre en
// el build Debug y, en Release, sólo si el server marcó la cuenta como GM.
//
// Los rechazos que desconectan (HackPacketCheck, frames inválidos) y los
// chequeos en modo observar no mandan este evento.
//
// El cliente tiene la misma tabla en client/src/Net/Protocol/RejectReason.h;
// el texto de cada motivo es un ClientTextId (Client.txt).
enum REJECT_REASON
{
	REJECT_MOVE = 1,           // camino o posición rechazados (MoveCheck = 2)
	REJECT_ATTACK_SPEED,       // ataque antes de tiempo (AttackCheck = 2)
	REJECT_SKILL_STATE,        // muerto o teletransportándose
	REJECT_SKILL_SAFE_ZONE,    // skill de área en zona segura
	REJECT_SKILL_NOT_LEARNED,  // el personaje no tiene el skill
	REJECT_SKILL_DELAY,        // el skill todavía no se puede volver a usar
	REJECT_SKILL_CLASS,        // la clase no puede usarlo
	REJECT_SKILL_SEQUENCE,     // 0x1D sin el 0x1E previo
	REJECT_SKILL_NO_ARROWS,    // Triple Shot sin arco o flechas
	REJECT_DROP_RANGE,         // tirar un item lejos del personaje
	REJECT_NPC_MISSING,        // la tienda está abierta sin NPC que la atienda
	REJECT_NPC_RANGE,          // el NPC quedó lejos
	REJECT_NPC_NO_REPAIR,      // el NPC no repara
	REJECT_TRADE_RANGE,        // trade cancelado por distancia
	REJECT_REPAIR_NO_SHOP,     // reparar al precio del NPC sin su tienda abierta
	REJECT_REASON_COUNT
};
