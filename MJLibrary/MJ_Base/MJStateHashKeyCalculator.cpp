#include "MJStateHashKeyCalculator.h"
#include <random>

array<std::unique_ptr<uint64_t[]>, SUIT_COUNT> MJStateHashKeyCalculator::g_llZhash_HandTile_Suit;
array<std::unique_ptr<uint64_t[]>, HONOR_COUNT> MJStateHashKeyCalculator::g_llZhash_HandTile_Honor;
array<uint64_t, 2> MJStateHashKeyCalculator::g_llOneMoreChance;
array<uint64_t, MAX_DRAW_COUNT> MJStateHashKeyCalculator::g_llRemainDrawCount;
uint64_t MJStateHashKeyCalculator::g_ullSuitTableSize;
uint64_t MJStateHashKeyCalculator::g_ullHonorTableSize;
bool MJStateHashKeyCalculator::g_bActive = false;

void MJStateHashKeyCalculator::makeTable()
{
	if (!g_bActive) {
		g_ullSuitTableSize = static_cast<uint64_t>(pow(SAME_TILE_COUNT + 1, MAX_SUIT_TILE_RANK));
		g_ullHonorTableSize = static_cast<uint64_t>(pow(SAME_TILE_COUNT + 1, MAX_HONOR_TILE_RANK));

		const int iSuitCount = g_llZhash_HandTile_Suit.size();
		const int iHonorCount = g_llZhash_HandTile_Honor.size();
		for (int i = 0; i < iSuitCount; i++) {
			g_llZhash_HandTile_Suit[i] = std::make_unique<uint64_t[]>(g_ullSuitTableSize);
		}
		for (int i = 0; i < iHonorCount; i++) {
			g_llZhash_HandTile_Honor[i] = std::make_unique<uint64_t[]>(g_ullHonorTableSize);
		}

		std::random_device rd;
		std::default_random_engine gen = std::default_random_engine(rd());
		std::uniform_int_distribution<uint64_t> dis_int(0, INT_MAX);
		auto random_int = bind(dis_int, gen);


		for (int i = 0; i < iSuitCount; i++) {
			for (int j = 0; j < g_ullSuitTableSize; j++) {
				g_llZhash_HandTile_Suit[i][j] = random_int();
			}
		}

		for (int i = 0; i < iHonorCount; i++) {
			for (int j = 0; j < g_ullHonorTableSize; j++) {
				g_llZhash_HandTile_Honor[i][j] = random_int();
			}
		}

		for (int i = 0; i < g_llOneMoreChance.size(); i++) {
			g_llOneMoreChance[i] = random_int();
		}

		for (int i = 0; i < g_llRemainDrawCount.size(); i++) {
			g_llRemainDrawCount[i] = random_int();
		}

		g_bActive = true;
	}
}

HashKey_t MJStateHashKeyCalculator::calHashKey(const HandTile_t& oHandTile, const bool& bOneMoreChance)
{
	if (!g_bActive)
		return 0;

	HashKey_t llHashKey = getHandTileHashKey(oHandTile);

	//one more chance
	if (bOneMoreChance) {
		llHashKey ^= g_llOneMoreChance[0];
	}
	else {
		llHashKey ^= g_llOneMoreChance[1];
	}

	return llHashKey;
}

HashKey_t MJStateHashKeyCalculator::calHashKey(const PlayerTile & oPlayerTile)
{
	return calHashKey(oPlayerTile.getHandTile());
}

HashKey_t MJStateHashKeyCalculator::calHashKey(const HandTile_t & oHandTile, const int & iRemainDrawCount)
{
	assert(iRemainDrawCount >= 0 && iRemainDrawCount <= MAX_DRAW_COUNT);
	if (!g_bActive)
		return 0;
	return getHandTileHashKey(oHandTile) ^ g_llRemainDrawCount[iRemainDrawCount];
}

HashKey_t MJStateHashKeyCalculator::calHashKey(const PlayerTile & oPlayerTile, const int & iRemainDrawCount)
{
	return calHashKey(oPlayerTile.getHandTile(), iRemainDrawCount);
}

HashKey_t MJStateHashKeyCalculator::getHandTileHashKey(const HandTile_t & oHandTile)
{
	HashKey_t llHashKey = 0;
	const auto vHandTileIds = oHandTile.getHandTileId();
	const int iSuitCount = g_llZhash_HandTile_Suit.size();
	const int iHonorCount = g_llZhash_HandTile_Honor.size();

	//suit
	for (int i = 0; i < iSuitCount; i++) {
		assert(vHandTileIds[i] < g_ullSuitTableSize);
		llHashKey ^= (HashKey_t)g_llZhash_HandTile_Suit[i][vHandTileIds[i]];
	}

	//honor
	for (int i = 0, index = iSuitCount; i < iHonorCount; i++, index++) {
		assert(vHandTileIds[index] < g_ullHonorTableSize);
		llHashKey ^= g_llZhash_HandTile_Honor[i][vHandTileIds[index]];
	}

	return llHashKey;
}
