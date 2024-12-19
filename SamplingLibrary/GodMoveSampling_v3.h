#pragma once
#include <array>
#include "SamplingData/GodMoveSamplingData.h"
#include "../MJLibrary/Base/SgfManager.h"
#include "DrawCountTable.h"
using std::array;
typedef GodMoveSamplingData SamplingDataType;
typedef GetTileType GetTileTypeTmp;

class GodMoveSampling_v3
{
public:
	enum class SampleMode { Mode_Time, Mode_Count };
private:
	static array<array<int, MAX_MINLACK + 1>, MAX_DRAW_COUNT + 1> g_vDrawCountTable;			// [remain draw times], [min lack]
	static array<array<int, MAX_MINLACK + 1>, MAX_DRAW_COUNT + 1> g_vDrawCountTable_v2;

public:
	GodMoveSampling_v3(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const int& iLeftDrawCount = -1);
	GodMoveSampling_v3(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile, const SampleMode& oSampleMode, const uint32_t& llSampleParameter, const int& iLeftDrawCount = -1);
	GodMoveSampling_v3() {};
	~GodMoveSampling_v3() {};

	//public function
	Tile throwTile(const bool& bUseTooManyMeldStrategy = true, const bool& bConsideringChunk = true, const bool& bUseUcb = false);
	vector<SamplingDataType> throwTile_detail(/*const bool& bUseTooManyMeldStrategy = true*/);
	vector<SamplingDataType> throwTile_detail(vector<Tile>& vThrownTiles/*, const bool& bUseTooManyMeldStrategy = true*/);
	vector<SamplingDataType> sampleTheseMove(vector<Move>& vMoves);
	MoveType meld(const Tile& oTile, const bool& bCanEat, const bool& bCanKong, const bool& bUseTooManyMeldStrategy = true, const bool& bConsideringChunk = true);
	vector<SamplingDataType> meld_detail(const Tile& oTile, const bool& bCanEat, const bool& bCanKong);
	std::pair<MoveType, Tile> darkKongOrUpgradeKong(const bool& bUseTooManyMeldStrategy = true, const bool& bConsideringChunk = true);
	vector<SamplingDataType> darkKongOrUpgradeKong_detail();
	//Tile throwTile_TooManyMelds();//old method
	Move tooManyMeldsStrategy();

	//NOTICE: getGoodEnoughThrow() should be called after throwTile(), meld() or darkKongOrUpgradeKong().
	vector<Move> getGoodEnoughMove() const;

	//testing
	MoveType meld_v2(const Tile& oTile, const bool& bCanEat, const bool& bCanKong, const bool& bUseTooManyMeldStrategy = true, const bool& bConsideringChunk = true);
	vector<SamplingDataType> meld_v2_detail(const Tile& oTile, const bool& bCanEat, const bool& bCanKong);
	void makeMeldSamplingData_method2(const Tile& oTile, const MoveType& oMakeMeldType);
	void testTime_PP();

	//void tuneDrawCountTable(const string& sFileName, const int& iIterationCount);
	//void loadDrawCountTable(const string& sFileName);
	//bool checkValidDrawCountTableVersion(ifstream& fin);
	//void initRandomState(const int& iHandTileNumber);

public:
	//member function
	void init();
	void reset();
	SamplingDataType makeMoveDecision(const bool& bConsideringChunk = true);
	SamplingDataType makeMoveDecisionWithChunkAvoidingType1();
	SamplingDataType makeMoveDecisionWithChunkAvoidingType2();
	SamplingDataType makeMoveDecision_UCB(const bool& bConsideringChunk = true);
	void sampleAllCandidates();
	void sampleAllCandidatesWithChunkAvoiding();
	void sampleOneCandidate();
	void sampleAllCandidatesNoPruning();
	static void sampleCandidate(SamplingDataType& oData, const unsigned int& iSampleCount);
	static void sampleCandidateWithChunkAvoiding(SamplingDataType& oData, const unsigned int& iSampleCount);
	void prune();
	SamplingDataType getBestCandidate();
	SamplingDataType getBestCandidateWithChunkAvoiding();
	void makeMeldSamplingData(const Tile& oTile, const MoveType& oMakeMeldType);
	void selectionSortCandidate();
	void selectionSortCandidate(const int& iMoCount);
	bool isTooManyMelds() const;
	int getAvoidingChunkType() const;
	void printCurrentCandidate() const;
	void printSetting() const;
	//static SamplingDataType getBestThrowWinRate(const PlayerTile& oPlayerTile, const RemainTile oRemainTile, const SampleMode oSampleMode, const uint32_t& cullSampleParam, const int& iLeftDrawCount);
	static std::pair<SamplingDataType, uint32_t> getBestThrowWinRate(const PlayerTile& oPlayerTile, const RemainTile_t oRemainTile, const SampleMode oSampleMode, const uint32_t& cullSampleParam, const int& iLeftDrawCount);
	//get
	inline PlayerTile getPlayerTile() const { return m_oPlayerTile; }
	inline uint32_t getUsedSampleCount() const { return m_uiUsedSampleCount; }
	inline vector<SamplingDataType> getCandidates() const { return m_vCandidates; }
	inline uint32_t getSampleParameter() const { return m_uiSampleParameter; }

	//sub
	void makeWinRateTimeStamp();

	//ucb(test)
	int getUcbBestDataIndex() const;
	WinRate_t getUcbScore(const SamplingDataType& oData) const;

public:
	//for experiment
	uint32_t m_iSampleCountG1D1;
	uint32_t m_iSampleCountG1D2;
	uint32_t m_iSampleCountG2D1;
	uint32_t m_iSampleCountG2D2;
	//time_t sampleUsedTime;

private:
	PlayerTile m_oPlayerTile;
	RemainTile_t m_oRemainTile;
	int m_iLeftDrawCount;
	uint32_t m_uiUsedSampleCount;
	time_t m_uiStartTime;
	int m_iUsedSDCount;
	DrawCountTable m_oDrawCountTable;

	vector<SamplingDataType> m_vCandidates;
	bool m_bFirstRound;
	int m_iLeftCandidateCount;

	//options (read from ini file)
	static bool m_bFirstSetup;
	static SampleMode m_oSampleMode;
	static uint32_t m_uiSampleParameter;
	static uint32_t m_uiSampleFirstRound;
	static uint32_t m_uiSamplePerRound;
	static bool m_bUseMultiThread;
	static bool m_bPruning;
	static bool m_bUseTooManyMeldsStrategy;
	static bool m_bUseRule2;
	static bool m_bUseRule2FastReturn;
	static bool m_bUseDepth1FastReturn;
	static bool m_bPrintDebugMsg;

	//0: No considering chunk, 1: Considering chunk at final decision only, 2: Considering chunk during sampling
	static int m_iConsideringChunkType;
	static bool m_bNoConsideringChunkAtBeginning;
	static GetTileTypeTmp m_oGetTileType;

	SgfManager m_oSgfManager;

};
