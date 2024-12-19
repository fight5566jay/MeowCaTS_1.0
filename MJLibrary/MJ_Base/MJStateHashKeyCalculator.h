#pragma once
#include <memory>
#include "MahjongBaseConfig.h"
#include "PlayerTile.h"
#include "../Base/OpenAddressHashTable.h"
typedef HashKey HashKey_t;

using std::array;
class MJStateHashKeyCalculator {
public:
	static void makeTable();
	static HashKey_t calHashKey(const HandTile_t& oHandTile, const bool& bOneMoreChance = false);
	static HashKey_t calHashKey(const PlayerTile& oPlayerTile);
	static HashKey_t calHashKey(const HandTile_t& oHandTile, const int& iRemainDrawCount);
	static HashKey_t calHashKey(const PlayerTile& oPlayerTile, const int& iRemainDrawCount);

protected:
	static HashKey_t getHandTileHashKey(const HandTile_t& oHandTile);

private:
	static array<std::unique_ptr<uint64_t[]>, SUIT_COUNT> g_llZhash_HandTile_Suit;
	static array<std::unique_ptr<uint64_t[]>, HONOR_COUNT> g_llZhash_HandTile_Honor;
	static array<uint64_t, 2> g_llOneMoreChance;//for ExpectimaxTree
	static array<uint64_t, MAX_DRAW_COUNT> g_llRemainDrawCount;
	static uint64_t g_ullSuitTableSize;
	static uint64_t g_ullHonorTableSize;
	static bool g_bActive;
};