#pragma once
#include "Net/Protocol/GameServerProtocol.h"

// Menú M del DLL (MoveList.cpp). La lista y los requisitos llegan en el
// catálogo (sección F3:F5).
class CMoveList {
public:
    // Sección F3:F5.  false si algún registro no es válido (queda la anterior).
    bool SetCatalog(const BYTE* records, int recordSize, int count);
    // Al desconectar: la lista llega una vez por conexión.
    void Clear();
    // Al cambiar de personaje: sólo lo que depende de él.
    void ResetCharacter();
    void Toggle();
    bool IsOpen() const { return m_Open; }
    void Close() { m_Open = false; }
    void UpdateMouse();
    void Render();
    int Count() const { return m_Count; }
    const Proto::MOVE_LIST_INFO* Get(int index) const;
    void SetResets(DWORD resets) { m_Resets = resets; }
    DWORD Resets() const { return m_Resets; }

private:
    bool Blocked() const;
    int VisibleRows() const;
    bool CheckMove(const Proto::MOVE_LIST_INFO& map) const;
    int  MinLevelFor(const Proto::MOVE_LIST_INFO& map) const;
    Proto::MOVE_LIST_INFO m_Maps[255] = {};
    int m_Count = 0;
    int m_Page = 0;
    bool m_Open = false;
    bool m_Received = false;
    BYTE m_PKLimitFree = 0;
    DWORD m_Resets = 0;
};
extern CMoveList gMoveList;
