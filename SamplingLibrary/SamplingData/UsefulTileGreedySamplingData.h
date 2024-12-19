#pragma once
#include "BaseSamplingData.h"
enum class SamplingType { SamplingType_OnePlayer = 1, SamplingType_FourPlayers = 2 };

class UsefulTileGreedySamplingData : public BaseSamplingData
{
public:
	UsefulTileGreedySamplingData(const Move& oAction, const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iDrawCount);
	UsefulTileGreedySamplingData(const Move& oAction, const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iDrawCount, const SamplingType& oSamplingType);
	UsefulTileGreedySamplingData();
	~UsefulTileGreedySamplingData();

public:
	void sample(const uint32_t& iSampleCount, const int& iNeedMeldCount = NEED_GROUP) override;
	void sample_singlePlayer(const uint32_t& iSampleCount, const int& iNeedMeldCount = NEED_GROUP);
	void sample_fourPlayer(const uint32_t& iSampleCount, const int& iNeedMeldCount = NEED_GROUP);
	uint16_t sampleOnce_singlePlayers(const int& iNeedMeldCount);
	uint16_t sampleOnce_fourPlayers(const int& iNeedMeldCount);
	static Move getGreedyBestMove2(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iNeedMeldCount, const bool& bCanKong);
	static Move getGreedyBestMove1(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iNeedMeldCount, const Tile& oTargetTile, const bool& bCanEat, const bool& bCanKong);

private:
	void _resetDrawnTiles();
	bool _goUntilThrowOrWin(PlayerTile& oPlayerTile, RemainTile_t& oRemainTile, const int& iNeedMeldCount, const bool& bCanKong);

private:
	SamplingType m_oSamplingType;
	array<int, MAX_DIFFERENT_TILE_COUNT> m_vDrawnTiles;
	vector<array<Tile, PLAYER_COUNT>> m_vDrawnTiles_Order;

private:
	//for gtest
	friend class UsefulTileGreedySamplingDataTest_ConstructorTest_Test;
};

