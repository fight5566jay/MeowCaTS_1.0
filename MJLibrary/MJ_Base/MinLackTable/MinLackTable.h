#pragma once
#include <cmath>
#include <string>
#include <memory>
#include "MinLackEntry.h"
#include "TileGroup.h"
typedef std::unique_ptr<MinLackEntry[]> MinLackEntryArray;
using std::string;
enum MJGameType { MJGameType_16, MJGameType_Jpn };

class MinLackTable
{
public:
	static void makeTable(/*const MJGameType& mjGameType*/);
	static int getMinLackNumber(const int pHandeTileID[]);
	static int getMinLackNumber(const int pHandeTileID[], int iNeedGroup, int iNeedEye = 1);
	static int getNormalMinLackNumber(const int pHandeTileID[], int iNeedGroup, int iNeedEye);
	static int getSevenPairMinLackNumber(const int pHandeTileID[], int iNeedGroup, int iNeedEye);
	static int getGoshimusoMinLackNumber(const int pHandeTileID[], int iNeedGroup, int iNeedEye);
	static int getHandTileNumber(const int pHandeTileID[]);
	static int getNeedMeldNumber(const int pHandeTileID[]);
	static int getMeldNumber(const int pHandeTileID[]);
	static int getPairAndIncompleteSequNumber(const int pHandeTileID[]);

private:
	static int pickOutMeld(TileGroup &tiles, const int &startPickOutIndex);
	static int pickOutPair(TileGroup &tiles, const int &startPickOutIndex);
	static int pickOutSequ(TileGroup &tiles, const int &startPickOutIndex);
	static int pickOutAllPair(TileGroup &tiles);
	static bool hasCostPair(TileGroup tiles, const int &meldNumber, const int &pairNumber, const int &sequNumber, const int &startPickOutIndex);
	static bool hasLeftTile(const TileGroup & tiles, const int &index);
	static MinLackEntry findSecondChoice(TileGroup &tiles);
	static void setSevenPair(TileGroup &tiles, MinLackEntry &entry);
	static void setSuitUsefulBits(TileGroup &tiles, MinLackEntry &entry);
	static void setHonorEntry(TileGroup &tiles, MinLackEntry &entry);
	static void setSuitEntry(TileGroup &tiles, MinLackEntry &entry);
	static void saveTable();
	static void loadTable();

private:
	static uint64_t g_ullSuitTableSize;
	static uint64_t g_ullHonorTableSize;
	static MinLackEntryArray g_vSuits;
	static MinLackEntryArray g_vHonors;
	static bool g_bFirstInit;
	//static MJGameType g_mjGameType;

	static string g_sTablePath;
};