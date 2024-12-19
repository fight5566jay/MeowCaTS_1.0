#include "Tile.h"
#include <iostream>
#include <cassert>
using std::cerr;
using std::endl;
bool Tile::g_bFirstSetup = true;
int Tile::g_vTileIds[MAX_RANK + 2];

Tile::Tile() : m_iIndex(-1)
{
	if (g_bFirstSetup) {
		_init();
		g_bFirstSetup = false;
	}
}

Tile::Tile(const int& iTileIndex) : m_iIndex(iTileIndex)
{
	assert(iTileIndex >= 0 && iTileIndex < MAX_DIFFERENT_TILE_COUNT);
	if (g_bFirstSetup) {
		_init();
		g_bFirstSetup = false;
	}
}

Tile::Tile(const int& iTileType, const int& iRank)
{
	if (g_bFirstSetup) {
		_init();
		g_bFirstSetup = false;
	}

	if (iTileType >= 0 && iTileType < SUIT_COUNT) {
		assert(iRank >= 1 && iRank <= MAX_SUIT_TILE_RANK);
		m_iIndex = iTileType * MAX_SUIT_TILE_RANK + iRank - 1;
	}
	else if(iTileType < TILE_TYPE_COUNT){
		assert(iRank >= 1 && iRank <= MAX_HONOR_TILE_RANK);
		m_iIndex = DIFF_SUIT_TILE_COUNT + (iTileType - SUIT_COUNT) * MAX_HONOR_TILE_RANK + iRank - 1;
	}
	else {
		std::cerr << "[Tile::Tile] invalid iTileType: iTileType = " << iTileType << ", but the valid range is [0, " << TILE_TYPE_COUNT - 1 << "]" << std::endl;
		assert(iTileType >= 0 && iTileType <= (TILE_TYPE_COUNT - 1));
	}
}

Tile::Tile(const string& sTile)
{
	if (g_bFirstSetup) {
		_init();
		g_bFirstSetup = false;
	}

	//[Format] (integer)(a~z||A~Z)
	//The first integer is tile's rank.
	//If the last char is lower case (a~z), then the tile is a suit tile.
	//If the last char is upper case (A~Z), then the tile is a honor tile.
	assert(!sTile.empty());
	size_t strLen = sTile.length();
	char cType = sTile[strLen - 1];

	int iRank = atoi(sTile.substr(0, strLen - 1).c_str());
	if (islower(cType)) {//suit
		int iType = cType - 'a';
		assert(iType >= 0 && iType < SUIT_COUNT);
		m_iIndex = iType * MAX_SUIT_TILE_RANK + iRank - 1;
	}
	else {//honor
		int iType = cType - 'A'/* + SUIT_COUNT*/;
		assert(iType >= 0 && iType < HONOR_COUNT);
		m_iIndex = DIFF_SUIT_TILE_COUNT + iType * MAX_HONOR_TILE_RANK + iRank - 1;
	}
}

int Tile::getTileType() const
{
	if (isNull())
		return -1;
	if (isSuit())
		return m_iIndex / MAX_SUIT_TILE_RANK;//ex: input: 15, output:15 / 9 = 1
	return  SUIT_COUNT + (m_iIndex - DIFF_SUIT_TILE_COUNT) / MAX_HONOR_TILE_RANK;//ex: input: 33, output: 3 + (33-9*3) / 7 = 3
}

int Tile::getRank() const
{
	if (isNull())
		return -1;
	if (isSuit())
		return m_iIndex % MAX_SUIT_TILE_RANK + 1;//ex: input: 15, output:15 % 9 + 1 = 7
	return (m_iIndex - DIFF_SUIT_TILE_COUNT) % MAX_HONOR_TILE_RANK + 1;//ex: input: 33, output:(33-9*3) % 7 + 1 = 7
}

string Tile::toString() const
{
	if (isNull())
		return "()";

	string str("(");
	char cRank = '0' + getRank();
	str += cRank;
	str += getTileTypeStr();
	str.append(")");
	return str;
}

char Tile::getTileTypeStr() const
{
	return isSuit() ? 'a' + getTileType() : 'A' + getTileType() - SUIT_COUNT;
}

Tile Tile::sgfIdToTile(const int& iTile)
{
	int iType = iTile / PROTOCOL_SYMBOL_PLACE - 1;
	int iRank = (iTile / PROTOCOL_NUMBER_PLACE) % 10;
	return Tile(iType, iRank);
}

void Tile::_init()
{
	setupTileIds();
}

void Tile::setupTileIds()
{
	if (g_vTileIds[2] == SAME_TILE_COUNT + 1) {
		//already setup
		return;
	}
		
	int id = 1;
	const int mul = SAME_TILE_COUNT + 1;
	g_vTileIds[0] = -1;
	for (int i = 1; i <= MAX_RANK + 2; i++) {
		g_vTileIds[i] = id;
		id *= mul;
	}
}
