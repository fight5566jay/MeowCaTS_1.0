#include "ChanceNode2.h"

ChanceNode2::ChanceNode2(const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const Move & oMove, const int & iDrawCount, const bool& bUseTT, const uint32_t& uiTTType)
	: BaseTreeNode2(oPlayerTile, oRemainTile, oMove, iDrawCount, bUseTT, uiTTType)
{
}

ChanceNode2::ChanceNode2(BaseTreeNode2 * pParent, const Move & oMove, const int & iDrawCount)
	: BaseTreeNode2(pParent, oMove, iDrawCount)
{
}

ChanceNode2::ChanceNode2(BaseTreeNode2 * pParent, const Move & oMove, const int & iDrawCount, BaseTreeNode2* pTTNode)
	: BaseTreeNode2(pParent, oMove, iDrawCount, pTTNode)
{
}

string ChanceNode2::getSgfCommand() const
{
	stringstream ss;
	ss << BaseTreeNode2::getSgfCommand();
	ss << "Childrens\' chance & weight:" << std::endl;
	if (m_vChildren.empty()) {
		ss << "Not expand yet." << std::endl;
	}
	else {
		BaseTreeNode2* pChild;
		for (int i = 0; i < m_vChances.size(); i++) {
			pChild = m_vChildren.at(i).get();
			ss << pChild->getMove().toString() << ": "
				<< m_vChances.at(i) << " " << m_vWeights.at(i);
			if (pChild->getMove().getMoveType() == MoveType::Move_Draw) {
				ss << " (" << getSamplingData().m_oRemainTile.getRemainNumber(pChild->getMove().getTargetTile())
					<< "/" << getSamplingData().m_oRemainTile.getRemainNumber() << ")";
			}

			ss << std::endl;
		}
	}

	return ss.str();
}

void ChanceNode2::setWeightWithModifiedChance(const vector<NodeWinRate_t>& vModifiedChance)
{
	assert(vModifiedChance.size() == m_vChances.size());
	m_vWeights.resize(m_vChances.size());

	//compute weights
	for (int i = 0; i < m_vChances.size(); i++) {
		m_vWeights[i] = m_vChances[i] / vModifiedChance[i];
	}

	//[tmp]change old chance to new chance
	for (int i = 0; i < m_vChances.size(); i++) {
		m_vChances[i] = vModifiedChance[i];
	}
}

array<NodeWinRate_t, MAX_DIFFERENT_TILE_COUNT> ChanceNode2::getChanceConsideringMeldFactor(const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile)
{
	//[NEED TO TEST]
	//useless tile should not consider meld factor?
	array<NodeWinRate_t, MAX_DIFFERENT_TILE_COUNT> vChances;
	bool vCanPongOrWin[MAX_DIFFERENT_TILE_COUNT];
	bool vCanEat[MAX_DIFFERENT_TILE_COUNT];
	int iTotalCount = 0, iRemainTileCount;
	PlayerTile oNewPlayerTile = oPlayerTile;
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		if (oRemainTile.getRemainNumber(i) == 0)
			continue;

		//check win
		vCanPongOrWin[i] = oPlayerTile.canWin(i);
		//if (vCanPongOrWin[i])
			//std::cerr << Tile(i).toString() << " win." << std::endl;

		//if no win, check effective pong
		if (!vCanPongOrWin[i] && oPlayerTile.canPong(i)) {
			oNewPlayerTile.pong(i);
			//std::cerr << Tile(i).toString() << " " << oPlayerTile.getMinLack() << " " << oNewPlayerTile.getMinLack() << std::endl;
			vCanPongOrWin[i] = oNewPlayerTile.getMinLack() < oPlayerTile.getMinLack();
			oNewPlayerTile.undoAction(Move(MoveType::Move_Pong, Tile(i)));
		}

		//if no effective pong, check effective eat
		if (!vCanPongOrWin[i]) {
			vCanEat[i] = false;//if vCanPongOrWin is true, vCanEat is don't care.
			if (oPlayerTile.canEatLeft(i)) {
				//eat left
				oNewPlayerTile.eatLeft(i);
				vCanEat[i] = oNewPlayerTile.getMinLack() < oPlayerTile.getMinLack();
				oNewPlayerTile.undoAction(Move(MoveType::Move_EatLeft, Tile(i)));
			}
			else if (oPlayerTile.canEatMiddle(i)) {
				//eat middle
				oNewPlayerTile.eatMiddle(i);
				vCanEat[i] = vCanEat[i] || oNewPlayerTile.getMinLack() < oPlayerTile.getMinLack();
				oNewPlayerTile.undoAction(Move(MoveType::Move_EatMiddle, Tile(i)));
			}
			else if (oPlayerTile.canEatRight(i)) {
				//eat right
				oNewPlayerTile.eatRight(i);
				vCanEat[i] = vCanEat[i] || oNewPlayerTile.getMinLack() < oPlayerTile.getMinLack();
				oNewPlayerTile.undoAction(Move(MoveType::Move_EatRight, Tile(i)));
			}
		}

		//modify total tile count according to meld factor(vCanPongOrWin, vCanEat)
		iRemainTileCount = oRemainTile.getRemainNumber(i);
		if (vCanPongOrWin[i]) {
			iTotalCount += iRemainTileCount * 4;
		}
		else if (vCanEat[i]) {
			iTotalCount += iRemainTileCount * 2;
		}
		else {
			iTotalCount += iRemainTileCount;
		}

	}

	//compute chances
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		iRemainTileCount = oRemainTile.getRemainNumber(i);
		if (vCanPongOrWin[i]) {
			vChances[i] = static_cast<NodeWinRate_t>(iRemainTileCount) * 4.0 / iTotalCount;
			//std::cerr << "4";
		}
		else if (vCanEat[i]) {
			vChances[i] = static_cast<NodeWinRate_t>(iRemainTileCount) * 2.0 / iTotalCount;
			//std::cerr << "2";
		}
		else {
			vChances[i] = static_cast<NodeWinRate_t>(iRemainTileCount) / iTotalCount;
			//std::cerr << "1";
		}
		//if (i % MAX_SUIT_TILE_RANK == MAX_SUIT_TILE_RANK - 1)
			//std::cerr << std::endl;
	}

	return vChances;
}
