#pragma once
#include "BaseTree2.h"

class ChanceNode2 :	public BaseTreeNode2
{
public:
	ChanceNode2() : BaseTreeNode2() {};//do not use this constructor
	ChanceNode2(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const Move & oMove, const int& iDrawCount, const bool& bUseTT, const uint32_t& uiTTType);
	ChanceNode2(BaseTreeNode2* pParent, const Move & oMove, const int& iDrawCount);
	ChanceNode2(BaseTreeNode2* pParent, const Move & oMove, const int& iDrawCount, BaseTreeNode2* pTTNode);
	~ChanceNode2() {};

public:
	//inline double getChance() const { return m_dChance; };
	inline NodeWinRate_t getChance(const int& i) const { return m_vChances.at(i); };
	inline NodeWinRate_t getWeight(const int& i) const { return m_vWeights.at(i); };
	void addChildNode(std::shared_ptr<BaseTreeNode2> pChildNode);
	void addChildNodeWithChance(std::shared_ptr<BaseTreeNode2> pChildNode, const NodeWinRate_t& dChance);
	string getSgfCommand() const override;

	void setWeightWithModifiedChance(const vector<NodeWinRate_t>& vModifiedChance);
	static array<NodeWinRate_t, MAX_DIFFERENT_TILE_COUNT> getChanceConsideringMeldFactor(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile);

public:
	vector<NodeWinRate_t> m_vChances;
	vector<NodeWinRate_t> m_vWeights;
};

