#pragma once
// SoundManager.h — CSound: volúmenes y música de fondo.
//
// La música la reproducía un proceso aparte, MuPlayer.exe, que PlayMp3 lanzaba
// con WinExec y StopMp3 cerraba con WM_CLOSE (IDA PlayMp3 / StopMp3 0x4127F0).
// Acá la reproduce miniaudio (lib/miniaudio) dentro del cliente, leyendo el
// archivo en streaming: acepta mp3, wav y flac. El tema suena una sola vez; la
// decisión de qué tema suena sigue en Music_PlayTrack / Music_StopTrack, con la
// lógica del binario.
//
// Los efectos de sonido siguen por DirectSound (Sound.cpp / Sound_DS3D.cpp);
// pasarlos también a miniaudio es parte de la Fase 4 (Linux).
//
// Volúmenes (DESVIACION, como el DLL): Config.ini [Sound] SoundLevel y
// MusicLevel, de 0 (mudo) a 9 (volumen original). Sin la clave, 9.

#include <windows.h>

class CSound {
public:
    // Niveles 0..9. Fuera de rango se recorta; -1 = sin configurar (9).
    void SetLevels(int soundLevel, int musicLevel);

    // Aplica el volumen de efectos a un buffer recién cargado (LoadWaveFile).
    void ApplySoundVolume(int buffer);

    // Empieza a reproducir `fileName`. Devuelve false si no existe o no se puede
    // decodificar. Corta lo que estuviera sonando.
    bool PlayMusic(const char* fileName);
    void StopMusic();

    // Para la música y libera el motor de audio (cierre del cliente).
    void Shutdown();

private:
    static long  LevelToDirectSound(int level);   // centésimas de dB
    static float LevelToGain(int level);          // ganancia lineal
    bool EnsureEngine();

    int   m_SoundLevel = 9;
    int   m_MusicLevel = 9;

    void* m_Engine = nullptr;   // ma_engine*
    void* m_Music  = nullptr;   // ma_sound*
};

extern CSound gSound;
