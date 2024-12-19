#include "RemainTile_old.h"
#include "../Base/Tools.h"

RemainTile_old::RemainTile_old(const string& sRemainTile, const RemainTileType& oType) : m_oRemainTileType(oType)
{
	assert(sRemainTile.length() == MAX_DIFFERENT_TILE_COUNT);
	m_iTotalRemainNumber = 0;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		assert(sRemainTile[i] >= '0' && sRemainTile[i] <= SAME_TILE_COUNT + '0');
		m_vRemainNumber[i] = sRemainTile[i] - '0';
		m_iTotalRemainNumber += sRemainTile[i] - '0';
	}
	m_iRemainDrawNumber = m_iTotalRemainNumber - NONPICKABLE_TILE_COUNT;
	if (oType == RemainTileType::Type_Playing) {
		m_iRemainDrawNumber -= (MAX_HANDTILE_COUNT - 1) * (PLAYER_COUNT - 1);
	}
}

void RemainTile_old::init(const RemainTileType& oType)
{
	m_oRemainTileType = oType;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; ++i)
		m_vRemainNumber[i] = SAME_TILE_COUNT;
	m_iTotalRemainNumber = TOTAL_TILE_COUNT;
	m_iRemainDrawNumber = TOTAL_TILE_COUNT - NONPICKABLE_TILE_COUNT;
	if (oType == RemainTileType::Type_Playing) {
		m_iRemainDrawNumber -= (MAX_HANDTILE_COUNT - 1) * (PLAYER_COUNT - 1);
	}

	/*const int iMaxSuitId = pow(SAME_TILE_COUNT + 1, MAX_SUIT_TILE_RANK) - 1;//5^9-1
	const int iMaxHonorId = pow(SAME_TILE_COUNT + 1, MAX_HONOR_TILE_RANK) - 1;//5^7-1
	for (int i = 0; i < SUIT_COUNT; i++) {
		m_vId[i] = iMaxSuitId;
	}
	for (int i = 0; i < HONOR_COUNT; i++) {
		m_vId[i] = iMaxHonorId;
	}*/
}

void RemainTile_old::addTile(const Tile& oTile, const int& iNumber)
{
	assert(m_vRemainNumber[oTile] + iNumber <= SAME_TILE_COUNT);
	m_vRemainNumber[oTile] += iNumber;
	m_iTotalRemainNumber += iNumber;
	//m_vId[oTile.getTileType()] += oTile.getId() * iNumber;
}

void RemainTile_old::popTile(const Tile& oTile, const int& iNumber, const bool& bDrawTile)
{
	//bDrawTile: true if pop by drawing a tile. False if pop by opponent's melding action or else
	assert(m_vRemainNumber[oTile] >= iNumber && m_iTotalRemainNumber > 0);
	assert(m_oRemainTileType == RemainTileType::Type_Playing || m_iRemainDrawNumber > 0);
	m_vRemainNumber[oTile] -= iNumber;
	m_iTotalRemainNumber -= iNumber;
	//m_vId[oTile.getTileType()] -= oTile.getId() * iNumber;
	if (bDrawTile) {
		m_iRemainDrawNumber -= iNumber;
	}
	/*else {
		CERR("popTile with bDrawTile = false\n");
	}*/
}

void RemainTile_old::undoPopTile(const Tile& oTile, const int& iNumber, const bool& bDrawTile)
{
	//bDrawTile: true if pop by drawing a tile. False if pop by opponent's melding action or else
	assert(m_vRemainNumber[oTile] + iNumber <= SAME_TILE_COUNT);
	m_vRemainNumber[oTile] += iNumber;
	m_iTotalRemainNumber += iNumber;
	//m_vId[oTile.getTileType()] -= oTile.getId() * iNumber;
	if (bDrawTile) {
		m_iRemainDrawNumber += iNumber;
	}
	else {
		CERR("undoPopTile with bDrawTile = false\n");
	}
}

Tile RemainTile_old::randomPopTile()
{
	assert(m_iTotalRemainNumber > 0);
	int iSumWeight = 0;
	Tile oPopTile;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; ++i) {
		if (m_vRemainNumber[i] > 0 && rand(iSumWeight, m_vRemainNumber[i]))
			oPopTile = i;
		iSumWeight += m_vRemainNumber[i];
	}
	popTile(oPopTile, 1, true);
	return oPopTile;
}

Tile RemainTile_old::randomPopTileByHandTile(const HandTile& oHandTile, const int& iMeldNumber)
{
	assert(m_iTotalRemainNumber > 0);
	int iSumWeight = 0;
	Tile oPopTile;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; ++i) {
		if (m_vRemainNumber[i] > 0) {
			int iWeight = m_vRemainNumber[i];

			if (oHandTile.canWin(i, iMeldNumber) || oHandTile.canPong(i))
				iWeight *= 4;
			else if (oHandTile.canEat(i))
				iWeight *= 2;

			if (rand(iSumWeight, iWeight))
				oPopTile = i;
			iSumWeight += iWeight;
		}
	}
	popTile(oPopTile, 1, true);
	return oPopTile;
}

double RemainTile_old::getDangerousFactor(const Tile& oTile) const
{
	//not implement yet
	return 0.0;
}

string RemainTile_old::toString() const
{
	stringstream ss;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		ss << m_vRemainNumber[i];
	}
	return ss.str();
}

string RemainTile_old::getReadableString() const
{
	stringstream ss;
	int iIndex = 0;
	for (int i = 0; i < SUIT_COUNT; i++) {
		for (int j = 0; j < MAX_SUIT_TILE_RANK; j++) {
			ss << m_vRemainNumber[iIndex++] << " ";
		}
		ss << endl;
	}
	for (int i = 0; i < HONOR_COUNT; i++) {
		for (int j = 0; j < MAX_HONOR_TILE_RANK; j++) {
			ss << m_vRemainNumber[iIndex++] << " ";
		}
		ss << endl;
	}

	return ss.str();
}

bool RemainTile_old::operator==(const RemainTile_old & rhs) const
{
	if (m_oRemainTileType != rhs.m_oRemainTileType
		|| m_iTotalRemainNumber != rhs.m_iTotalRemainNumber
		|| m_iRemainDrawNumber != rhs.m_iRemainDrawNumber)
		return false;

	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++)
		if (m_vRemainNumber[i] != rhs.m_vRemainNumber[i])
			return false;
	return true;
}

/*void RemainTile_old::operator=(const RemainTile & rhs)
{
	RemainTile oRemainTile;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		m_vRemainNumber[i] = rhs.m_vRemainNumber[i];
	}
	m_oRemainTileType = rhs.m_oRemainTileType;
	m_iTotalRemainNumber = rhs.m_iTotalRemainNumber;
	m_iRemainDrawNumber = rhs.m_iRemainDrawNumber;
}*/
