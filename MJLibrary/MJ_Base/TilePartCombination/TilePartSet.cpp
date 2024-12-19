#include "TilePartSet.h"
#include <iostream>
#include <algorithm>

vector<TilePartSet> TilePartSet::getAllPossibleTripletAndSequence(vector<int>& vTileCounts, const bool& bNoSequence, const int& iGivenMinIndex)
{
	vector<TilePartSet> vResults;
	int iCurrentMaxPartCount = 0;
	const int ciMaxIndex_Triplet = vTileCounts.size();//expect 9 or 7
	const int ciMaxIndex_Sequence = 7;

	//find min tile in handtile
	int iMinIndex = -1;//[0, 8]
	int iStartIndex = iGivenMinIndex == -1 ? 0 : iGivenMinIndex;
	for (int i = iStartIndex; i < vTileCounts.size(); i++) {
		if (vTileCounts.at(i) > 0) {
			iMinIndex = i;
			break;
		}
	}
	if (iMinIndex == -1) {//empty handtile
		return vector<TilePartSet>();
	}

	
	//try to pick out a triplet
	for (int i = iMinIndex; i < ciMaxIndex_Triplet; i++) {
		if (vTileCounts.at(i) >= 3) {
			if (i == iGivenMinIndex) {
				//avoid to pick i, i+1, i+2 and i, i, i duplicately (ex: 111, 123) (will be done in sequence's part)
				//maybe need verification?
				continue;
			}

			//pick the triplet out
			vTileCounts[i] -= 3;

			//get the possible combination of remained handtile recursively
			vector<TilePartSet> vSubsets = getAllPossibleTripletAndSequence(vTileCounts, bNoSequence, i);

			//if the combination of remained handtile is empty -> generate a combination including one triplet
			if (vSubsets.empty()) {
				if (iCurrentMaxPartCount <= 1) {
					TilePartSet oSubset;
					oSubset.addToTilePart(TilePart(TilePartType::Type_Triplet, 0, i + 1));
					vResults.push_back(oSubset);
					iCurrentMaxPartCount = 1;
				}
			}
			else {//if the combination of remained handtile is not empty
				//add subset into the set list
				int iMaxPartCount_Triplet = vSubsets.at(0).getSize() + 1;
				if (iMaxPartCount_Triplet > iCurrentMaxPartCount) {
					//better than previous result -> replace it
					vResults.clear();
					for (TilePartSet oSubset : vSubsets) {
						oSubset.addToTilePart(TilePart(TilePartType::Type_Triplet, 0, i + 1));
						vResults.push_back(oSubset);
					}
					iCurrentMaxPartCount = iMaxPartCount_Triplet;
				}
				else if (iMaxPartCount_Triplet == iCurrentMaxPartCount) {
					//both are good -> merge(add into previous result)
					for (TilePartSet oSubset : vSubsets) {
						if (oSubset.getSize() + 1 == iCurrentMaxPartCount) {
							oSubset.addToTilePart(TilePart(TilePartType::Type_Triplet, 0, i + 1));
							vResults.push_back(oSubset);
						}
					}
				}
				else {
					//worse than previous result -> discard this result
					//Do nothing
				}
			}

			//put the triplet back
			vTileCounts[i] += 3;
		}
	}

	if(bNoSequence)
		return vResults;
	

	
	//sequence
	for (int i = iMinIndex; i < ciMaxIndex_Sequence; i++) {
		if (vTileCounts.at(i) >= 1 && vTileCounts.at(i + 1) >= 1 && vTileCounts.at(i + 2) >= 1) {
			//pick the sequence out
			vTileCounts[i]--;
			vTileCounts[i + 1]--;
			vTileCounts[i + 2]--;

			//get the possible combination of remained handtile recursively
			vector<TilePartSet> vSubsets = getAllPossibleTripletAndSequence(vTileCounts, bNoSequence, i);
			
			//if the combination of remained handtile is empty
			if (vSubsets.empty()) {
				if (iCurrentMaxPartCount < 1) {
					//Max part count: current result (<1) < sequence result (=1) -> replace by new result
					vResults.clear();
					TilePartSet oSubset;
					oSubset.addToTilePart(TilePart(TilePartType::Type_Sequence, 0, i + 1));
					vResults.push_back(oSubset);
					iCurrentMaxPartCount = 1;
				}
				else if (iCurrentMaxPartCount == 1) {
					//Max part count: current result (=1) == sequence result (=1) -> merge
					TilePartSet oSubset;
					oSubset.addToTilePart(TilePart(TilePartType::Type_Sequence, 0, i + 1));
					vResults.push_back(oSubset);
				}
				else {
					//Max part count: current result (>1) > sequence result (=1) -> no update
					//Do nothing.
				}
				
			}
			else {
				//get max part count of sequence's result
				int iMaxPartCount_Sequence = vSubsets.at(0).getSize() + 1;

				if (iCurrentMaxPartCount < iMaxPartCount_Sequence) {
					//replace by new result
					vResults.clear();
					for (TilePartSet oSubset : vSubsets) {
						oSubset.addToTilePart(TilePart(TilePartType::Type_Sequence, 0, i + 1));
						vResults.push_back(oSubset);
					}
					iCurrentMaxPartCount = iMaxPartCount_Sequence;
				}
				else  if (iCurrentMaxPartCount == iMaxPartCount_Sequence) {
					//merge
					for (TilePartSet oSubset : vSubsets) {
						oSubset.addToTilePart(TilePart(TilePartType::Type_Sequence, 0, i + 1));
						vResults.push_back(oSubset);
					}
				}
				else {
					//Max part count: current result  > sequence result -> no update
					//Do nothing.
				}
				
			}

			//put the sequence back
			vTileCounts[i]++;
			vTileCounts[i + 1]++;
			vTileCounts[i + 2]++;
		}
	}

	return vResults;
}

/*
bool TilePartSet::isAllPair() const
{
	return m_bAllPair;
}
*/

bool TilePartSet::isLegal() const
{
	return m_bLegal;
}

int TilePartSet::getSize() const
{
	return m_vTilePart.size();
}

void TilePartSet::addToTilePart(const TilePart& tilePart)
{
	if (m_vTilePart.size() == 0) {
		m_vTilePart.push_back(tilePart);
		return;
	}

	for (int i = 0; i < m_vTilePart.size(); i++) {
		if (m_vTilePart.at(i).getPartType() > tilePart.getPartType()) {
			m_vTilePart.insert(m_vTilePart.begin() + i, tilePart);
			return;
		}

		if (m_vTilePart.at(i).getPartType() == tilePart.getPartType() && m_vTilePart.at(i).getNumber() > tilePart.getNumber()) {
			m_vTilePart.insert(m_vTilePart.begin() + i, tilePart);
			return;
		}
	}

	m_vTilePart.push_back(tilePart);
}

void TilePartSet::addToTilePartAndGreedyMerge(const TilePart & tilePart, const bool& bIsHonor)
{
	if (m_vTilePart.size() == 0) {
		m_vTilePart.push_back(tilePart);
		return;
	}

	int iSmallestNumber = tilePart.getNumber();
	TilePartType oAddedPartType = tilePart.getPartType();

	//check if can merge
	if (oAddedPartType == TilePartType::Type_Alone) {
		for (int i = 0; i < m_vTilePart.size(); i++) {
			TilePart oPart = m_vTilePart.at(i);
			TilePartType oTilePartType = oPart.getPartType();
			if (!bIsHonor && oTilePartType == TilePartType::Type_SubSequence_TwoSide && oPart.getNumber() == iSmallestNumber - 2) {//ex: <12> + 3 -> <123>
				TilePart oNewTilePart(TilePartType::Type_Sequence, oPart.getTileType(), iSmallestNumber - 2);
				m_vTilePart.erase(m_vTilePart.begin() + i);
				addToTilePart(oNewTilePart);
				return;
			}
			else if (!bIsHonor && oTilePartType == TilePartType::Type_SubSequence_MiddleHole && oPart.getNumber() == iSmallestNumber - 1) {//ex: <13> + 2 -> <123>
				TilePart oNewTilePart(TilePartType::Type_Sequence, oPart.getTileType(), iSmallestNumber - 1);
				m_vTilePart.erase(m_vTilePart.begin() + i);
				addToTilePart(oNewTilePart);
				return;
			}
			else if (!bIsHonor && oTilePartType == TilePartType::Type_SubSequence_TwoSide && oPart.getNumber() == iSmallestNumber + 2) {//ex: <23> + 1 -> <123>
				TilePart oNewTilePart(TilePartType::Type_Sequence, oPart.getTileType(), iSmallestNumber);
				m_vTilePart.erase(m_vTilePart.begin() + i);
				addToTilePart(oNewTilePart);
				return;
			}
			else if (oTilePartType == TilePartType::Type_Pair && oPart.getNumber() == iSmallestNumber) {//ex: <11> + 1 -> <111>
				TilePart oNewTilePart(TilePartType::Type_Triplet, oPart.getTileType(), iSmallestNumber);
				m_vTilePart.erase(m_vTilePart.begin() + i);
				addToTilePart(oNewTilePart);
				return;
			}
			else if (oTilePartType == TilePartType::Type_Alone) {//ex: <1> + 1 -> <11>
				TilePartType oType;
				int iNumber;
				if (oPart.getNumber() == iSmallestNumber) {//ex: <1> + 1 -> <11>
					oType = TilePartType::Type_Pair;
					iNumber = iSmallestNumber;
				}
					
				else if (!bIsHonor && oPart.getNumber() == iSmallestNumber - 1) {//ex: <1> + 2 -> <12>
					oType = TilePartType::Type_SubSequence_TwoSide;
					iNumber = oPart.getNumber();
				}

				else if (!bIsHonor && oPart.getNumber() == iSmallestNumber + 1) {//ex: <2> + 1 -> <12>
					oType = TilePartType::Type_SubSequence_TwoSide;
					iNumber = iSmallestNumber;
				}

				else if (!bIsHonor && oPart.getNumber() == iSmallestNumber - 2) {//ex: <1> + 3 -> <13>
					oType = TilePartType::Type_SubSequence_MiddleHole;
					iNumber = oPart.getNumber();
				}

				else if (!bIsHonor && oPart.getNumber() == iSmallestNumber + 2) {//ex: <3> + 1 -> <13>
					oType = TilePartType::Type_SubSequence_MiddleHole;
					iNumber = iSmallestNumber;
				}
				else 
					continue;
					
				TilePart oNewTilePart(oType, oPart.getTileType(), iNumber);
				m_vTilePart.erase(m_vTilePart.begin() + i);
				addToTilePart(oNewTilePart);
				return;
			}
		}
	}
	if (oAddedPartType == TilePartType::Type_Pair || oAddedPartType == TilePartType::Type_SubSequence_TwoSide || oAddedPartType == TilePartType::Type_SubSequence_MiddleHole) {
		for (int i = 0; i < m_vTilePart.size(); i++) {
			TilePart oPart = m_vTilePart.at(i);
			TilePartType oTilePartType = oPart.getPartType();
			if (oTilePartType != TilePartType::Type_Alone)
				continue;

			if (oAddedPartType == TilePartType::Type_Pair && oPart.getNumber() == iSmallestNumber) {//ex: <1> + 11 -> <111>
				TilePart oNewTilePart(TilePartType::Type_Sequence, oPart.getTileType(), iSmallestNumber);
				m_vTilePart.erase(m_vTilePart.begin() + i);
				addToTilePart(oNewTilePart);
				return;
			}
			else if (!bIsHonor && oAddedPartType == TilePartType::Type_SubSequence_TwoSide && oPart.getNumber() == iSmallestNumber - 1) {//ex: <1> + 23 -> <123>
				TilePart oNewTilePart(TilePartType::Type_Sequence, oPart.getTileType(), iSmallestNumber - 1);
				m_vTilePart.erase(m_vTilePart.begin() + i);
				addToTilePart(oNewTilePart);
				return;
			}
			else if (!bIsHonor && oAddedPartType == TilePartType::Type_SubSequence_MiddleHole && oPart.getNumber() == iSmallestNumber + 1) {//ex: <2> + 13 -> <123>
				TilePart oNewTilePart(TilePartType::Type_Sequence, oPart.getTileType(), iSmallestNumber);
				m_vTilePart.erase(m_vTilePart.begin() + i);
				addToTilePart(oNewTilePart);
				return;
			}
			else if (!bIsHonor && oAddedPartType == TilePartType::Type_SubSequence_TwoSide && oPart.getNumber() == iSmallestNumber + 2) {//ex: <3> + 12 -> <123>
				TilePart oNewTilePart(TilePartType::Type_Sequence, oPart.getTileType(), iSmallestNumber);
				m_vTilePart.erase(m_vTilePart.begin() + i);
				addToTilePart(oNewTilePart);
				return;
			}
		}
	}

	//no need to merge
	addToTilePart(tilePart);
}

/*
void TilePartSet::addToAllPairTilePart(const TilePart& tilePart)
{
	m_vAllPairTilePart.push_back(tilePart);
}


void TilePartSet::setToAllPair()
{
	m_bAllPair = true;
}

TilePartSet TilePartSet::getAllPairTilePart() const
{
	return TilePartSet(m_vAllPairTilePart);
}
*/

string TilePartSet::toString() const
{
	string result = "[ ";
	for (int i = 0; i < getSize(); i++) {
		result += m_vTilePart[i].toString();
		if (i == getSize() - 1)
			break;
		result += ", ";
	}
	result += " ]";
	return result;
}

TilePartSet TilePartSet::operator+(const TilePartSet& rhs) const
{
	TilePartSet result = TilePartSet(*this);
	result.m_vTilePart.insert(result.m_vTilePart.end(), rhs.m_vTilePart.begin(), rhs.m_vTilePart.end());
	return result;
}

TilePart& TilePartSet::operator[](const int& index)
{
	return m_vTilePart[index];
}

bool TilePartSet::operator==(const TilePartSet & rhs) const
{
	if (m_vTilePart.size() != rhs.m_vTilePart.size())
		return false;
	for (int i = 0; i < m_vTilePart.size(); i++) {
		if (m_vTilePart.at(i) != rhs.m_vTilePart.at(i))
			return false;
	}
	return true;
}
