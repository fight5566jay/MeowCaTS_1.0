#include "MinLackTable.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include "../../Base/Tools.h"
using std::stringstream;
using std::cout;
using std::cerr;
using std::endl;

uint64_t MinLackTable::g_ullSuitTableSize;
uint64_t MinLackTable::g_ullHonorTableSize;
MinLackEntryArray MinLackTable::g_vSuits;
MinLackEntryArray MinLackTable::g_vHonors;
bool MinLackTable::g_bFirstInit = true;
//MJGameType MinLackTable::g_mjGameType;
string MinLackTable::g_sTablePath;

void MinLackTable::makeTable(/*const MJGameType& mjGameType*/)
{
	if (!g_bFirstInit)
		return;

	g_bFirstInit = false;
	g_ullSuitTableSize = static_cast<uint64_t>(pow(SAME_TILE_COUNT + 1, MAX_SUIT_TILE_RANK));
	g_ullHonorTableSize = static_cast<uint64_t>(pow(SAME_TILE_COUNT + 1, MAX_HONOR_TILE_RANK));
	g_vSuits = std::make_unique<MinLackEntry[]>(g_ullSuitTableSize);
	g_vHonors = std::make_unique<MinLackEntry[]>(g_ullHonorTableSize);

	stringstream ss;
	ss << getCurrentPath() << "MinlackTable("
		<< NEED_GROUP
		<< "," << SAME_TILE_COUNT
		<< "," << MAX_SUIT_TILE_RANK
		<< "," << MAX_HONOR_TILE_RANK
		<< ")";
	g_sTablePath = ss.str();
	std::cerr << "sTablePath = " << g_sTablePath << std::endl;

	ofstream existFd;
	existFd.open(g_sTablePath, std::ios::in);
	if (existFd.is_open()) {
		existFd.close();
		loadTable();
		return;
	} else
		existFd.close();

	TileGroup remainTiles;

	for (TileGroup honorTiles = 0; honorTiles < g_ullHonorTableSize; ++honorTiles) {
		if (honorTiles.getTileNumber() > MAX_HANDTILE_COUNT)
			continue;
		setHonorEntry(honorTiles, g_vHonors[honorTiles]);
	}

	for (TileGroup suitTiles = 0; suitTiles < g_ullSuitTableSize; ++suitTiles) {
		if (suitTiles.getTileNumber() > MAX_HANDTILE_COUNT)
			continue;
		setSuitEntry(suitTiles, g_vSuits[suitTiles]);
	}

	saveTable();
	std::cout << "[MinLackTable::makeTable] make table finished." << std::endl;
}

void MinLackTable::saveTable()
{
	ofstream outputFd;
	outputFd.open(g_sTablePath, std::ios::out | std::ios::binary);
	for (TileGroup honorTiles = 0; honorTiles < g_ullHonorTableSize; ++honorTiles) {
		if (honorTiles.getTileNumber() <= MAX_HANDTILE_COUNT)
			g_vHonors[honorTiles].outputEntry(outputFd);
	}
	for (TileGroup suitTiles = 0; suitTiles < g_ullSuitTableSize; ++suitTiles) {
		if (suitTiles.getTileNumber() <= MAX_HANDTILE_COUNT)
			g_vSuits[suitTiles].outputEntry(outputFd);
	}
	outputFd.close();
	CERR("[MinLackTable::saveTable] MinLackTable is saved.\n");
}

void MinLackTable::loadTable()
{
	ifstream inputFd;
	inputFd.open(g_sTablePath, std::ios::in | std::ios::binary);
	if (!inputFd.is_open()) {
		std::cerr << "cannot open the table: " << g_sTablePath << std::endl;
	}

	for (TileGroup honorTiles = 0; honorTiles < g_ullHonorTableSize; ++honorTiles) {
		if (honorTiles.getTileNumber() <= MAX_HANDTILE_COUNT)
			g_vHonors[honorTiles].inputEntry(inputFd);
	}
	for (TileGroup suitTiles = 0; suitTiles < g_ullSuitTableSize; ++suitTiles) {
		if (suitTiles.getTileNumber() <= MAX_HANDTILE_COUNT)
			g_vSuits[suitTiles].inputEntry(inputFd);
	}
	inputFd.close();
}

int MinLackTable::pickOutMeld(TileGroup &tiles, const int &startPickOutIndex)
{
	TileGroup sourceTiles = tiles;
	TileGroup remainTiles;
	TileGroup _remainTiles;
	int maxMeld = 0;
	int bestPair = 0;
	int bestSequ = 0;
	int bestLack = 0;
	int meldNumber;
	int pairNumber;
	int sequNumber;
	int lackNumber;

	for (int i = startPickOutIndex; i < TileGroup::g_allMeld.size(); i++) {
		if (sourceTiles.canPickOut(TileGroup::g_allMeld[i])) {
			remainTiles = sourceTiles;
			remainTiles.pickOut(TileGroup::g_allMeld[i]);
			meldNumber = pickOutMeld(remainTiles, i) + 1;

			_remainTiles = remainTiles;
			
			pairNumber = g_vSuits[remainTiles].m_iPair;
			sequNumber = g_vSuits[remainTiles].m_iSequ;
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

int MinLackTable::pickOutPair(TileGroup &tiles, const int &startPickOutIndex)
{
	TileGroup sourceTiles = tiles;
	TileGroup remainTiles = tiles;
	TileGroup _remainTiles;
	int bestPair = 0;
	int bestSequ = pickOutSequ(remainTiles, 0);
	int bestLack = bestSequ;
	int pairNumber;
	int sequNumber;
	int lackNumber;

	for (int i = startPickOutIndex; i < TileGroup::g_allPair.size(); i++) {
		if (sourceTiles.canPickOut(TileGroup::g_allPair[i])) {
			remainTiles = sourceTiles;
			remainTiles.pickOut(TileGroup::g_allPair[i]);
			pairNumber = pickOutPair(remainTiles, i) + 1;

			_remainTiles = remainTiles;

			sequNumber = g_vSuits[remainTiles].m_iSequ;
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

int MinLackTable::pickOutSequ(TileGroup &tiles, const int &startPickOutIndex)
{
	TileGroup sourceTiles = tiles;
	TileGroup remainTiles;
	int maxSequNumber = 0;
	int sequNumber;

	for (int i = startPickOutIndex; i < TileGroup::g_allSubsequ.size(); i++) {
		if (sourceTiles.canPickOut(TileGroup::g_allSubsequ[i])) {
			remainTiles = sourceTiles;
			remainTiles.pickOut(TileGroup::g_allSubsequ[i]);
			sequNumber = pickOutSequ(remainTiles, i) + 1;

			if (maxSequNumber < sequNumber) {
				maxSequNumber = sequNumber;
				tiles = remainTiles;
			}
		}
	}

	return maxSequNumber;
}

int MinLackTable::pickOutAllPair(TileGroup &tiles)
{
	int result = 0;
	for (int i = 0; i < TileGroup::g_allPair.size(); i++) {
		while(tiles.canPickOut(TileGroup::g_allPair[i])) {
			tiles.pickOut(TileGroup::g_allPair[i]);
			result += 1;
		}
	}
	return result;
}

bool MinLackTable::hasCostPair(TileGroup tiles, const int &meldNumber, const int &pairNumber, const int &sequNumber, const int &startPickOutIndex)
{
	if (meldNumber == 0) {
		bool condition1 = pickOutAllPair(tiles) > pairNumber;
		bool condition2 = pickOutSequ(tiles, 0) < sequNumber;
		return condition1 && condition2;
	}
	
	TileGroup remainTiles = tiles;
	for (int i = startPickOutIndex; i < TileGroup::g_allMeld.size(); i++) {
		if (tiles.canPickOut(TileGroup::g_allMeld[i])) {
			remainTiles = tiles;
			remainTiles.pickOut(TileGroup::g_allMeld[i]);
			if (hasCostPair(remainTiles, meldNumber - 1, pairNumber, sequNumber, i))
				return true;
		}
	}

	return false;
}

MinLackEntry MinLackTable::findSecondChoice(TileGroup &tiles)
{
	MinLackEntry result;
	TileGroup remainTiles;
	int meldNumber;
	int pairNumber;
	int sequNumber;
	int lackNumber;

	for (int i = 0; i < TileGroup::g_allPair.size(); i++) {
		if (tiles.canPickOut(TileGroup::g_allPair[i])) {
			remainTiles = tiles;
			remainTiles.pickOut(TileGroup::g_allPair[i]);

			meldNumber = g_vSuits[remainTiles].m_iMeld;
			pairNumber = g_vSuits[remainTiles].m_iPair + 1;
			sequNumber = g_vSuits[remainTiles].m_iSequ;
			lackNumber = pairNumber + sequNumber;

			if (g_vSuits[tiles].m_iMeld - 1 <= meldNumber && g_vSuits[tiles].m_iPair + g_vSuits[tiles].m_iSequ + 3 <= lackNumber) {
				result.m_iMeld = meldNumber;
				result.m_iPair = pairNumber;
				result.m_iSequ = sequNumber;
				return result;
			}
		}
	}

	for (int i = 0; i < TileGroup::g_allSubsequ.size(); i++) {
		if (tiles.canPickOut(TileGroup::g_allSubsequ[i])) {
			remainTiles = tiles;
			remainTiles.pickOut(TileGroup::g_allSubsequ[i]);

			meldNumber = g_vSuits[remainTiles].m_iMeld;
			pairNumber = g_vSuits[remainTiles].m_iPair;
			sequNumber = g_vSuits[remainTiles].m_iSequ + 1;
			lackNumber = pairNumber + sequNumber;

			if (g_vSuits[tiles].m_iMeld - 1 <= meldNumber && g_vSuits[tiles].m_iPair + g_vSuits[tiles].m_iSequ + 3 <= lackNumber) {
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

int MinLackTable::getMinLackNumber(const int pHandeTileID[])
{
	return getMinLackNumber(pHandeTileID, getNeedMeldNumber(pHandeTileID));
}

int MinLackTable::getMinLackNumber(const int pHandeTileID[], int iNeedGroup, int iNeedEye)
{
	return getNormalMinLackNumber(pHandeTileID, iNeedGroup, iNeedEye);
	/*
	if (g_mjGameType == MJGameType::MJGameType_16)
		return getNormalMinLackNumber(pHandeTileID, iNeedGroup, iNeedEye);
	else if (g_mjGameType == MJGameType::MJGameType_Jpn) {
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
	*/
}

int MinLackTable::getNormalMinLackNumber(const int pHandeTileID[], int iNeedGroup, int iNeedEye)
{
	array<MinLackEntry*, SUIT_COUNT> suits;
	int minLack;
	int totalMeldNumber = iNeedGroup;
	int needMeldNumber;
	//int hasMeldNumber = g_vHonors[pHandeTileID[3]].m_iMeld;
	//int pairNumber = g_vHonors[pHandeTileID[3]].m_iPair;
	int hasMeldNumber = 0;
	int pairNumber = 0;
	int incompleteSequNumber = 0;
	int lackTileToPairNumber;
	int lackOneNumber;
	int lackTwoNumber;

	for (int i = 0; i < SUIT_COUNT; i++) {
		suits[i] = &g_vSuits[pHandeTileID[i]];
		hasMeldNumber += suits[i]->m_iMeld;
		pairNumber += suits[i]->m_iPair;
		incompleteSequNumber += suits[i]->m_iSequ;
	}
	needMeldNumber = totalMeldNumber - hasMeldNumber;

	if (incompleteSequNumber - 2 >= needMeldNumber) {
		for (int i = 0; i < SUIT_COUNT; i++) {
			if (suits[i]->hasCostPair()) {
				pairNumber += 1;
				incompleteSequNumber -= 2;
				break;
			}
		}
	}

	for (int i = 0; needMeldNumber > incompleteSequNumber + pairNumber && i < SUIT_COUNT; i++) {
		if (suits[i]->hasSecondChoice()) {
			hasMeldNumber += suits[i]->m_eSecond->m_iMeld - suits[i]->m_iMeld;
			pairNumber += suits[i]->m_eSecond->m_iPair - suits[i]->m_iPair;
			incompleteSequNumber += suits[i]->m_eSecond->m_iSequ - suits[i]->m_iSequ;
			needMeldNumber = totalMeldNumber - hasMeldNumber;
		}
	}

	lackTileToPairNumber = pairNumber > 0 ? 0 : 1;

	lackOneNumber = std::max(0, std::min(needMeldNumber, incompleteSequNumber + std::max(0, pairNumber - 1)));
	lackTwoNumber = std::max(0, needMeldNumber - lackOneNumber);

	minLack = lackOneNumber + 2 * lackTwoNumber + lackTileToPairNumber;

	return minLack;
}

int MinLackTable::getSevenPairMinLackNumber(const int pHandeTileID[], int iNeedGroup, int iNeedEye)
{
	//not general mahjong
	int sevenPairMinLack;
	int sevenPairLackPairNumber = 7 - g_vHonors[pHandeTileID[3]].m_iLargerOne;
	int aloneNumber = g_vHonors[pHandeTileID[3]].m_iAlone;

	for (int i = 0; i < 3; i++) {
		sevenPairLackPairNumber -= g_vSuits[pHandeTileID[i]].m_iLargerOne;
		aloneNumber += g_vSuits[pHandeTileID[i]].m_iAlone;
	}

	sevenPairMinLack = aloneNumber < sevenPairLackPairNumber ? 2 * sevenPairLackPairNumber - aloneNumber : sevenPairLackPairNumber;
	return sevenPairMinLack;
}

int MinLackTable::getGoshimusoMinLackNumber(const int pHandeTileID[], int iNeedGroup, int iNeedEye)
{
	//not general mahjong
	int goshimusoMinLack;
	int goshimusoPart = g_vHonors[pHandeTileID[3]].m_iGoshimusoPart;
	bool goshimusoPair = g_vHonors[pHandeTileID[3]].m_bGoshimusoPair;

	for (int i = 0; i < 3; i++) {
		goshimusoPart += g_vSuits[pHandeTileID[i]].m_iGoshimusoPart;
		goshimusoPair |= g_vSuits[pHandeTileID[i]].m_bGoshimusoPair;
	}

	goshimusoMinLack = 13 - goshimusoPart;
	if (!goshimusoPair)
		goshimusoMinLack += 1;
	return goshimusoMinLack;
}

int MinLackTable::getHandTileNumber(const int pHandeTileID[])
{
	int tileNumber = 0;
	for (int i = 0; i < SUIT_COUNT; i++)
		tileNumber += g_vSuits[pHandeTileID[i]].m_iTileNumber;
	for (int i = 0; i < HONOR_COUNT; i++)
		tileNumber += g_vHonors[pHandeTileID[SUIT_COUNT + i]].m_iTileNumber;
	return tileNumber;
}

int MinLackTable::getNeedMeldNumber(const int pHandeTileID[])
{
	return getHandTileNumber(pHandeTileID) / 3;
}

int MinLackTable::getMeldNumber(const int pHandeTileID[])
{
	//[Caution] Not general yet.
	MinLackEntry *suitTiles, *honorTiles;
	int totalMeldNumber = getNeedMeldNumber(pHandeTileID);
	int needMeldNumber;
	int hasMeldNumber =0;
	int pairNumber = 0;
	int incompleteSequNumber = 0;

	for (int i = 0; i < SUIT_COUNT; i++) {
		suitTiles = &g_vSuits[pHandeTileID[i]];
		hasMeldNumber += suitTiles->m_iMeld;
		pairNumber += suitTiles->m_iPair;
		incompleteSequNumber += suitTiles->m_iSequ;
	}
	for (int i = 0; i < HONOR_COUNT; i++) {
		honorTiles = &g_vHonors[pHandeTileID[SUIT_COUNT + i]];
		hasMeldNumber += honorTiles->m_iMeld;
		pairNumber += honorTiles->m_iPair;
	}
	needMeldNumber = totalMeldNumber - hasMeldNumber;

	if (incompleteSequNumber - 2 >= needMeldNumber) {
		for (int i = 0; i < SUIT_COUNT; i++) {
			suitTiles = &g_vSuits[pHandeTileID[i]];
			if (suitTiles->hasCostPair()) {
				pairNumber += 1;
				incompleteSequNumber -= 2;
				break;
			}
		}
	}

	for (int i = 0; needMeldNumber > incompleteSequNumber + pairNumber && i < SUIT_COUNT; i++) {
		suitTiles = &g_vSuits[pHandeTileID[i]];
		if (suitTiles->hasSecondChoice()) {
			hasMeldNumber += suitTiles->m_eSecond->m_iMeld - suitTiles->m_iMeld;
			pairNumber += suitTiles->m_eSecond->m_iPair - suitTiles->m_iPair;
			incompleteSequNumber += suitTiles->m_eSecond->m_iSequ - suitTiles->m_iSequ;
			needMeldNumber = totalMeldNumber - hasMeldNumber;
		}
	}

	return hasMeldNumber;
}

int MinLackTable::getPairAndIncompleteSequNumber(const int pHandeTileID[])
{
	//[Caution] Not general yet.
	MinLackEntry *suitTiles, *honorTiles;
	int totalMeldNumber = getNeedMeldNumber(pHandeTileID);
	int needMeldNumber;
	int hasMeldNumber = g_vHonors[pHandeTileID[3]].m_iMeld;
	int pairNumber = g_vHonors[pHandeTileID[3]].m_iPair;
	int incompleteSequNumber = 0;

	for (int i = 0; i < SUIT_COUNT; i++) {
		suitTiles = &g_vSuits[pHandeTileID[i]];
		hasMeldNumber += suitTiles->m_iMeld;
		pairNumber += suitTiles->m_iPair;
		incompleteSequNumber += suitTiles->m_iSequ;
	}
	for (int i = 0; i < HONOR_COUNT; i++) {
		honorTiles = &g_vHonors[pHandeTileID[SUIT_COUNT + i]];
		hasMeldNumber += honorTiles->m_iMeld;
		pairNumber += honorTiles->m_iPair;
	}
	needMeldNumber = totalMeldNumber - hasMeldNumber;

	if (incompleteSequNumber - 2 >= needMeldNumber) {
		for (int i = 0; i < SUIT_COUNT; i++) {
			suitTiles = &g_vSuits[pHandeTileID[i]];
			if (suitTiles->hasCostPair()) {
				pairNumber += 1;
				incompleteSequNumber -= 2;
				break;
			}
		}
	}

	for (int i = 0; needMeldNumber > incompleteSequNumber + pairNumber && i < SUIT_COUNT; i++) {
		suitTiles = &g_vSuits[pHandeTileID[i]];
		if (suitTiles->hasSecondChoice()) {
			hasMeldNumber += suitTiles->m_eSecond->m_iMeld - suitTiles->m_iMeld;
			pairNumber += suitTiles->m_eSecond->m_iPair - suitTiles->m_iPair;
			incompleteSequNumber += suitTiles->m_eSecond->m_iSequ - suitTiles->m_iSequ;
			needMeldNumber = totalMeldNumber - hasMeldNumber;
		}
	}

	return pairNumber + incompleteSequNumber;
}

bool MinLackTable::hasLeftTile(const TileGroup & tiles, const int &index)
{
	return index >= 0 && index < MAX_RANK && tiles[index] < (SAME_TILE_COUNT + '0');
}

void MinLackTable::setSevenPair(TileGroup &tiles, MinLackEntry &entry)
{
	/*if (g_mjGameType == MJGameType::MJGameType_Jpn) {
		for (int i = 0; i < MAX_RANK; i++) {
			if (tiles[i] >= '2')
				entry.m_iLargerOne += 1;
			else if (tiles[i] == '1')
				entry.m_iAlone += 1;
		}
	}*/
}

void MinLackTable::setSuitUsefulBits(TileGroup &tiles, MinLackEntry &entry)
{
	for (int i = 0; i < MAX_SUIT_TILE_RANK; i++) {
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
		/*if (g_mjGameType == MJGameType::MJGameType_Jpn) {
			if (tiles[i] == '1')
				entry.m_iSevenPairUsefulBits |= (0x01 << i);
		}*/
	}
}

void MinLackTable::setHonorEntry(TileGroup &tiles, MinLackEntry &entry)
{
	for (int i = 0; i < MAX_HONOR_TILE_RANK; i++) {
		if (tiles[i] >= '3')
			entry.m_iMeld += 1;
		else if (tiles[i] == '2')
			entry.m_iPair += 1;
	}
	for (int i = 0; i < MAX_HONOR_TILE_RANK; i++) {
		if (tiles[i] >= '1' && tiles[i] < (SAME_TILE_COUNT + '0'))
			entry.m_iUsefulBits |= (0x01 << i);

		/*if (g_mjGameType == MJGameType::MJGameType_Jpn) {
			if (tiles[i] == '0' || (entry.m_iPair == 0 && entry.m_iMeld == 0))
				entry.m_iGoshimusoUsefulBits |= (0x01 << i);
			if (tiles[i] == '1')
				entry.m_iSevenPairUsefulBits |= (0x01 << i);
			if (tiles[i] > '0')
				entry.m_iGoshimusoPart += 1;
			if (tiles[i] >= '2')
				entry.m_bGoshimusoPair = true;
		}*/
	}
	entry.m_iTileNumber = tiles.getTileNumber();
	setSevenPair(tiles, entry);
}

void MinLackTable::setSuitEntry(TileGroup &tiles, MinLackEntry &entry)
{
	setSuitUsefulBits(tiles, entry);

	TileGroup remainTiles = tiles;
	entry.m_iMeld = pickOutMeld(remainTiles, 0);

	if (entry.m_iMeld > 0) {
		entry.m_iPair = g_vSuits[remainTiles].m_iPair;
		entry.m_iSequ = g_vSuits[remainTiles].m_iSequ;
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
			/*if (g_mjGameType == MJGameType::MJGameType_Jpn) {
				secondEntry.m_iSevenPairUsefulBits = entry.m_iSevenPairUsefulBits;
				secondEntry.m_iGoshimusoPart = entry.m_iGoshimusoPart;
				secondEntry.m_bGoshimusoPair = entry.m_bGoshimusoPair;
			}*/
			entry.setSecondChoice(secondEntry);
		}
	}

	/*if (g_mjGameType == MJGameType::MJGameType_Jpn) {
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
	}*/
}