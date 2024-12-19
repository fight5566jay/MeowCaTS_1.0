#include "BaseSamplingData.h"
#include <iostream>
using std::cout;
using std::endl;

BaseSamplingData::BaseSamplingData(const Move & oAction, const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const int & iDrawCount) :
	m_oAction(oAction), m_oPlayerTile(oPlayerTile), m_oRemainTile(oRemainTile), m_iDrawCount(iDrawCount), m_iUsedSampleCount(0), m_bIsPruned(false), m_iUsedTime(0)
{
	clearData();
}

void BaseSamplingData::clearData()
{
	m_iUsedSampleCount = 0;
	std::fill(m_vWinCount.begin(), m_vWinCount.end(), 0);
	std::fill(m_vWinRate.begin(), m_vWinRate.end(), 0.0);
	m_bIsPruned = false;
	m_iUsedTime = 0;
}

void BaseSamplingData::printResult() const
{
	std::cout << "Used time: " << m_iUsedTime << " ms." << std::endl;
	std::cout << "Sample count: " << m_iUsedSampleCount << std::endl;
	for (int i = 0; i <= m_iDrawCount; i++) {
		std::cout << i << "\t\t";
	}
	std::cout << std::endl;

	for (int i = 0; i <= m_iDrawCount; i++) {
		std::cout << m_vWinCount[i] << "(" << m_vWinRate[i] << ")\t";
	}
	std::cout << std::endl;
}

bool BaseSamplingData::cmpData(const BaseSamplingData & a, const BaseSamplingData & b)
{
	return a.getWinRate() > b.getWinRate();
}
