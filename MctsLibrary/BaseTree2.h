#pragma once
#include <vector>
#include <array>
#include <memory>
#include <bitset>
//#include "../MJLibrary/Base/ConfigManager.h"
#include "../MJLibrary/Base/Ini.h"
#include "../MJLibrary/Base/DebugLogger.h"
#include "../SamplingLibrary/SamplingData/GodMoveSamplingData.h"
//#include "../SamplingLibrary/SamplingData/UsefulTileGreedySamplingData.h"
#include "../MJLibrary/Base/OpenAddressHashTable2.h"
#include "../MJLibrary/MJ_Base/MJStateHashKeyCalculator.h"

using std::vector;
using std::array;
using std::bitset;
typedef float NodeWinCount_t;
typedef float NodeWinRate_t;
typedef GodMoveSamplingData SamplingData;
//typedef UsefulTileGreedySamplingData SamplingData;
class BaseTreeNode2;
typedef OpenAddressHashTable2<BaseTreeNode2*> TT;

class BaseTreeConfig {
public:
	BaseTreeConfig()
		: m_bFirstSetup(true)
		, m_bUseTT(false)
		, m_uiTTSize(0)
		, m_uiTTType(1)
		, m_bUsePruning(false)
		, m_bLogTreeSgf(false) {};
	BaseTreeConfig(Ini& oIni) { loadFromConfigFile(oIni); };
	~BaseTreeConfig() {};

	int loadFromDefaultConfigFile(const bool& bForceLoadConfig = false)
	{
		Ini oIni;
		return loadFromConfigFile(oIni, bForceLoadConfig);
	}

	virtual int loadFromConfigFile(Ini& oIni, const bool& bForceLoadConfig = false)
	{
		if (m_bFirstSetup || bForceLoadConfig) {
			m_bUseTT = oIni.getIntIni("MCTS.UseTT") > 0;
			m_uiTTSize = oIni.getIntIni("MCTS.TTSize");
			m_uiTTType = oIni.getIntIni("MCTS.TTType");
			m_bUsePruning = oIni.getIntIni("MCTS.UsePruning") > 0;
			m_bLogTreeSgf = oIni.getIntIni("Sgf.LogTreeSgf") > 0;
			m_bFirstSetup = false;
		}
		return 0;
	}

	bool m_bFirstSetup;
	bool m_bUseTT;
	uint32_t m_uiTTSize;
	uint32_t m_uiTTType;
	bool m_bUsePruning;
	bool m_bLogTreeSgf;
};

class BaseTreeNodeBasicData {
public:
	BaseTreeNodeBasicData();
	~BaseTreeNodeBasicData() {};

	inline NodeWinRate_t getWinRate(const int& iDrawCount) const { return static_cast<NodeWinRate_t>(m_vNoTTWinCounts.at(iDrawCount)) / m_iNoTTVisitCount; };

public:
	int m_iNoTTVisitCount;
	array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1> m_vNoTTWinCounts;

	//path recording (for computing correct TT draw count)
	array<bitset<MAX_DIFFERENT_TILE_COUNT>, SAME_TILE_COUNT> m_vDrawnTileBitsets;
	array<bitset<MAX_DIFFERENT_TILE_COUNT>, SAME_TILE_COUNT> m_vThrownTileBitsets;
};

class BaseTreeNodeTTData {
public:
	BaseTreeNodeTTData();
	BaseTreeNodeTTData(const SamplingData& oData);
	~BaseTreeNodeTTData() {};

	inline NodeWinRate_t getWinRate(const int& iDrawCount) const { return static_cast<NodeWinRate_t>(m_vTTWinCounts.at(iDrawCount)) / m_iTTVisitCount; };

public:
	SamplingData m_oData;
	int m_iTTVisitCount;
	array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1> m_vTTWinCounts;

};

class BaseTreeNode2
{
public:
	BaseTreeNode2();
	BaseTreeNode2(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const Move & oMove, const int& iDrawCount, const bool& bUseTT, const uint32_t& uiTTType);
	BaseTreeNode2(BaseTreeNode2* pParent, const Move & oMove, const int& iDrawCount);
	BaseTreeNode2(BaseTreeNode2* pParent, const Move & oMove, const int& iDrawCount, BaseTreeNode2* pTTNode);
	~BaseTreeNode2();

public:
	void completeSetup(const bool& bUseTT, const uint32_t uiTTType, const bool& bStoreToTT);
	inline bool isCompleteSetup() const { return m_bCompleteSetup; };
	inline bool isRoot() const { return m_pParent == nullptr; };
	inline bool isLeaf() const { return m_vChildren.empty(); };
	inline bool isTerminal() const { return isWin() || isLose() || isDraw(); };//modified it if you need. (by sctang 20191211)
	inline bool isWin() const { return m_oMove.getMoveType() == MoveType::Move_WinBySelf || m_oMove.getMoveType() == MoveType::Move_WinByOther; };
	inline bool isLose() const { return m_iDrawCount == 0; };
	inline bool isDraw() const { return false; };
	inline int getChildId() const { return m_iChildId; };
	inline bool existTTNode() const { return m_pTTNode != this; };
	inline BaseTreeNode2* getTTNode() const { return m_pTTNode; };
	inline BaseTreeNodeTTData* getTTData() const {
		//return existTTNode() ? m_pTTNode->m_pTTData.get() : m_pTTData.get();
		return m_pTTNode->m_pTTData.get();//same as above
	};

	inline Move getMove() const { return m_oMove; };
	string getSgfString() const;
	virtual string getSgfCommand() const;
	inline string getMyCommand() const { return m_sAdditionalCommand; };
	inline void addMyCommand(const string& sCommand) { m_sAdditionalCommand.append(sCommand); };

	void addWinCountTT(const array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1>& vWinCounts, const int& iBias = 0);
	void addWinCountNoTT(const array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1>& vWinCounts, const int& iBias = 0);
	inline int getChildrenCount() const { return m_vChildren.size(); };
	inline const BaseTreeNode2* getChildNodePtr(const int& i) const { return m_vChildren.at(i).get(); };
	inline SamplingData& getSamplingData() const {	return getTTData()->m_oData; };
	inline void clearSamplingData() { getTTData()->m_oData.clearData(); }
	inline void sample(const int& iSampleCount) { getTTData()->m_oData.sample(iSampleCount); }
	inline int getMinLack() const { return m_iMinLack; };//return the pre-computed min lack

	inline int getVisitCountTT() const { return getTTData()->m_iTTVisitCount; };
	inline void addVisitCountTT(const int& iVisitCount) { getTTData()->m_iTTVisitCount += iVisitCount; };
	inline NodeWinCount_t getWinCountTT(const int& iDrawCount) const { return getTTData()->m_vTTWinCounts.at(iDrawCount); };
	inline NodeWinCount_t getWinCountTT() const { return getWinCountTT(m_iDrawCount); };
	inline array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1>& getWinCountsTT() const { return getTTData()->m_vTTWinCounts; };
	inline NodeWinRate_t getWinRateTT(const int& iDrawCount) const { return getTTData()->getWinRate(iDrawCount);	};
	inline NodeWinRate_t getWinRateTT() const { return getWinRateTT(m_iDrawCount); };

	inline int getVisitCountNoTT() const { return m_pBasicData? m_pBasicData->m_iNoTTVisitCount : 0; };
	inline void addVisitCountNoTT(const int& iVisitCount) { m_pBasicData->m_iNoTTVisitCount += iVisitCount; };
	inline NodeWinCount_t getWinCountNoTT(const int& iDrawCount) const { return m_pBasicData? m_pBasicData->m_vNoTTWinCounts.at(iDrawCount) : 0; };
	inline NodeWinCount_t getWinCountNoTT() const { return getWinCountNoTT(m_iDrawCount); };
	inline array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1>& getWinCountsNoTT() const { return m_pBasicData->m_vNoTTWinCounts; };
	inline NodeWinRate_t getWinRateNoTT(const int& iDrawCount) const { return m_pBasicData->getWinRate(iDrawCount); };
	inline NodeWinRate_t getWinRateNoTT() const { return getWinRateNoTT(m_iDrawCount); };

	inline int getDrawCount() const { return m_iDrawCount; };
	inline int getSampleDrawCount() const { return getTTData()->m_oData.m_iDrawCount; };
	inline array<bitset<MAX_DIFFERENT_TILE_COUNT>, SAME_TILE_COUNT>& getDrawnTileBitsets() { return m_pBasicData->m_vDrawnTileBitsets; };
	inline array<bitset<MAX_DIFFERENT_TILE_COUNT>, SAME_TILE_COUNT>& getThrownTileBitsets() { return m_pBasicData->m_vThrownTileBitsets; };
	inline bitset<MAX_DIFFERENT_TILE_COUNT>& getDrawnTileBitset(const int& i) { return m_pBasicData->m_vDrawnTileBitsets.at(i); };
	inline bitset<MAX_DIFFERENT_TILE_COUNT>& getThrownTileBitset(const int& i) { return m_pBasicData->m_vThrownTileBitsets.at(i); };

	NodeWinRate_t getErrorRangeNoTT(const uint32_t& uiStdDevCount) const;
	string getPathString() const;

	//Transposition Table related
	inline HashKey CalHashKey(const SamplingData& oData, const uint32_t uiTTType) const {
		if (uiTTType == 1) {
			return MJStateHashKeyCalculator::calHashKey(oData.m_oPlayerTile, oData.m_iDrawCount);
		}
		else if (uiTTType == 2) {
			return MJStateHashKeyCalculator::calHashKey(oData.m_oPlayerTile);
		}
		return MJStateHashKeyCalculator::calHashKey(oData.m_oPlayerTile);
	};
	inline bool isEquilaventState(const SamplingData& oData1, const SamplingData& oData2) const {
		return oData1.m_oPlayerTile == oData2.m_oPlayerTile/* && oData1.m_iDrawCount == oData2.m_iDrawCount && pNode1->m_oData.m_oRemainTile == pNode2->m_oData.m_oRemainTile*/;
	};

protected:
	void _initTTData(const SamplingData& oData, const bool& bUseTT, const uint32_t uiTTType, const bool& bStoreToTT);
	void _setupSampleDrawCount();

private:
	void _setupMinLack(PlayerTile& oPlayerTile, const Move& oMove);
	void _initPathBitset();
	void _setupBitset();
	int _computeSampleDrawCount();

public:
	BaseTreeNode2* m_pParent;
	BaseTreeNode2* m_pSibling;
	vector<std::shared_ptr<BaseTreeNode2>> m_vChildren;
	int m_iChildId;

	//for pruning
	bool m_bIsPruned;

protected:
	Move m_oMove;//need to independent from TT data(the move could be different to m_pTTNode)
	int m_iDrawCount;
	string m_sAdditionalCommand;//for Sgf
	BaseTreeNode2* m_pTTNode;//if TT node is not exist, pTTNode will point to itself (pTTNode = this)
	vector<BaseTreeNode2*> m_vOtherTTNodes;//record whose m_pTTNode point to this node (for debugging)

	bool m_bCompleteSetup;
	std::shared_ptr<BaseTreeNodeBasicData> m_pBasicData;
	std::shared_ptr<BaseTreeNodeTTData> m_pTTData;

	//additional
	int m_iMinLack;
};

class BaseTree2 {
public:
	BaseTree2() : m_pRootNode(nullptr) {};
	BaseTree2(const SamplingData& oData, const BaseTreeConfig& oConfig, const bool& bStoreConfig);
	~BaseTree2() {};

public:
	void init(const SamplingData& oData);
	void reset() { m_pRootNode.reset(); };
	const BaseTreeNode2* getRootNodePtr() const { return m_pRootNode.get(); };
	void writeToSgf(const string sAppendName = string());

protected:
	void _init(const SamplingData& oData, const BaseTreeConfig& oConfig, const bool& bStoreConfig);
	void _initConfig(const BaseTreeConfig& oConfig);
	void _setupTT();

protected:
	std::shared_ptr<BaseTreeNode2> m_pRootNode;
	//static bool g_bFirstSetup;
	//static bool g_bUseTT;
	//BaseTreeConfig m_oConfig;
	std::shared_ptr<BaseTreeConfig> m_pBaseConfig;
};

