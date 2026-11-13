// Model_CustomAnim.cpp — emotes propios (Data/Player/CustomAnim.bmd) para el jugador.
//
// Formato del archivo: tools/gen_customanim.py y tools/fortnite/build_customanim.py
// (BMD v10, 0 mallas, mismos huesos que player.bmd, N animaciones). Los huesos se
// validan por nombre y padre; si no coinciden el archivo se ignora entero.
//
// DESVIACION: la accion actual de una entidad es un byte (0x105), asi que el modelo
// no puede tener mas de 255 acciones. Para no limitar la cantidad de emotes, estos
// NO se agregan uno por uno a la tabla del modelo: se agregan solo kChannels
// "canales" al final, y cada vez que un jugador arranca un emote se enlaza uno de
// esos canales a sus datos (punteros). Un canal se reutiliza solo cuando ninguna
// entidad lo esta reproduciendo. La cantidad de emotes distintos que se pueden
// definir es ilimitada; solo hay un tope de kChannels reproduciendose a la vez.
#include "stdafx.h"
#include "assets/model/ModelTypes.h"
#include "assets/model/BMDFileTypes.h"
#include "assets/model/Model_CustomAnim.h"
#include "render/Texture/Texture.h"
#include "globals.h"
#include "functions.h"
#include <vector>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstring>

extern "C" void DbgLogPublic(const char* msg);

namespace {
constexpr int   kChannels = 32;
constexpr float kDefaultPlaySpeed = 0.32f;

struct Emote {
    std::string name;
    std::string sound;                      // candidatos separados por '|'
    bool        loop = false;
    int         loopOverride = -1;                  // -1 = el del sidecar; 0/1 = forzado en esta sesion
    float       speed = kDefaultPlaySpeed;
    int         frames = 0;
    std::vector<ModelAnimationKeys> keys;   // uno por hueso (vacio en los dummy)
};

std::vector<Emote> s_emotes;
int  s_first = -1;                  // primer canal (indice de accion del modelo)
int  s_channelEmote[kChannels];     // emote enlazado a cada canal (-1 = ninguno)
unsigned s_channelStamp[kChannels]; // ultimo uso, para elegir a quien desalojar
unsigned s_clock = 0;

BMD* PlayerModel()
{
    return AsModel(reinterpret_cast<BYTE*>(DAT_05828d58) + BMD_STRIDE * 390);
}

void Bind(int channel, int emote)
{
    BMD* model = PlayerModel();
    const Emote& e = s_emotes[emote];
    auto* actions = reinterpret_cast<ModelAnimationAction*>(model->Actions);
    ModelAnimationAction& a = actions[s_first + channel];
    memset(&a, 0, sizeof(a));
    a.FrameCount = (std::int16_t)e.frames;
    a.LockPositions = 0;
    memcpy(&a.Unknown0[4], &e.speed, sizeof(e.speed));
    auto* bones = reinterpret_cast<ModelAnimationBone*>(model->Bones);
    for (int b = 0; b < model->NumBones; ++b)
        if (!bones[b].Dummy) bones[b].Keys[s_first + channel] = e.keys[b];
    s_channelEmote[channel] = emote;
}

bool ChannelInUse(int channel)
{
    const int action = s_first + channel;
    BYTE* base = (BYTE*)(uintptr_t)DAT_07abf5d0;
    if (!base) return false;
    for (int s = 0; s < 400; ++s) {
        BYTE* sp = base + s * CHARACTER_STRIDE;
        if (!sp[0]) continue;
        if (AsObject(sp)->CurrentAction == action || AsObject(sp)->PriorAction == action) return true;
    }
    return false;
}
}

int CustomAnim_FirstAction() { return s_first; }
int CustomAnim_Count()       { return (int)s_emotes.size(); }
const char* CustomAnim_Name(int n) { return (n >= 0 && n < (int)s_emotes.size()) ? s_emotes[n].name.c_str() : ""; }

int CustomAnim_Acquire(int emote)
{
    if (s_first < 0 || emote < 0 || emote >= (int)s_emotes.size()) return -1;
    ++s_clock;
    int victim = -1;
    for (int c = 0; c < kChannels; ++c)
        if (s_channelEmote[c] == emote) { s_channelStamp[c] = s_clock; return s_first + c; }
    // Primero un canal sin usar; si no hay, el que hace mas tiempo que no se pide.
    for (int c = 0; c < kChannels && victim < 0; ++c)
        if (s_channelEmote[c] < 0 || !ChannelInUse(c)) victim = c;
    if (victim < 0) {
        victim = 0;
        for (int c = 1; c < kChannels; ++c)
            if (s_channelStamp[c] < s_channelStamp[victim]) victim = c;
    }
    Bind(victim, emote);
    s_channelStamp[victim] = s_clock;
    return s_first + victim;
}

int CustomAnim_EmoteOfAction(int action)
{
    const int c = action - s_first;
    return (s_first >= 0 && c >= 0 && c < kChannels) ? s_channelEmote[c] : -1;
}
const char* CustomAnim_Sound(int emote) { return (emote >= 0 && emote < (int)s_emotes.size()) ? s_emotes[emote].sound.c_str() : ""; }
float CustomAnim_Duration(int emote)
{
    if (emote < 0 || emote >= (int)s_emotes.size()) return 0.0f;
    const Emote& e = s_emotes[emote];
    return e.speed > 0.0f ? (e.frames - 1) / (e.speed * 25.0f) : 0.0f;
}
bool CustomAnim_IsLoop(int emote)
{
    if (emote < 0 || emote >= (int)s_emotes.size()) return false;
    const Emote& e = s_emotes[emote];
    return e.loopOverride >= 0 ? e.loopOverride != 0 : e.loop;
}
void CustomAnim_SetLoopOverride(int emote, int value)
{
    if (emote >= 0 && emote < (int)s_emotes.size()) s_emotes[emote].loopOverride = value;
}

bool CustomAnim_EndsOnFinish(int action)
{
    const int c = action - s_first;
    if (s_first < 0 || c < 0 || c >= kChannels) return false;
    const int e = s_channelEmote[c];
    return e >= 0 && !CustomAnim_IsLoop(e);
}

int CustomAnim_AppendToPlayer()
{
    if (!s_emotes.empty()) return (int)s_emotes.size();  // ya cargados
    BMD* model = PlayerModel();
    if (!model || !model->Actions || !model->Bones || model->NumBones <= 0) {
        DbgLogPublic("CustomAnim: el modelo del jugador no esta cargado");
        return 0;
    }

    char hdPath[300];
    FILE* f = crt_fopen(Texture_HDAssetPath("Data\\Player\\CustomAnim.bmd", hdPath, sizeof(hdPath)), "rb");
    if (!f) { DbgLogPublic("CustomAnim: no se pudo abrir Data/Player/CustomAnim.bmd"); return 0; }
    fseek(f, 0, SEEK_END); const long size = ftell(f); fseek(f, 0, SEEK_SET);
    std::vector<unsigned char> buf(size > 0 ? size : 0);
    if (size < (long)sizeof(BmdFile::Header) ||
        fread(buf.data(), 1, buf.size(), f) != buf.size()) { fclose(f); return 0; }
    fclose(f);

    BmdFile::Header h;
    memcpy(&h, buf.data(), sizeof(h));
    if (memcmp(h.Signature, "BMD", 3) != 0 || h.Version != 0x0A ||
        h.MeshCount != 0 || h.BoneCount != model->NumBones || h.ActionCount <= 0) {
        DbgLogPublic("CustomAnim: cabecera incompatible, se ignora");
        return 0;
    }
    const int oldN = model->NumActions, addN = h.ActionCount, newN = oldN + kChannels;
    if (newN > 255) { DbgLogPublic("CustomAnim: no entran los canales en 8 bits, se ignora"); return 0; }

    // Cabeceras de cada animacion (sin LockPositions: los emotes no lo usan).
    size_t cur = sizeof(h);
    std::vector<Emote> emotes(addN);
    for (int a = 0; a < addN; ++a) {
        if (cur + sizeof(BmdFile::ActionHeader) > buf.size()) return 0;
        BmdFile::ActionHeader ah; memcpy(&ah, &buf[cur], sizeof(ah)); cur += sizeof(ah);
        if (ah.FrameCount <= 0 || ah.FrameCount > 1000 || ah.LockPositions) return 0;
        emotes[a].frames = ah.FrameCount;
        emotes[a].keys.assign(model->NumBones, ModelAnimationKeys{});
    }

    // Claves por hueso, en el orden del archivo.
    auto* bones = reinterpret_cast<ModelAnimationBone*>(model->Bones);
    for (int b = 0; b < model->NumBones; ++b) {
        if (cur >= buf.size()) return 0;
        const unsigned char dummy = buf[cur++];
        if (dummy != bones[b].Dummy) { DbgLogPublic("CustomAnim: huesos dummy distintos"); return 0; }
        if (dummy) continue;
        BmdFile::BoneHeader bh;
        if (cur + sizeof(bh) > buf.size()) return 0;
        memcpy(&bh, &buf[cur], sizeof(bh)); cur += sizeof(bh);
        if (strncmp(bh.Name, bones[b].Name, sizeof(bh.Name)) != 0 || bh.Parent != bones[b].Parent) {
            DbgLogPublic("CustomAnim: huesos distintos a player.bmd, se ignora"); return 0;
        }
        for (int a = 0; a < addN; ++a) {
            const int frames = emotes[a].frames;
            const size_t bytes = (size_t)frames * BmdFile::VectorBytes;
            if (cur + 2 * bytes > buf.size()) return 0;
            ModelAnimationKeys& e = emotes[a].keys[b];
            e.Positions   = static_cast<float*>(operator_new(bytes));
            e.EulerAngles = static_cast<float*>(operator_new(bytes));
            e.Quaternions = static_cast<float*>(operator_new((size_t)frames * 4 * sizeof(float)));
            memcpy(e.Positions,   &buf[cur], bytes); cur += bytes;
            memcpy(e.EulerAngles, &buf[cur], bytes); cur += bytes;
            for (int fr = 0; fr < frames; ++fr)
                EulerToQuat(e.EulerAngles + fr * 3, e.Quaternions + fr * 4, 0, 0);
        }
    }

    // Sidecar CustomAnim.txt: "nombre loop velocidad" por animacion.
    {
        char hdTxt[300];
        FILE* tf = crt_fopen(Texture_HDAssetPath("Data\\Player\\CustomAnim.txt", hdTxt, sizeof(hdTxt)), "rb");
        if (tf) {
            std::string content; char tmp[512]; size_t got;
            while ((got = fread(tmp, 1, sizeof(tmp), tf)) > 0) content.append(tmp, got);
            fclose(tf);
            std::istringstream in(content); std::string line; int a = 0;
            while (a < addN && std::getline(in, line)) {
                std::istringstream ls(line); std::string name, snd; int loop = 0; float spd = kDefaultPlaySpeed;
                if (!(ls >> name >> loop >> spd)) continue;
                if (ls >> snd && snd != "-") emotes[a].sound = snd;
                emotes[a].name = name; emotes[a].loop = loop != 0;
                if (spd > 0.0f && spd < 10.0f) emotes[a].speed = spd;
                ++a;
            }
        }
    }

    // Se agregan solo los canales: tabla de acciones y de claves ampliadas.
    auto* newActions = static_cast<BYTE*>(operator_new(newN * sizeof(ModelAnimationAction)));
    memset(newActions, 0, newN * sizeof(ModelAnimationAction));
    memcpy(newActions, model->Actions, oldN * sizeof(ModelAnimationAction));
    for (int b = 0; b < model->NumBones; ++b) {
        if (bones[b].Dummy) continue;
        auto* keys = static_cast<ModelAnimationKeys*>(operator_new(newN * sizeof(ModelAnimationKeys)));
        memset(keys, 0, newN * sizeof(ModelAnimationKeys));
        memcpy(keys, bones[b].Keys, oldN * sizeof(ModelAnimationKeys));
        bones[b].Keys = keys;
    }
    model->Actions = newActions;
    model->NumActions = (short)newN;

    s_emotes.swap(emotes);
    s_first = oldN;
    for (int c = 0; c < kChannels; ++c) { s_channelEmote[c] = -1; s_channelStamp[c] = 0; }
    for (int c = 0; c < kChannels; ++c) Bind(c, 0);   // datos validos desde el arranque
    for (int c = 0; c < kChannels; ++c) s_channelEmote[c] = -1;

    char line[120];
    snprintf(line, sizeof(line), "CustomAnim: %d emotes, %d canales (acciones %d..%d)",
             addN, kChannels, oldN, newN - 1);
    DbgLogPublic(line);
    return addN;
}
