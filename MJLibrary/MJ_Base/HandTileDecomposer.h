#pragma once
//#include "HandTile.h"
#include "PlayerTile.h"
#include <vector>
#include <array>
#include <memory>
using std::array;

class HandTileDecomposer
{
	//static const int g_vTileIndexOfSets[MAX_DIFFERENT_TILE_COUNT + SUIT_COUNT * (MAX_SUIT_TILE_RANK - 2)][3];//Triplet(34) + Sequence(7*3)
	//static const int g_vTileIndexOfPairs[34][2];//Pair(34)
	//static const int g_vTileIndexOfSubSequences[45][2];//SubSeqeunce_TwoSide(8*3) + SubSequence_MiddleHole(7*3)
	static int g_vTileIndexOfMelds[2 * MAX_DIFFERENT_TILE_COUNT + SUIT_COUNT * (3 * MAX_SUIT_TILE_RANK - 5)][3];
	static const int g_iTripletStartIndex;
	static const int g_iSequStartIndex;
	static const int g_iPairStartIndex;
	static const int g_iSubsequTwoSideStartIndex;
	static const int g_iSubsequMiddleHoleStartIndex;
	static const int g_iMeldTableSize;
	static bool m_bFirstInit;

public:
	HandTileDecomposer(const HandTile_t& oHandTile);
	HandTileDecomposer(const vector<int>& vTileCount);
	HandTileDecomposer(const array<int, MAX_DIFFERENT_TILE_COUNT> vTileCount);
	HandTileDecomposer() {};
	~HandTileDecomposer() {};

public:
	vector<array<int, MAX_DIFFERENT_TILE_COUNT>> getAllPossibleWinningPattern(const int& iNeedMeldCount);
	vector<array<int, MAX_DIFFERENT_TILE_COUNT>> getAllPossibleMeldCombination(const int& iNeedMeldCount);

	//only for gtest usage
	inline int getTileCount_gtest(const int& iIndex) const { return m_vTileCount[iIndex]; };

private:
	static void initTileIndexTable();
	void solve_winningPattern(const int& iCurrentDepth, const int& iStartIndex);
	void solve_meld(const int& iCurrentDepth, const int& iStartIndex);
private:
	array<int, MAX_DIFFERENT_TILE_COUNT> m_vTileCount;
	int m_iNeedMeldCount;
	std::unique_ptr<int[]> m_pSetsIndex;
	vector<array<int, MAX_DIFFERENT_TILE_COUNT>> m_vAns;
	//int m_iTotalSetCount;
};

