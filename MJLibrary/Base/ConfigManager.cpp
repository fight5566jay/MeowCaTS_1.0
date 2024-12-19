#include "ConfigManager.h"
bool ConfigManager::g_bFirstInit = true;
bool ConfigManager::g_bUseSocket = false;
uint32_t ConfigManager::g_uiSampleCount = 0;
uint32_t ConfigManager::g_iSampleCountPerRound = 1;
bool ConfigManager::g_bUseTime = false;
time_t ConfigManager::g_uiTimeLimitMs = 2500;
double ConfigManager::g_dExplorationTerm = 0.1;
bool ConfigManager::g_bUseImportanceSampling = false;
bool ConfigManager::g_bUsePruning = false;
bool ConfigManager::g_bMergeAloneTile = true;
bool ConfigManager::g_bUseTT = false;
uint32_t ConfigManager::g_uiTTType = 1;
uint32_t ConfigManager::g_uiTTSize = 21;
bool ConfigManager::g_bKeepSelectionDownThroughTTNode = false;
bool ConfigManager::g_bUpdateDeeperTTNode = false;
bool ConfigManager::g_bDynamicTrainExplorationTerm = false;
uint32_t ConfigManager::g_uiMctsThreadCount = 0;
bool ConfigManager::g_bWaitReleaseTree = false;
bool ConfigManager::g_bSelectBestWinRateCandidate = false;
bool ConfigManager::g_bConsiderMeldFactor = true;
uint32_t ConfigManager::g_uiUcbFormulaType = 1;
double ConfigManager::g_fTTWinRateWeight = 0.1;
bool ConfigManager::g_bUseFlatMCMeld = false;
bool ConfigManager::g_bUseMoveOrdering = true;
bool ConfigManager::g_bLogTreeSgf = false;
bool ConfigManager::g_bMoreUsefulChance = false;

void ConfigManager::loadConfig()
{
	if (!g_bFirstInit)
		return;

	Ini& oIni = Ini::getInstance();

	//communication
	g_bUseSocket = oIni.getIntIni("Socket.UseSocket") > 0;

	//mcts
	g_uiSampleCount = oIni.getIntIni("MCTS.TotalSampleCount");
	g_iSampleCountPerRound = oIni.getIntIni("MCTS.SampleCountPerRound");
	g_bUseTime = oIni.getIntIni("MCTS.UseTime") > 0;
	g_uiTimeLimitMs = oIni.getIntIni("MCTS.TimeLimitMs");
	//g_dExplorationTerm = oIni.getDoubleIni("MCTS.ExplorationTerm"); //use explorationTerm table
	g_bUseImportanceSampling = oIni.getIntIni("MCTS.UseImportanceSampling") > 0;
	g_bUsePruning = oIni.getIntIni("MCTS.UsePruning") > 0;
	g_bMergeAloneTile = oIni.getIntIni("MCTS.MergeAloneTile") > 0;
	g_bUseTT = oIni.getIntIni("MCTS.UseTT") > 0;
	g_uiTTType = oIni.getIntIni("MCTS.TTType");
	g_uiTTSize = oIni.getIntIni("MCTS.TTSize");
	g_bKeepSelectionDownThroughTTNode = oIni.getIntIni("MCTS.KeepSelectionDownThroughTTNode") > 0;
	g_bUpdateDeeperTTNode = oIni.getIntIni("MCTS.UpdateDeeperTTNode") > 0;
	g_bDynamicTrainExplorationTerm = oIni.getIntIni("MCTS.DynamicTrainExplorationTermTable") > 0;
	g_uiMctsThreadCount = oIni.getIntIni("MCTS.ThreadNum");
	g_bWaitReleaseTree = oIni.getIntIni("MCTS.WaitReleaseTree") > 0;
	g_bSelectBestWinRateCandidate = oIni.getIntIni("MCTS.SelectBestWinRateCandidate") > 0;
	g_uiUcbFormulaType = oIni.getIntIni("MCTS.UcbFormulaType");
	g_fTTWinRateWeight = oIni.getDoubleIni("MCTS.TTWinRateWeight");
	g_bConsiderMeldFactor = oIni.getIntIni("MCTS.ConsiderMeldFactor") > 0;
	g_bUseFlatMCMeld = oIni.getIntIni("MCTS.UseFlatMCMeld") > 0;
	g_bUseMoveOrdering = oIni.getIntIni("MCTS.UseMoveOrdering") > 0;
	g_bMoreUsefulChance = oIni.getIntIni("MCTS.MoreUsefulChance") > 0;

	//sgf
	g_bLogTreeSgf = oIni.getIntIni("Sgf.LogTreeSgf") > 0;

	g_bFirstInit = false;
}
