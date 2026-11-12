#pragma once

class CEventTimeManager
{
	enum eEventTimeState
	{
		EVENT_STATE_BLANK = 0,
		EVENT_STATE_STAND = 1,
		EVENT_STATE_OPEN = 2,
		EVENT_STATE_START = 3,
	};

public:

	CEventTimeManager();

	virtual ~CEventTimeManager();

	void GCEventTimeSend(int aIndex);

	// Vivos por mapa de las invasiones en curso (tooltip del panel), al final
	// del C2:91: escribe la cantidad y los registros desde `size` y devuelve el
	// nuevo tamaño.  Antes era el F3:F2.
	int AppendInvasionDetail(BYTE* send, int size, int capacity);
};

extern CEventTimeManager gEventTimeManager;