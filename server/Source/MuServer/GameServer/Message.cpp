#include "stdafx.h"
#include "Message.h"
#include "ReadScript.h"
#include "Util.h"

CMessage gMessage;

CMessage::CMessage()
{
	memset(this->m_DefaultMessage, 0, sizeof(this->m_DefaultMessage));

	for (int i = 0; i < MAX_LANGUAGE; i++)
	{
		this->m_MessageInfo[i].clear();
	}
}

CMessage::~CMessage()
{

}

void CMessage::Load(const char* path, int lang)
{
	if (lang < 0 || lang >= MAX_LANGUAGE)
	{
		ErrorMessageBox("Invalid language number on Message::Load: %d", lang);

		return;
	}

	CReadScript* lpReadScript = new CReadScript;

	if (lpReadScript == NULL)
	{
		ErrorMessageBox(READ_SCRIPT_ALLOC_ERROR, path);

		return;
	}

	if (!lpReadScript->Load(path))
	{
		ErrorMessageBox(READ_SCRIPT_FILE_ERROR, path);

		delete lpReadScript;

		return;
	}

	this->m_MessageInfo[lang].clear();

	try
	{
		eTokenResult token;

		while (true)
		{
			token = lpReadScript->GetToken();

			if (token == TOKEN_END || token == TOKEN_END_SECTION)
			{
				break;
			}

			MESSAGE_INFO info;

			info.Index = lpReadScript->GetNumber();

			strcpy_s(info.Message, lpReadScript->GetAsString());

			this->m_MessageInfo[lang].insert(std::pair<int, MESSAGE_INFO>(info.Index, info));
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpReadScript->GetError());
	}

	delete lpReadScript;
}

// Mensajes agregados después de una versión publicada. Un Data/ viejo puede no
// tenerlos: en Docker y Pterodactyl Data/ es un volumen que no se pisa al
// actualizar la imagen. Sin esto el jugador veía "Could not find message".
// Mismo texto que Message_<idioma>.txt, en el orden de LANGUAGE_*.
static const char* BuiltInMessage(int index, int lang)
{
	struct Entry { int index; const char* text[MAX_LANGUAGE]; };

	static const Entry entries[] = {
		{ 113, { "'%s' defeated '%s' (%d/%d)", "'%s' derrot\xC3\xB3 a '%s' (%d/%d)", "'%s' derrotou '%s' (%d/%d)" } },
	};

	for (const Entry& entry : entries)
	{
		if (entry.index == index)
		{
			return entry.text[(lang >= 0 && lang < MAX_LANGUAGE) ? lang : 0];
		}
	}

	return nullptr;
}

char* CMessage::GetTextMessage(int index, int lang)
{
	if (lang >= 0 && lang < MAX_LANGUAGE)
	{
		std::map<int, MESSAGE_INFO>::iterator it = this->m_MessageInfo[lang].find(index);

		if (it != this->m_MessageInfo[lang].end())
		{
			return it->second.Message;
		}
	}

	if (const char* builtIn = BuiltInMessage(index, lang))
	{
		strcpy_s(this->m_DefaultMessage, builtIn);

		return this->m_DefaultMessage;
	}

	wsprintf(this->m_DefaultMessage, "Could not find message %d, lang %d!", index, lang);

	return this->m_DefaultMessage;
}
