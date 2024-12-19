#pragma once
#include "Ini.h"
class ConfigManager {
public:
	static void loadConfig();

	static bool g_bFirstInit;

	//communication
	static bool g_bUseSocket;

	//mcts
	static uint32_t g_uiSampleCount;
	static uint32_t g_iSampleCountPerRound;
	static bool g_bUseTime;
	static time_t g_uiTimeLimitMs;
	static double g_dExplorationTerm;
	static bool g_bUseImportanceSampling;
	static bool g_bUsePruning;
	static bool g_bMergeAloneTile;
	static bool g_bUseTT;
	static uint32_t g_uiTTType;
	static uint32_t g_uiTTSize;
	static bool g_bKeepSelectionDownThroughTTNode;
	static bool g_bUpdateDeeperTTNode;
	static bool g_bDynamicTrainExplorationTerm;
	static uint32_t g_uiMctsThreadCount;
	static bool g_bWaitReleaseTree;
	static bool g_bSelectBestWinRateCandidate;
	static bool g_bConsiderMeldFactor;
	static uint32_t g_uiUcbFormulaType;
	static double g_fTTWinRateWeight;
	static bool g_bUseFlatMCMeld;
	static bool g_bUseMoveOrdering;
	static bool g_bMoreUsefulChance;

	//sgf
	static bool g_bLogTreeSgf;
};