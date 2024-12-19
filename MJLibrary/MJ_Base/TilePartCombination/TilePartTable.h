#ifndef __Mahjong__MeldTable__
#define __Mahjong__MeldTable__

#define SUIT_TABLE_SIZE 1953125
#define HONOR_TABLE_SIZE 78125

#include <vector>
#include <memory>
#include "../MinLackTable/TileGroup.h"
#include "TilePartEntry.h"
#include "TilePartCombination.h"

typedef std::unique_ptr<TilePartEntry[]> TilePartEntryArray;

class TilePartTable
{
public:
	static void makeTable();
	static vector<TilePartCombination> getTilePartSetList(const int* id);

private:
	static void makeHonorTable();
	static void makeSuitTable();
	//static void setGoshimuso();
	//static bool isSevenPairs(const int *id);
	static vector<TilePartSet> pickOutMeldAndPair(TileGroup &tiles, TilePartSet &buildingSet, const int& startIndex);
	static vector<TilePartSet> pickOutTripletAndPair(TileGroup &tiles, TilePartSet &buildingSet, const int& startIndex);
	static TilePart getMeldByIndex(const int& index);

	static void updateHonorEntry(TileGroup& tiles);
	static void updateSuitEntry(TileGroup& tiles);
	static void updateAddSuitSequence(TileGroup& tiles);
	static void updateAddSuitPair(TileGroup& tiles);
	static void updateAddSuitSubSequence(TileGroup& tiles);
	static void updateAddSuitAloneTile(TileGroup& tiles);

	static bool readTable();
	static void writeTable();

public:
	static TilePartEntryArray g_vSuit;
	static TilePartEntryArray g_vHonor;

private:
	static bool m_bFirstInit;
	static uint64_t m_ullSuitTableSize;
	static uint64_t m_ullHonorTableSize;
};
#endif