#ifndef __MIN_LACK_TABLE__
#define __MIN_LACK_TABLE__

#define SUIT_TABLE_SIZE 1953125
#define HONOR_TABLE_SIZE 78125

#include "MinLackEntry.h"
#include "TileGroup.h"
typedef TileGroup TileGroup_t;

enum class MJGameType2 { MJGameType_16, MJGameType_Jpn };

class MinLackTable_old
{
public:
	static void makeTable(const MJGameType2& mjGameType);
	static int getMinLackNumber(const int *pHandeTileID);
	static int getMinLackNumber(const int *pHandeTileID, int iNeedGroup, int iNeedEye = 1);
	static int getNormalMinLackNumber(const int *pHandeTileID, int iNeedGroup, int iNeedEye);
	static int getSevenPairMinLackNumber(const int *pHandeTileID, int iNeedGroup, int iNeedEye);
	static int getGoshimusoMinLackNumber(const int *pHandeTileID, int iNeedGroup, int iNeedEye);
	static int getHandTileNumber(const int *pHandeTileID);
	static int getNeedMeldNumber(const int *pHandeTileID);
	static int getMeldNumber(const int *pHandeTileID);
	static int getPairAndIncompleteSequNumber(const int *pHandeTileID);

private:
	static int pickOutMeld(TileGroup_t &tiles, const int &startPickOutIndex);
	static int pickOutPair(TileGroup_t &tiles, const int &startPickOutIndex);
	static int pickOutSequ(TileGroup_t &tiles, const int &startPickOutIndex);
	static int pickOutAllPair(TileGroup_t &tiles);
	static bool hasCostPair(TileGroup_t tiles, const int &meldNumber, const int &pairNumber, const int &sequNumber, const int &startPickOutIndex);
	static bool hasLeftTile(const TileGroup_t & tiles, const int &index);
	static MinLackEntry findSecondChoice(TileGroup_t &tiles);
	static void setSevenPair(TileGroup_t &tiles, MinLackEntry &entry);
	static void setSuitUsefulBits(TileGroup_t &tiles, MinLackEntry &entry);
	static void setHonorEntry(TileGroup_t &tiles, MinLackEntry &entry);
	static void setSuitEntry(TileGroup_t &tiles, MinLackEntry &entry);
	static void saveTable();
	static void loadTable();

public:
	static MinLackEntry g_eSuit[SUIT_TABLE_SIZE];
	static MinLackEntry g_eHonor[HONOR_TABLE_SIZE];
	static MJGameType2 g_mjGameType;
	static bool g_bFirstSetup;
	static TileGroup_t allTile[9];
	static TileGroup_t allMeld[16];
	static TileGroup_t allPair[9];
	static TileGroup_t allSequ[15];

	static string m_sTablePath;
	static int iMaxHandTileNumber;
};
#endif