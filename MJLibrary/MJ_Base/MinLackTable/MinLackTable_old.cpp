#include "MinLackTable_old.h"
//#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#ifdef LINUX
#include "Tools.h"
#else
#include "..\..\Base\Tools.h"
#endif



using std::string;
using std::ifstream;
using std::ofstream;

MinLackEntry MinLackTable_old::g_eSuit[SUIT_TABLE_SIZE];
MinLackEntry MinLackTable_old::g_eHonor[HONOR_TABLE_SIZE];
MJGameType2 MinLackTable_old::g_mjGameType;
bool MinLackTable_old::g_bFirstSetup = true;

TileGroup_t MinLackTable_old::allTile[9] = { 1, 5, 25, 125, 625, 3125, 15625, 78125, 390625 };
TileGroup_t MinLackTable_old::allMeld[16] = { 31, 155, 775, 3875, 19375, 96875, 484375, 3, 15, 75, 375, 1875, 9375, 46875, 234375, 1171875 };
TileGroup_t MinLackTable_old::allPair[9] = { 2, 10, 50, 250, 1250, 6250, 31250, 156250, 781250 };
TileGroup_t MinLackTable_old::allSequ[15] = { 6, 30, 150, 750, 3750, 18750, 93750, 468750, 26, 130, 650, 3250, 16250, 81250, 406250 };

string MinLackTable_old::m_sTablePath;
int MinLackTable_old::iMaxHandTileNumber;

void MinLackTable_old::makeTable(const MJGameType2& mjGameType)
{
	if (!g_bFirstSetup)
		return;

	g_bFirstSetup = false;
	g_mjGameType = mjGameType;
	if (g_mjGameType == MJGameType2::MJGameType_16) {
		m_sTablePath = getCurrentPath() + "MinLackTable_old(Sampling)";
		iMaxHandTileNumber = 17;
	}
	else if (g_mjGameType == MJGameType2::MJGameType_Jpn) {
		m_sTablePath = getCurrentPath() + "MinLackTable_old(Jpn)";
		iMaxHandTileNumber = 14;
	}
	
	TileGroup::g_bFirstInit = true;

	ifstream existFd(m_sTablePath);
	if (existFd.is_open()) {
		existFd.close();
		CERR("[MinLackTable_old::makeTable] " + m_sTablePath + " was found. Load from the built file.\n");
		loadTable();
		return;
	} else
		existFd.close();

	CERR("[MinLackTable_old::makeTable] " + m_sTablePath + " was not found. Start making MinLackTable.\n");
	//TileGroup_t remainTiles;
	//std::cout << "[MinLackTable_old::makeTable] start setHonorEntry." << std::endl;
	for (TileGroup_t honorTiles = 0; honorTiles < HONOR_TABLE_SIZE; ++honorTiles) {
		if (honorTiles.getTileNumber() > iMaxHandTileNumber)
			continue;
		setHonorEntry(honorTiles, g_eHonor[honorTiles]);
		//if((int)honorTiles % 1000 == 0){
			//std::cerr << "[MinLackTable_old::makeTable] honorTiles=" << (int)honorTiles << " done." << std::endl;
		//}
	}
	
	//std::cout << "[MinLackTable_old::makeTable] start setSuitEntry." << std::endl;
	for (TileGroup_t suitTiles = 0; suitTiles < SUIT_TABLE_SIZE; ++suitTiles) {
		if (suitTiles.getTileNumber() > iMaxHandTileNumber)
			continue;
		setSuitEntry(suitTiles, g_eSuit[suitTiles]);
		//if((int)suitTiles % 1000 == 0){
			//std::cerr << "[MinLackTable_old::makeTable] suitTiles=" << (int)suitTiles << " done." << std::endl;
		//}
	}

	CERR("[MinLackTable_old::makeTable] Start saving table to " + m_sTablePath + " .\n");
	saveTable();
	CERR("[MinLackTable_old::makeTable] The table was saved to " + m_sTablePath + " .\n");
}

void MinLackTable_old::saveTable()
{
	ofstream outputFd;
	outputFd.open(m_sTablePath, std::ios::out | std::ios::binary);
	for (TileGroup_t honorTiles = 0; honorTiles < HONOR_TABLE_SIZE; ++honorTiles) {
		if (honorTiles.getTileNumber() <= iMaxHandTileNumber)
			g_eHonor[honorTiles].outputEntry(outputFd);
	}
	for (TileGroup_t suitTiles = 0; suitTiles < SUIT_TABLE_SIZE; ++suitTiles) {
		if (suitTiles.getTileNumber() <= iMaxHandTileNumber)
			g_eSuit[suitTiles].outputEntry(outputFd);
	}
	outputFd.close();
}

void MinLackTable_old::loadTable()
{
	ifstream inputFd;
	inputFd.open(m_sTablePath, std::ios::in | std::ios::binary);
	for (TileGroup_t honorTiles = 0; honorTiles < HONOR_TABLE_SIZE; ++honorTiles) {
		if (honorTiles.getTileNumber() <= iMaxHandTileNumber)
			g_eHonor[honorTiles].inputEntry(inputFd);
	}
	for (TileGroup_t suitTiles = 0; suitTiles < SUIT_TABLE_SIZE; ++suitTiles) {
		if (suitTiles.getTileNumber() <= iMaxHandTileNumber)
			g_eSuit[suitTiles].inputEntry(inputFd);
	}
	inputFd.close();
}

int MinLackTable_old::pickOutMeld(TileGroup_t &tiles, const int &startPickOutIndex)
{
	TileGroup_t sourceTiles = tiles;
	TileGroup_t remainTiles;
	TileGroup_t _remainTiles;
	int maxMeld = 0;
	int bestPair = 0;
	int bestSequ = 0;
	int bestLack = 0;
	int meldNumber;
	int pairNumber;
	int sequNumber;
	int lackNumber;

	for (int i = startPickOutIndex; i < 16; i++) {
		if (sourceTiles.canPickOut(allMeld[i])) {
			remainTiles = sourceTiles;
			remainTiles.pickOut(allMeld[i]);
			meldNumber = pickOutMeld(remainTiles, i) + 1;

			_remainTiles = remainTiles;
			
			pairNumber = g_eSuit[remainTiles].m_iPair;
			sequNumber = g_eSuit[remainTiles].m_iSequ;
			lackNumber = pairNumber + sequNumber;

			if (maxMeld < meldNumber || (maxMeld == meldNumber && bestLack < lackNumber) || (maxMeld == meldNumber && bestLack == lackNumber && bestPair < pairNumber)) {
				maxMeld = meldNumber;
				bestPair = pairNumber;
				bestSequ = sequNumber;
				bestLack = lackNumber;
				tiles = _remainTiles;
			}
		}
	}

	return maxMeld;
}

int MinLackTable_old::pickOutPair(TileGroup_t &tiles, const int &startPickOutIndex)
{
	TileGroup_t sourceTiles = tiles;
	TileGroup_t remainTiles = tiles;
	TileGroup_t _remainTiles;
	int bestPair = 0;
	int bestSequ = pickOutSequ(remainTiles, 0);
	int bestLack = bestSequ;
	int pairNumber;
	int sequNumber;
	int lackNumber;

	for (int i = startPickOutIndex; i < 9; i++) {
		if (sourceTiles.canPickOut(allPair[i])) {
			remainTiles = sourceTiles;
			remainTiles.pickOut(allPair[i]);
			pairNumber = pickOutPair(remainTiles, i) + 1;

			_remainTiles = remainTiles;

			sequNumber = g_eSuit[remainTiles].m_iSequ;
			lackNumber = pairNumber + sequNumber;

			if (bestLack < lackNumber || (bestLack == lackNumber && bestPair < pairNumber)) {
				bestPair = pairNumber;
				bestSequ = sequNumber;
				bestLack = lackNumber;
				tiles = _remainTiles;
			}
		}
	}

	return bestPair;
}

int MinLackTable_old::pickOutSequ(TileGroup_t &tiles, const int &startPickOutIndex)
{
	TileGroup_t sourceTiles = tiles;
	TileGroup_t remainTiles;
	int maxSequNumber = 0;
	int sequNumber;

	for (int i = startPickOutIndex; i < 15; i++) {
		if (sourceTiles.canPickOut(allSequ[i])) {
			remainTiles = sourceTiles;
			remainTiles.pickOut(allSequ[i]);
			sequNumber = pickOutSequ(remainTiles, i) + 1;

			if (maxSequNumber < sequNumber) {
				maxSequNumber = sequNumber;
				tiles = remainTiles;
			}
		}
	}

	return maxSequNumber;
}

int MinLackTable_old::pickOutAllPair(TileGroup_t &tiles)
{
	int result = 0;
	for (int i = 0; i < 9; i++) {
		while(tiles.canPickOut(allPair[i])) {
			tiles.pickOut(allPair[i]);
			result += 1;
		}
	}
	return result;
}

bool MinLackTable_old::hasCostPair(TileGroup_t tiles, const int &meldNumber, const int &pairNumber, const int &sequNumber, const int &startPickOutIndex)
{
	if (meldNumber == 0) {
		bool condition1 = pickOutAllPair(tiles) > pairNumber;
		bool condition2 = pickOutSequ(tiles, 0) < sequNumber;
		return condition1 && condition2;
	}
	
	TileGroup_t remainTiles = tiles;
	for (int i = startPickOutIndex; i < 16; i++) {
		if (tiles.canPickOut(allMeld[i])) {
			remainTiles = tiles;
			remainTiles.pickOut(allMeld[i]);
			if (hasCostPair(remainTiles, meldNumber - 1, pairNumber, sequNumber, i))
				return true;
		}
	}

	return false;
}

MinLackEntry MinLackTable_old::findSecondChoice(TileGroup_t &tiles)
{
	MinLackEntry result;
	TileGroup_t remainTiles;
	int meldNumber;
	int pairNumber;
	int sequNumber;
	int lackNumber;

	for (int i = 0; i < 9; i++) {
		if (tiles.canPickOut(allPair[i])) {
			remainTiles = tiles;
			remainTiles.pickOut(allPair[i]);

			meldNumber = g_eSuit[remainTiles].m_iMeld;
			pairNumber = g_eSuit[remainTiles].m_iPair + 1;
			sequNumber = g_eSuit[remainTiles].m_iSequ;
			lackNumber = pairNumber + sequNumber;

			if (g_eSuit[tiles].m_iMeld - 1 <= meldNumber && g_eSuit[tiles].m_iPair + g_eSuit[tiles].m_iSequ + 3 <= lackNumber) {
				result.m_iMeld = meldNumber;
				result.m_iPair = pairNumber;
				result.m_iSequ = sequNumber;
				return result;
			}
		}
	}

	for (int i = 0; i < 15; i++) {
		if (tiles.canPickOut(allSequ[i])) {
			remainTiles = tiles;
			remainTiles.pickOut(allSequ[i]);

			meldNumber = g_eSuit[remainTiles].m_iMeld;
			pairNumber = g_eSuit[remainTiles].m_iPair;
			sequNumber = g_eSuit[remainTiles].m_iSequ + 1;
			lackNumber = pairNumber + sequNumber;

			if (g_eSuit[tiles].m_iMeld - 1 <= meldNumber && g_eSuit[tiles].m_iPair + g_eSuit[tiles].m_iSequ + 3 <= lackNumber) {
				result.m_iMeld = meldNumber;
				result.m_iPair = pairNumber;
				result.m_iSequ = sequNumber;
				return result;
			}
		}
	}

	result.m_iMeld = -1;
	return result;
}

int MinLackTable_old::getMinLackNumber(const int *pHandeTileID)
{
	return getMinLackNumber(pHandeTileID, getNeedMeldNumber(pHandeTileID));
}

int MinLackTable_old::getMinLackNumber(const int *pHandeTileID, int iNeedGroup, int iNeedEye)
{
	if (g_mjGameType == MJGameType2::MJGameType_16)
		return getNormalMinLackNumber(pHandeTileID, iNeedGroup, iNeedEye);
	else if (g_mjGameType == MJGameType2::MJGameType_Jpn) {
		int minLack = getNormalMinLackNumber(pHandeTileID, iNeedGroup, iNeedEye);
		int tileNumber = getHandTileNumber(pHandeTileID);

		if (tileNumber == 13 || tileNumber == 14) {
			int sevenPairMinLack = getSevenPairMinLackNumber(pHandeTileID, iNeedGroup, iNeedEye);
			int goshimusoMinLack = getGoshimusoMinLackNumber(pHandeTileID, iNeedGroup, iNeedEye);

			if (sevenPairMinLack < minLack)
				minLack = sevenPairMinLack;
			if (goshimusoMinLack < minLack)
				minLack = goshimusoMinLack;
		}
		return minLack;
	} else
		return -1;
}

int MinLackTable_old::getNormalMinLackNumber(const int *pHandeTileID, int iNeedGroup, int iNeedEye)
{
	MinLackEntry *tiles[3];
	int minLack;
	int totalMeldNumber = iNeedGroup;
	int needMeldNumber;
	int hasMeldNumber = g_eHonor[pHandeTileID[3]].m_iMeld;
	int pairNumber = g_eHonor[pHandeTileID[3]].m_iPair;
	int incompleteSequNumber = 0;
	int lackTileToPairNumber;
	int lackOneNumber;
	int lackTwoNumber;

	for (int i = 0; i < 3; i++) {
		tiles[i] = &g_eSuit[pHandeTileID[i]];
		hasMeldNumber += tiles[i]->m_iMeld;
		pairNumber += tiles[i]->m_iPair;
		incompleteSequNumber += tiles[i]->m_iSequ;
	}
	needMeldNumber = totalMeldNumber - hasMeldNumber;

	if (incompleteSequNumber - 2 >= needMeldNumber) {
		for (int i = 0; i < 3; i++) {
			if (tiles[i]->hasCostPair()) {
				pairNumber += 1;
				incompleteSequNumber -= 2;
				break;
			}
		}
	}

	for (int i = 0; needMeldNumber > incompleteSequNumber + pairNumber && i < 3; i++) {
		if (tiles[i]->hasSecondChoice()) {
			hasMeldNumber += tiles[i]->m_eSecond->m_iMeld - tiles[i]->m_iMeld;
			pairNumber += tiles[i]->m_eSecond->m_iPair - tiles[i]->m_iPair;
			incompleteSequNumber += tiles[i]->m_eSecond->m_iSequ - tiles[i]->m_iSequ;
			needMeldNumber = totalMeldNumber - hasMeldNumber;
		}
	}

	lackTileToPairNumber = pairNumber > 0 ? 0 : 1;

	lackOneNumber = std::max(0, std::min(needMeldNumber, incompleteSequNumber + std::max(0, pairNumber - 1)));
	lackTwoNumber = std::max(0, needMeldNumber - lackOneNumber);

	minLack = lackOneNumber + 2 * lackTwoNumber + lackTileToPairNumber;

	return minLack;
}

int MinLackTable_old::getSevenPairMinLackNumber(const int *pHandeTileID, int iNeedGroup, int iNeedEye)
{
	int sevenPairMinLack;
	int sevenPairLackPairNumber = 7 - g_eHonor[pHandeTileID[3]].m_iLargerOne;
	int aloneNumber = g_eHonor[pHandeTileID[3]].m_iAlone;

	for (int i = 0; i < 3; i++) {
		sevenPairLackPairNumber -= g_eSuit[pHandeTileID[i]].m_iLargerOne;
		aloneNumber += g_eSuit[pHandeTileID[i]].m_iAlone;
	}

	sevenPairMinLack = aloneNumber < sevenPairLackPairNumber ? 2 * sevenPairLackPairNumber - aloneNumber : sevenPairLackPairNumber;
	return sevenPairMinLack;
}

int MinLackTable_old::getGoshimusoMinLackNumber(const int *pHandeTileID, int iNeedGroup, int iNeedEye)
{
	int goshimusoMinLack;
	int goshimusoPart = g_eHonor[pHandeTileID[3]].m_iGoshimusoPart;
	bool goshimusoPair = g_eHonor[pHandeTileID[3]].m_bGoshimusoPair;

	for (int i = 0; i < 3; i++) {
		goshimusoPart += g_eSuit[pHandeTileID[i]].m_iGoshimusoPart;
		goshimusoPair |= g_eSuit[pHandeTileID[i]].m_bGoshimusoPair;
	}

	goshimusoMinLack = 13 - goshimusoPart;
	if (!goshimusoPair)
		goshimusoMinLack += 1;
	return goshimusoMinLack;
}

int MinLackTable_old::getHandTileNumber(const int *pHandeTileID)
{
	int tileNumber = g_eHonor[pHandeTileID[3]].m_iTileNumber;
	for (int i = 0; i < 3; i++)
		tileNumber += g_eSuit[pHandeTileID[i]].m_iTileNumber;
	return tileNumber;
}

int MinLackTable_old::getNeedMeldNumber(const int *pHandeTileID)
{
	return getHandTileNumber(pHandeTileID) / 3;
}

int MinLackTable_old::getMeldNumber(const int *pHandeTileID)
{
	MinLackEntry *tiles[3];
	int totalMeldNumber = getNeedMeldNumber(pHandeTileID);
	int needMeldNumber;
	int hasMeldNumber = g_eHonor[pHandeTileID[3]].m_iMeld;
	int pairNumber = g_eHonor[pHandeTileID[3]].m_iPair;
	int incompleteSequNumber = 0;

	for (int i = 0; i < 3; i++) {
		tiles[i] = &g_eSuit[pHandeTileID[i]];
		hasMeldNumber += tiles[i]->m_iMeld;
		pairNumber += tiles[i]->m_iPair;
		incompleteSequNumber += tiles[i]->m_iSequ;

	}
	needMeldNumber = totalMeldNumber - hasMeldNumber;

	if (incompleteSequNumber - 2 >= needMeldNumber) {
		for (int i = 0; i < 3; i++) {
			if (tiles[i]->hasCostPair()) {
				pairNumber += 1;
				incompleteSequNumber -= 2;
				break;
			}
		}
	}

	for (int i = 0; needMeldNumber > incompleteSequNumber + pairNumber && i < 3; i++) {
		if (tiles[i]->hasSecondChoice()) {
			hasMeldNumber += tiles[i]->m_eSecond->m_iMeld - tiles[i]->m_iMeld;
			pairNumber += tiles[i]->m_eSecond->m_iPair - tiles[i]->m_iPair;
			incompleteSequNumber += tiles[i]->m_eSecond->m_iSequ - tiles[i]->m_iSequ;
			needMeldNumber = totalMeldNumber - hasMeldNumber;
		}
	}

	return hasMeldNumber;
}

int MinLackTable_old::getPairAndIncompleteSequNumber(const int *pHandeTileID)
{
	MinLackEntry *tiles[3];
	int totalMeldNumber = getNeedMeldNumber(pHandeTileID);
	int needMeldNumber;
	int hasMeldNumber = g_eHonor[pHandeTileID[3]].m_iMeld;
	int pairNumber = g_eHonor[pHandeTileID[3]].m_iPair;
	int incompleteSequNumber = 0;

	for (int i = 0; i < 3; i++) {
		tiles[i] = &g_eSuit[pHandeTileID[i]];
		hasMeldNumber += tiles[i]->m_iMeld;
		pairNumber += tiles[i]->m_iPair;
		incompleteSequNumber += tiles[i]->m_iSequ;
	}
	needMeldNumber = totalMeldNumber - hasMeldNumber;

	if (incompleteSequNumber - 2 >= needMeldNumber) {
		for (int i = 0; i < 3; i++) {
			if (tiles[i]->hasCostPair()) {
				pairNumber += 1;
				incompleteSequNumber -= 2;
				break;
			}
		}
	}

	for (int i = 0; needMeldNumber > incompleteSequNumber + pairNumber && i < 3; i++) {
		if (tiles[i]->hasSecondChoice()) {
			hasMeldNumber += tiles[i]->m_eSecond->m_iMeld - tiles[i]->m_iMeld;
			pairNumber += tiles[i]->m_eSecond->m_iPair - tiles[i]->m_iPair;
			incompleteSequNumber += tiles[i]->m_eSecond->m_iSequ - tiles[i]->m_iSequ;
			needMeldNumber = totalMeldNumber - hasMeldNumber;
		}
	}

	return pairNumber + incompleteSequNumber;
}

bool MinLackTable_old::hasLeftTile(const TileGroup_t & tiles, const int &index)
{
	return index >= 0 && index < 9 && tiles[index] < '4';
}

void MinLackTable_old::setSevenPair(TileGroup_t &tiles, MinLackEntry &entry)
{
	if (g_mjGameType == MJGameType2::MJGameType_Jpn) {
		for (int i = 0; i < 9; i++) {
			if (tiles[i] >= '2')
				entry.m_iLargerOne += 1;
			else if (tiles[i] == '1')
				entry.m_iAlone += 1;
		}
	}
}

void MinLackTable_old::setSuitUsefulBits(TileGroup_t &tiles, MinLackEntry &entry)
{
	for (int i = 0; i < 9; i++) {
		if (tiles[i] > '0') {
			if (hasLeftTile(tiles, i))
				entry.m_iUsefulBits |= (0x01 << i);
			if (hasLeftTile(tiles, i - 2))
				entry.m_iUsefulBits |= (0x01 << (i - 2));
			if (hasLeftTile(tiles, i - 1))
				entry.m_iUsefulBits |= (0x01 << (i - 1));
			if (hasLeftTile(tiles, i + 1))
				entry.m_iUsefulBits |= (0x01 << (i + 1));
			if (hasLeftTile(tiles, i + 2))
				entry.m_iUsefulBits |= (0x01 << (i + 2));
		}
		if (g_mjGameType == MJGameType2::MJGameType_Jpn) {
			if (tiles[i] == '1')
				entry.m_iSevenPairUsefulBits |= (0x01 << i);
		}
	}
}

void MinLackTable_old::setHonorEntry(TileGroup_t &tiles, MinLackEntry &entry)
{
	for (int i = 0; i < 7; i++) {
		if (tiles[i] >= '3')
			entry.m_iMeld += 1;
		else if (tiles[i] == '2')
			entry.m_iPair += 1;
	}
	for (int i = 0; i < 7; i++) {
		if (tiles[i] >= '1' && tiles[i] < '4')
			entry.m_iUsefulBits |= (0x01 << i);

		if (g_mjGameType == MJGameType2::MJGameType_Jpn) {
			if (tiles[i] == '0' || (entry.m_iPair == 0 && entry.m_iMeld == 0))
				entry.m_iGoshimusoUsefulBits |= (0x01 << i);
			if (tiles[i] == '1')
				entry.m_iSevenPairUsefulBits |= (0x01 << i);
			if (tiles[i] > '0')
				entry.m_iGoshimusoPart += 1;
			if (tiles[i] >= '2')
				entry.m_bGoshimusoPair = true;
		}
	}
	entry.m_iTileNumber = tiles.getTileNumber();
	setSevenPair(tiles, entry);
}

void MinLackTable_old::setSuitEntry(TileGroup_t &tiles, MinLackEntry &entry)
{
	setSuitUsefulBits(tiles, entry);

	TileGroup_t remainTiles = tiles;
	entry.m_iMeld = pickOutMeld(remainTiles, 0);

	if (entry.m_iMeld > 0) {
		entry.m_iPair = g_eSuit[remainTiles].m_iPair;
		entry.m_iSequ = g_eSuit[remainTiles].m_iSequ;
	} else {
		entry.m_iPair = pickOutPair(remainTiles, 0);
		entry.m_iSequ = pickOutSequ(remainTiles, 0);
	}
	entry.m_bCostPair = hasCostPair(tiles, entry.m_iMeld, entry.m_iPair, entry.m_iSequ, 0);

	entry.m_iTileNumber = tiles.getTileNumber();
	setSevenPair(tiles, entry);

	if (tiles.hasPair()) {
		MinLackEntry secondEntry = findSecondChoice(tiles);
		if (secondEntry.m_iMeld >= 0) {
			secondEntry.m_bCostPair = hasCostPair(tiles, secondEntry.m_iMeld, secondEntry.m_iPair, secondEntry.m_iSequ, 0);
			secondEntry.m_iLargerOne = entry.m_iLargerOne;
			secondEntry.m_iAlone = entry.m_iAlone;
			if (g_mjGameType == MJGameType2::MJGameType_Jpn) {
				secondEntry.m_iSevenPairUsefulBits = entry.m_iSevenPairUsefulBits;
				secondEntry.m_iGoshimusoPart = entry.m_iGoshimusoPart;
				secondEntry.m_bGoshimusoPair = entry.m_bGoshimusoPair;
			}
			entry.setSecondChoice(secondEntry);
		}
	}

	if (g_mjGameType == MJGameType2::MJGameType_Jpn) {
		if (tiles[0] > '0')
			entry.m_iGoshimusoPart += 1;
		if (tiles[8] > '0')
			entry.m_iGoshimusoPart += 1;
		if (tiles[0] >= '2' || tiles[8] >= '2')
			entry.m_bGoshimusoPair = true;
		if (tiles[0] == '0' || (tiles[0] == '1' && tiles[8] < '2'))
			entry.m_iGoshimusoUsefulBits |= (0x01 << 0);
		if (tiles[8] == '0' || (tiles[8] == '1' && tiles[0] < '2'))
			entry.m_iGoshimusoUsefulBits |= (0x01 << 8);
	}
}