#pragma once
#include "../../MJLibrary/MJ_Base/PlayerTile.h"
#include "../../MJLibrary/MJ_Base/MJBaseTypedef.h"
#include "../../MJLibrary/MJ_Base/Move.h"
#include <array>
typedef float WinRate_t;

class BaseSamplingData {
public:
	//---------- Constructor / Destructor ----------
	BaseSamplingData(const Move& oAction, const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iDrawCount);

	BaseSamplingData() {};
	~BaseSamplingData() {};

	//---------- Public function ----------
	//template function
	virtual void sample(const uint32_t& iSampleCount, const int& iNeedMeldCount = NEED_GROUP) = 0;
	virtual void clearData();

	//get/set function
	inline int getWinCount() const { return getWinCount(m_iDrawCount); };
	inline int getWinCount(const int& iDrawCount) const { return m_vWinCount[iDrawCount]; };
	inline WinRate_t getWinRate() const { return getWinRate(m_iDrawCount); };
	inline WinRate_t getWinRate(const int& iDrawCount) const { return static_cast<WinRate_t>(m_vWinCount[iDrawCount]) / m_iUsedSampleCount; };
	inline WinRate_t getSD() const { return getSD(m_iDrawCount); };
	inline WinRate_t getSD(const int& iDrawCount) const { return sqrt(getWinRate(iDrawCount) * (1 - getWinRate(iDrawCount)) / m_iUsedSampleCount); };

	//other function
	void printResult() const;
	static bool cmpData(const BaseSamplingData& a, const BaseSamplingData& b);
	
	//---------- Member variable ----------
public:
	Move m_oAction;
	int m_iUsedSampleCount;
	array<uint32_t, AVERAGE_MAX_DRAW_COUNT + 1> m_vWinCount;
	array<WinRate_t, AVERAGE_MAX_DRAW_COUNT + 1> m_vWinRate;
	PlayerTile m_oPlayerTile;
	RemainTile_t m_oRemainTile;
	int m_iDrawCount;
	bool m_bIsPruned;
	time_t m_iUsedTime;
};