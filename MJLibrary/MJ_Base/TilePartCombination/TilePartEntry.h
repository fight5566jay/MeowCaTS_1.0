#pragma once
#include "TilePartSet.h"

class TilePartEntry
{
public:
	TilePartEntry() {};
	~TilePartEntry() {};

public:
	void setup(const TilePart& oPart, const TilePartEntry& oEntry);
	void setupWithGreedyMerge(const TilePart& oPart, const TilePartEntry& oEntry);
	string toString() const;

private:
	void updateTileNumber(const TilePart& oPart);

public:
	int m_iTileNumber;
	vector<TilePartSet> m_vTilePartSets;
};

