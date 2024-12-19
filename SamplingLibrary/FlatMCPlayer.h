#pragma once
#include "../CommunicationLibrary/BasePlayer.h"
#include "GodMoveSampling_v3.h"

class FlatMCPlayer : public BasePlayer
{
public:
	FlatMCPlayer() : BasePlayer() {};
	~FlatMCPlayer() {};

	void reset() override {};
	inline Tile askThrow() override { return GodMoveSampling_v3(m_oPlayerTile, m_oRemainTile).throwTile(); };
	MoveType askEat(const Tile& oTile) override 
	{
		MoveType oBestMoveType = getBestMeldMove(oTile, true, false);
		if (oBestMoveType == MoveType::Move_EatLeft || oBestMoveType == MoveType::Move_EatMiddle || oBestMoveType == MoveType::Move_EatRight)
			return oBestMoveType;
		return MoveType::Move_Pass;
	};
	inline bool askPong(const Tile& oTile, const bool& bCanEat, const bool& bCanKong) override { return getBestMeldMove(oTile, bCanEat, bCanKong) == MoveType::Move_Pong; };
	inline bool askKong(const Tile& oTile) override { return getBestMeldMove(oTile, false, true) == MoveType::Move_Kong; };
	inline std::pair<MoveType, Tile> askDarkKongOrUpgradeKong() override { return GodMoveSampling_v3(m_oPlayerTile, m_oRemainTile).darkKongOrUpgradeKong(); };

	inline MoveType getBestMeldMove(const Tile& oTile, const bool& bCanEat, const bool& bCanKong) { return GodMoveSampling_v3(m_oPlayerTile, m_oRemainTile).meld(oTile, bCanEat, bCanKong); };
};

