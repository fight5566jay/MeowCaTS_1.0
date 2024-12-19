#include "TileGroup.h"
#include <iostream>
#include <cassert>
using std::cerr;
using std::endl;

array<TileGroup, MAX_RANK> TileGroup::g_allTile;
array<TileGroup, MAX_SUIT_TILE_RANK - 2 + MAX_RANK> TileGroup::g_allMeld;
array<TileGroup, MAX_RANK> TileGroup::g_allPair;
array<TileGroup, MAX_SUIT_TILE_RANK * 2 - 3> TileGroup::g_allSubsequ;
/*size_t TileGroup::g_iAllTileSize;
size_t TileGroup::g_iAllMeldSize;
size_t TileGroup::g_iAllPairSize;
size_t TileGroup::g_iAllSequSize;*/
bool TileGroup::g_bFirstInit = true;
string TileGroup::m_sInitTiles;

void TileGroup::init()
{
#ifdef MJ_DEBUG_MODE
	//std::cout << "[TileGroup::init] g_bFirstInit=" << g_bFirstInit << ", m_sInitTiles=" << m_sInitTiles << std::endl;
	assert(g_bFirstInit || !m_sInitTiles.empty());
#endif
	if (g_bFirstInit) {
		//setup m_sInitTiles
		m_sInitTiles.resize(MAX_RANK);
		std::fill(m_sInitTiles.begin(), m_sInitTiles.end(), '0');

		g_bFirstInit = false;
		setupRegularId();//don't move this line before "m_bFirstInit = false;"
	}

	m_sTiles = m_sInitTiles;
}

void TileGroup::setupRegularId()
{
	uint64_t id = 1;
	const uint64_t mul = SAME_TILE_COUNT + 1;
	const size_t sz = g_allTile.size();
	
	for (int i = 0; i < sz; i++) {
		g_allTile[i] = id;
		id *= mul;
	}

	//sequence
	id = 1 + (SAME_TILE_COUNT + 1) + (SAME_TILE_COUNT + 1) * (SAME_TILE_COUNT + 1);//1 + 5 + 25
	for (int i = 0; i < MAX_SUIT_TILE_RANK - 2; i++) {
		g_allMeld[i] = id;
		id *= mul;
	}
	//triplet
	for (int i = 0; i < MAX_RANK; i++) {
		g_allMeld[MAX_SUIT_TILE_RANK - 2 + i] = g_allTile[i] + g_allTile[i] + g_allTile[i];//allTile[i] * 3
	}

	for (int i = 0; i < MAX_RANK; i++) {
		g_allPair[i] = g_allTile[i] + g_allTile[i];//allTile[i] * 2
	}

	id = 1 + (SAME_TILE_COUNT + 1);//5^0 + 5^1
	for (int i = 0; i < MAX_SUIT_TILE_RANK - 1; i++) {
		g_allSubsequ[i] = id;
		id *= mul;
	}
	id = 1  + (SAME_TILE_COUNT + 1) * (SAME_TILE_COUNT + 1);//5^0 + 5^2
	for (int i = 0; i < MAX_SUIT_TILE_RANK - 2; i++) {
		g_allSubsequ[MAX_SUIT_TILE_RANK - 1 + i] = id;
		id *= mul;
	}
}

string TileGroup::convertIdToString(const int& id)
{
	string result = m_sInitTiles;
	int cloneID = id;
	for (int i = 0; i < MAX_RANK; i++) {
		result[i] += cloneID % (SAME_TILE_COUNT + 1);
		cloneID /= (SAME_TILE_COUNT + 1);
	}

	//std::cout << "[TileGroup::convertIdToString] id=" << id << ", result=" << result << std::endl;
	return result;
}

int TileGroup::convertStringToId(const string& tiles)
{
	int result = 0;
	for (int i = tiles.length() - 1; i >= 0; i--)
		result = result * (SAME_TILE_COUNT + 1) + tiles[i] - '0';
	return result;
}

int TileGroup::getTileNumber() const
{
	int result = 0;
	for (int i = 0; i < m_sTiles.size(); i++)
		result += m_sTiles[i] - '0';
	return result;
}

bool TileGroup::hasPair() const
{
	for (int i = 0; i < m_sTiles.size(); i++) {
		if (m_sTiles[i] >= '2')
			return true;
	}
	return false;
}

bool TileGroup::hasMeld() const
{
	for (int i = 0; i < m_sTiles.size(); i++) {
		if (m_sTiles[i] >= '3')
			return true;
	}

	if (m_sTiles.size() == MAX_SUIT_TILE_RANK) {
		for (int i = 0; i < MAX_SUIT_TILE_RANK - 2; i++) {
			if (m_sTiles[i] >= '1' && m_sTiles[i + 1] >= '1' && m_sTiles[i + 2] >= '1')
				return true;
		}
	}
	return false;
}

bool TileGroup::canPickOut(const TileGroup& tiles) const
{
	for (int i = 0; i < m_sTiles.size(); i++) {
		if (m_sTiles[i] < tiles[i])
			return false;
	}
	return true;
}

bool TileGroup::canPickOut(const string& tiles) const
{
	for (int i = 0; i < tiles.size(); i++) {
		if (m_sTiles[i] < tiles[i])
			return false;
	}
	return true;
}

void TileGroup::pickOut(const TileGroup& tiles)
{
	for (int i = 0; i < m_sTiles.size(); i++)
		m_sTiles[i] -= tiles[i] - '0';
	m_iID -= tiles.m_iID;
}

bool TileGroup::canPutIn(const TileGroup& tiles)
{
	for (int i = 0; i < m_sTiles.size(); i++) {
		if (m_sTiles[i] + tiles[i] > 2 * '0' + 4)
			return false;
	}
	return true;
}

bool TileGroup::canPutIn(const string& tiles)
{
	for (int i = 0; i < m_sTiles.size(); i++) {
		if (m_sTiles[i] + tiles[i] > 2 * '0' + 4)
			return false;
	}
	return true;
}

void TileGroup::putIn(const TileGroup& tiles)
{
	for (int i = 0; i < m_sTiles.size(); i++)
		m_sTiles[i] += tiles[i] - '0';
	m_iID += tiles.m_iID;
}

bool TileGroup::isOnePair() const
{
#ifdef MJ_DEBUG_MODE
	assert(m_iID <= g_allPair[MAX_SUIT_TILE_RANK - 1]);
#endif
	for (int i = 0; i < 9; i++) {
		if (m_iID > g_allPair[i])
			continue;
		if (m_iID < g_allPair[i])
			return false;
		return true;
	}
	return false;
}

int TileGroup::getFirstPair() const
{
	for (int i = 0; i < 9; i++) {
		if (m_sTiles[i] == '2')
			return i;
	}
	return -1;
}

bool TileGroup::isAllPair() const
{
	for (int i = 0; i < 9; i++) {
		if (m_sTiles[i] != '0' && m_sTiles[i] != '2')
			return false;
	}
	return true;
}

TileGroup TileGroup::operator=(const int& id)
{
	m_iID = id;
	//m_sTiles = convertIdToString(id);//segmentation fault. WHY??
	string sTiles = convertIdToString(id);
	m_sTiles = sTiles;
	return *this;
}

TileGroup& TileGroup::operator++()
{
	m_iID += 1;
	for (int i = 0; i < m_sTiles.size(); i++) {
		if (m_sTiles[i] < '4') {
			m_sTiles[i] += 1;
			for (int j = 0; j < i; j++)
				m_sTiles[j] = '0';
			break;
		}
	}
	return *this;
}