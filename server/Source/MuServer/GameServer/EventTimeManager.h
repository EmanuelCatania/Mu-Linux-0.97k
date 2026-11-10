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

	// F3:F2: vivos por mapa de las invasiones en curso (tooltip del panel).
	void GCInvasionDetailSend(int aIndex);
};

extern CEventTimeManager gEventTimeManager;