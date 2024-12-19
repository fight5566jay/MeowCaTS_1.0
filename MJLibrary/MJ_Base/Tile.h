#pragma once
#include <string>
#include <cassert>
#include "MahjongBaseConfig.h"
#include "../Base/Tools.h"
using std::string;

class Tile {
public:
	Tile();
	Tile(const int& iTileIndex);
	Tile(const int& tileType, const int& rank);
	Tile(const string& sTile);

	int getTileType() const;
	int getRank() const;
	inline bool isNull() const { return m_iIndex < 0; };
	inline bool isHonor() const { return m_iIndex >= DIFF_SUIT_TILE_COUNT; };
	inline bool isSuit() const { return m_iIndex >= 0 && m_iIndex < DIFF_SUIT_TILE_COUNT; };
	string toString() const;
	static int getId(const int& iRank) { assert(iRank > 0 && iRank <= MAX_RANK + 1); return g_vTileIds[iRank]; };
	static int getDivId() { return g_vTileIds[2]; };
	inline int getSgfId() const { return (getTileType() + 1) * PROTOCOL_SYMBOL_PLACE + getRank() * PROTOCOL_NUMBER_PLACE; };
	char getTileTypeStr() const;

	inline void operator=(const int& rhs) { m_iIndex = rhs; };
	inline bool operator==(const int& rhs) const { return m_iIndex == rhs; };
	inline bool operator!=(const int& rhs) const { return !(*this == rhs); };
	inline Tile operator+(const int& rhs) const { return Tile(m_iIndex + rhs); };
	inline Tile operator-(const int& rhs) const { return Tile(m_iIndex - rhs); };
	inline operator int() const { return m_iIndex; };

	inline static Tile sgfIdToTile(const string& sTile) { return sgfIdToTile(toInt(sTile)); };
	static Tile sgfIdToTile(const int& iTile);

	static void setupTileIds();

private:
	void _init();

private:
	static bool g_bFirstSetup;
	static int g_vTileIds[MAX_RANK + 2];

protected:
	int m_iIndex;
};