#include "TilePartEntry.h"
#include <iostream>
#include <cassert>
#include <sstream>
using std::stringstream;
using std::cerr;
using std::endl;

void TilePartEntry::setup(const TilePart & oPart, const TilePartEntry& oEntry)
{
	*this = oEntry;
	for (int i = 0; i < m_vTilePartSets.size(); i++) {
		m_vTilePartSets[i].addToTilePart(oPart);
	}
}

void TilePartEntry::setupWithGreedyMerge(const TilePart & oAddedPart, const TilePartEntry& oEntry)
{
	//This function will check if there is some TileParts can merge together, ex: <1> + 23 -> <123>.
	//But this function is used for TilePartTable, and will not perfectly find all pairs of TilePart that can merge
	//Ingored case: existing TilePart's Index > added TilePart's index
	//ex: <23> + 1 -> <123> will be ignored

	*this = oEntry;

	//update m_vTilePartSets
	//add new TilePartSet if entry is empty
	if (m_vTilePartSets.size() == 0) {
		TilePartSet oSet;
		oSet.addToTilePart(oAddedPart);
		m_vTilePartSets.push_back(oSet);
		return;
	}

	int iSmallestNumber = oAddedPart.getNumber();
	TilePartType oAddedPartType = oAddedPart.getPartType();
	
	//check if can merge
	if (oAddedPartType == TilePartType::Type_Alone) {
		for (int i = 0; i < m_vTilePartSets.size(); i++) {
			for (int j = 0; j < m_vTilePartSets.at(i).getSize(); j++) {
				TilePart oPart = m_vTilePartSets.at(i).at(j);
				TilePartType oTilePartType = oPart.getPartType();
				if (oTilePartType == TilePartType::Type_SubSequence_TwoSide && oPart.getNumber() == iSmallestNumber - 2) {//ex: <12> + 3 -> <123>
					TilePart oNewTilePart(TilePartType::Type_Sequence, 0, iSmallestNumber - 2);
					m_vTilePartSets[i].erase(j);
					m_vTilePartSets[i].addToTilePart(oNewTilePart);
					return;
				}
				else if (oTilePartType == TilePartType::Type_SubSequence_MiddleHole && oPart.getNumber() == iSmallestNumber - 1) {//ex: <13> + 2 -> <123>
					TilePart oNewTilePart(TilePartType::Type_Sequence, 0, iSmallestNumber - 1);
					m_vTilePartSets[i].erase(j);
					m_vTilePartSets[i].addToTilePart(oNewTilePart);
					return;
				}
			}
		}
	}
	if (oAddedPartType == TilePartType::Type_Pair || oAddedPartType == TilePartType::Type_SubSequence_TwoSide) {
		for (int i = 0; i < m_vTilePartSets.size(); i++) {
			for (int j = 0; j < m_vTilePartSets.at(i).getSize(); j++) {
				TilePart oPart = m_vTilePartSets.at(i).at(j);
				TilePartType oTilePartType = oPart.getPartType();
				if (oTilePartType != TilePartType::Type_Alone)
					continue;

				if (oAddedPartType == TilePartType::Type_Pair && oPart.getNumber() == iSmallestNumber) {//ex: <1> + 11 -> <111>
					TilePart oNewTilePart(TilePartType::Type_Sequence, 0, iSmallestNumber);
					m_vTilePartSets[i].erase(j);
					m_vTilePartSets[i].addToTilePart(oNewTilePart);
					return;
				}
				else if (oAddedPartType == TilePartType::Type_SubSequence_TwoSide && oPart.getNumber() == iSmallestNumber - 1) {//ex: <1> + 23 -> <123>
					TilePart oNewTilePart(TilePartType::Type_Sequence, 0, iSmallestNumber - 1);
					m_vTilePartSets[i].erase(j);
					m_vTilePartSets[i].addToTilePart(oNewTilePart);
					return;
				}
			}
		}
	}

	//no need to merge
	for (int i = 0; i < m_vTilePartSets.size(); i++) {
		m_vTilePartSets[i].addToTilePart(oAddedPart);
	}
	
}

string TilePartEntry::toString() const
{
	stringstream ss;
	ss << "Tile number: " << m_iTileNumber << std::endl;
	ss << "TilePartSets:" << std::endl;
	for (int i = 0; i < m_vTilePartSets.size(); i++) {
		ss << m_vTilePartSets.at(i).toString() << " ";
	}
	ss << std::endl;
	return ss.str();
}

void TilePartEntry::updateTileNumber(const TilePart & oPart)
{
	switch (oPart.getPartType()) {
	case TilePartType::Type_Alone:
		m_iTileNumber++;
		break;
	case TilePartType::Type_Pair:
	case TilePartType::Type_SubSequence_TwoSide:
	case TilePartType::Type_SubSequence_MiddleHole:
		m_iTileNumber += 2;
		break;
	case TilePartType::Type_Triplet:
	case TilePartType::Type_Sequence:
	case TilePartType::Type_Eat:
	case TilePartType::Type_Pong:
		m_iTileNumber += 3;
		break;
	case TilePartType::Type_Kong:
	case TilePartType::Type_DarkKong:
		m_iTileNumber += 4;
		break;
	default:
		std::cerr << "[TilePartEntry] Unknown TilePart Type: " << oPart.toString() << std::endl;
		assert(0);
	}
}
