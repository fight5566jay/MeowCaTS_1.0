#pragma once
#include "Mcts2.h"
#include "../CommunicationLibrary/BasePlayer.h"
#include "../SamplingLibrary/DrawCountTable.h"
#include "ExplorationTermTable.h"
#include <future>
#include <mutex>
#include "../SamplingLibrary/GodMoveSampling_v3.h"
#define MCTS_LIST_COUNT 4
typedef Mcts2 Mcts_t;
using std::array;
using std::vector;
using std::future;

class Candidate {
public:
	inline NodeWinCount_t getWinCount() const { return m_vWinCounts.at(m_uiDrawCount); };
	inline WinRate_t getWinRate() const { return getWinRate(m_uiDrawCount); };
	inline WinRate_t getWinRate(const int& iDrawCount) const { return m_vWinCounts.at(iDrawCount) / m_uiVisitCount; };

	Move m_oMove;
	string m_sMove;//for print result
	uint32_t m_uiDrawCount;
	uint32_t m_uiVisitCount;
	array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1> m_vWinCounts;
	uint32_t m_uiVote;
};

class MctsPlayerConfig : public MctsConfig
{
public:
	MctsPlayerConfig() : MctsConfig() {};
	MctsPlayerConfig(Ini& oIni) { loadFromConfigFile(oIni); };
	~MctsPlayerConfig() {};
	
	int loadFromConfigFile(Ini& oIni, const bool& bForceLoadConfig = false) override
	{
		MctsConfig::loadFromConfigFile(oIni, bForceLoadConfig);
		m_uiSampleCount = oIni.getIntIni("MCTS.TotalSampleCount");
		m_uiMctsThreadCount = oIni.getIntIni("MCTS.ThreadNum");
		m_bDynamicTrainExplorationTerm = oIni.getIntIni("MCTS.DynamicTrainExplorationTermTable") > 0;
		m_bUseTime = oIni.getIntIni("MCTS.UseTime") > 0;
		m_uiTimeLimitMs = oIni.getIntIni("MCTS.TimeLimitMs");
		m_bWaitReleaseTree = oIni.getIntIni("MCTS.WaitReleaseTree") > 0;
		m_bUseFlatMCMeld = oIni.getIntIni("MCTS.UseFlatMCMeld") > 0;
		m_bSelectBestWinRateCandidate = oIni.getIntIni("MCTS.SelectBestWinRateCandidate") > 0;
		return 0;
	}

	uint32_t m_uiSampleCount;
	uint32_t m_uiMctsThreadCount;
	bool m_bDynamicTrainExplorationTerm;
	bool m_bUseTime;
	time_t m_uiTimeLimitMs;
	bool m_bWaitReleaseTree;
	bool m_bUseFlatMCMeld;
	bool m_bSelectBestWinRateCandidate;
};

class MctsPlayer : public BasePlayer
{
public:
	MctsPlayer();
	MctsPlayer(const uint32_t uiSampleCount);
	MctsPlayer(const uint32_t uiSampleCount, const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile);
	MctsPlayer(const MctsPlayerConfig& oPlayerConfig);
	MctsPlayer(const uint32_t uiSampleCount, const MctsPlayerConfig& oPlayerConfig);
	MctsPlayer(const uint32_t uiSampleCount, const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const MctsPlayerConfig& oPlayerConfig);
	~MctsPlayer() {};

	void init();
	void reset() override;

	Tile askThrow() override;
	MoveType askEat(const Tile& oTile) override;
	bool askPong(const Tile& oTile, const bool& bCanChow, const bool& bCanKong) override;
	bool askKong(const Tile& oTile) override;
	std::pair<MoveType, Tile> askDarkKongOrUpgradeKong() override;
	uint32_t getUsedSampleCount() const;

	//additional
	inline void resetup(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile) {
		m_oPlayerTile = oPlayerTile;
		m_oRemainTile = oRemainTile;
	}

protected:
	void initAllMctsLists();
	MoveType getBestMeldMove(const Tile& oTile, const bool& bCanEat, const bool& bCanKong);
	inline MoveType getBestMeldMove_FlatMC(const Tile& oTile, const bool& bCanEat, const bool& bCanKong) { return GodMoveSampling_v3(m_oPlayerTile, m_oRemainTile).meld(oTile, bCanEat, bCanKong); };
	void runMcts();
	void runMcts_multiThread();
	static void sample(vector<Mcts_t>& vMctsList, const int& iThreadId, const int& iSampleCount, vector<Candidate>& g_vCandidates);
	static void sampleUntilTimesUp(vector<Mcts_t>& vMctsList, const int& iThreadId, const time_t& iEndTime, vector<Candidate>& g_vCandidates);
	static void resetMctsList(array<vector<Mcts_t>, MCTS_LIST_COUNT>& vMctsLists, const int& iListIndex);
	static void resetMcts(vector<Mcts_t>& vMctsList, const int& iTreeIndex);
	void waitPreviousMctsReleased();
	vector<Mcts_t>& getCurrentMctsList() { return m_vMctsLists[m_iCurrentMctsListIndex]; };
	void initMctsCandidates();
	Move getBestMove();
	void printInfo(const string& sFunctionName);
	void trainExplorationTerm();

	vector<Move> getMeldMoveList(const Tile& oTargetTile, const bool& bCanEat, const bool& bCanKong) const;
	static int getDrawCount(const int& iOriginDrawCount, const Move& oMove);

private:
	//[experiment]
	void analyzeAloneTileRate(const Tile& oPopTile) const;

protected:
	DrawCountTable m_oDrawCountTable;

	//static vector<Mcts_t> g_vMcts;
	array<vector<Mcts_t>, MCTS_LIST_COUNT> m_vMctsLists;
	array<future<void>, MCTS_LIST_COUNT> m_vMctsResetHandlers;
	uint32_t m_iCurrentMctsListIndex;
	vector<Move> m_vLegalMoves;
	int m_iDrawCount;
	//uint32_t m_uiUsedSampleCount;

	MctsPlayerConfig m_oConfig;
	//dynamic adjust exploration term related
	//bool m_bDynamicTrainExplorationTerm;
	//bool m_bUseTime;

	//multi-thread
	vector<Candidate> m_vCandidates;
};

