#include "HandTile.h"
#include <cassert>

using std::cerr;
using std::endl;


HandTile::HandTile()
{
	clear();
}

HandTile::HandTile(const string& sInput)
{
	clear();
	stringstream ss(sInput);
	string tok;
	while (!ss.eof()) {
		ss >> tok;//123a
		if (tok.empty())
			break;

		//parse the token string
		//char cTileType = tok[tok.length() - 1];
		char cTileType = tok.back();
		int iTileType;
		if (isupper(cTileType))
			iTileType = cTileType - 'A' + SUIT_COUNT;
		else if(islower(cTileType))
			iTileType = cTileType - 'a';
		else {
			std::cerr << "[HandTile::HandTile] illegal input string: " << sInput << std::endl;
			assert(isupper(cTileType) || islower(cTileType));
		}

		//setup member variable
		int iTileRank;
		for (int i = 0; i < tok.length() - 1; i++) {
			iTileRank = tok[i] - '0';
			putTile(Tile(iTileType, iTileRank));
		}

		if (iTileType == TILE_TYPE_COUNT - 1)
			break;
	}
}

void HandTile::clear()
{
	for (int i = 0; i < TILE_TYPE_COUNT; ++i)
		m_vIds[i] = 0;
	m_iTotalTileNumber = 0;
}

int HandTile::getTileNumber(const Tile& oTile) const
{
	int iRank = oTile.getRank();
	return m_vIds[oTile.getTileType()] % Tile::getId(iRank + 1) / Tile::getId(iRank);
}

int HandTile::getTileNumberList(HandTileNumberArray& vTileNumbers) const
{
	const int ciSize = MAX_DIFFERENT_TILE_COUNT;
	int iHandTileId, iHandTileIdIndex = 0, iTile = 0;
	int iTileCount;
	const int div = Tile::getDivId();

	//suit
	for (int i = 0; i < SUIT_COUNT; i++) {
		iHandTileId = m_vIds[iHandTileIdIndex++];
		for (int j = 0; j < MAX_SUIT_TILE_RANK; j++) {
			vTileNumbers[iTile++] = iHandTileId % div;
			iHandTileId /= div;
		}
	}

	//honor
	for (int i = 0; i < HONOR_COUNT; i++) {
		iHandTileId = m_vIds[iHandTileIdIndex++];
		for (int j = 0; j < MAX_HONOR_TILE_RANK; j++) {
			vTileNumbers[iTile++] = iHandTileId % div;
			iHandTileId /= div;
		}
	}

	return ciSize;
}

void HandTile::putTile(const Tile& oTile, const int& iNumber)
{
#ifdef MJ_DEBUG_MODE
	if (getTileNumber(oTile) >= SAME_TILE_COUNT) {
		std::cerr << "[HandTile::putTile] Too many same tile:" << std::endl;
		std::cerr << "HandTile: " << toString() << std::endl;
		std::cerr << "Try to put tile: " << oTile.toString() << std::endl;
		assert(getTileNumber(oTile) < SAME_TILE_COUNT);
	}
#endif
	m_vIds[oTile.getTileType()] += iNumber * Tile::getId(oTile.getRank());
	m_iTotalTileNumber += iNumber;
}

void HandTile::popTile(const Tile& oTile, const int& iNumber)
{
#ifdef MJ_DEBUG_MODE
	assert(getTileNumber(oTile) >= iNumber);
#endif
	m_vIds[oTile.getTileType()] -= iNumber * Tile::getId(oTile.getRank());
	m_iTotalTileNumber -= iNumber;
}

string HandTile::toString() const
{
	if (m_iTotalTileNumber == 0)
		return string();

	string str;
	HandTileId_t id;
	int iTileCount;
	int iType = 0;
	const int div = Tile::getDivId();

	//suit
	for (int i = 0; i < SUIT_COUNT; i++) {
		id = m_vIds[iType];
		for (int j = 1; j <= MAX_SUIT_TILE_RANK; j++) {
			iTileCount = id % div;
			for (int k = 0; k < iTileCount; k++) {
				str += std::to_string(j);
			}
			id /= div;
		}
		str += Tile(iType, 1).getTileTypeStr();
		str += " ";
		iType++;
	}
	//honor
	for (int i = 0; i < HONOR_COUNT; i++) {
		id = m_vIds[iType];
		for (int j = 1; j <= MAX_HONOR_TILE_RANK; j++) {
			iTileCount = id % div;
			for (int k = 0; k < iTileCount; k++) {
				str += std::to_string(j);
			}
			id /= div;
		}
		str += Tile(iType, 1).getTileTypeStr();
		str += " ";
		iType++;
	}

	str.erase(str.end() - 1);
	return str;
}

array<HandTileId_t, TILE_TYPE_COUNT> HandTile::getHandTileId() const
{
	array<HandTileId_t, TILE_TYPE_COUNT> vIds;
	for (int i = 0; i < TILE_TYPE_COUNT; i++) {
		vIds[i] = m_vIds[i];
	}
	return vIds;
}

bool HandTile::canEatRight(const Tile& oTile) const
{
	return oTile.isSuit() && oTile.getRank() > 2 && getTileNumber(oTile - 2) > 0 && getTileNumber(oTile - 1) > 0;
}

bool HandTile::canEatMiddle(const Tile& oTile) const
{
	const int iRank = oTile.getRank();
	return oTile.isSuit() && iRank > 1 && iRank < MAX_SUIT_TILE_RANK && getTileNumber(oTile - 1) > 0 && getTileNumber(oTile + 1) > 0;
}

bool HandTile::canEatLeft(const Tile& oTile) const
{
	return oTile.isSuit() && oTile.getRank() < MAX_SUIT_TILE_RANK - 1 && getTileNumber(oTile + 1) > 0 && getTileNumber(oTile + 2) > 0;
}

bool HandTile::canDarkKong() const
{
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		if (canDarkKong(i))
			return true;
	}
	return false;
}

vector<Tile> HandTile::getCanDarkKongTile() const
{
	vector<Tile> result;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; ++i) {
		if (getTileNumber(i) == 4)
			result.push_back(i);
	}
	return result;
}

bool HandTile::isAloneTile(const Tile& oTile) const
{
	if (getTileNumber(oTile) != 1)
		return false;
	if (oTile.isHonor())
		return true;

	const int iRank = oTile.getRank();
	if (iRank > 2 && getTileNumber(oTile - 2) != 0)
		return false;
	if (iRank > 1 && getTileNumber(oTile - 1) != 0)
		return false;
	if (iRank < 8 && getTileNumber(oTile + 2) != 0)
		return false;
	if (iRank < 9 && getTileNumber(oTile + 1) != 0)
		return false;
	return true;
}

bool HandTile::willBeAloneTile(const Tile & oTile) const
{
	//[TODO]Refector: use id to get tile count instead of calling getTileNumber()
	if (getTileNumber(oTile) > 0)
		return false;
	if (oTile.isHonor())
		return true;

	const int iRank = oTile.getRank();
	if (iRank > 2 && getTileNumber(oTile - 2) > 0)
		return false;
	if (iRank > 1 && getTileNumber(oTile - 1) > 0)
		return false;
	if (iRank < MAX_SUIT_TILE_RANK && getTileNumber(oTile + 1) > 0)
		return false;
	if (iRank < MAX_SUIT_TILE_RANK - 1 && getTileNumber(oTile + 2) > 0)
		return false;
	return true;
}

bool HandTile::isTooManyMeld(const int& iNeedMeldCount) const
{
	/*bool bTooManyMeld = getMinLack(iNeedMeldCount + 1) - getMinLack(iNeedMeldCount) < 1;
	if (!bTooManyMeld)
		return false;*/
	return getMinLack(iNeedMeldCount + 1) - getMinLack(iNeedMeldCount) < 1;
}

int HandTile::getId(HandTileIdArray& vIds) const
{
	for (int i = 0; i < TILE_TYPE_COUNT; i++) {
		vIds[i] = m_vIds[i];
	}
	return TILE_TYPE_COUNT;
}

bool HandTile::canWin(const Tile& oTile, const int& iMeldNumber) const
{
	HandTile oHand = *this;
	oHand.putTile(oTile);
	return oHand.isWin(iMeldNumber);
}

bool HandTile::operator==(const HandTile & rhs) const
{
	for (int i = 0; i < TILE_TYPE_COUNT; i++) {
		if (m_vIds[i] != rhs.m_vIds[i])
			return false;
	}
	return true;
}

void HandTile::_init()
{
	
}



/*void HandTile::operator=(const HandTile & rhs)
{
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		m_vTileNumbers[i] = rhs.m_vTileNumbers[i];
	}
	m_iTotalTileNumber = rhs.m_iTotalTileNumber;
}*/
