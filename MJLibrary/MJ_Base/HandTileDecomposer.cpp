#include "HandTileDecomposer.h"
#include <cassert>
#include <memory>
#include <iostream>//debug
#include "TilePartCombination/TilePart.h"

int HandTileDecomposer::g_vTileIndexOfMelds[2 * MAX_DIFFERENT_TILE_COUNT + SUIT_COUNT * (3 * MAX_SUIT_TILE_RANK - 5)][3];
const int HandTileDecomposer::g_iTripletStartIndex = 0;
const int HandTileDecomposer::g_iSequStartIndex = MAX_DIFFERENT_TILE_COUNT;
const int HandTileDecomposer::g_iPairStartIndex = g_iSequStartIndex + SUIT_COUNT * (MAX_SUIT_TILE_RANK - 2);
const int HandTileDecomposer::g_iSubsequTwoSideStartIndex = g_iPairStartIndex + MAX_DIFFERENT_TILE_COUNT;
const int HandTileDecomposer::g_iSubsequMiddleHoleStartIndex = g_iSubsequTwoSideStartIndex + SUIT_COUNT * (MAX_SUIT_TILE_RANK - 1);
const int HandTileDecomposer::g_iMeldTableSize = g_iSubsequMiddleHoleStartIndex + SUIT_COUNT * (MAX_SUIT_TILE_RANK - 2);
bool HandTileDecomposer::m_bFirstInit = true;

HandTileDecomposer::HandTileDecomposer(const HandTile_t & oHandTile)
{
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		m_vTileCount[i] = oHandTile.getTileNumber(i);
	}
}

HandTileDecomposer::HandTileDecomposer(const vector<int>& vTileCount)
{
#ifdef MJ_DEBUG_MODE
	assert(vTileCount.size() == MAX_DIFFERENT_TILE_COUNT);
#endif
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		m_vTileCount[i] = vTileCount.at(i);
	}
}

HandTileDecomposer::HandTileDecomposer(const array<int, MAX_DIFFERENT_TILE_COUNT> vTileCount) : m_vTileCount(vTileCount)
{

}

vector<array<int, MAX_DIFFERENT_TILE_COUNT>> HandTileDecomposer::getAllPossibleWinningPattern(const int & iNeedMeldCount)
{
	m_vAns.clear();
	m_iNeedMeldCount = iNeedMeldCount;
	m_pSetsIndex = std::make_unique<int[]>(iNeedMeldCount);
	solve_winningPattern(0, 0);
	return m_vAns;
}

vector<array<int, MAX_DIFFERENT_TILE_COUNT>> HandTileDecomposer::getAllPossibleMeldCombination(const int& iNeedMeldCount)
{
	m_vAns.clear();
	m_iNeedMeldCount = iNeedMeldCount;
	m_pSetsIndex = std::make_unique<int[]>(iNeedMeldCount);
	solve_meld(0, 0);
	return m_vAns;
}

void HandTileDecomposer::initTileIndexTable()
{
	//triplet
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		g_vTileIndexOfMelds[i][0] = i;
		g_vTileIndexOfMelds[i][1] = i;
		g_vTileIndexOfMelds[i][2] = i;
	}
	//sequence
	for (int i = 0; i < SUIT_COUNT; i++) {
		int startIndex = i * MAX_SUIT_TILE_RANK;
		int endIndex = startIndex + MAX_SUIT_TILE_RANK - 2;
		for (int index = startIndex; index < endIndex; index++) {
			g_vTileIndexOfMelds[g_iSequStartIndex + index][0] = i;
			g_vTileIndexOfMelds[g_iSequStartIndex + index][1] = i + 1;
			g_vTileIndexOfMelds[g_iSequStartIndex + index][2] = i + 2;
		}
	}
	//pair
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		g_vTileIndexOfMelds[g_iPairStartIndex + i][0] = i;
		g_vTileIndexOfMelds[g_iPairStartIndex + i][1] = i;
		g_vTileIndexOfMelds[g_iPairStartIndex + i][2] = -1;
	}

	//subsequence_twoSide
	for (int i = 0; i < SUIT_COUNT; i++) {
		int startIndex = i * MAX_SUIT_TILE_RANK;
		int endIndex = startIndex + MAX_SUIT_TILE_RANK - 1;
		for (int index = startIndex; index < endIndex; index++) {
			g_vTileIndexOfMelds[g_iSubsequTwoSideStartIndex + index][0] = i;
			g_vTileIndexOfMelds[g_iSubsequTwoSideStartIndex + index][1] = i + 1;
			g_vTileIndexOfMelds[g_iSubsequTwoSideStartIndex + index][2] = -1;
		}
	}

	//subsequence_middleHole
	for (int i = 0; i < SUIT_COUNT; i++) {
		int startIndex = i * MAX_SUIT_TILE_RANK;
		int endIndex = startIndex + MAX_SUIT_TILE_RANK - 2;
		for (int index = startIndex; index < endIndex; index++) {
			g_vTileIndexOfMelds[g_iSubsequMiddleHoleStartIndex + index][0] = i;
			g_vTileIndexOfMelds[g_iSubsequMiddleHoleStartIndex + index][1] = i + 2;
			g_vTileIndexOfMelds[g_iSubsequMiddleHoleStartIndex + index][2] = -1;
		}
	}

}

void HandTileDecomposer::solve_winningPattern(const int & iCurrentDepth, const int & iStartIndex)
{
	if (iCurrentDepth == 0) {
		//find pair
		for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
			if (m_vTileCount[i] >= 2) {
				m_vTileCount[i] -= 2;
				m_pSetsIndex[0] = i;
				solve_winningPattern(1, 0);
				m_vTileCount[i] += 2;
			}
		}
		return;
	}

	if (iCurrentDepth == m_iNeedMeldCount) {
		//get enough sets
		//generate answer and put into ans array
		array<int, MAX_DIFFERENT_TILE_COUNT> vAnsTileCount;
		vAnsTileCount.fill(0);

		vAnsTileCount[m_pSetsIndex[0]] += 2;
		//std::cerr << "{ " << TilePart(Type_Pair, Tile(m_pSetsIndex[0]).getTileType(), Tile(m_pSetsIndex[0]).getRank()).toString() << " }";
		for (int i = 1; i < m_iNeedMeldCount; i++) {
			for (int j = 0; j < 3; j++) {
				int iTileIndex = g_vTileIndexOfMelds[m_pSetsIndex[i]][j];
				vAnsTileCount[iTileIndex]++;
			}

			//debug
			//std::cerr << ", { ";
			//Tile oMinTile(g_vTileIndexOfMelds[m_pSetsIndex[i]][0]);
			//if (m_pSetsIndex[i] < 34) {
			//	std::cerr << TilePart(Type_Triplet, oMinTile.getTileType(), oMinTile.getRank()).toString();
			//}
			//if (m_pSetsIndex[i] >= 34 && m_pSetsIndex[i] < 55) {
			//	std::cerr << TilePart(Type_Sequence, oMinTile.getTileType(), oMinTile.getRank()).toString();
			//}
			//std::cerr << " }";
		}
		//std::cerr << std::endl;

		m_vAns.push_back(vAnsTileCount);
		return;
	}

	//triplet
	for (int i = iStartIndex; i < g_iSequStartIndex; i++) {
		if (m_vTileCount[i] >= 3) {
			m_vTileCount[i] -= 3;
			m_pSetsIndex[iCurrentDepth] = i;
			solve_winningPattern(iCurrentDepth + 1, i);
			m_vTileCount[i] += 3;
		}
	}

	//sequence
	for (int i = std::max(g_iSequStartIndex, iStartIndex); i < g_iPairStartIndex; i++) {
		if (m_vTileCount[g_vTileIndexOfMelds[i][0]] >= 1 
			&& m_vTileCount[g_vTileIndexOfMelds[i][1]] >= 1 
			&& m_vTileCount[g_vTileIndexOfMelds[i][2]] >= 1) {
			m_vTileCount[g_vTileIndexOfMelds[i][0]]--;
			m_vTileCount[g_vTileIndexOfMelds[i][1]]--;
			m_vTileCount[g_vTileIndexOfMelds[i][2]]--;
			m_pSetsIndex[iCurrentDepth] = i;
			solve_winningPattern(iCurrentDepth + 1, i);
			m_vTileCount[g_vTileIndexOfMelds[i][0]]++;
			m_vTileCount[g_vTileIndexOfMelds[i][1]]++;
			m_vTileCount[g_vTileIndexOfMelds[i][2]]++;
		}
	}
}

void HandTileDecomposer::solve_meld(const int& iCurrentDepth, const int& iStartIndex)
{
	if (iCurrentDepth == m_iNeedMeldCount) {
		//get enough melds
		//generate answer and put into ans array
		array<int, MAX_DIFFERENT_TILE_COUNT> vAnsTileCount;
		vAnsTileCount.fill(0);

		for (int i = 0; i < m_iNeedMeldCount; i++) {
			for (int j = 0; j < 3; j++) {
				int iTileIndex = g_vTileIndexOfMelds[m_pSetsIndex[i]][j];
				if (iTileIndex != -1) {
					vAnsTileCount[iTileIndex]++;
				}
			}

			//debug
			if (i > 0) {
				std::cerr << ", ";
			}
			std::cerr << "{ ";
			Tile oMinTile(g_vTileIndexOfMelds[m_pSetsIndex[i]][0]);
			if (m_pSetsIndex[i] < g_iSequStartIndex) {
				std::cerr << TilePart(TilePartType::Type_Triplet, oMinTile.getTileType(), oMinTile.getRank()).toString();
			}
			if (m_pSetsIndex[i] >= g_iSequStartIndex && m_pSetsIndex[i] <g_iPairStartIndex) {
				std::cerr << TilePart(TilePartType::Type_Sequence, oMinTile.getTileType(), oMinTile.getRank()).toString();
			}
			if (m_pSetsIndex[i] >= g_iPairStartIndex && m_pSetsIndex[i] < g_iSubsequTwoSideStartIndex) {
				std::cerr << TilePart(TilePartType::Type_Pair, oMinTile.getTileType(), oMinTile.getRank()).toString();
			}
			if (m_pSetsIndex[i] >= g_iSubsequTwoSideStartIndex && m_pSetsIndex[i] < g_iSubsequMiddleHoleStartIndex) {
				std::cerr << TilePart(TilePartType::Type_SubSequence_TwoSide, oMinTile.getTileType(), oMinTile.getRank()).toString();
			}
			if (m_pSetsIndex[i] >= g_iSubsequMiddleHoleStartIndex) {
				std::cerr << TilePart(TilePartType::Type_SubSequence_MiddleHole, oMinTile.getTileType(), oMinTile.getRank()).toString();
			}
			std::cerr << " }";

			
		}
		std::cerr << std::endl;
		m_vAns.push_back(vAnsTileCount);
		return;
	}

	for (int i = iStartIndex; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		if (m_vTileCount[i] == 0)
			continue;

		bool bHasTriplet = false;
		if (m_vTileCount[i] >= 3) {//triplet
			//assert(m_vTileCount[i] == m_vTileCount[g_vTileIndexOfMelds[i][0]]);
			m_vTileCount[i] -= 3;
			m_pSetsIndex[iCurrentDepth] = i;
			solve_meld(iCurrentDepth + 1, i);
			m_vTileCount[i] += 3;
			bHasTriplet = true;
		}
		else if (m_vTileCount[i] >= 2) {//pair
			//assert(m_vTileCount[i] == m_vTileCount[g_vTileIndexOfMelds[55 + i][0]]);
			m_vTileCount[i] -= 2;
			m_pSetsIndex[iCurrentDepth] = g_iPairStartIndex + i;
			solve_meld(iCurrentDepth + 1, i);
			m_vTileCount[i] += 2;
		}

		bool bIsHonor = i >= DIFF_SUIT_TILE_COUNT;
		bool bSkipSequence = bHasTriplet && m_vTileCount[i] == 3;//need verification
		if (bIsHonor || bSkipSequence)
			continue;

		Tile oTile(i);
		int iTileType = oTile.getTileType();
		int iTileRank = oTile.getRank();
		if (iTileRank <= MAX_RANK - 2 && m_vTileCount[i + 1] > 0 && m_vTileCount[i + 2] > 0) {//sequence
			//assert(m_vTileCount[i + 1] == m_vTileCount[g_vTileIndexOfMelds[34 + iTileType * 7 + iTileNumber - 1][1]] && m_vTileCount[i + 2] == m_vTileCount[g_vTileIndexOfMelds[34 + iTileType * 7 + iTileNumber - 1][2]]);
			m_vTileCount[i]--;
			m_vTileCount[i + 1]--;
			m_vTileCount[i + 2]--;
			m_pSetsIndex[iCurrentDepth] = g_iSequStartIndex + iTileType * (MAX_SUIT_TILE_RANK - 2) + iTileRank - 1;
			solve_meld(iCurrentDepth + 1, i);
			m_vTileCount[i]++;
			m_vTileCount[i + 1]++;
			m_vTileCount[i + 2]++;
		}
		else if (iTileRank <= MAX_RANK - 1 && m_vTileCount[i + 1] > 0) {//subSequence_TwoSide
			//assert(m_vTileCount[i + 1] == m_vTileCount[g_vTileIndexOfMelds[89 + iTileType * 8 + iTileNumber - 1][1]]);
			m_vTileCount[i]--;
			m_vTileCount[i + 1]--;
			m_pSetsIndex[iCurrentDepth] = g_iSubsequTwoSideStartIndex + iTileType * (MAX_SUIT_TILE_RANK - 1) + iTileRank - 1;
			solve_meld(iCurrentDepth + 1, i);
			m_vTileCount[i]++;
			m_vTileCount[i + 1]++;
		}
		else if (iTileRank <= MAX_RANK - 2 && m_vTileCount[i + 2] > 0) {//subSequence_MiddleHole
			//assert(m_vTileCount[i + 2] == m_vTileCount[g_vTileIndexOfMelds[113 + iTileType * 7 + iTileNumber - 1][1]]);
			m_vTileCount[i]--;
			m_vTileCount[i + 2]--;
			m_pSetsIndex[iCurrentDepth] = g_iSubsequMiddleHoleStartIndex + iTileType * (MAX_SUIT_TILE_RANK - 2) + iTileRank - 1;
			solve_meld(iCurrentDepth + 1, i);
			m_vTileCount[i]++;
			m_vTileCount[i + 2]++;
		}
	}

	
}

/*const int HandTileDecomposer::g_vTileIndexOfSets[MAX_DIFFERENT_TILE_COUNT + SUIT_COUNT * (MAX_SUIT_TILE_RANK - 2)][3] =
{
	{0, 0, 0},	{1, 1, 1},	{2, 2, 2},	{3, 3, 3},	{4, 4, 4},	{5, 5, 5},	{6, 6, 6},	{7, 7, 7},	{8, 8, 8},
	{9, 9, 9},	{10, 10, 10},	{11, 11, 11},	{12, 12, 12},	{13, 13, 13},	{14, 14, 14},	{15, 15, 15},	{16, 16, 16},	{17, 17, 17},
	{18, 18, 18},	{19, 19, 19},	{20, 20, 20},	{21, 21, 21},	{22, 22, 22},	{23, 23, 23},	{24, 24, 24},	{25, 25, 25},	{26, 26, 26},
	{27, 27, 27},	{28, 28, 28},	{29, 29, 29},	{30, 30, 30},	{31, 31, 31},	{32, 32, 32},	{33, 33, 33},
	{0, 1, 2},	{1, 2, 3},	{2, 3, 4},	{3, 4, 5},	{4, 5, 6},	{5, 6, 7},	{6, 7, 8},
	{9, 10, 11},	{10, 11, 12},	{11, 12, 13},	{12, 13, 14},	{13, 14, 15},	{14, 15, 16},	{15, 16, 17},
	{18, 19, 20},	{19, 20, 21},	{20, 21, 22},	{21, 22, 23},	{22, 23, 24},	{23, 24, 25},	{24, 25, 26}
};*/

/*const int HandTileDecomposer::g_vTileIndexOfPairs[34][2] = 
{
	{0, 0},	{1, 1},	{2, 2},	{3, 3},	{4, 4},	{5, 5},	{6, 6},	{7, 7},	{8, 8},
	{9, 9},	{10, 10},	{11, 11},	{12, 12},	{13, 13},	{14, 14},	{15, 15},	{16, 16},	{17, 17},
	{18, 18},	{19, 19},	{20, 20},	{21, 21},	{22, 22},	{23, 23},	{24, 24},	{25, 25},	{26, 26},
	{27, 27},	{28, 28},	{29, 29},	{30, 30},	{31, 31},	{32, 32},	{33, 33}
};*/

/*const int HandTileDecomposer::g_vTileIndexOfSubSequences[45][2] =
{
	{0, 1},	{1, 2},	{2, 3},	{3, 4},	{4, 5},	{5, 6},	{6, 7},	{7, 8},
	{9, 10},	{10, 11},	{11, 12},	{12, 13},	{13, 14},	{14, 15},	{15, 16},	{17, 18},
	{18, 19},	{19, 20},	{20, 21},	{21, 22},	{22, 23},	{23, 24},	{24, 25}, {25, 26},
	{0, 2},	{1, 3},	{2, 4},	{3, 5},	{4, 6},	{5, 7},	{6, 8},
	{9, 11},	{10, 12},	{11, 13},	{12, 14},	{13, 15},	{14, 16},	{15, 17},
	{18, 20},	{19, 21},	{20, 22},	{21, 23},	{22, 24},	{23, 25},	{24, 26}
};*/