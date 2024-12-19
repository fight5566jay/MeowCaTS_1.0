#include "GodMoveSamplingData.h"
#include "../../MJLibrary/MJ_Base/HandTileDecomposer.h"

GodMoveSamplingData::GodMoveSamplingData(const Move & oAction, const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const int & iDrawCount, const GetTileType & oGetTileType) :
	BaseSamplingData(oAction, oPlayerTile, oRemainTile, iDrawCount), m_oGetTileType(oGetTileType)
{
	const int ciMaxDrawCount = m_oRemainTile.getRemainNumber() / PLAYER_COUNT;
	if (iDrawCount > ciMaxDrawCount) {
		std::cerr << "[SamplingData_v2::SamplingData_v2] Warning: Given draw count (" << iDrawCount
			<< ") is greater than RemainTile's count (" << m_oRemainTile.getRemainNumber()
			<< ") / PLAYER_COUNT (" << PLAYER_COUNT
			<< ") = "
			<< ciMaxDrawCount << "." << std::endl
			<< "Program will adjust the draw count to " << ciMaxDrawCount << "." << std::endl;
		m_iDrawCount = ciMaxDrawCount;
	}
}

void GodMoveSamplingData::sample(const uint32_t & iSampleCount, const int & iNeedMeldCount)
{
	time_t startTime = clock();
	m_iUsedSampleCount += iSampleCount;
	for (int i = 0; i < iSampleCount; ++i) {
		//sample once
		int iWinTurn;
		switch (m_oGetTileType) {
		case GetTileType::GetTileType_OnePlayerMo:
			iWinTurn = _sample_singlePLayer(iNeedMeldCount);
			break;
		case GetTileType::GetTileType_FourPlayerMo:
			iWinTurn = _sample_fourPLayer(iNeedMeldCount);
			break;
		default:
			std::cerr << "[SamplingData_v2::sample]Unknown m_oGetTileType" << std::endl;
			system("pause");
			exit(87);
		}

		//update
		if (iWinTurn >= 0) {
			for (int j = iWinTurn; j <= m_iDrawCount; j++) {
				m_vWinCount[j]++;
			}
		}

		assert(m_iUsedSampleCount >= 0);
	}

	//convert win count to win rate
	for (int i = 0; i <= m_iDrawCount; i++) {
		m_vWinRate[i] = static_cast<WinRate_t>(m_vWinCount[i]) / m_iUsedSampleCount;
	}

	m_iUsedTime += clock() - startTime;
}

void GodMoveSamplingData::sample_withChunk(const uint32_t & iSampleCount, const int & iNeedMeldCount)
{
	time_t startTime = clock();
	m_iUsedSampleCount += iSampleCount;
	for (int i = 0; i < iSampleCount; ++i) {
		//sample once
		int iWinTurn = _sample_withChunk(iNeedMeldCount);
		/*switch (m_oGetTileType) {
		case GetTileType_OnePlayerMo:
			iWinTurn = _sample_withChunk(iNeedMeldCount);
			break;
		case GetTileType_FourPlayerMo:
			iWinTurn = _sample_withChunk(iNeedMeldCount);
			break;
		default:
			std::cerr << "[SamplingData_v2::sample]Unknown m_oGetTileType: " << m_oGetTileType << std::endl;
			system("pause");
			exit(87);
		}*/

		//update
		if (iWinTurn >= 0) {
			for (int j = iWinTurn; j <= m_iDrawCount; j++) {
				m_vWinCount[j] += 1;
			}
		}

		//debug msg
		/*if ((i + 1) % 1000 == 0) {
			std::cerr << "Sample " << i + 1 << " times..." << std::endl;
		}*/
	}

	//convert win count to win rate
	for (int i = 0; i <= m_iDrawCount; i++) {
		m_vWinRate[i] = static_cast<WinRate_t>(m_vWinCount[i]) / m_iUsedSampleCount;
	}
	m_iUsedTime += clock() - startTime;
}

int GodMoveSamplingData::_sample_singlePLayer(const int & iNeedMeldCount)
{
	m_iDrawnTileSize = 0;
	for (int i = 0; i < m_iDrawCount; ++i) {
		if (m_oPlayerTile.isWin(iNeedMeldCount)) {
			for (int j = 0; j < m_iDrawnTileSize; j++) {
				m_oPlayerTile.popTileFromHandTile(m_vDrawnTiles[j]);
				m_oRemainTile.addTile(m_vDrawnTiles[j]);
			}
			return i;
		}

		Tile oDrawnTile = m_oRemainTile.randomPopTile();
		m_oPlayerTile.putTileToHandTile(oDrawnTile);
		m_vDrawnTiles[m_iDrawnTileSize++] = oDrawnTile;
	}

	for (int j = 0; j < m_iDrawnTileSize; j++) {
		m_oPlayerTile.popTileFromHandTile(m_vDrawnTiles[j]);
		m_oRemainTile.addTile(m_vDrawnTiles[j]);
	}
	return m_oPlayerTile.isWin(iNeedMeldCount) ? m_iDrawCount : -1;
}

int GodMoveSamplingData::_sample_fourPLayer(const int & iNeedMeldCount)
{
	if (m_oPlayerTile.getMinLack() == 0)
		return 0;

	m_vPossiblePlayerTiles.clear();
	m_vPossiblePlayerTiles.push_back(m_oPlayerTile);
	resetDrawnTiles();

	int iPossiblePlayerTileSize = m_vPossiblePlayerTiles.size();
	for (int i = 0; i < m_iDrawCount; ++i) {
		for (int j = 0; j < iPossiblePlayerTileSize; j++) {
			if (m_vPossiblePlayerTiles[j].isWin(iNeedMeldCount)) {
				undoPopTile_fourPlayer();
				return i;
			}
		}

		getTile_fourPlayer(iNeedMeldCount);
	}

	for (int j = 0; j < iPossiblePlayerTileSize; j++) {
		if (m_vPossiblePlayerTiles[j].isWin(iNeedMeldCount)) {
			undoPopTile_fourPlayer();
			return m_iDrawCount;
		}
	}

	undoPopTile_fourPlayer();
	return -1;
}

bool GodMoveSamplingData::getTile_fourPlayer(const int & iNeedMeldCount)
{
	/*std::cerr << "[Before]Current possible PlayerTile:" << std::endl;
	for (int i = 0; i < m_vPossiblePlayerTiles.size(); i++) {
		std::cerr << m_vPossiblePlayerTiles.at(i).toString() << std::endl;
	}*/

	vector<PlayerTile> vNewPlayerTiles;
	array<Tile, PLAYER_COUNT> vDrawnTilesRound;
	int iPossiblePlayerTileSize;
	for (int i = 0; i < PLAYER_COUNT; i++) {
		Tile oDrawnTile = m_oRemainTile.randomPopTile();
		m_vDrawnTiles_fourPlayers[oDrawnTile]++;
		vDrawnTilesRound[i] = oDrawnTile;
		//std::cerr << "Draw tile: " << oDrawnTile.toString() << std::endl;
		iPossiblePlayerTileSize = m_vPossiblePlayerTiles.size();
		for (int j = 0; j < iPossiblePlayerTileSize; j++) {
			//hu case
			if (m_vPossiblePlayerTiles[j].canWin(oDrawnTile, iNeedMeldCount)) {
				//std::cerr << "hu: " << oDrawnTile.toString() << " " << m_vPossiblePlayerTiles[j].toString() << std::endl;
				m_vPossiblePlayerTiles[j].putTileToHandTile(oDrawnTile);
				return true;//There is a PlayerTile wins
			}

			//draw case
			if (i == PLAYER_COUNT - 1) {
				//std::cerr << "draw: " << oDrawnTile.toString() << " " << m_vPossiblePlayerTiles[j].toString() << std::endl;
				/*for (int k = 0; k < 34; k++) {
					int iDrawTileCount = k == oDrawnTile ? 1 : 0;
					if (m_vPossiblePlayerTiles[j].getHandTileNumber(k) + m_oRemainTile.getRemainNumber(k) + iDrawTileCount > 4) {
						std::cerr << "[error]" << std::endl << m_vPossiblePlayerTiles[j].toString() << std::endl << m_oRemainTile.toFeature().toString() << std::endl;
					}
				}*/
				m_vPossiblePlayerTiles[j].putTileToHandTile(oDrawnTile);
				continue;
			}

			//pong case
			if (m_vPossiblePlayerTiles[j].canPong(oDrawnTile)) {
				//std::cerr << "pong: " << oDrawnTile.toString() << " " << m_vPossiblePlayerTiles.at(j).toString() << std::endl;
				PlayerTile oPlayerTileAfterPong = m_vPossiblePlayerTiles[j];
				oPlayerTileAfterPong.pong(oDrawnTile);
				vNewPlayerTiles.push_back(oPlayerTileAfterPong);
			}

			//eat left case
			if (i == PLAYER_COUNT - 2) {
				if (m_vPossiblePlayerTiles[j].canEatLeft(oDrawnTile)) {
					//std::cerr << "eat left: " << oDrawnTile.toString() << " " << m_vPossiblePlayerTiles.at(j).toString() << std::endl;
					PlayerTile oPlayerTileAfterEatLeft = m_vPossiblePlayerTiles[j];
					oPlayerTileAfterEatLeft.eatLeft(oDrawnTile);
					vNewPlayerTiles.push_back(oPlayerTileAfterEatLeft);
				}

				//eat middle case
				if (m_vPossiblePlayerTiles[j].canEatMiddle(oDrawnTile)) {
					//std::cerr << "eat middle: " << oDrawnTile.toString() << " " << m_vPossiblePlayerTiles.at(j).toString() << std::endl;
					PlayerTile oPlayerTileAfterEatMiddle = m_vPossiblePlayerTiles[j];
					oPlayerTileAfterEatMiddle.eatMiddle(oDrawnTile);
					vNewPlayerTiles.push_back(oPlayerTileAfterEatMiddle);
				}

				//eat right case
				if (m_vPossiblePlayerTiles[j].canEatRight(oDrawnTile)) {
					//std::cerr << "eat right: " << oDrawnTile.toString() << " " << m_vPossiblePlayerTiles.at(j).toString() << std::endl;
					PlayerTile oPlayerTileAfterEatRight = m_vPossiblePlayerTiles[j];
					oPlayerTileAfterEatRight.eatRight(oDrawnTile);
					vNewPlayerTiles.push_back(oPlayerTileAfterEatRight);
				}
			}
		}
	}

	/*std::cerr << "[Before concat]m_vPossiblePlayerTiles:" << std::endl;
	for (int i = 0; i < m_vPossiblePlayerTiles.size(); i++) {
		std::cerr << m_vPossiblePlayerTiles.at(i).toString() << std::endl;
	}
	std::cerr << "vNewPlayerTiles:" << std::endl;
	for (int i = 0; i < vNewPlayerTiles.size(); i++) {
		std::cerr << vNewPlayerTiles.at(i).toString() << std::endl;
	}*/

	//add new possible PlayerTile to m_vPossiblePlayerTiles
	m_vPossiblePlayerTiles.reserve(m_vPossiblePlayerTiles.size() + vNewPlayerTiles.size());
	m_vPossiblePlayerTiles.insert(m_vPossiblePlayerTiles.end()
		, std::make_move_iterator(vNewPlayerTiles.begin())
		, std::make_move_iterator(vNewPlayerTiles.end()));

	/*std::cerr << "[After]Current possible PlayerTile:" << std::endl;
	for (int i = 0; i < m_vPossiblePlayerTiles.size(); i++) {
		std::cerr << m_vPossiblePlayerTiles.at(i).toString() << std::endl;
	}*/

	m_vDrawnTiles_Order.push_back(vDrawnTilesRound);

	return false;//No PlayerTile is win
}

void GodMoveSamplingData::undoPopTile_fourPlayer()
{
	int size = m_vDrawnTiles_fourPlayers.size();
	for (int i = 0; i < size; i++) {
		if (m_vDrawnTiles_fourPlayers[i] > 0)
			m_oRemainTile.undoPopTile(Tile(i), m_vDrawnTiles_fourPlayers[i], true);
	}
}

int GodMoveSamplingData::_sample_withChunk(const int & iNeedMeldCount)
{
	if (m_oPlayerTile.isWin(iNeedMeldCount))
		return 0;
	m_vPossiblePlayerTiles.clear();
	m_vPossiblePlayerTiles.push_back(m_oPlayerTile);
	resetDrawnTiles();

	int iDrawCount = 0;
	while (iDrawCount < m_iDrawCount) {
		++iDrawCount;
		bool bIsWin = getTile_fourPlayer(iNeedMeldCount);

		if (bIsWin) {
			const int ciPossiblePlayerTileSize = m_vPossiblePlayerTiles.size();
			for (int i = 0; i < ciPossiblePlayerTileSize; i++) {
				if (m_vPossiblePlayerTiles[i].isWin(iNeedMeldCount)) {
					//get thrown tile list
					vector<array<int, MAX_DIFFERENT_TILE_COUNT>> vThrownTilesCount
						= getAllPossibleThrow(m_vPossiblePlayerTiles[i].getHandTile()
							, iNeedMeldCount - m_vPossiblePlayerTiles[i].getMeldCount());

					//pick the best throwing path
					int iBestIndex = 0;
					WinRate_t dMaxAvoidingChunkRate = 0.0;
					for (int j = 0; j < vThrownTilesCount.size(); j++) {
						WinRate_t dAvoidingChunkRate = getAvoidingChunkProbability(vThrownTilesCount[j], m_oRemainTile);
						//std::cerr << "[SamplingData::_sample_withChunk]" << j << ") " << dAvoidingChunkRate << std::endl;
						if (dAvoidingChunkRate > dMaxAvoidingChunkRate) {
							iBestIndex = j;
							dMaxAvoidingChunkRate = dAvoidingChunkRate;
						}
					}

					//random with the probability to decide chunk or not
					//CERR(FormatString("Chunk rate: %lf\n", dMaxAvoidingChunkRate));
					if (hitRate(dMaxAvoidingChunkRate)) {//no chunk -> win
						//CERR("Avoid chunk!\n");
						undoPopTile_fourPlayer();
						return iDrawCount;
					}
					else {//chunk -> delete the player tile
						//CERR("Chunk...\n");
						/*/
						return -1;
						/*/
						m_vPossiblePlayerTiles.erase(m_vPossiblePlayerTiles.begin() + i);
						i--;
						/**/
					}
				}
			}
		}
	}

	//No PlayerTile wins
	undoPopTile_fourPlayer();
	return -1;
}

vector<array<int, MAX_DIFFERENT_TILE_COUNT>> GodMoveSamplingData::getAllPossibleThrow(const HandTile_t & oHandTile, const int & iNeedMeldCount)
{
	vector<array<int, MAX_DIFFERENT_TILE_COUNT>> vPossibleHand = HandTileDecomposer(oHandTile).getAllPossibleWinningPattern(iNeedMeldCount);
	vector<array<int, MAX_DIFFERENT_TILE_COUNT>> vResult(vPossibleHand.size());
	for (int i = 0; i < vPossibleHand.size(); i++) {
		for (int j = 0; j < MAX_DIFFERENT_TILE_COUNT; j++) {
			//assert(oHandTile.getTileNumber(j) >= vPossibleHand[i][j]);
			vResult[i][j] = oHandTile.getTileNumber(j) - vPossibleHand[i][j];
		}
	}

	return vResult;
}

WinRate_t GodMoveSamplingData::getAvoidingChunkProbability(const array<int, MAX_DIFFERENT_TILE_COUNT>& vThrownTileCounts, const RemainTile_t & oRemainTile) const
{
	WinRate_t dAvoidChunkProbability = 1.0;
	const int ciThrownTileCountsSize = vThrownTileCounts.size();
	for (int i = 0; i < ciThrownTileCountsSize; i++) {
		int iTileCount = vThrownTileCounts[i];
		if (iTileCount == 0)
			continue;
		WinRate_t dChunkProbability = oRemainTile.getDangerousFactor(i);
		dAvoidChunkProbability *= pow(1.0 - dChunkProbability, iTileCount);
		//CERR(FormatString("%d", iTileCount));
		//if (i % 9 == 8)
		//	CERR(" ");
	}
	//CERR("\n");
	return dAvoidChunkProbability;
}

void GodMoveSamplingData::resetDrawnTiles()
{
	m_vDrawnTiles_fourPlayers.fill(0);
	m_vDrawnTiles_Order.clear();
}
