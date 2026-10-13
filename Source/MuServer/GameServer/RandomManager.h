#pragma once

#include <cstdint>

struct RANDOM_MANAGER_INFO
{
	intptr_t value;
	int MinRate;
	int MaxRate;
};

class CRandomManager
{
public:

	CRandomManager();

	~CRandomManager();

	void Clear();

	template<typename T>
	bool GetRandomElement(T* value)
	{
		intptr_t selected;
		if (!this->SelectRandomElement(&selected))
		{
			return false;
		}
		*value = static_cast<T>(selected);
		return true;
	}

	void AddElement(intptr_t value, int rate);

private:

	bool SelectRandomElement(intptr_t* value);

	int m_MaxRate;

	std::vector<RANDOM_MANAGER_INFO> m_RandomManagerInfo;
};
