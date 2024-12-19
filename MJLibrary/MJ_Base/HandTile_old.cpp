#include "HandTile_old.h"
#include <cassert>
#include <boost/smart_ptr/make_shared_array.hpp>

HandTile_old::HandTile_old(const string& sInput)
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
		else if (islower(cTileType))
			iTileType = cTileType - 'a';
		else {
			cerr << "[HandTile_old::HandTile] illegal input string: " << sInput << endl;
			assert(isupper(cTileType) || islower(cTileType));
		}

		//setup member variable
		int iTileRank;
		for (int i = 0; i < tok.length() - 1; i++) {
			iTileRank = tok[i] - '0';
			m_vTileNumbers[Tile(iTileType, iTileRank)]++;
			m_iTotalTileNumber++;
		}

		if (iTileType == TILE_TYPE_COUNT - 1)
			break;
	}
}

void HandTile_old::clear()
{
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; ++i)
		m_vTileNumbers[i] = 0;
	m_iTotalTileNumber = 0;
}

void HandTile_old::putTile(const Tile& oTile, const int& iNumber)
{
#ifdef MJ_DEBUG_MODE
	assert(m_vTileNumbers[oTile] < SAME_TILE_COUNT);
#endif
	m_vTileNumbers[oTile] += iNumber;
	m_iTotalTileNumber += iNumber;
}

void HandTile_old::popTile(const Tile& oTile, const int& iNumber)
{
#ifdef MJ_DEBUG_MODE
	assert(m_vTileNumbers[oTile] >= iNumber);
#endif
	m_vTileNumbers[oTile] -= iNumber;
	m_iTotalTileNumber -= iNumber;
}

string HandTile_old::toString() const
{
	if (m_iTotalTileNumber == 0)
		return string();

	string str;
	int index = 0;
	//suit
	for (int i = 0; i < SUIT_COUNT; i++) {
		for (int j = 1; j <= MAX_SUIT_TILE_RANK; j++) {
			for (int k = 0; k < m_vTileNumbers[index]; k++) {
				str += to_string(j);
			}
			index++;
		}
		str += Tile(i, 1).getTileTypeStr();
		str += " ";
	}
	//honor
	for (int i = 0; i < HONOR_COUNT; i++) {
		for (int j = 1; j <= MAX_HONOR_TILE_RANK; j++) {
			for (int k = 0; k < m_vTileNumbers[index]; k++) {
				str += to_string(j);
			}
			index++;
		}
		str += Tile(SUIT_COUNT + i, 1).getTileTypeStr();
		str += " ";
	}

	str.erase(str.end() - 1);
	return str;
}

array<int, TILE_TYPE_COUNT> HandTile_old::getHandTileId() const
{
	array<int, TILE_TYPE_COUNT> vId = { 0 };
	int iTile = 0;
	for (int i = 0; i < SUIT_COUNT; i++) {
		int id = 0;
		for (int j = 1; j <= MAX_SUIT_TILE_RANK; j++) {
			id = id * (SAME_TILE_COUNT + 1) + m_vTileNumbers[iTile++];
		}
		vId[i] = id;
	}

	for (int i = 0; i < HONOR_COUNT; i++) {
		int id = 0;
		for (int j = 1; j <= MAX_HONOR_TILE_RANK; j++) {
			id = id * SAME_TILE_COUNT + m_vTileNumbers[iTile++];
		}
		vId[SUIT_COUNT + i] = id;
	}

	return vId;
}

bool HandTile_old::canEatRight(const Tile& oTile) const
{
	return oTile.isSuit() && oTile.getRank() > 2 && m_vTileNumbers[oTile - 2] > 0 && m_vTileNumbers[oTile - 1] > 0;
}

bool HandTile_old::canEatMiddle(const Tile& oTile) const
{
	const int iRank = oTile.getRank();
	return oTile.isSuit() && iRank > 1 && iRank < MAX_SUIT_TILE_RANK && m_vTileNumbers[oTile - 1] > 0 && m_vTileNumbers[oTile + 1] > 0;
}

bool HandTile_old::canEatLeft(const Tile& oTile) const
{
	return oTile.isSuit() && oTile.getRank() < MAX_SUIT_TILE_RANK - 1 && m_vTileNumbers[oTile + 1] > 0 && m_vTileNumbers[oTile + 2] > 0;
}

bool HandTile_old::canDarkKong() const
{
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		if (canDarkKong(i))
			return true;
	}
	return false;
}

vector<Tile> HandTile_old::getCanDarkKongTile() const
{
	vector<Tile> result;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; ++i) {
		if (m_vTileNumbers[i] == 4)
			result.push_back(i);
	}
	return result;
}

bool HandTile_old::isAloneTile(const Tile& oTile) const
{
	if (m_vTileNumbers[oTile] != 1)
		return false;
	if (oTile.isHonor())
		return true;

	const int iRank = oTile.getRank();
	if (iRank > 2 && m_vTileNumbers[oTile - 2] != 0)
		return false;
	if (iRank > 1 && m_vTileNumbers[oTile - 1] != 0)
		return false;
	if (iRank < 8 && m_vTileNumbers[oTile + 2] != 0)
		return false;
	if (iRank < 9 && m_vTileNumbers[oTile + 1] != 0)
		return false;
	return true;
}

bool HandTile_old::willBeAloneTile(const Tile & oTile) const
{
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

bool HandTile_old::isTooManyMeld(const int& iNeedMeldCount) const
{
	/*bool bTooManyMeld = getMinLack(iNeedMeldCount + 1) - getMinLack(iNeedMeldCount) < 1;
	if (!bTooManyMeld)
		return false;*/
	return getMinLack(iNeedMeldCount + 1) - getMinLack(iNeedMeldCount) < 1;
}

int HandTile_old::getId(boost::shared_ptr<int[]>& vIds) const
{
	//the program will not delete the memory?
	vIds = boost::make_shared<int[]>(TILE_TYPE_COUNT);
	for (int i = 0; i < SUIT_COUNT; i++) {
		vIds[i] = 0;
		for (int j = 0; j < MAX_SUIT_TILE_RANK; j++) {
			vIds[i] *= (SAME_TILE_COUNT + 1);
			vIds[i] += m_vTileNumbers[i * MAX_SUIT_TILE_RANK + j];
		}
	}
	const int iHonorStartIndex = DIFF_SUIT_TILE_COUNT;
	for (int i = 0; i < HONOR_COUNT; i++) {
		vIds[SUIT_COUNT + i] = 0;
		for (int j = 0; j < MAX_HONOR_TILE_RANK; j++) {
			vIds[SUIT_COUNT + i] *= (SAME_TILE_COUNT + 1);
			vIds[SUIT_COUNT + i] += m_vTileNumbers[iHonorStartIndex + i * MAX_HONOR_TILE_RANK + j];
		}
	}

	return 0;
}

bool HandTile_old::canWin(const Tile& oTile, const int& iMeldNumber) const
{
	HandTile_old oHand = *this;
	return oHand.isWin(iMeldNumber);
}

bool HandTile_old::operator==(const HandTile_old & rhs) const
{
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		if (m_vTileNumbers[i] != rhs.m_vTileNumbers[i])
			return false;
	}
	return true;
}

/*void HandTile_old::operator=(const HandTile & rhs)
{
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		m_vTileNumbers[i] = rhs.m_vTileNumbers[i];
	}
	m_iTotalTileNumber = rhs.m_iTotalTileNumber;
}*/
