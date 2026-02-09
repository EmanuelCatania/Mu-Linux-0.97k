#include "stdafx.h"
#include "Path.h"

CPath gPath;

CPath::CPath()
{

}

CPath::~CPath()
{

}

void CPath::SetMainPath(const char* path)
{
	strcpy_s(this->m_MainPath, path);

	size_t len = std::strlen(this->m_MainPath);
	if (len > 0 && this->m_MainPath[len - 1] != '\\' && this->m_MainPath[len - 1] != '/')
	{
		strcat_s(this->m_MainPath, "/");
	}
}

char* CPath::GetFullPath(const char* file)
{
	strcpy_s(this->m_FullPath, this->m_MainPath);

	strcat_s(this->m_FullPath, file);

#ifndef _WIN32
	for (size_t i = 0; this->m_FullPath[i] != '\0'; ++i)
	{
		if (this->m_FullPath[i] == '\\')
		{
			this->m_FullPath[i] = '/';
		}
	}
#endif

	return this->m_FullPath;
}

