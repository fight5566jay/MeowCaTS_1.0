#pragma once
#include "BaseTree2.h"
#include "ChanceNode2.h"
#include "ExplorationTermTable.h"
#include "../MJLibrary/Base/ReserviorOneSampler.h"

typedef BaseTreeNode2 TreeNode;
typedef TreeNode* TreeNodePtr;
typedef vector<TreeNodePtr> TreeNodePtrList;
typedef ChanceNode2* ChanceNodePtr;
enum class NodeEquilvalenceInPath2 { EquilaventToAncientNode, TTEquilaventToAncientNode, DifferentNode };

class MctsConfig : public BaseTreeConfig
{
public:
	MctsConfig()
		: BaseTreeConfig()
		, m_iSampleCountPerRound(10)
		, m_bMergeAloneTile(false)
		, m_bUseImportanceSampling(false)
		, m_bKeepSelectionDownThroughTTNode(false)
		, m_bUpdateDeeperTTNode(false)
		, m_bStopSelectionIfRunOutOfDrawCount(true)
		, m_bConsiderMeldFactor(true)
		, m_bUseMoveOrdering(true)
		, m_uiUcbFormulaType(2)
		, m_fTTWinRateWeight(1.0)
		, m_fExplorationTerm(0.1) {};
	MctsConfig(Ini& oIni) { loadFromConfigFile(oIni); };
	MctsConfig(const MctsConfig& oConfig) { *this = oConfig; };
	~MctsConfig() {};

	virtual int loadFromConfigFile(Ini& oIni, const bool& bForceLoadConfig = false) override
	{
		if (m_bFirstSetup || bForceLoadConfig) {
			BaseTreeConfig::loadFromConfigFile(oIni, bForceLoadConfig);
			m_iSampleCountPerRound = oIni.getIntIni("MCTS.SampleCountPerRound");
			m_bMergeAloneTile = oIni.getIntIni("MCTS.MergeAloneTile") > 0;
			m_bUseImportanceSampling = oIni.getIntIni("MCTS.UseImportanceSampling") > 0;
			m_bKeepSelectionDownThroughTTNode = oIni.getIntIni("MCTS.KeepSelectionDownThroughTTNode") > 0;
			m_bUpdateDeeperTTNode = oIni.getIntIni("MCTS.UpdateDeeperTTNode") > 0;
			m_bStopSelectionIfRunOutOfDrawCount = true;//[TODO]
			m_bConsiderMeldFactor = oIni.getIntIni("MCTS.ConsiderMeldFactor") > 0;
			m_bUseMoveOrdering = oIni.getIntIni("MCTS.UseMoveOrdering") > 0;
			m_uiUcbFormulaType = oIni.getIntIni("MCTS.UcbFormulaType");
			m_fTTWinRateWeight = oIni.getDoubleIni("MCTS.TTWinRateWeight");
			m_fExplorationTerm = oIni.getDoubleIni("MCTS.ExplorationTerm");
			m_bFirstSetup = false;
		}
		return 0;
	}

	int m_iSampleCountPerRound;
	bool m_bMergeAloneTile;
	bool m_bUseImportanceSampling;
	bool m_bKeepSelectionDownThroughTTNode;
	bool m_bUpdateDeeperTTNode;
	bool m_bStopSelectionIfRunOutOfDrawCount;
	bool m_bConsiderMeldFactor;
	bool m_bUseMoveOrdering;
	uint32_t m_uiUcbFormulaType;
	NodeWinRate_t m_fTTWinRateWeight;
	NodeWinRate_t m_fExplorationTerm;
};

class Mcts2 : public BaseTree2
{
public:
	Mcts2() {};
	/*
	Mcts2(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iDrawCount);
	Mcts2(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iDrawCount, const vector<Move>& vLegalMoves);
	Mcts2(const SamplingData& oData);
	*/
	Mcts2(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iDrawCount, const MctsConfig& oConfig, const bool& bStoreConfig);
	Mcts2(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iDrawCount, const vector<Move>& vLegalMoves, const MctsConfig& oConfig, const bool& bStoreConfig);
	Mcts2(const SamplingData& oData, const MctsConfig& oConfig, const bool& bStoreConfig);
	~Mcts2() {};

public:
	void init(const SamplingData & oData);
	void reset();
	//inline void reInit(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iDrawCount, const vector<Move>& vLegalMoves) { reInit(SamplingData(Move(), oPlayerTile, oRemainTile, max(iDrawCount, oPlayerTile.getMinLack()))); };
	void setupCandidateNodes(const vector<Move>& vMoves);
	void sample(const int& iSampleCount = 1);
	void sampleUntilTimesUp(const time_t& iEndTime);
	//void printPtrUsage() const;
	Move getBestMove() const;
	Move getBestMeldMove() const;
	inline uint32_t getUsedSampleCount() const { return m_pRootNode->getVisitCountNoTT(); };
	void writeToSgf(const string sAppendName = string());
	inline void printChildrenResult() const { printChildrenResult(std::cout); };//debug
	void printChildrenResult(std::ostream& out) const;
	void loadDefaultConfig(const bool& bForceLoadConfig = false);

	static vector<Move> getLegalMoves(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const bool& bCanDarkKongOrUpgradeKong, const bool& bCanWin, const bool bMergeAloneTile);

	inline MctsConfig& getConfig() const { return *static_cast<MctsConfig*>(m_pBaseConfig.get()); };

protected:
	TreeNodePtr select();
	inline TreeNodePtr expand(TreeNodePtr pNode) { return expand(pNode, getLegalMoves(pNode->getSamplingData().m_oPlayerTile, pNode->getSamplingData().m_oRemainTile, true, true, getConfig().m_bMergeAloneTile)); };
	TreeNodePtr expand(TreeNodePtr pNode, const vector<Move> vLegalMoves);
	array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1> simulate(TreeNodePtr pNode, const int& iSampleCount);
	void backPropagate(TreeNodePtr pNode, const array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1>& vWinCount, const int& iSampleCount);
	TreeNodePtr expandMeldMove(TreeNodePtr pNode, const vector<Move> vLegalMoves);

	TreeNodePtr getBestChild(const TreeNodePtr pParent);
	NodeWinRate_t getUcbValue(const TreeNodePtr pNode) const;
	TreeNodePtr select_chance(const TreeNodePtr pNode);
	NodeEquilvalenceInPath2 isEquivalantToAncient(const int& iIndex) const;

	void _init(const SamplingData& oData, const MctsConfig& oConfig, const bool& bStoreConfig);
	void _initConfig(const MctsConfig& oConfig);
	
	//void _init_FirstSetup() { loadDefaultConfig(); };
	bool isMaxNode(const TreeNodePtr pNode) const;
	bool isChanceNode(const TreeNodePtr pNode) const;

	//path control
	void pushNodeToPath(const TreeNodePtr pNode);
	void popNodeFromPath();
	void clearPath();
	
	//allocate new node
	void allocNewMaxNode(TreeNodePtr pParent, const Move& oMove, const int& iDrawCount);
	void allocNewChanceNode(TreeNodePtr pParent, const Move& oMove, const int& iDrawCount);
	void allocNewChanceNode(TreeNodePtr pParent, const Move& oMove, const int& iDrawCount, const TreeNodePtr pTTNode);

public:
	ExplorationTerm_t m_fExplorationTerm;
	uint32_t g_uiStdDevCount;
	//MctsConfig m_oConfig;

private:
	//MctsConfig* m_pMctsConfig;
	uint32_t m_iLeftCandidateCount;
	
	//KeepSelectionDownThroughTTNode
	vector<TreeNodePtr> m_vPath;
	uint32_t m_uiLeftDrawCount;

	ReserviorOneSampler<TreeNodePtr> oReserviorOneSampler;

};

