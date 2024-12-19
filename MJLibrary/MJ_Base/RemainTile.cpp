#include "RemainTile.h"
#include "../Base/Tools.h"

RemainTile::RemainTile(const string& sRemainTile, const RemainTileType& oType) : m_oRemainTileType(oType)
{
	assert(sRemainTile.length() == MAX_DIFFERENT_TILE_COUNT);
	Tile::setupTileIds();
	std::fill(m_vIds.begin(), m_vIds.end(), 0);

	int iTileCount;
	m_iTotalRemainNumber = 0;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		assert(sRemainTile[i] >= '0' && sRemainTile[i] <= SAME_TILE_COUNT + '0');
		iTileCount = sRemainTile[i] - '0';
		addTile(i, iTileCount);
	}
	m_iRemainDrawNumber = m_iTotalRemainNumber - NONPICKABLE_TILE_COUNT;
	if (oType == RemainTileType::Type_Playing) {
		m_iRemainDrawNumber -= (MAX_HANDTILE_COUNT - 1) * (PLAYER_COUNT - 1);
	}
}

void RemainTile::init(const RemainTileType& oType)
{
	Tile::setupTileIds();
	m_oRemainTileType = oType;
	int iIndex = 0;
	const int ciSuitInitId = Tile::getId(MAX_SUIT_TILE_RANK + 1) - 1;
	const int ciHonorInitId = Tile::getId(MAX_HONOR_TILE_RANK + 1) - 1;
	/*for (int i = 0; i < SUIT_COUNT; i++) {
		m_vIds[iIndex++] = ciSuitInitId;
	}

	for (int i = 0; i < HONOR_COUNT; i++) {
		m_vIds[iIndex++] = ciHonorInitId;
	}*/
	std::fill(m_vIds.begin(), m_vIds.begin() + SUIT_COUNT, Tile::getId(MAX_SUIT_TILE_RANK + 1) - 1);
	std::fill(m_vIds.begin() + SUIT_COUNT, m_vIds.end(), Tile::getId(MAX_HONOR_TILE_RANK + 1) - 1);

	m_iTotalRemainNumber = TOTAL_TILE_COUNT;
	m_iRemainDrawNumber = TOTAL_TILE_COUNT - NONPICKABLE_TILE_COUNT;
	if (oType == RemainTileType::Type_Playing) {
		m_iRemainDrawNumber -= (MAX_HANDTILE_COUNT - 1) * (PLAYER_COUNT - 1);
	}
}

void RemainTile::addTile(const Tile& oTile, const int& iNumber)
{
#ifdef MJ_DEBUG_MODE
	assert(getRemainNumber(oTile) + iNumber <= SAME_TILE_COUNT);
#endif
	m_vIds[oTile.getTileType()] += Tile::getId(oTile.getRank()) * iNumber;
	m_iTotalRemainNumber += iNumber;
}

void RemainTile::popTile(const Tile& oTile, const int& iNumber, const bool& bDrawTile)
{
	//bDrawTile: true if pop by drawing a tile. False if pop by opponent's melding action or else
#ifdef MJ_DEBUG_MODE
	assert(getRemainNumber(oTile) >= iNumber && m_iTotalRemainNumber > 0);
	assert(m_oRemainTileType == RemainTileType::Type_Playing || m_iRemainDrawNumber > 0);
#endif
	m_vIds[oTile.getTileType()] -= Tile::getId(oTile.getRank()) * iNumber;
	m_iTotalRemainNumber -= iNumber;
	if (bDrawTile) {
		m_iRemainDrawNumber -= iNumber; 
	}
	/*else {
		CERR("popTile with bDrawTile = false\n");
	}*/
}

void RemainTile::undoPopTile(const Tile& oTile, const int& iNumber, const bool& bDrawTile)
{
	//bDrawTile: true if pop by drawing a tile. False if pop by opponent's melding action or else
	addTile(oTile, iNumber);
	if (bDrawTile) {
		m_iRemainDrawNumber += iNumber;
	}
	/*else {
		CERR("undoPopTile with bDrawTile = false\n");
	}*/
}

Tile RemainTile::randomPopTile()
{
	assert(m_iTotalRemainNumber > 0);
	int iSumWeight = 0;
	Tile oPopTile;
	int iTileCount;

	int iNum = random_int() % m_iTotalRemainNumber;
	int id, index = 0, iTile = 0;
	const int div = Tile::getDivId();
	for (int i = 0; i < SUIT_COUNT; i++) {
		id = m_vIds[index++];
		for (int j = 0; j < MAX_SUIT_TILE_RANK; j++, iTile++) {
			iTileCount = id % div;
			iNum -= iTileCount;
			if (iNum < 0) {
				popTile(iTile, 1, true);
				return iTile;
			}
			id /= div;
		}
	}

	for (int i = 0; i < HONOR_COUNT; i++) {
		id = m_vIds[index++];
		for (int j = 0; j < MAX_HONOR_TILE_RANK; j++, iTile++) {
			iTileCount = id % div;
			iNum -= iTileCount;
			if (iNum < 0) {
				popTile(iTile, 1, true);
				return iTile;
			}
			id /= div;
		}
	}

	//Don't run to here
	std::cerr << "[RemainTile::randomPopTile] invalid pop tile: iNum = " << iNum << ", index = " << index << ", iTile = " << iTile << std::endl;
	assert(index < TILE_TYPE_COUNT && iTile < MAX_DIFFERENT_TILE_COUNT);
	assert(0);
	return oPopTile;
}

Tile RemainTile::randomPopTileByHandTile(const HandTile& oHandTile, const int& iMeldNumber)
{
	assert(m_iTotalRemainNumber > 0);
	int iSumWeight = 0;
	Tile oPopTile;
	int iTileCount;
	/*
	int iWeight;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; ++i) {
		iTileCount = getRemainNumber(i);
		if (iTileCount > 0) {
			iWeight = iTileCount;

			if (oHandTile.canWin(i, iMeldNumber) || oHandTile.canPong(i))
				iWeight *= 4;
			else if (oHandTile.canEat(i))
				iWeight *= 2;

			if (rand(iSumWeight, iWeight))
				oPopTile = i;
			iSumWeight += iWeight;
		}
	}*/

	int id, index = 0, iTile = 0, iWeight;
	const int div = Tile::getDivId();
	for (int i = 0; i < SUIT_COUNT; i++) {
		id = m_vIds[index++];
		for (int j = 0; j < MAX_SUIT_TILE_RANK; j++, iTile++) {
			iTileCount = id % div;
			id /= div;

			if (iTileCount > 0) {
				iWeight = iTileCount;

				if (oHandTile.canWin(iTile, iMeldNumber) || oHandTile.canPong(iTile))
					iWeight *= 4;
				else if (oHandTile.canEat(iTile))
					iWeight *= 2;

				if (rand(iSumWeight, iWeight))
					oPopTile = iTile;
				iSumWeight += iWeight;
			}
		}
	}

	for (int i = 0; i < HONOR_COUNT; i++) {
		id = m_vIds[index++];
		for (int j = 0; j < MAX_HONOR_TILE_RANK; j++, iTile++) {
			iTileCount = id % div;
			id /= div;

			if (iTileCount > 0) {
				iWeight = iTileCount;

				if (oHandTile.canWin(iTile, iMeldNumber) || oHandTile.canPong(iTile))
					iWeight *= 4;
				else if (oHandTile.canEat(iTile))
					iWeight *= 2;

				if (rand(iSumWeight, iWeight))
					oPopTile = iTile;
				iSumWeight += iWeight;
			}
		}
	}

	popTile(oPopTile, 1, true);
	return oPopTile;
}

double RemainTile::getDangerousFactor(const Tile& oTile) const
{
	//not implement yet
	return 0.0;
}

int RemainTile::getRemainNumber(const Tile & oTile) const
{
	int iRank = oTile.getRank();
	return m_vIds[oTile.getTileType()] % Tile::getId(iRank + 1) / Tile::getId(iRank);
}

int RemainTile::getRemainNumberList(RemainTileNumberArray& vTileNumbers)
{
	const int ciSize = MAX_DIFFERENT_TILE_COUNT;
	int iRemainTileId, iRemainTileIdIndex = 0, iTile = 0;
	const int div = Tile::getDivId();

	//suit
	for (int i = 0; i < SUIT_COUNT; i++) {
		iRemainTileId = m_vIds[iRemainTileIdIndex++];
		for (int j = 0; j < MAX_SUIT_TILE_RANK; j++) {
			vTileNumbers[iTile++] = iRemainTileId % div;
			iRemainTileId /= div;
		}
	}

	//honor
	for (int i = 0; i < HONOR_COUNT; i++) {
		iRemainTileId = m_vIds[iRemainTileIdIndex++];
		for (int j = 0; j < MAX_HONOR_TILE_RANK; j++) {
			vTileNumbers[iTile++] = iRemainTileId % div;
			iRemainTileId /= div;
		}
	}

	return ciSize;
}

string RemainTile::toString() const
{
	stringstream ss;
	RemainTileId_t id;
	const int div = Tile::getDivId();
	int iTypeIndex = 0;

	for (int i = 0; i < SUIT_COUNT; i++) {
		id = m_vIds[iTypeIndex++];
		for (int j = 0; j < MAX_SUIT_TILE_RANK; j++) {
			ss << id % div;
			id /= div;
		}
	}
	for (int i = 0; i < HONOR_COUNT; i++) {
		id = m_vIds[iTypeIndex++];
		for (int j = 0; j < MAX_HONOR_TILE_RANK; j++) {
			ss << id % div;
			id /= div;
		}
	}
	return ss.str();
}

string RemainTile::getReadableString() const
{
	stringstream ss;
	RemainTileId_t id;
	const int div = Tile::getDivId();
	int iTypeIndex = 0;

	for (int i = 0; i < SUIT_COUNT; i++) {
		id = m_vIds[iTypeIndex++];
		for (int j = 0; j < MAX_SUIT_TILE_RANK; j++) {
			ss << id % div << " ";
			id /= div;
		}
		ss << std::endl;
	}
	for (int i = 0; i < HONOR_COUNT; i++) {
		id = m_vIds[iTypeIndex++];
		for (int j = 0; j < MAX_HONOR_TILE_RANK; j++) {
			ss << id % div << " ";
			id /= div;
		}
		ss << std::endl;
	}
	return ss.str();
}

bool RemainTile::operator==(const RemainTile & rhs) const
{
	if(m_oRemainTileType != rhs.m_oRemainTileType
		|| m_iTotalRemainNumber != rhs.m_iTotalRemainNumber
		|| m_iRemainDrawNumber != rhs.m_iRemainDrawNumber)
	return false;

	for (int i = 0; i < TILE_TYPE_COUNT; i++)
		if (m_vIds[i] != rhs.m_vIds[i])
			return false;
	return true;
}

/*void RemainTile::operator=(const RemainTile & rhs)
{
	RemainTile oRemainTile;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		m_vRemainNumber[i] = rhs.m_vRemainNumber[i];
	}
	m_oRemainTileType = rhs.m_oRemainTileType;
	m_iTotalRemainNumber = rhs.m_iTotalRemainNumber;
	m_iRemainDrawNumber = rhs.m_iRemainDrawNumber;
}*/
