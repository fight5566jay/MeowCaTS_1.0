#pragma once
#include "Tile.h"
#include <cassert>

class Wind {
public:
	Wind() : m_iWind(-1) {};
	Wind(const int& iWind) : m_iWind(iWind) {
#ifdef MJ_DEBUG_MODE
		assert(iWind >= 0 && iWind <= 3);
#endif
	};
	~Wind() {};

	inline int getNextWind(const bool& bAllowUnknownWind = false) const {
		if (bAllowUnknownWind && m_iWind == -1) {
			return -1;
		}
		return m_iWind == 3 ? 0 : m_iWind + 1;
	}
	inline int getOppositeWind(const bool& bAllowUnknownWind = false) const {
		if (bAllowUnknownWind && m_iWind == -1) {
			return -1;
		}
		return m_iWind > 1 ? m_iWind - 2 : m_iWind + 2;
	}
	inline int getPreviousWind(const bool& bAllowUnknownWind = false) const {
		if (bAllowUnknownWind && m_iWind == -1) {
			return -1;
		}
		return m_iWind == 0 ? 3 : m_iWind - 1;
	}

	bool operator==(const Tile& rhs) { return m_iWind + DIFF_SUIT_TILE_COUNT == rhs; };
	bool operator!=(const Tile& rhs) { return m_iWind + DIFF_SUIT_TILE_COUNT != rhs; };

private:
	int m_iWind;//0~3
};