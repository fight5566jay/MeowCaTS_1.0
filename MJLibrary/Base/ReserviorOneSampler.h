#pragma once
#include "Tools.h"
#define MAX_RESERVIOR_SIZE 65536

template <class T>
class ReserviorOneSampler {
public:
	ReserviorOneSampler() : m_iCount(0) 
	{
		m_oDis = std::uniform_int_distribution<int>(0, MAX_RESERVIOR_SIZE);
	};
	~ReserviorOneSampler() {};

	inline void clear() { m_iCount = 0; };
	T input(const T& oData, const uint32_t& uiWeight = 1)
	{
#ifdef MJ_DEBUG_MODE
		//assert(uiWeight > 0 && m_iCount + uiWeight <= MAX_RESERVIOR_SIZE); //heavy cost when used by RemainTile::randomPopTile()
#endif
		m_iCount += uiWeight;
		if (randInt() % m_iCount < uiWeight) {
			m_oData = oData;
		}
		return m_oData;
	}
	inline T getData() const { return m_oData; };
	inline int randInt() { 
		return m_oDis(gen);
	};

private:
	T m_oData;
	uint32_t m_iCount;
	std::uniform_int_distribution<int> m_oDis;
};


