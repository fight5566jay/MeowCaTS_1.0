#include "TilePartCombination.h"
#include <iostream>
#include <cassert>
#include <algorithm>

using std::cerr;
using std::endl;

/*TilePartCombination::TilePartCombination(const TilePartSet& w, const TilePartSet& t, const TilePartSet& s, const TilePartSet& z)
{
	m_vTliePartSets[0] = w;
	m_vTliePartSets[1] = t;
	m_vTliePartSets[2] = s;
	m_vTliePartSets[3] = z;
}*/

int TilePartCombination::getMinLack(const int & iNeedMeldCount) const
{
	int iAns = 0;
	int iCompletedMeldCount = 0;
	int iUncompletedMeldCount = 0;
	bool bHasPair = false;
	int iAloneTileCount = 0;

	for (int i = 0; i < SUIT_COUNT; i++) {
		TilePartSet oTilePartSet = m_vTliePartSets[i];
		for (int j = 0; j < oTilePartSet.getSize(); j++) {
			TilePart oTilePart = oTilePartSet.at(j);
			switch (oTilePart.getPartType()) {
			case TilePartType::Type_Alone:
				iAloneTileCount++;
				break;
			case TilePartType::Type_Triplet:
			case TilePartType::Type_Sequence:
				iCompletedMeldCount++;
				break;
			case TilePartType::Type_Pair:
				bHasPair = true;
				iUncompletedMeldCount++;
				break;
			case TilePartType::Type_SubSequence_TwoSide:
			case TilePartType::Type_SubSequence_MiddleHole:
				iUncompletedMeldCount++;
				break;
			default:
				std::cerr << "[TileCombination::getMinLack] Unknown TilePart type!!: " << oTilePart.toString() << std::endl;
				assert(0);
			}
		}
	}

	if (bHasPair)
		iUncompletedMeldCount--;

	int iCurrentNeedMeldCount = iNeedMeldCount - iCompletedMeldCount;
	if (iCurrentNeedMeldCount > 0) {
		if (iUncompletedMeldCount > iCurrentNeedMeldCount) {
			//left meld are all uncompleted meld
			iAns += iCurrentNeedMeldCount;
			iUncompletedMeldCount -= iCurrentNeedMeldCount;
		}
		else {
			iAns += iUncompletedMeldCount;
			iCurrentNeedMeldCount -= iUncompletedMeldCount;
			if (iAloneTileCount > iCurrentNeedMeldCount) {
				//left meld are all alone tile
				iAns += iCurrentNeedMeldCount * 2;
				iAloneTileCount -= iCurrentNeedMeldCount;
			}
			else {
				//tile count is not enough
				iAns += iAloneTileCount * 2;
				iCurrentNeedMeldCount -= iAloneTileCount;
				iAns += iCurrentNeedMeldCount * 3;
			}
		}

	}
	//check pair
	if (!bHasPair) {
		if (iUncompletedMeldCount == 0 && iAloneTileCount == 0)
			iAns += 2;
		else
			iAns++;
	}
	return iAns;
}

bitset<MAX_DIFFERENT_TILE_COUNT> TilePartCombination::getUsefulTiles(const int& iNeedMeldCount, const bool& bNeedEyes) const
{
	bitset<MAX_DIFFERENT_TILE_COUNT> vPairAndSubSesquence;
	bitset<MAX_DIFFERENT_TILE_COUNT> vAlone;
	int iPairAndSubSesquenceCount = 0;
	bool bHasPair = false;
	int iFirstPairIndex = -1;

	//First, ingore alone TilePart
	for (int i = 0; i < SUIT_COUNT; i++) {
		TilePartSet oTilePartSet = m_vTliePartSets[i];
		for (int j = 0; j < oTilePartSet.getSize(); j++) {
			TilePart oTilePart = oTilePartSet.at(j);
			int iTileIndex = i * MAX_SUIT_TILE_RANK + oTilePart.getNumber() - 1;
			switch (oTilePart.getPartType()) {
			case TilePartType::Type_Alone:
				vAlone.set(iTileIndex);
				continue;
			case TilePartType::Type_Pair:
				if (bNeedEyes && !bHasPair) {
					iFirstPairIndex = iTileIndex;
					bHasPair = true;
				}
				else if (bNeedEyes) {//has pair
					vPairAndSubSesquence.set(iFirstPairIndex);
					vPairAndSubSesquence.set(iTileIndex);
					iPairAndSubSesquenceCount++;
				}
				else {//no need eyes
					vPairAndSubSesquence.set(iTileIndex);
					iPairAndSubSesquenceCount++;
				}
				break;
			case TilePartType::Type_Triplet:
			case TilePartType::Type_Sequence:
				break;
			case TilePartType::Type_SubSequence_TwoSide:
				if (oTilePart.getNumber() > 1) {
					vPairAndSubSesquence.set(iTileIndex - 1);
				}
				if (oTilePart.getNumber() < MAX_SUIT_TILE_RANK - 1) {
					vPairAndSubSesquence.set(iTileIndex + 2);
				}
				iPairAndSubSesquenceCount++;
				break;
			case TilePartType::Type_SubSequence_MiddleHole:
				vPairAndSubSesquence.set(iTileIndex + 1);
				iPairAndSubSesquenceCount++;
				break;
			}

		}
	}

	if (iPairAndSubSesquenceCount > iNeedMeldCount)
		return vPairAndSubSesquence;
	return vPairAndSubSesquence | vAlone;
}

string TilePartCombination::toString() const
{
	string result = "{ " + m_vTliePartSets.at(0).toString();
	for (int i = 1; i < TILE_TYPE_COUNT; i++) {
		result += ", ";
		result +=  m_vTliePartSets.at(i).toString();
	}
	result += " }";
	return result;
}

int TilePartCombination::getTilePartCount() const
{
	int result = 0;
	for (int i = 0; i < TILE_TYPE_COUNT; i++)
		result += m_vTliePartSets.at(i).getSize();
	return result;
}

TilePartCombination TilePartCombination::operator+(const TilePartCombination& rhs) const
{
	TilePartCombination result;
	result[0] = m_vTliePartSets[0] + rhs.m_vTliePartSets[0];
	result[1] = m_vTliePartSets[1] + rhs.m_vTliePartSets[1];
	result[2] = m_vTliePartSets[2] + rhs.m_vTliePartSets[2];
	result[3] = m_vTliePartSets[3] + rhs.m_vTliePartSets[3];
	return result;
}

TilePartSet& TilePartCombination::operator[](const int& index)
{
	return m_vTliePartSets[index];
}