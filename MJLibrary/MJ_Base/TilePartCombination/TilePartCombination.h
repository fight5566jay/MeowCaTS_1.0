#pragma once

#include <string>
#include <array>
#include "TilePartSet.h"
#include "../MeldTile.h"
#include <bitset>
using std::string;
using std::array;
using std::bitset;

class TilePartCombination
{
public:
	TilePartCombination() {};
	//TilePartCombination(const TilePartSet& w, const TilePartSet& t, const TilePartSet& s, const TilePartSet& z);
	TilePartCombination(const array<TilePartSet, TILE_TYPE_COUNT> sets) : m_vTliePartSets(sets) {};
	~TilePartCombination() {};

public:
	inline void addTilePart(const TilePart& oTilePart, const int& oTileType) { m_vTliePartSets[oTileType].addToTilePart(oTilePart); };
	inline void addMeldTile(const MeldTile& oMeldTile) { addTilePart(TilePart(oMeldTile), oMeldTile.getMinRankTile().getTileType()); };
	inline TilePartSet at(const int& iIndex) const { return m_vTliePartSets[iIndex]; };
	int getMinLack(const int& iMeldCount) const;
	bitset<MAX_DIFFERENT_TILE_COUNT> getUsefulTiles(const int& iNeedMeldCount, const bool& bNeedEyes) const;

	string toString() const;
	int getTilePartCount() const;

private:
	array<TilePartSet, TILE_TYPE_COUNT> m_vTliePartSets;

public:
	TilePartCombination operator+(const TilePartCombination& rhs) const;
	TilePartSet& operator[](const int& index);
};