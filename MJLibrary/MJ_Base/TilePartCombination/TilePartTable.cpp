#include "TilePartTable.h"
#include <iostream>
#include <deque>
#include <fstream>
#include <cassert>

using std::deque;
using std::fstream;
using std::cerr;
using std::endl;

//TileGroup allTriplet[9] = { 3, 15, 75, 375, 1875, 9375, 46875, 234375, 1171875 };

bool TilePartTable::m_bFirstInit = true;
TilePartEntryArray TilePartTable::g_vSuit;
TilePartEntryArray TilePartTable::g_vHonor;

void TilePartTable::makeTable()
{
	if (!m_bFirstInit)
		return;

	if (readTable()) {
		return;
	}

	m_ullSuitTableSize = pow(SAME_TILE_COUNT + 1, MAX_SUIT_TILE_RANK);
	m_ullHonorTableSize = pow(SAME_TILE_COUNT + 1, MAX_HONOR_TILE_RANK);
	g_vSuit = std::make_unique<TilePartEntry[]>(m_ullSuitTableSize);
	g_vHonor = std::make_unique<TilePartEntry[]>(m_ullHonorTableSize);

	makeSuitTable();
	makeHonorTable();
	//setGoshimuso();//only used in Jpn Mahjong
	writeTable();
	m_bFirstInit = false;
}

vector<TilePartCombination> TilePartTable::getTilePartSetList(const int* id)
{
	vector<TilePartCombination> result;

	/*
	//only used in Jpn Mahjong
	if (isSevenPairs(id)) {
		TilePartCombination element;
		element[0] = g_vSuit[id[0]][0].getAllPairTilePart();
		element[1] = g_vSuit[id[1]][0].getAllPairTilePart();
		element[2] = g_vSuit[id[2]][0].getAllPairTilePart();
		element[3] = g_vSuit[id[3]][0].getAllPairTilePart();
		result.push_back(element);

		if (!(g_vSuit[id[0]][0].isLegal() && g_vSuit[id[1]][0].isLegal() && g_vSuit[id[2]][0].isLegal() && g_vHonor[id[3]][0].isLegal()))
			return result;
	}*/

	//[NOT DEBUG YET]
	array<TilePartSet, TILE_TYPE_COUNT> vSets;
	array<int, TILE_TYPE_COUNT> vCurrentIndex = { 0 };
	//init
	for (int i = 0; i < TILE_TYPE_COUNT; i++) {
		vSets[i] = g_vSuit[id[i]].m_vTilePartSets.at(0);
	}

	bool bFinished = false;
	while (!bFinished) {
		//gen combination
		result.push_back(TilePartCombination(vSets));
		//find next indexs
		for (int i = 0; i < TILE_TYPE_COUNT; i++) {
			vCurrentIndex[i]++;
			if (vCurrentIndex.at(i) < g_vSuit[id[i]].m_vTilePartSets.size()) {
				break;
			}
			//else
			vCurrentIndex[i] = 0;
			if (i == TILE_TYPE_COUNT - 1)
				bFinished = true;
		}
	}

	/*
	(TilePartSet w : g_vSuit[id[0]].m_vTilePartSets)
		for (TilePartSet t : g_vSuit[id[1]].m_vTilePartSets)
			for (TilePartSet s : g_vSuit[id[2]].m_vTilePartSets)
				for (TilePartSet z : g_vHonor[id[3]].m_vTilePartSets)
					result.push_back(TilePartCombination(w, t, s, z));
	*/

	return result;
}

void TilePartTable::makeHonorTable()
{
	g_vHonor[0].m_vTilePartSets.push_back(TilePartSet());
	for (TileGroup honor = 0; honor < HONOR_TABLE_SIZE; ++honor) {
		if (honor.getTileNumber() > MAX_HANDTILE_COUNT)
			continue;

		updateHonorEntry(honor);

		/*std::cout << "Entry: " << honor << "(" << honor.toString() << ")" << std::endl;
		std::cout << g_vHonor[honor].toString() << std::endl;
		std::cout << "Done." << std::endl;*/
	}
}

void TilePartTable::makeSuitTable()
{
	g_vSuit[0].m_vTilePartSets.push_back(TilePartSet());
	for (TileGroup suit = 0; suit < SUIT_TABLE_SIZE; ++suit) {
		if (suit.getTileNumber() > MAX_HANDTILE_COUNT)
			continue;

		updateSuitEntry(suit);

		/*std::cout << "Entry: " << suit << "(" << suit.toString() << ")" << std::endl;
		std::cout << g_vSuit[suit].toString() << std::endl;
		std::cout << "Done." << std::endl;*/
	}
}

/*
//only used in Jpn Mahjong
void TilePartTable::setGoshimuso()
{
	TilePartSet tilePartSet;

	tilePartSet = TilePartSet();
	tilePartSet.addToTilePart(TilePart(Type_Alone, 1));
	tilePartSet.addToTilePart(TilePart(Type_Alone, 9));
	g_vSuit[TileGroup("100000001")][0] = tilePartSet;
	
	tilePartSet = TilePartSet();
	tilePartSet.addToTilePart(TilePart(Type_Alone, 1));
	tilePartSet.addToTilePart(TilePart(Type_Alone, 1));
	tilePartSet.addToTilePart(TilePart(Type_Alone, 9));
	g_vSuit[TileGroup("200000001")][0] = tilePartSet;

	tilePartSet = TilePartSet();
	tilePartSet.addToTilePart(TilePart(Type_Alone, 1));
	tilePartSet.addToTilePart(TilePart(Type_Alone, 9));
	tilePartSet.addToTilePart(TilePart(Type_Alone, 9));
	g_vSuit[TileGroup("100000002")][0] = tilePartSet;

	tilePartSet = TilePartSet();
	for (int i = 1; i <= 7; i++)
		tilePartSet.addToTilePart(TilePart(Type_Alone, i));
	g_vHonor[TileGroup("1111111")][0] = tilePartSet;

	for (int i = 0; i < 7; i++) {
		string tiles = "1111111";
		tiles[i] += 1;

		tilePartSet = TilePartSet();
		for (int j = 1; j <= 7; j++) {
			if (j == i + 1)
				tilePartSet.addToTilePart(TilePart(Type_Alone, j));
			tilePartSet.addToTilePart(TilePart(Type_Alone, j));
		}
		g_vHonor[TileGroup(tiles)][0] = tilePartSet;
	}
}

bool TilePartTable::isSevenPairs(const int *id)
{
	bool allPair = g_vSuit[id[0]][0].isAllPair() && g_vSuit[id[1]][0].isAllPair() && g_vSuit[id[2]][0].isAllPair() && g_vHonor[id[3]][0].isAllPair();
	if (!allPair)
		return false;

	int handTileNumber = 0;
	for (int i = 0; i < 4; i++)
		handTileNumber += TileGroup(id[i]).getTileNumber();
	
	if (handTileNumber != 14)
		return false;
	return true;
}
*/

vector<TilePartSet> TilePartTable::pickOutMeldAndPair(TileGroup& tiles, TilePartSet& buildingSet, const int& startIndex)
{
	vector<TilePartSet> result;
	TileGroup remainTiles = tiles;
	TilePartSet nextBuildingSet = buildingSet;

	if (remainTiles.isOnePair()) {
		int pairIndex = remainTiles.getFirstPair();
		nextBuildingSet.addToTilePart(TilePart(TilePartType::Type_Pair, 0, pairIndex + 1));
		remainTiles.pickOut(TileGroup::g_allTile[pairIndex]);
	}
	if (remainTiles.isEmpty()) {
		result.push_back(nextBuildingSet);
		return result;
	}

	for (int i = startIndex; i < MAX_HANDTILE_COUNT - 1; i++) {
		if (remainTiles.canPickOut(TileGroup::g_allMeld[i])) {
			remainTiles.pickOut(TileGroup::g_allMeld[i]);
			nextBuildingSet.addToTilePart(getMeldByIndex(i));

			vector<TilePartSet> partResult = pickOutMeldAndPair(remainTiles, nextBuildingSet, i);
			result.insert(result.end(), partResult.begin(), partResult.end());

			remainTiles = tiles;
			nextBuildingSet = buildingSet;
		}
	}

	return result;
}

vector<TilePartSet> TilePartTable::pickOutTripletAndPair(TileGroup& tiles, TilePartSet& buildingSet, const int& startIndex)
{
	vector<TilePartSet> result;
	TileGroup remainTiles = tiles;
	TilePartSet nextBuildingSet = buildingSet;

	if (remainTiles.isOnePair()) {
		int pairIndex = remainTiles.getFirstPair();
		nextBuildingSet.addToTilePart(TilePart(TilePartType::Type_Pair, 0, pairIndex + 1));
		remainTiles.pickOut(TileGroup::g_allPair[pairIndex]);
	}
	if (remainTiles.isEmpty()) {
		result.push_back(nextBuildingSet);
		return result;
	}

	for (int i = startIndex; i < MAX_RANK; i++) {
		if (remainTiles.canPickOut(TileGroup::g_allMeld[MAX_SUIT_TILE_RANK - 2 + i])) { //triplet
			remainTiles.pickOut(TileGroup::g_allMeld[MAX_SUIT_TILE_RANK - 2 + i]);
			nextBuildingSet.addToTilePart(TilePart(TilePartType::Type_Triplet, 0, i + 1));

			vector<TilePartSet> partResult = pickOutTripletAndPair(remainTiles, nextBuildingSet, i);
			result.insert(result.end(), partResult.begin(), partResult.end());

			remainTiles = tiles;
			nextBuildingSet = buildingSet;
		}
	}

	return result;
}

TilePart TilePartTable::getMeldByIndex(const int& index)
{
	if (index < 7)
		return TilePart(TilePartType::Type_Sequence, 0, index + 1);
	else
		return TilePart(TilePartType::Type_Triplet, 0, index - 6);
}

void TilePartTable::updateHonorEntry(TileGroup & tiles)
{
	TileGroup newTiles = tiles;
	for (int i = 0; i < MAX_HONOR_TILE_RANK; i++) {
		TileGroup oPickOutTiles;
		TilePart oTilePart;
		if (newTiles.canPickOut(TileGroup::g_allMeld[MAX_SUIT_TILE_RANK - 2 + i])) { //triplet
			oPickOutTiles = TileGroup::g_allMeld[MAX_SUIT_TILE_RANK - 2 + i];
			oTilePart = TilePart(TilePartType::Type_Triplet, 0, i + 1);
		}
		else if (newTiles.canPickOut(TileGroup::g_allPair[i])) {
			oPickOutTiles = TileGroup::g_allPair[i];
			oTilePart = TilePart(TilePartType::Type_Pair, 0, i + 1);
		}
		else if (newTiles.canPickOut(TileGroup::g_allTile[i])) {
			oPickOutTiles = TileGroup::g_allTile[i];
			oTilePart = TilePart(TilePartType::Type_Alone, 0, i + 1);
		}
		else continue;

		newTiles.pickOut(oPickOutTiles);
		for (int j = 0; j < g_vHonor[newTiles].m_vTilePartSets.size(); j++) {
			TilePartSet oSet = g_vHonor[newTiles].m_vTilePartSets.at(j);
			oSet.addToTilePartAndGreedyMerge(oTilePart, true);

			bool bRepeatSet = false;
			for (int k = 0; k < g_vHonor[tiles].m_vTilePartSets.size(); k++) {
				if (oSet == g_vHonor[tiles].m_vTilePartSets.at(k)) {
					bRepeatSet = true;
					break;
				}
			}
			if(!bRepeatSet)
				g_vHonor[tiles].m_vTilePartSets.push_back(oSet);
		}
		newTiles.putIn(oPickOutTiles);
	}
}

void TilePartTable::updateSuitEntry(TileGroup & tiles)
{
	TileGroup newTiles = tiles;
	deque<bool> vDontPick(3, false);
	for (int i = 0; i < MAX_SUIT_TILE_RANK; i++) {
		TileGroup oPickOutTiles;
		TilePart oTilePart;
		bool bIsTripletOrPair = false;
		bool bIsSequenceOrSubsequence = false;

		//triplet & pair
		if (newTiles.canPickOut(TileGroup::g_allMeld[MAX_SUIT_TILE_RANK - 2 + i])) { //triplet
			oPickOutTiles = TileGroup::g_allMeld[MAX_SUIT_TILE_RANK - 2 + i];
			oTilePart = TilePart(TilePartType::Type_Triplet, 0, i + 1);
			bIsTripletOrPair = true;
		}
		else if (newTiles.canPickOut(TileGroup::g_allPair[i])) {
			oPickOutTiles = TileGroup::g_allPair[i];
			oTilePart = TilePart(TilePartType::Type_Pair, 0, i + 1);
			bIsTripletOrPair = true;
		}

		if (bIsTripletOrPair) {
			newTiles.pickOut(oPickOutTiles);
			for (int j = 0; j < g_vSuit[newTiles].m_vTilePartSets.size(); j++) {
				TilePartSet oSet = g_vSuit[newTiles].m_vTilePartSets.at(j);
				oSet.addToTilePartAndGreedyMerge(oTilePart, false);

				bool bRepeatSet = false;
				for (int k = 0; k < g_vSuit[tiles].m_vTilePartSets.size(); k++) {
					if (oSet == g_vSuit[tiles].m_vTilePartSets.at(k)) {
						bRepeatSet = true;
						break;
					}
				}
				if (!bRepeatSet)
					g_vSuit[tiles].m_vTilePartSets.push_back(oSet);
			}
			newTiles.putIn(oPickOutTiles);
		}
		
		if (i < MAX_SUIT_TILE_RANK - 2 && newTiles.canPickOut(TileGroup::g_allMeld[i])) {
			oPickOutTiles = TileGroup::g_allMeld[i];
			oTilePart = TilePart(TilePartType::Type_Sequence, 0, i + 1);
			vDontPick[1] = vDontPick[2] = true;
			bIsSequenceOrSubsequence = true;
		}
		else if (i < MAX_SUIT_TILE_RANK - 1 && newTiles.canPickOut(TileGroup::g_allSubsequ[i])) {
			oPickOutTiles = TileGroup::g_allSubsequ[i];
			oTilePart = TilePart(TilePartType::Type_SubSequence_TwoSide, 0, i + 1);
			vDontPick[1] = true;
			bIsSequenceOrSubsequence = true;
		}
		else if (i < MAX_SUIT_TILE_RANK - 2 && newTiles.canPickOut(TileGroup::g_allSubsequ[i + MAX_SUIT_TILE_RANK - 1])) {
			oPickOutTiles = TileGroup::g_allSubsequ[i + MAX_SUIT_TILE_RANK - 1];
			oTilePart = TilePart(TilePartType::Type_SubSequence_MiddleHole, 0, i + 1);
			vDontPick[2] = true;
			bIsSequenceOrSubsequence = true;
		}

		if (bIsSequenceOrSubsequence) {
			newTiles.pickOut(oPickOutTiles);
			for (int j = 0; j < g_vSuit[newTiles].m_vTilePartSets.size(); j++) {
				TilePartSet oSet = g_vSuit[newTiles].m_vTilePartSets.at(j);
				oSet.addToTilePartAndGreedyMerge(oTilePart, false);

				bool bRepeatSet = false;
				for (int k = 0; k < g_vSuit[tiles].m_vTilePartSets.size(); k++) {
					if (oSet == g_vSuit[tiles].m_vTilePartSets.at(k)) {
						bRepeatSet = true;
						break;
					}
				}
				if (!bRepeatSet)
					g_vSuit[tiles].m_vTilePartSets.push_back(oSet);
			}
			newTiles.putIn(oPickOutTiles);
		}


		if (!bIsTripletOrPair && !bIsSequenceOrSubsequence && newTiles.canPickOut(TileGroup::g_allTile[i]) && !vDontPick.at(0)) {
			oPickOutTiles = TileGroup::g_allTile[i];
			oTilePart = TilePart(TilePartType::Type_Alone, 0, i + 1);
			newTiles.pickOut(oPickOutTiles);
			for (int j = 0; j < g_vSuit[newTiles].m_vTilePartSets.size(); j++) {
				TilePartSet oSet = g_vSuit[newTiles].m_vTilePartSets.at(j);
				oSet.addToTilePartAndGreedyMerge(oTilePart, false);

				bool bRepeatSet = false;
				for (int k = 0; k < g_vSuit[tiles].m_vTilePartSets.size(); k++) {
					if (oSet == g_vSuit[tiles].m_vTilePartSets.at(k)) {
						bRepeatSet = true;
						break;
					}
				}
				if (!bRepeatSet)
					g_vSuit[tiles].m_vTilePartSets.push_back(oSet);
			}
			newTiles.putIn(oPickOutTiles);
		}

		vDontPick.pop_front();
		vDontPick.push_back(false);
	}
}

void TilePartTable::updateAddSuitSequence(TileGroup & tiles)
{
	TileGroup newTiles = tiles;
	for (int i = 0; i < MAX_SUIT_TILE_RANK - 2; i++) {
		if (newTiles.canPutIn(TileGroup::g_allMeld[i])) {
			newTiles.putIn(TileGroup::g_allMeld[i]);

			TilePart oTilePart(TilePartType::Type_Sequence, 0, i + 1);
			g_vSuit[newTiles].setup(oTilePart, g_vSuit[tiles]);

			newTiles.pickOut(TileGroup::g_allMeld[i]);
		}
	}
}

void TilePartTable::updateAddSuitPair(TileGroup & tiles)
{
	TileGroup newTiles = tiles;
	for (int i = 0; i < MAX_SUIT_TILE_RANK; i++) {
		if (newTiles.canPutIn(TileGroup::g_allPair[i])) {
			newTiles.putIn(TileGroup::g_allPair[i]);

			TilePart oTilePart(TilePartType::Type_Pair, 0, i + 1);
			g_vSuit[newTiles].setupWithGreedyMerge(oTilePart, g_vSuit[tiles]);

			newTiles.pickOut(TileGroup::g_allPair[i]);
		}
	}
}

void TilePartTable::updateAddSuitSubSequence(TileGroup & tiles)
{
	TileGroup newTiles = tiles;
	for (int i = 0; i < MAX_SUIT_TILE_RANK - 1; i++) {
		if (newTiles.canPutIn(TileGroup::g_allSubsequ[i])) {
			newTiles.putIn(TileGroup::g_allSubsequ[i]);

			TilePart oTilePart(TilePartType::Type_SubSequence_TwoSide, 0, i + 1);
			g_vSuit[newTiles].setupWithGreedyMerge(oTilePart, g_vSuit[tiles]);

			newTiles.pickOut(TileGroup::g_allSubsequ[i]);
		}
	}

	for (int i = 0; i < MAX_SUIT_TILE_RANK - 2; i++) {
		if (newTiles.canPutIn(TileGroup::g_allSubsequ[i + MAX_SUIT_TILE_RANK - 1])) {
			newTiles.putIn(TileGroup::g_allSubsequ[i + MAX_SUIT_TILE_RANK - 1]);

			TilePart oTilePart(TilePartType::Type_SubSequence_MiddleHole, 0, i + 1);
			g_vSuit[newTiles].setupWithGreedyMerge(oTilePart, g_vSuit[tiles]);

			newTiles.pickOut(TileGroup::g_allSubsequ[i + MAX_SUIT_TILE_RANK - 1]);
		}
	}
}

void TilePartTable::updateAddSuitAloneTile(TileGroup & tiles)
{
	TileGroup newTiles = tiles;
	for (int i = 0; i < MAX_SUIT_TILE_RANK; i++) {
		if (newTiles.canPutIn(TileGroup::g_allTile[i])) {
			newTiles.putIn(TileGroup::g_allTile[i]);

			TilePart oTilePart(TilePartType::Type_Alone, 0, i + 1);
			g_vSuit[newTiles].setupWithGreedyMerge(oTilePart, g_vSuit[tiles]);

			newTiles.pickOut(TileGroup::g_allTile[i]);
		}
	}
}

bool TilePartTable::readTable()
{
	const int ciSuitTripletStartIndex = 0;
	const int ciSuitSequenceStartIndex = MAX_RANK;//9
	const int ciSuitPairStartIndex = ciSuitSequenceStartIndex + MAX_SUIT_TILE_RANK - 2;//16
	const int ciSuitSubSequenceTwoSideStartIndex = ciSuitPairStartIndex + MAX_RANK;//25
	const int ciSuitSubSequenceMiddleHoleStartIndex = ciSuitSubSequenceTwoSideStartIndex + MAX_SUIT_TILE_RANK - 1;//33
	const int ciSuitOneTileStartIndex = ciSuitSubSequenceMiddleHoleStartIndex + MAX_SUIT_TILE_RANK - 2;//40
	const int ciSuitSetDataCount = ciSuitOneTileStartIndex + MAX_RANK;//Triplet(9) + Sequence(7) + Pair(9) + SequenceTwoSide(8) + SequenceMiddleHole(7) + One tile(9)

	std::cerr << "[TilePartTable]Start reading tile part table..." << std::endl;
	fstream fin("TilePartTable(16)", std::ios::in);
	if (!fin.is_open()) {
		std::cerr << "Cannot find file \"TilePartTable(16)\". readTable failed." << std::endl;
		return false;
	}

	//read suit tile
	for (TileGroup suit = 0; suit < SUIT_TABLE_SIZE; ++suit) {
		if (suit.getTileNumber() > MAX_HANDTILE_COUNT)
			continue;

		string sInputString;
		fin >> sInputString;
		while (sInputString != "-1") {
			TilePartSet set;

			//Triplet
			for (int i = 0; i < MAX_RANK; i++) {
				int iIndex = ciSuitTripletStartIndex + i;
				int iTilePartCount = sInputString.at(iIndex) - '0';
				TilePart part(TilePartType::Type_Triplet, 0, i + 1);
				for (int j = 0; j < iTilePartCount; j++) {
					set.addToTilePart(part);
				}
			}

			//Sequence
			for (int i = 0; i < MAX_SUIT_TILE_RANK - 2; i++) {
				int iIndex = ciSuitSequenceStartIndex + i;
				int iTilePartCount = sInputString.at(iIndex) - '0';
				TilePart part(TilePartType::Type_Sequence, 0, i + 1);
				for (int j = 0; j < iTilePartCount; j++) {
					set.addToTilePart(part);
				}
			}

			//Pair
			for (int i = 0; i < MAX_RANK; i++) {
				int iIndex = ciSuitPairStartIndex + i;
				int iTilePartCount = sInputString.at(iIndex) - '0';
				TilePart part(TilePartType::Type_Pair, 0, i + 1);
				for (int j = 0; j < iTilePartCount; j++) {
					//set.addToTilePart(part);
					set.addToTilePartAndGreedyMerge(part, false);
				}
			}

			//SubSequnce_TwoSide
			for (int i = 0; i < MAX_SUIT_TILE_RANK - 1; i++) {
				int iIndex = ciSuitSubSequenceTwoSideStartIndex + i;
				int iTilePartCount = sInputString.at(iIndex) - '0';
				TilePart part(TilePartType::Type_SubSequence_TwoSide, 0, i + 1);
				for (int j = 0; j < iTilePartCount; j++) {
					set.addToTilePart(part);
				}
			}

			//SubSequnce_MiddleHole
			for (int i = 0; i < MAX_SUIT_TILE_RANK - 2; i++) {
				int iIndex = ciSuitSubSequenceMiddleHoleStartIndex + i;
				int iTilePartCount = sInputString.at(iIndex) - '0';
				TilePart part(TilePartType::Type_SubSequence_MiddleHole, 0, i + 1);
				for (int j = 0; j < iTilePartCount; j++) {
					set.addToTilePart(part);
				}
			}

			//OneTile
			for (int i = 0; i < MAX_RANK; i++) {
				int iIndex = ciSuitOneTileStartIndex + i;
				int iTilePartCount = sInputString.at(iIndex) - '0';
				TilePart part(TilePartType::Type_Alone, 0, i + 1);
				for (int j = 0; j < iTilePartCount; j++) {
					set.addToTilePart(part);
				}
			}

			g_vSuit[suit].m_vTilePartSets.push_back(set);
			fin >> sInputString;
		}

	}

	const int ciHonorTripletStartIndex = 0;
	const int ciHonorPairStartIndex = MAX_RANK;
	const int ciHonorOneTileStartIndex = ciHonorPairStartIndex + MAX_RANK;

	//read honor tile
	for (TileGroup honor = 0; honor < HONOR_TABLE_SIZE; ++honor) {
		if (honor.getTileNumber() > MAX_HANDTILE_COUNT)
			continue;

		string sInputString;
		fin >> sInputString;
		while (sInputString != "-1") {
			TilePartSet set;

			//Triplet
			for (int i = 0; i < MAX_RANK; i++) {
				int iIndex = ciHonorTripletStartIndex + i;
				int iTilePartCount = sInputString.at(iIndex) - '0';
				TilePart part(TilePartType::Type_Triplet, 0, i + 1);
				for (int j = 0; j < iTilePartCount; j++) {
					set.addToTilePart(part);
				}
			}

			//Pair
			for (int i = 0; i < MAX_RANK; i++) {
				int iIndex = ciHonorPairStartIndex + i;
				int iTilePartCount = sInputString.at(iIndex) - '0';
				TilePart part(TilePartType::Type_Pair, 0, i + 1);
				for (int j = 0; j < iTilePartCount; j++) {
					set.addToTilePart(part);
				}
			}

			//OneTile
			for (int i = 0; i < MAX_RANK; i++) {
				int iIndex = ciHonorOneTileStartIndex + i;
				int iTilePartCount = sInputString.at(iIndex) - '0';
				TilePart part(TilePartType::Type_Alone, 0, i + 1);
				for (int j = 0; j < iTilePartCount; j++) {
					set.addToTilePart(part);
				}
			}

			g_vHonor[honor].m_vTilePartSets.push_back(set);
			fin >> sInputString;
		}

	}

	fin.close();

	std::cerr << "[TilePartTable]Reading tile part table is success!" << std::endl;
	return true;
}

void TilePartTable::writeTable()
{
	const int ciSuitTripletStartIndex = 0;
	const int ciSuitSequenceStartIndex = MAX_RANK;//9
	const int ciSuitPairStartIndex = ciSuitSequenceStartIndex + MAX_SUIT_TILE_RANK - 2;//16
	const int ciSuitSubSequenceTwoSideStartIndex = ciSuitPairStartIndex + MAX_RANK;//25
	const int ciSuitSubSequenceMiddleHoleStartIndex = ciSuitSubSequenceTwoSideStartIndex + MAX_SUIT_TILE_RANK - 1;//33
	const int ciSuitOneTileStartIndex = ciSuitSubSequenceMiddleHoleStartIndex + MAX_SUIT_TILE_RANK - 2;//40
	const int ciSuitSetDataCount = ciSuitOneTileStartIndex + MAX_RANK;//Triplet(9) + Sequence(7) + Pair(9) + SequenceTwoSide(8) + SequenceMiddleHole(7) + One tile(9)
	
	std::cerr << "[TilePartTable]Start writing tile part table..." << std::endl;
	fstream fout("TilePartTable(16)", std::ios::out);

	//write suit tile
	for (TileGroup suit = 0; suit < SUIT_TABLE_SIZE; ++suit) {
		if (suit.getTileNumber() > MAX_HANDTILE_COUNT)
			continue;

		for (TilePartSet set : g_vSuit[suit].m_vTilePartSets) {
			int iSetData[ciSuitSetDataCount] = { 0 };

			for (int i = 0; i < set.getSize(); i++) {
				TilePart part = set.at(i);
				int iTileNumberIndex = part.getNumber() - 1;
				
				//update iSetData every TilePart
				switch (part.getPartType()) {
				case TilePartType::Type_Triplet:
					iSetData[ciSuitTripletStartIndex + iTileNumberIndex]++;
					break;
				case TilePartType::Type_Sequence:
					iSetData[ciSuitSequenceStartIndex + iTileNumberIndex]++;
					break;
				case TilePartType::Type_Pair:
					iSetData[ciSuitPairStartIndex + iTileNumberIndex]++;
					break;
				case TilePartType::Type_SubSequence_TwoSide:
					iSetData[ciSuitSubSequenceTwoSideStartIndex + iTileNumberIndex]++;
					break;
				case TilePartType::Type_SubSequence_MiddleHole:
					iSetData[ciSuitSubSequenceMiddleHoleStartIndex + iTileNumberIndex]++;
					break;
				case TilePartType::Type_Alone:
					iSetData[ciSuitOneTileStartIndex + iTileNumberIndex]++;
					break;
				default:
					std::cerr << "[TilePartTable::writeTable] Unknown TilePart Type!!:" << part.toString() << std::endl;
					//system("pause");
					assert(0);
				}
			}

			//write SetData to file
			for (int i = 0; i < ciSuitSetDataCount; i++) {
				fout << iSetData[i];
			}
			fout << std::endl;
		}

		//write -1 means end of this TileGroup
		fout << "-1" << std::endl;

	}

	const int ciHonorTripletStartIndex = 0;
	const int ciHonorPairStartIndex = MAX_RANK;
	const int ciHonorOneTileStartIndex = ciHonorPairStartIndex + MAX_RANK;
	const int ciHonorSetDataCount = ciHonorOneTileStartIndex + MAX_RANK;//Triplet(9) + Pair(9) + One tile(9)

	//write honor tile
	for (TileGroup honor = 0; honor < HONOR_TABLE_SIZE; ++honor) {
		if (honor.getTileNumber() > MAX_HANDTILE_COUNT)
			continue;

		for  (TilePartSet set : g_vHonor[honor].m_vTilePartSets) {
			int iSetData[ciSuitSetDataCount] = { 0 };

			for (int i = 0; i < set.getSize(); i++) {
				TilePart part = set.at(i);
				int iTileNumber = part.getNumber();

				//update iSetData every TilePart
				switch (part.getPartType()) {
				case TilePartType::Type_Triplet:
					iSetData[ciHonorTripletStartIndex + iTileNumber]++;
					break;
				case TilePartType::Type_Pair:
					iSetData[ciHonorPairStartIndex + iTileNumber]++;
					break;
				case TilePartType::Type_Alone:
					iSetData[ciHonorOneTileStartIndex + iTileNumber]++;
					break;
				default:
					std::cerr << "[TilePartTable::writeTable] Unknown TilePart Type!!:" << part.toString() << std::endl;
					//system("pause");
					assert(0);
				}
			}

			//write SetData to file
			for (int i = 0; i < ciHonorSetDataCount; i++) {
				fout << iSetData[i];
			}
			fout << std::endl;
		}

		//write -1 means end of this TileGroup
		fout << "-1" << std::endl;

	}

	fout.close();
	std::cerr << "[TilePartTable]Writing tile part table is finished." << std::endl;
}
