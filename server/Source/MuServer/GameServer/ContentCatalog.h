#pragma once

#include "ProtocolDefines.h"
#include "ItemExtras.h"

// Catálogo de contenido (0.97.20): el server es la única autoridad de las
// definiciones de items, monstruos y fog por mapa.  Se arma una vez a partir
// de Item.txt / Monster.txt (más los txt del Encoder, fusionados para quien
// trae archivos viejos) y se manda completo a cada cliente antes de la
// respuesta del login.  El cliente sólo resuelve los assets locales.

#define CATALOG_VERSION 1

#define CATALOG_ITEM_HAS_MODEL 0x01
#define CATALOG_ITEM_HAS_GLOW  0x02
#define CATALOG_ITEM_HAS_POSE  0x04
#define CATALOG_ITEM_HAS_ENTITY 0x08

#define CATALOG_MONSTER_HAS_MODEL 0x01

// Efectos de un item (Data/Custom/Items/*.json).  El cliente trae un juego
// fijo de primitivas; el JSON elige cuáles usa cada item y con qué datos.
#define CATALOG_EFFECT_ON_EQUIPPED 0x01

#define CATALOG_EFFECT_SPRITE   1  // sprite en huesos, cada frame
#define CATALOG_EFFECT_PARTICLE 2  // partícula vanilla en huesos, con probabilidad

#define CATALOG_EFFECT_MAX_BONES 32

// Pets (Data/Custom/Pets/*.json).
#define CATALOG_PET_MOVE_FOLLOW 1  // vuela detrás del dueño
#define CATALOG_PET_MOVE_ORBIT  2  // da vueltas alrededor del dueño

#define CATALOG_PET_FLAG_COLLECT 0x01  // vuela hasta el zen cercano antes de levantarlo

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_CATALOG_HEAD
{
	PSWMSG_HEAD header; // C2:F3:[E7|E8|E9]
	BYTE version;
	WORD recordSize;
	WORD count;
	WORD chunk;
	WORD totalChunks;
};

struct CATALOG_ITEM
{
	WORD Index;
	WORD Behavior;
	BYTE Slot;
	BYTE Skill;
	BYTE Width;
	BYTE Height;
	BYTE TwoHand;
	BYTE Flags;
	BYTE Durability;
	BYTE MagicDurability;
	char Name[32];
	WORD Level;
	WORD DamageMin;
	WORD DamageMax;
	WORD AttackSpeed;
	WORD WalkSpeed;
	WORD Defense;
	WORD MagicDefense;
	WORD DefenseSuccessRate;
	WORD MagicDamageRate;
	WORD RequireLevel;
	WORD RequireStrength;
	WORD RequireDexterity;
	WORD RequireEnergy;
	WORD RequireVitality;
	BYTE RequireClass[4];
	BYTE Resistance[4];
	DWORD Value;
	DWORD BuyMoney;
	DWORD GlowColor;
	float PositionX;
	float PositionY;
	float RotationX;
	float RotationY;
	float RotationZ;
	float Scale;
	char ModelFolder[40];
	char ModelName[28];
	// Ala custom (CustomWing.txt): índice + 1 que viaja en el CharSet (0 = no
	// es ala custom) y las constantes de defensa y daño para el tooltip.
	BYTE CustomWing;
	BYTE Reserved;
	WORD WingDefenseConstA;
	WORD WingIncDamageConstA;
	WORD WingIncDamageConstB;
	WORD WingDecDamageConstA;
	WORD WingDecDamageConstB;
	// Modelo con que se dibuja el item puesto (montura, pet) cuando no es el
	// mismo del inventario.
	char EntityFolder[40];
	char EntityName[28];
};

struct CATALOG_MONSTER
{
	WORD Index;
	BYTE Kind;
	BYTE Golden;
	float Scale;
	WORD Level;
	WORD Flags;
	char Name[32];
	char ModelFolder[32];
	char ModelName[20];
};

struct CATALOG_MAP_FOG
{
	BYTE Map;
	BYTE DayRGB[3];
	BYTE NightRGB[3];
	BYTE Reserved;
	// Distancias de la niebla lineal (unidades del mundo). Los clientes que
	// leen 8 bytes las ignoran; los nuevos usan 2000/2700 si no llegan.
	WORD Start;
	WORD End;
};

struct CATALOG_EFFECT
{
	WORD Item;
	BYTE On;            // CATALOG_EFFECT_ON_*
	BYTE Type;          // CATALOG_EFFECT_*
	WORD Bitmap;        // textura vanilla; 0xFFFF = la de Texture
	WORD Particle;      // tipo de partícula vanilla
	BYTE SubType;
	BYTE Chance;        // 1 de cada N frames (0/1 = siempre)
	BYTE BoneCount;
	BYTE Reserved;
	BYTE Bones[CATALOG_EFFECT_MAX_BONES];
	float Color[3];
	float Scale;
	float PulseSpeed;   // pulso |sin(WorldTime * speed)|
	float PulseScale;   // + pulso * esto a la escala
	float PulseColor;   // + pulso * esto a cada componente del color
	float Offset[3];    // desplazamiento en el espacio del hueso
	char Texture[48];   // bajo Data\, con extensión .jpg / .tga
};

// Línea propia del tooltip de un item ("tooltip" en Data/Custom/Items).
#define CATALOG_TOOLTIP_MAX_LINES 6

struct CATALOG_TOOLTIP
{
	WORD Item;
	BYTE Color;         // 0 blanco 1 azul 2 rojo 3 amarillo 4 verde 5 rojo oscuro 6 violeta 7 azul oscuro 8 amarillo oscuro
	BYTE Reserved;
	char Text[60];      // UTF-8
};

struct CATALOG_PET
{
	WORD Item;
	BYTE Movement;      // CATALOG_PET_MOVE_*
	BYTE BlendMesh;     // malla aditiva del modelo; 0xFF = ninguna
	float Radius;       // órbita: radio
	float Period;       // órbita: ms por vuelta
	float Height;       // altura sobre el dueño
	float Scale;
	BYTE Action;        // animación del modelo
	BYTE Flags;         // CATALOG_PET_FLAG_*
	BYTE CollectRange;  // en casillas: hasta dónde va a buscar el zen
	BYTE Reserved;
};

// Textos traducidos (0.97.40): Data/Text/<Eng|Spn|Por>/*.txt.  El server
// manda sólo los del idioma que pidió el cliente con el F1:04; lo que no esté
// traducido sigue saliendo de los .bmd del cliente (o del Name de E7/E8 para
// los items y monstruos agregados).
#define CATALOG_TEXT_ITEM    1
#define CATALOG_TEXT_MONSTER 2
#define CATALOG_TEXT_SKILL   3
#define CATALOG_TEXT_CLIENT  4   // textos propios del cliente (ClientTextId)
#define CATALOG_TEXT_TOOLTIP 5   // línea del tooltip de un item: Index = item, Reserved = línea (0 a 5)

struct CATALOG_TEXT
{
	BYTE Table;         // CATALOG_TEXT_*
	BYTE Reserved;      // en los tooltips, el número de línea
	WORD Index;         // item (sección * 512 + índice), monstruo, skill o ClientTextId
	char Text[64];      // UTF-8 en NFC
};

// Gates (Fase 5): el cliente las usa para detectar que el personaje pisa una
// entrada y pedir el teleport, y para el minimapa. Antes salían de su Gate.bmd.
struct CATALOG_GATE
{
	WORD Index;
	BYTE Flag;          // 0 destino, 1 entrada, 2 salida
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE TX;
	BYTE TY;
	WORD Target;
	BYTE Dir;
	BYTE Reserved;
	WORD MinLevel;      // 0 = sin mínimo
	WORD Reserved2;
};

// Skills (Fase 5): lo que el cliente usa para decidir si puede lanzar un skill
// y hasta dónde camina antes de hacerlo. Antes salía de su skill.bmd.
struct CATALOG_SKILL
{
	WORD Index;
	WORD Mana;
	WORD BP;            // AG
	BYTE Range;         // en casillas
	BYTE Reserved;
};

// Caché del catálogo en el cliente (Fase 5).  Antes del login el cliente dice
// qué secciones tiene guardadas de este server (F1:06); al aceptar el login el
// server manda la lista de secciones con su hash (F3:F6), marcando cuáles puede
// tomar de su caché, y después sólo las que cambiaron.
#define CATALOG_CACHE_MAX 16

// hash: SHA-256 truncado a 8 bytes del tamaño de registro + los registros de
// la sección (CatalogSectionHash en Common/Sha256.h).  Lo recalcula el cliente
// al leer su caché, así una sección editada no coincide y se vuelve a mandar.
struct CATALOG_CACHE_ENTRY
{
	BYTE subh;
	BYTE cached;        // en F3:F6: 1 = el cliente la toma de su caché
	WORD Reserved;
	BYTE hash[8];
};

struct PMSG_CATALOG_CACHE_RECV
{
	PSBMSG_HEAD header; // C1:F1:06
	BYTE count;
	BYTE Reserved[3];
	CATALOG_CACHE_ENTRY entry[CATALOG_CACHE_MAX];
};

struct PMSG_CATALOG_SYNC_SEND
{
	PSBMSG_HEAD header; // C1:F3:F6
	BYTE count;
	BYTE Reserved[3];
	CATALOG_CACHE_ENTRY entry[CATALOG_CACHE_MAX];
};

// Firma del manifiesto (Fase 5, C3).  Va justo antes del F3:F6.  El server firma
// con su clave de identidad (ServerIdentity) la lista de secciones y hashes, así
// el cliente sabe que el catálogo y lo que tiene en caché son de ese server.
//
// Mensaje firmado: CATALOG_SIGNATURE_CONTEXT + count + por cada sección del
// F3:F6, en el mismo orden, subh (1 byte) + hash (8 bytes).  El campo `cached`
// no entra: depende de cada cliente.
#define CATALOG_SIGNATURE_CONTEXT "MuCatalog1"

#define CATALOG_SIGNATURE_HAS_ROTATION 0x01

struct PMSG_CATALOG_SIGNATURE_SEND
{
	PSBMSG_HEAD header; // C1:F3:FC
	BYTE flags;         // CATALOG_SIGNATURE_*
	BYTE Reserved[3];
	BYTE publicKey[32];
	BYTE signature[64];
	// Con HAS_ROTATION: la clave anterior del server y su firma sobre la actual
	// (SERVER_IDENTITY_ROTATION_CONTEXT + publicKey).
	BYTE previousKey[32];
	BYTE rotationSignature[64];
};

static_assert(sizeof(PMSG_CATALOG_SIGNATURE_SEND) == 200, "PMSG_CATALOG_SIGNATURE_SEND");

// Música por mapa (Fase 5, `music` de Map.toml): sólo los mapas que la fijan.
// Lista del menú M (Move.txt), sección F3:F5.  Antes viajaba en la lista F3:E5
// del DLL, por personaje: el server bajaba el nivel mínimo a 2/3 para el Magic
// Gladiator.  Ahora la lista es la misma para todos y esa regla la aplica el
// cliente (CGate::GetMoveLevel); el server sigue validando el /move.
#define CATALOG_MOVE_PK_FREE 0x01   // se puede mover con PK (PKLimitFree)

struct CATALOG_MOVE
{
	BYTE Map;
	BYTE Flags;         // CATALOG_MOVE_*
	char Name[32];
	WORD Reserved;
	DWORD Money;
	short MinLevel;     // -1 = sin mínimo
	short MaxLevel;
	short MinReset;
	short MaxReset;
	short AccountLevel;
	WORD Reserved2;
};

static_assert(sizeof(CATALOG_MOVE) == 52, "CATALOG_MOVE");

struct CATALOG_MAP_MUSIC
{
	BYTE Map;
	BYTE Reserved[3];
	char Music[28];     // nombre de Data/Music del cliente, sin extensión
};

struct PMSG_CATALOG_END_SEND
{
	PSBMSG_HEAD header; // C1:F3:EA
	WORD itemCount;
	WORD monsterCount;
	DWORD hash;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct ITEM_POSE_INFO
{
	bool Valid;
	float PositionX;
	float PositionY;
	float RotationX;
	float RotationY;
	float RotationZ;
	float Scale;
};

class CContentCatalog
{
public:

	CContentCatalog();

	// Fusión de los txt del Encoder (CustomItem, CustomGlow, CustomBow,
	// ItemPosition) sobre la tabla de items ya cargada, y fog por mapa.
	void LoadLegacyItems(const char* folder);

	// Fog por mapa (MapFog.txt del Encoder). LoadLegacyItems la incluye.
	void LoadMapFog(const char* folder);

	// Fase 5: los datos extra de Data/Items/NN-nombre.toml (modelo, glow, arco,
	// pose, efectos, tooltip) sobre la tabla de items ya cargada. Reemplaza a
	// LoadLegacyItems (salvo la fog) y a LoadItemJson.
	void ApplyItemExtras(const std::vector<ItemExtraDef>& items);

	// CustomMonster.txt sobre la tabla de monstruos ya cargada.
	void LoadLegacyMonsters(const char* folder);

	// Efectos y demás datos por item en JSON (Data/Custom/Items/*.json).
	void LoadItemJson(const char* folder);

	// Textos traducidos: folder\<Eng|Spn|Por>\{Item,Monster,Skill}.txt.
	void LoadTexts(const char* folder);

	void Invalidate() { this->m_Built = false; }

	void Send(int aIndex);

	// Sólo las gates (F3:F3), a quien ya tiene el resto del catálogo: se usa al
	// recargar las gates.
	void SendGates(int aIndex);

	// Sólo los skills (F3:F4), al recargar Skill.txt.
	void SendSkills(int aIndex);

	// Sólo la música por mapa (F3:F7), al recargar los Map.toml.
	void SendMapMusic(int aIndex);

	// Sólo la lista de /move (F3:F5), al recargar Move.txt.
	void SendMoveList(int aIndex);

	// F1:06: secciones que el cliente tiene en caché.
	void SetClientCache(int aIndex, const PMSG_CATALOG_CACHE_RECV* lpMsg);

	const ITEM_POSE_INFO* GetPose(int index) const;

private:

	void Build();

	void BuildSection(BYTE subh, const BYTE* records, int recordSize, int count, std::vector<std::vector<BYTE>>& packets);

	bool m_Built;

	std::map<int, ITEM_POSE_INFO> m_ItemPose;

	std::vector<CATALOG_MAP_FOG> m_MapFog;

	std::vector<CATALOG_EFFECT> m_Effects;

	std::vector<CATALOG_TOOLTIP> m_Tooltips;

	std::vector<std::vector<BYTE>> m_Packets;

	std::vector<std::vector<BYTE>> m_GatePackets;

	std::vector<std::vector<BYTE>> m_SkillPackets;

	std::vector<std::vector<BYTE>> m_MapMusicPackets;

	std::vector<std::vector<BYTE>> m_MoveListPackets;

	// Rango de m_Packets de cada sección y su hash (para la caché del cliente).
	struct SectionRange
	{
		BYTE subh;
		BYTE hash[8];
		size_t begin;
		size_t end;
	};

	std::vector<SectionRange> m_Sections;

	void MarkSection(BYTE subh, size_t begin);

	std::vector<CATALOG_TEXT> m_Texts[MAX_LANGUAGE];

	std::vector<std::vector<BYTE>> m_TextPackets[MAX_LANGUAGE];
};

extern CContentCatalog gContentCatalog;
