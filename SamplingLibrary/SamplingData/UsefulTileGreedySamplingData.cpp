#include "UsefulTileGreedySamplingData.h"

UsefulTileGreedySamplingData::UsefulTileGreedySamplingData(const Move & oAction, const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const int & iDrawCount)
	: BaseSamplingData(oAction, oPlayerTile, oRemainTile, iDrawCount), m_oSamplingType(SamplingType::SamplingType_FourPlayers)
{

}

UsefulTileGreedySamplingData::UsefulTileGreedySamplingData(const Move & oAction, const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const int & iDrawCount, const SamplingType& oSamplingType)
	: BaseSamplingData(oAction, oPlayerTile, oRemainTile, iDrawCount), m_oSamplingType(oSamplingType)
{

}

UsefulTileGreedySamplingData::UsefulTileGreedySamplingData() : BaseSamplingData()
{

}


UsefulTileGreedySamplingData::~UsefulTileGreedySamplingData()
{

}

void UsefulTileGreedySamplingData::sample(const uint32_t & iSampleCount, const int & iNeedMeldCount)
{
	time_t startTime = clock();

	switch (m_oSamplingType) {
	case SamplingType::SamplingType_OnePlayer:
		sample_singlePlayer(iSampleCount, iNeedMeldCount);
		break;
	case SamplingType::SamplingType_FourPlayers:
		sample_fourPlayer(iSampleCount, iNeedMeldCount);
		break;
	default:
		std::cerr << "[UsefulTileGreedySamplingData::sample]Unknown SamplingType" << std::endl;
		system("pause");
		exit(87);
	}

	m_iUsedTime += clock() - startTime;
}

void UsefulTileGreedySamplingData::sample_singlePlayer(const uint32_t & iSampleCount, const int & iNeedMeldCount)
{
	if (m_iDrawCount > 0) {
		int iWinTurn;
		for (int i = 0; i < iSampleCount; i++) {
			//sample once
			iWinTurn = sampleOnce_singlePlayers(iNeedMeldCount);

			//update
			if (iWinTurn >= 0) {
				for (int j = iWinTurn; j <= m_iDrawCount; j++) {
					m_vWinCount[j]++;
				}
			}
		}
	}
	else if (m_iDrawCount == 0 && m_oPlayerTile.isWin(iNeedMeldCount)) {
		m_vWinCount[0] += iSampleCount;
	}
	

	//convert win count to win rate
	m_iUsedSampleCount += iSampleCount;
	for (int i = 0; i <= m_iDrawCount; i++) {
		m_vWinRate[i] = static_cast<WinRate_t>(m_vWinCount[i]) / m_iUsedSampleCount;
	}
}

void UsefulTileGreedySamplingData::sample_fourPlayer(const uint32_t & iSampleCount, const int & iNeedMeldCount)
{
	if (m_iDrawCount > 0) {
		int iWinTurn;
		for (int i = 0; i < iSampleCount; i++) {
			//sample once
			iWinTurn = sampleOnce_fourPlayers(iNeedMeldCount);

			//update
			if (iWinTurn >= 0) {
				for (int j = iWinTurn; j <= m_iDrawCount; j++) {
					m_vWinCount[j]++;
				}
			}
		}
	}
	else if (m_iDrawCount == 0 && m_oPlayerTile.isWin(iNeedMeldCount)) {
		m_vWinCount[0] += iSampleCount;
	}
	

	//convert win count to win rate
	m_iUsedSampleCount += iSampleCount;
	for (int i = 0; i <= m_iDrawCount; i++) {
		m_vWinRate[i] = static_cast<WinRate_t>(m_vWinCount[i]) / m_iUsedSampleCount;
	}
}

uint16_t UsefulTileGreedySamplingData::sampleOnce_singlePlayers(const int & iNeedMeldCount)
{
	if (m_oPlayerTile.isWin(iNeedMeldCount)) {
		return 0;
	}

	PlayerTile oPlayerTile = m_oPlayerTile;
	RemainTile_t oRemainTile = m_oRemainTile;
	int iUsedDrawCount;
	Tile oDrawnTile;
	for (iUsedDrawCount = 1; iUsedDrawCount <= m_iDrawCount; iUsedDrawCount++) {
		oDrawnTile = oRemainTile.randomPopTile();
		oPlayerTile.putTileToHandTile(oDrawnTile);
		m_vDrawnTiles[oDrawnTile]++;

		if (oPlayerTile.isWin(iNeedMeldCount)) {
			return iUsedDrawCount;
		}

		Move oMove = getGreedyBestMove2(oPlayerTile, oRemainTile, iNeedMeldCount, true);
		oPlayerTile.doAction(oMove);
		if (oMove.getMoveType() == MoveType::Move_DarkKong || oMove.getMoveType() == MoveType::Move_UpgradeKong)
			iUsedDrawCount--;
	}

	return -1;
}

uint16_t UsefulTileGreedySamplingData::sampleOnce_fourPlayers(const int & iNeedMeldCount)
{
	if (m_oPlayerTile.isWin(iNeedMeldCount)) {
		return 0;
	}

	PlayerTile oPlayerTile = m_oPlayerTile;
	RemainTile_t oRemainTile = m_oRemainTile;
	int iUsedDrawCount;
	array<Tile, PLAYER_COUNT> vDrawnTiles;
	Move oMove;

	//if need to discard first, discard a tile
	if (oPlayerTile.getHandTileNumber() % 3 == 2) {
		if (_goUntilThrowOrWin(oPlayerTile, oRemainTile, iNeedMeldCount, true)) {//[bug]can kong?
			return 0;
		}
	}

	//repeatly draw tile and discard/meld
	for (iUsedDrawCount = 1; iUsedDrawCount <= m_iDrawCount; iUsedDrawCount++) {
		//std::cout << "UsedDrawCount: " << iUsedDrawCount << std::endl;
		//std::cout << "PlayerTile: " << oPlayerTile.toString() << std::endl;
		//std::cout << "vDrawnTiles:";
		for (int i = 0; i < vDrawnTiles.size(); i++) {
			vDrawnTiles[i] = oRemainTile.randomPopTile();
			//std::cout << " " << vDrawnTiles.at(i).toString();
		}
		//std::cout << std::endl;

		bool bMeld = false;
		bool bCanEat, bCanKong;
		for (int i = 0; i < vDrawnTiles.size() - 1; i++) {
			bCanEat = i == vDrawnTiles.size() - 2;//can eat only if previous player discards
			bCanKong = i != vDrawnTiles.size() - 2;//can kong except previous player discards
			oMove = getGreedyBestMove1(oPlayerTile, oRemainTile, iNeedMeldCount, vDrawnTiles.at(i), bCanEat, bCanKong);
			//std::cout << "[" << i << "] " << oMove.toString() << std::endl;
			switch (oMove.getMoveType()) {
			case MoveType::Move_WinByOther:
				return iUsedDrawCount;
			case MoveType::Move_Kong:
				oPlayerTile.kong(vDrawnTiles.at(i));
				oPlayerTile.putTileToHandTile(oRemainTile.randomPopTile());
				if (_goUntilThrowOrWin(oPlayerTile, oRemainTile, iNeedMeldCount, true)) {
					return iUsedDrawCount;
				}
				bMeld = true;
				break;
			case MoveType::Move_Pong:
				oPlayerTile.pong(vDrawnTiles.at(i));
				if (_goUntilThrowOrWin(oPlayerTile, oRemainTile, iNeedMeldCount, false)) {
					return iUsedDrawCount;
				}
				bMeld = true;
				break;
			case MoveType::Move_EatLeft:
				oPlayerTile.eatLeft(vDrawnTiles.at(i));
				if (_goUntilThrowOrWin(oPlayerTile, oRemainTile, iNeedMeldCount, false)) {
					return iUsedDrawCount;
				}
				bMeld = true;
				break;
			case MoveType::Move_EatMiddle:
				oPlayerTile.eatMiddle(vDrawnTiles.at(i));
				if (_goUntilThrowOrWin(oPlayerTile, oRemainTile, iNeedMeldCount, false)) {
					return iUsedDrawCount;
				}
				bMeld = true;
				break;
			case MoveType::Move_EatRight:
				oPlayerTile.eatRight(vDrawnTiles.at(i));
				if (_goUntilThrowOrWin(oPlayerTile, oRemainTile, iNeedMeldCount, false)) {
					return iUsedDrawCount;
				}
				bMeld = true;
				break;
			case MoveType::Move_Pass:
				break;
			default:
				std::cerr << "[UsefulTileGreedySamplingData::sampleOnce_fourPlayers] Illegal move: " << oMove.toString() << std::endl;
				exit(87);
			}
		}
		
		if (!bMeld) {
			//draw tile
			oPlayerTile.putTileToHandTile(vDrawnTiles.at(vDrawnTiles.size() - 1));
			if (oPlayerTile.isWin()) {//self-draw
				return iUsedDrawCount;
			}
			if (_goUntilThrowOrWin(oPlayerTile, oRemainTile, iNeedMeldCount, true)) {//self-draw after dark kong or upgrade kong
				return iUsedDrawCount;
			}
		}
		assert(oPlayerTile.getHandTileNumber() % 3 == 1);
	}

	return -1;
}

Move UsefulTileGreedySamplingData::getGreedyBestMove2(const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const int & iNeedMeldCount, const bool & bCanKong)
{
	if (oPlayerTile.isWin())
		return Move(MoveType::Move_WinBySelf);

	PlayerTile oCopiedPlayerTile = oPlayerTile;
	Move oBestMove;
	uint16_t uiMaxUsefulTileCount = 0;
	int iMinLackBeforeDiscard = oCopiedPlayerTile.getMinLack(iNeedMeldCount);
	assert(iMinLackBeforeDiscard > 0);

	//setup candidate moves
	vector<Move> vCandidateMoves;
	vCandidateMoves.reserve(MAX_HANDTILE_COUNT);
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		if (oCopiedPlayerTile.getHandTileNumber(i) == 0)
			continue;
		//discard
		oCopiedPlayerTile.popTileFromHandTile(i);
		if (oCopiedPlayerTile.getMinLack(iNeedMeldCount) == iMinLackBeforeDiscard) {
			vCandidateMoves.push_back(Move(MoveType::Move_Throw, Tile(i)));
		}
		oCopiedPlayerTile.putTileToHandTile(i);
		//darkKong
		if (oCopiedPlayerTile.canDarkKong(i)) {
			oCopiedPlayerTile.darkKong(i);
			if (oCopiedPlayerTile.getMinLack(iNeedMeldCount) == iMinLackBeforeDiscard) {
				vCandidateMoves.push_back(Move(MoveType::Move_DarkKong, Tile(i)));
			}
			oCopiedPlayerTile.undoAction(Move(MoveType::Move_DarkKong, Tile(i)));
		}
		//upgradeKong
		if (oCopiedPlayerTile.canUpgradeKong(i)) {
			oCopiedPlayerTile.upgradeKong(i);
			if (oCopiedPlayerTile.getMinLack(iNeedMeldCount) == iMinLackBeforeDiscard) {
				vCandidateMoves.push_back(Move(MoveType::Move_UpgradeKong, Tile(i)));
			}
			oCopiedPlayerTile.undoAction(Move(MoveType::Move_UpgradeKong, Tile(i)));
		}
	}

	//calculate useful tile counts
	int iMinLackAfterDiscard = iMinLackBeforeDiscard;
	uint16_t uiUsefulTileCount;
	for (auto oMove : vCandidateMoves) {
		oCopiedPlayerTile.doAction(oMove);

		uiUsefulTileCount = 0;
		for (int j = 0; j < MAX_DIFFERENT_TILE_COUNT; j++) {
			if (oRemainTile.getRemainNumber(j) == 0)
				continue;
			//draw tile
			oCopiedPlayerTile.putTileToHandTile(j);
			//if it is useful draw move, sum to useful tile count
			if (oCopiedPlayerTile.getMinLack(iNeedMeldCount) < iMinLackAfterDiscard) {
				//add to useful tile count
				uiUsefulTileCount += oRemainTile.getRemainNumber(j);
			}
			//undo draw tile
			oCopiedPlayerTile.popTileFromHandTile(j);
		}
		
		//compare to best move and update
		if (uiUsefulTileCount > uiMaxUsefulTileCount) {
			oBestMove = oMove;
			uiMaxUsefulTileCount = uiUsefulTileCount;
		}
		else {
			MoveType oMoveType = oMove.getMoveType();
			MoveType oBestMoveType = oBestMove.getMoveType();
			if (uiUsefulTileCount == uiMaxUsefulTileCount
				&& (oMoveType == MoveType::Move_UpgradeKong
				|| oBestMoveType == MoveType::Move_Throw && oMoveType == MoveType::Move_DarkKong)) {
				oBestMove = oMove;
				uiMaxUsefulTileCount = uiUsefulTileCount;
			}
		}

		oCopiedPlayerTile.undoAction(oMove);
	}

	//Exception: every move's useful tile count = 0
	if (uiMaxUsefulTileCount == 0) {
		for (int i = MAX_DIFFERENT_TILE_COUNT - 1; i >= 0; i--) {
			if(oCopiedPlayerTile.getHandTileNumber(i) > 0)
				return Move(MoveType::Move_Throw, Tile(i));
		}
	}

	return oBestMove;
}

Move UsefulTileGreedySamplingData::getGreedyBestMove1(const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const int & iNeedMeldCount, const Tile& oTargetTile, const bool & bCanEat, const bool & bCanKong)
{
	if (oPlayerTile.canWin(oTargetTile))
		return Move(MoveType::Move_WinByOther, oTargetTile);

	PlayerTile oCopiedPlayerTile = oPlayerTile;
	Move oBestMove;
	uint16_t uiMaxUsefulTileCount = 0;
	int iMinLackBeforeDiscard = oCopiedPlayerTile.getMinLack(iNeedMeldCount);
	assert(iMinLackBeforeDiscard > 0);
	vector<Move> vMeldMoves;

	//setup candidate moves
	if (bCanEat && oCopiedPlayerTile.canEatLeft(oTargetTile))
		vMeldMoves.push_back(Move(MoveType::Move_EatLeft, oTargetTile));
	if (bCanEat && oCopiedPlayerTile.canEatMiddle(oTargetTile))
		vMeldMoves.push_back(Move(MoveType::Move_EatMiddle, oTargetTile));
	if (bCanEat && oCopiedPlayerTile.canEatRight(oTargetTile))
		vMeldMoves.push_back(Move(MoveType::Move_EatRight, oTargetTile));
	if (oCopiedPlayerTile.canPong(oTargetTile))
		vMeldMoves.push_back(Move(MoveType::Move_Pong, oTargetTile));
	if (bCanKong && oCopiedPlayerTile.canKong(oTargetTile))
		vMeldMoves.push_back(Move(MoveType::Move_Kong, oTargetTile));

	int iMinLackAfterDiscard;
	int uiUsefulTileCount;
	for (auto oMove : vMeldMoves) {
		oCopiedPlayerTile.doAction(oMove);

		iMinLackAfterDiscard = oCopiedPlayerTile.getMinLack(iNeedMeldCount);
		if (iMinLackAfterDiscard < iMinLackBeforeDiscard) {//useful move
			uiUsefulTileCount = 0;
			for (int j = 0; j < MAX_DIFFERENT_TILE_COUNT; j++) {
				if (oRemainTile.getRemainNumber(j) == 0)
					continue;
				//draw tile
				oCopiedPlayerTile.putTileToHandTile(j);
				//if it is useful draw move, sum to useful tile count
				if (oCopiedPlayerTile.getMinLack(iNeedMeldCount) < iMinLackAfterDiscard) {
					//add to useful tile count
					uiUsefulTileCount += oRemainTile.getRemainNumber(j);
				}
				//undo draw tile
				oCopiedPlayerTile.popTileFromHandTile(j);
			}

			//compare to best move and update
			if (uiUsefulTileCount > uiMaxUsefulTileCount) {
				oBestMove = oMove;
				uiMaxUsefulTileCount = uiUsefulTileCount;
			}
		}

		oCopiedPlayerTile.undoAction(oMove);
	}
	
	if (uiMaxUsefulTileCount == 0)//no useful meld move
		return Move(MoveType::Move_Pass);

	return oBestMove;
}

void UsefulTileGreedySamplingData::_resetDrawnTiles()
{
	m_vDrawnTiles.fill(0);
	m_vDrawnTiles_Order.clear();
}

bool UsefulTileGreedySamplingData::_goUntilThrowOrWin(PlayerTile& oPlayerTile, RemainTile_t& oRemainTile, const int& iNeedMeldCount, const bool& bCanKong)
{
	assert(oPlayerTile.getHandTileNumber() % 3 == 2);
	Move oMove;
	while (oMove.getMoveType() != MoveType::Move_Throw) {
		oMove = getGreedyBestMove2(oPlayerTile, oRemainTile, iNeedMeldCount, bCanKong);
		//std::cout << "[UsefulTileGreedySamplingData::_goUntilThrow] " << oMove.toString() << std::endl;
		oPlayerTile.doAction(oMove);
		if (oMove.getMoveType() == MoveType::Move_DarkKong || oMove.getMoveType() == MoveType::Move_UpgradeKong) {
			Tile oDrawnTile = oRemainTile.randomPopTile();
			oPlayerTile.putTileToHandTile(oDrawnTile);
			// << "[UsefulTileGreedySamplingData::_goUntilThrow] draw " << oDrawnTile.toString() << std::endl;
			if (oPlayerTile.isWin())
				return true;//win by self-draw
		}
	}
	//assert(oPlayerTile.getHandTileNumber() % 3 == 1);
	return false;//not win yet
}
