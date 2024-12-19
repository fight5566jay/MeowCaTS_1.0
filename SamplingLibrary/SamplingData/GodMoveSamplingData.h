#pragma once
#include "BaseSamplingData.h"
enum class GetTileType { GetTileType_OnePlayerMo = 1, GetTileType_FourPlayerMo = 2 };

class GodMoveSamplingData : public BaseSamplingData 
{
public:
	GodMoveSamplingData(const Move& oAction, const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iDrawCount) : GodMoveSamplingData(oAction, oPlayerTile, oRemainTile, iDrawCount, GetTileType::GetTileType_FourPlayerMo) {};
	GodMoveSamplingData(const Move& oAction, const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iDrawCount, const GetTileType& oGetTileType);

	GodMoveSamplingData() {};
	~GodMoveSamplingData() {};

	void sample(const uint32_t& iSampleCount, const int& iNeedMeldCount = NEED_GROUP);
	void sample_withChunk(const uint32_t& iSampleCount, const int& iNeedMeldCount = NEED_GROUP);

private:
	int _sample_singlePLayer(const int& iNeedMeldCount);
	int _sample_fourPLayer(const int& iNeedMeldCount);
	bool getTile_fourPlayer(const int& iNeedMeldCount);
	void undoPopTile_fourPlayer();

	int _sample_withChunk(const int& iNeedMeldCount);
	vector<array<int, MAX_DIFFERENT_TILE_COUNT>> getAllPossibleThrow(const HandTile_t& oHandTile, const int& iNeedMeldCount);
	WinRate_t getAvoidingChunkProbability(const array<int, MAX_DIFFERENT_TILE_COUNT>& vThrownTileCounts, const RemainTile_t& oRemainTile) const;
	
	void resetDrawnTiles();

public:
	GetTileType m_oGetTileType;

	//For _sample_singlePLayer
	array<Tile, AVERAGE_MAX_DRAW_COUNT> m_vDrawnTiles;
	int m_iDrawnTileSize;

	//For _sample_fourPlayer
	vector<PlayerTile> m_vPossiblePlayerTiles;
	array<int, MAX_DIFFERENT_TILE_COUNT> m_vDrawnTiles_fourPlayers;
	vector<array<Tile, PLAYER_COUNT>> m_vDrawnTiles_Order;
};