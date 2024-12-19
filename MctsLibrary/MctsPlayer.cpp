#include "MctsPlayer.h"
//#include "../MJLibrary/Base/ConfigManager.h"

//vector<Mcts_t> MctsPlayer::g_vMcts;
/*
vector<Mcts_t> MctsPlayer::m_vMctsLists[MCTS_LIST_COUNT];
future<void> MctsPlayer::m_vMctsResetHandlers[MCTS_LIST_COUNT];
uint32_t MctsPlayer::m_iCurrentMctsListIndex = 0;
vector<Candidate> MctsPlayer::m_vCandidates;
*/

MctsPlayer::MctsPlayer()
{
	m_oConfig.loadFromDefaultConfigFile();
	initAllMctsLists();
}

MctsPlayer::MctsPlayer(const uint32_t uiSampleCount)
{
	m_oConfig.loadFromDefaultConfigFile();
	initAllMctsLists();
	m_oConfig.m_uiSampleCount = uiSampleCount;
}

MctsPlayer::MctsPlayer(const uint32_t uiSampleCount, const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile)
{
	m_oConfig.loadFromDefaultConfigFile();
	initAllMctsLists();
	m_oConfig.m_uiSampleCount = uiSampleCount;
	m_oPlayerTile = oPlayerTile;
	m_oRemainTile = oRemainTile;
}

MctsPlayer::MctsPlayer(const MctsPlayerConfig& oPlayerConfig) : m_oConfig(oPlayerConfig)
{
	initAllMctsLists();
}

MctsPlayer::MctsPlayer(const uint32_t uiSampleCount, const MctsPlayerConfig& oPlayerConfig) : m_oConfig(oPlayerConfig)
{
	initAllMctsLists();
	m_oConfig.m_uiSampleCount = uiSampleCount;
}

MctsPlayer::MctsPlayer(const uint32_t uiSampleCount, const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const MctsPlayerConfig& oPlayerConfig) : m_oConfig(oPlayerConfig)
{
	initAllMctsLists();
	m_oConfig.m_uiSampleCount = uiSampleCount;
	m_oPlayerTile = oPlayerTile;
	m_oRemainTile = oRemainTile;
}

void MctsPlayer::init()
{

	//m_oConfig.loadDefaultConfig();
	initAllMctsLists();
	//m_oConfig.m_bDynamicTrainExplorationTerm = m_oConfig.m_bDynamicTrainExplorationTerm;
	//m_oConfig.m_bUseTime = m_oConfig.m_bUseTime;
}

void MctsPlayer::reset()
{
	m_vMctsResetHandlers[m_iCurrentMctsListIndex] = std::async(std::launch::async, resetMctsList, std::ref(m_vMctsLists), m_iCurrentMctsListIndex);
	if (m_oConfig.m_bWaitReleaseTree) {
		//wait for reset handler
		assert(m_vMctsResetHandlers[m_iCurrentMctsListIndex].valid());
		m_vMctsResetHandlers[m_iCurrentMctsListIndex].get();
	}

	m_iCurrentMctsListIndex = (m_iCurrentMctsListIndex == MCTS_LIST_COUNT - 1)? 0 : m_iCurrentMctsListIndex + 1;
	CERR(FormatString("[MctsPlayer::reset] Current Mcts List index is %d.\n", m_iCurrentMctsListIndex));
	CERR("[MctsPlayer::reset] Ready for next search.\n");
}

Tile MctsPlayer::askThrow()
{
	//setup
	const int ciMinLack = m_oPlayerTile.getMinLack();
	m_iDrawCount = DrawCountTable::getDrawCount(ciMinLack, m_oRemainTile.getRemainDrawNumber() / PLAYER_COUNT);
	m_vLegalMoves = Mcts_t::getLegalMoves(m_oPlayerTile, m_oRemainTile, false, false, m_oConfig.m_bMergeAloneTile);
	printInfo("MctsPlayer::askThrow");

	//start mcts
	if (m_oConfig.m_uiMctsThreadCount > 0) {
		runMcts_multiThread();
	}
	else {
		runMcts();
	}

	//release mcts
	reset();

	//return answer
	//return getBestMove().getTargetTile();
	Tile oAnsTile = getBestMove().getTargetTile();

	//[experiment]
	//analyzeAloneTileRate(oAnsTile);
	
	//system("pause");
	return oAnsTile;
}

MoveType MctsPlayer::askEat(const Tile & oTile)
{
	MoveType oBestMoveType = getBestMeldMove(oTile, true, false);
	if (oBestMoveType == MoveType::Move_EatLeft || oBestMoveType == MoveType::Move_EatMiddle || oBestMoveType == MoveType::Move_EatRight)
		return oBestMoveType;
	return MoveType::Move_Pass;
}

bool MctsPlayer::askPong(const Tile & oTile, const bool & bCanEat, const bool & bCanKong)
{
	return getBestMeldMove(oTile, bCanEat, bCanKong) == MoveType::Move_Pong;
}

bool MctsPlayer::askKong(const Tile & oTile)
{
	return getBestMeldMove(oTile, false, true) == MoveType::Move_Kong;
}

std::pair<MoveType, Tile> MctsPlayer::askDarkKongOrUpgradeKong()
{
	//setup
	const int ciMinLack = m_oPlayerTile.getMinLack();
	m_iDrawCount = DrawCountTable::getDrawCount(ciMinLack, m_oRemainTile.getRemainDrawNumber() / PLAYER_COUNT);
	m_vLegalMoves = Mcts_t::getLegalMoves(m_oPlayerTile, m_oRemainTile, true, true, m_oConfig.m_bMergeAloneTile);
	printInfo("MctsPlayer::askDarkKongOrUpgradeKong");

	//start mcts
	if (m_oConfig.m_uiMctsThreadCount > 0) {
		runMcts_multiThread();
	}
	else {
		runMcts();
	}

	//release mcts
	reset();

	//return answer
	Move oBestMove = getBestMove();
	if (oBestMove.getMoveType() == MoveType::Move_DarkKong || oBestMove.getMoveType() == MoveType::Move_UpgradeKong)
		return std::pair<MoveType, Tile>(oBestMove.getMoveType(), oBestMove.getTargetTile());
	return std::pair<MoveType, Tile>(MoveType::Move_Pass, Tile());
}

uint32_t MctsPlayer::getUsedSampleCount() const
{
	uint32_t iTotalSampleCount = 0;
	int size = m_vCandidates.size();
	for (int i = 0; i < size; i++) {
		iTotalSampleCount += m_vCandidates[i].m_uiVisitCount;
	}

	return iTotalSampleCount;
}

void MctsPlayer::initAllMctsLists()
{
	const int iMctsCountPerList = (m_oConfig.m_uiMctsThreadCount == 0)? 1 : m_oConfig.m_uiMctsThreadCount;
	if (m_vMctsLists[0].empty()) {
		for (int i = 0; i < MCTS_LIST_COUNT; i++) {
			m_vMctsLists[i].reserve(iMctsCountPerList);
			for (int j = 0; j < iMctsCountPerList; j++) {
				m_vMctsLists[i].emplace_back();
			}
		}
	}
	
#ifdef MJ_DEBUG_MODE
	if (iMctsCountPerList == 0) {
		for (int i = 0; i < MCTS_LIST_COUNT; i++) {
			assert(m_vMctsLists[i].size() == 1);
		}
	}
	else {
		for (int i = 0; i < MCTS_LIST_COUNT; i++) {
			assert(m_vMctsLists[i].size() == iMctsCountPerList);
		}
	}
#endif
}

MoveType MctsPlayer::getBestMeldMove(const Tile & oTargetTile, const bool & bCanEat, const bool & bCanKong)
{
	//[CAUTION] Mcts currently make weak pass move in some game states. (ex: 112a 5699b 235566c A ( 222A ), ask eat 4b)
	//GodMoveSamplingData compute win rate of player tiles after meld & discard tile will lower than pass move in the wrong way.
	//Now, we used GodMoveSampling_v3::getBestMeldMove() instead, because it will use the win rate of after meld move but before discard move for comparison.
	//This will make meld move decision better than GodMoveSamplingData's simulation + belowing code.
	if (m_oConfig.m_bUseFlatMCMeld) {
		return getBestMeldMove_FlatMC(oTargetTile, bCanEat, bCanKong);
	}

	//The code below still can work if this simulating problem is fixed. 
	//by sctang 20230131

	//setup
	const int ciMinLack = m_oPlayerTile.getMinLack();
	m_iDrawCount = DrawCountTable::getDrawCount(ciMinLack, m_oRemainTile.getRemainDrawNumber() / PLAYER_COUNT);
	m_vLegalMoves = getMeldMoveList(oTargetTile, bCanEat, bCanKong);
	printInfo("MctsPlayer::getBestMeldMove");
	CERR(FormatString("[MctsPlayer::getBestMeldMove] Target tile: %s\n", oTargetTile.toString().c_str()));
	CERR(FormatString("[MctsPlayer::getBestMeldMove] bCanEat: %d\n", bCanEat));
	CERR(FormatString("[MctsPlayer::getBestMeldMove] bCanKong: %d\n", bCanKong));

	//start mcts
	Move oBestMove;
	if (m_oConfig.m_uiMctsThreadCount > 0) {
		runMcts_multiThread();
	}
	else {
		runMcts();
	}

	//release mcts
	reset();

	//return answer
	return getBestMove().getMoveType();
}

void MctsPlayer::runMcts()
{
	//make sure the previous mcts has been released.
	waitPreviousMctsReleased();

	//setup mcts
	//g_vMcts.clear();
	//g_vMcts.emplace_back(m_oPlayerTile, m_oRemainTile, m_iDrawCount, m_vLegalMoves);
	getCurrentMctsList()[0] = Mcts_t(m_oPlayerTile, m_oRemainTile, m_iDrawCount, m_vLegalMoves, m_oConfig, true);
	initMctsCandidates();

	//start sample
	if (m_oConfig.m_bUseTime) {//sample with time limit
		sampleUntilTimesUp(getCurrentMctsList(), 0, clock() + m_oConfig.m_uiTimeLimitMs, m_vCandidates);
	}
	else {//sample with given sample count
		sample(getCurrentMctsList(), 0, m_oConfig.m_uiSampleCount, m_vCandidates);
	}
	
	//future<void> oFuture = std::async(std::launch::async, sample, 0, m_oConfig.m_uiSampleCount);
	//oFuture.get();
	

	//[Optional] dynamic adjust exploration term
	CERR(FormatString("[MctsPlayer::runMcts] m_bDynamicTrainExplorationTerm = %d\n", m_oConfig.m_bDynamicTrainExplorationTerm));
	if (m_oConfig.m_bDynamicTrainExplorationTerm) {
		trainExplorationTerm();
	}

	//write sgf
	if (m_oConfig.m_bLogTreeSgf)
		getCurrentMctsList().at(0).writeToSgf();
}

void MctsPlayer::runMcts_multiThread()
{
	vector<future<void>> vFutures;
	//uint32_t uiSampleCountPerTree = *m_puiSampleCount / m_oConfig.m_uiMctsThreadCount;
	uint32_t uiSampleCountPerTree = m_oConfig.m_uiSampleCount;
	assert(uiSampleCountPerTree > 0);

	//run thread
	//g_vMcts.clear();
	//g_vMcts.reserve(m_oConfig.m_uiMctsThreadCount);
	vFutures.reserve(m_oConfig.m_uiMctsThreadCount);

	//make sure the previous mcts has been released.
	waitPreviousMctsReleased();

	if (m_oConfig.m_bUseTime) {//sample with time limit
		//first thread
		time_t iEndTime = clock() + m_oConfig.m_uiTimeLimitMs;
		//g_vMcts.emplace_back(m_oPlayerTile, m_oRemainTile, m_iDrawCount, m_vLegalMoves);
		getCurrentMctsList()[0] = Mcts_t(m_oPlayerTile, m_oRemainTile, m_iDrawCount, m_vLegalMoves, m_oConfig, true);
		initMctsCandidates();
		vFutures.emplace_back(std::async(std::launch::async, sampleUntilTimesUp, std::ref(getCurrentMctsList()), 0, iEndTime, std::ref(m_vCandidates)));

		//other thread
		for (int i = 1; i < m_oConfig.m_uiMctsThreadCount; i++) {
			//g_vMcts.emplace_back(m_oPlayerTile, m_oRemainTile, m_iDrawCount, m_vLegalMoves);
			getCurrentMctsList()[i] = Mcts_t(m_oPlayerTile, m_oRemainTile, m_iDrawCount, m_vLegalMoves, m_oConfig, true);
			vFutures.emplace_back(std::async(std::launch::async, sampleUntilTimesUp, std::ref(getCurrentMctsList()), i, iEndTime, std::ref(m_vCandidates)));
		}
	}
	else {//sample with given sample count
		//first thread
		//g_vMcts.emplace_back(m_oPlayerTile, m_oRemainTile, m_iDrawCount, m_vLegalMoves);
		getCurrentMctsList()[0] = Mcts_t(m_oPlayerTile, m_oRemainTile, m_iDrawCount, m_vLegalMoves, m_oConfig, true);
		initMctsCandidates();
		vFutures.emplace_back(std::async(std::launch::async, sample, std::ref(getCurrentMctsList()), 0, uiSampleCountPerTree, std::ref(m_vCandidates)));

		//other thread
		for (int i = 1; i < m_oConfig.m_uiMctsThreadCount; i++) {
			//g_vMcts.emplace_back(m_oPlayerTile, m_oRemainTile, m_iDrawCount, m_vLegalMoves);
			getCurrentMctsList()[i] = Mcts_t(m_oPlayerTile, m_oRemainTile, m_iDrawCount, m_vLegalMoves, m_oConfig, true);
			vFutures.emplace_back(std::async(std::launch::async, sample, std::ref(getCurrentMctsList()), i, uiSampleCountPerTree, std::ref(m_vCandidates)));
		}
	}

	//wait for result
	for (int i = 0; i < m_oConfig.m_uiMctsThreadCount; i++) {
		vFutures.at(i).get();
	}

	
}

void MctsPlayer::sample(vector<Mcts_t>& vMctsList, const int & iThreadId, const int & iSampleCount, vector<Candidate>& vCandidates)
{
	assert(iThreadId < vMctsList.size());
	CERR(FormatString("[MctsPlayer::sample (id:%d)] Sample Count: %d\n", iThreadId, iSampleCount));
	vMctsList[iThreadId].sample(iSampleCount);
	CERR(FormatString("[MctsPlayer::sample (id:%d)] Sample %d times done.\n", iThreadId, iSampleCount));

	static thread_local std::mutex g_oCandidateMutex;
	std::lock_guard<std::mutex> lock(g_oCandidateMutex);
	CERR(FormatString("[MctsPlayer::sample (id:%d)] Result:\n", iThreadId));
	if(Ini::getInstance().getIntIni("Debug.PrintDebugMsg") > 0)
		vMctsList[iThreadId].printChildrenResult(cerr);

	const BaseTreeNode2* pRoot = vMctsList.at(iThreadId).getRootNodePtr();
	int uiBestCandidateId = 0;
	int uiBestCandidateVisitCountTT = pRoot->m_vChildren.at(0)->getVisitCountTT();
	int uiVisitCountTT;
	const int ciCandidateCount = vCandidates.size();
	int iWinCountsSize;
	for (int i = 0; i < ciCandidateCount; i++) {
		uiVisitCountTT = pRoot->m_vChildren.at(i)->getVisitCountTT();
		vCandidates[i].m_uiVisitCount += uiVisitCountTT;
		auto vWinCounts = pRoot->m_vChildren.at(i)->getWinCountsTT();
		assert(vCandidates[i].m_vWinCounts.size() == vWinCounts.size());
		iWinCountsSize = vWinCounts.size();
		for (int j = 0; j < iWinCountsSize; j++) {
			vCandidates[i].m_vWinCounts[j] += vWinCounts[j];
		}

		if (uiVisitCountTT >= uiBestCandidateVisitCountTT) {
			uiBestCandidateId = i;
			uiBestCandidateVisitCountTT = uiVisitCountTT;
		}
	}

	//vote for the best candidate (for move decision)
	//[Future work]Maybe not best candidate only? Good enough candidate can be voted too?
	vCandidates.at(uiBestCandidateId).m_uiVote++;

	CERR(FormatString("[MctsPlayer::sample (id:%d)] Update result done.\n", iThreadId));
}

void MctsPlayer::sampleUntilTimesUp(vector<Mcts_t>& vMctsList, const int & iThreadId, const time_t & iEndTime, vector<Candidate>& vCandidates)
{
	assert(iThreadId < vMctsList.size());
	CERR(FormatString("[MctsPlayer::sample (id:%d)] Sample time: %d ms.\n", iThreadId, iEndTime - clock()));
	vMctsList[iThreadId].sampleUntilTimesUp(iEndTime);
	CERR(FormatString("[MctsPlayer::sample (id:%d)] Sample done.\n", iThreadId));

	static thread_local std::mutex g_oCandidateMutex;
	std::lock_guard<std::mutex> lock(g_oCandidateMutex);
	CERR(FormatString("[MctsPlayer::sample (id:%d)] Result:\n", iThreadId));
	vMctsList[iThreadId].printChildrenResult(cerr);

	const BaseTreeNode2* pRoot = vMctsList.at(iThreadId).getRootNodePtr();
	int uiBestCandidateId = 0;
	int uiBestCandidateVisitCountTT = pRoot->m_vChildren.at(0)->getVisitCountTT();
	int uiVisitCountTT;
	const int ciCandidateCount = vCandidates.size();
	int iWinCountsSize;
	for (int i = 0; i < ciCandidateCount; i++) {
		uiVisitCountTT = pRoot->m_vChildren.at(i)->getVisitCountTT();
		vCandidates[i].m_uiVisitCount += uiVisitCountTT;

		auto vWinCounts = pRoot->m_vChildren.at(i)->getWinCountsTT();
		assert(vCandidates[i].m_vWinCounts.size() == vWinCounts.size());
		iWinCountsSize = vWinCounts.size();
		for (int j = 0; j < iWinCountsSize; j++) {
			vCandidates[i].m_vWinCounts[j] += vWinCounts[j];
		}

		if (uiVisitCountTT >= uiBestCandidateVisitCountTT) {
			uiBestCandidateId = i;
			uiBestCandidateVisitCountTT = uiVisitCountTT;
		}
	}

	//vote for the best candidate (for move decision)
	//[Future work]Maybe not best candidate only? Good enough candidate can be voted too?
	vCandidates.at(uiBestCandidateId).m_uiVote++;

	CERR(FormatString("[MctsPlayer::sample (id:%d)] Update result done.\n", iThreadId));
}

void MctsPlayer::resetMctsList(array<vector<Mcts_t>, MCTS_LIST_COUNT>& vMctsLists, const int& iListIndex)
{
	CERR(FormatString("[MctsPlayer::resetMctsList] Releasing trees in list %d...\n", iListIndex));
	const int size = vMctsLists[iListIndex].size();
	vector<future<void>> vFutures;
	vFutures.reserve(size);
	for (int i = 0; i < size; i++) {
		vFutures.emplace_back(std::async(std::launch::async, resetMcts, std::ref(vMctsLists[iListIndex]), i));
		//getCurrentMctsList()[i].reset();
	}

	for (int i = 0; i < size; i++) {
		vFutures[i].get();
	}
	CERR(FormatString("[MctsPlayer::resetMctsList] Finish releasing trees in list %d...\n", iListIndex));
}

void MctsPlayer::resetMcts(vector<Mcts_t>& vMctsList, const int & iTreeIndex)
{
	//CERR(FormatString("[MctsPlayer::resetMcts] Resetting tree %d in list %d...\n", iTreeIndex, iListNdex));
	vMctsList[iTreeIndex].reset();
	//CERR(FormatString("[MctsPlayer::resetMcts] Resetting tree %d in list %d done.\n", iTreeIndex, iListNdex));
}

void MctsPlayer::waitPreviousMctsReleased()
{
	//make sure the previous mcts has been released.
	if (!m_oConfig.m_bWaitReleaseTree && m_vMctsResetHandlers[m_iCurrentMctsListIndex].valid()) {
		m_vMctsResetHandlers[m_iCurrentMctsListIndex].get();
	}
}

void MctsPlayer::initMctsCandidates()
{
	//The candidates will setup based on g_vMcts[0]
	//Please make sure g_vMcts[0] has correct candidate legal moves
	assert(!getCurrentMctsList().empty());
	int iCandidateCount = getCurrentMctsList()[0].getRootNodePtr()->m_vChildren.size();
	m_vCandidates.resize(iCandidateCount);
	Move oMove;

	for (int i = 0; i < iCandidateCount; i++) {
		oMove = getCurrentMctsList()[0].getRootNodePtr()->m_vChildren.at(i)->getMove();
		m_vCandidates[i].m_oMove = oMove;
		m_vCandidates[i].m_sMove = m_vCandidates[i].m_oMove.toString();
		if (oMove.getMoveType() == MoveType::Move_EatLeft
			|| oMove.getMoveType() == MoveType::Move_EatMiddle
			|| oMove.getMoveType() == MoveType::Move_EatRight
			|| oMove.getMoveType() == MoveType::Move_Pong)
			m_vCandidates[i].m_sMove += getCurrentMctsList()[0].getRootNodePtr()->m_vChildren[i]->m_vChildren.at(0)->getMove().toString();
		m_vCandidates[i].m_uiDrawCount = getDrawCount(m_iDrawCount, m_vCandidates[i].m_oMove);
		m_vCandidates[i].m_uiVisitCount = 0;
		std::fill(m_vCandidates[i].m_vWinCounts.begin(), m_vCandidates[i].m_vWinCounts.end(), 0.0);
		m_vCandidates[i].m_uiVote = 0;
		
	}
}

Move MctsPlayer::getBestMove()
{
	//find best move in map
	CERR("[MctsPlayer::getBestMove] Total result:\n");
	CERR("Action\tVisitCount\t WinCount(WinRate)\n");
	Candidate oBestCandidate = m_vCandidates.at(0);
	auto dBestWinRate = oBestCandidate.m_vWinCounts.at(oBestCandidate.m_uiDrawCount) / oBestCandidate.m_uiVisitCount;
	auto dWinRate = dBestWinRate;
	if (m_oConfig.m_bSelectBestWinRateCandidate) {
		//select by win rate (only prefer for multi-thread mode)
		bool bElected = false;
		const float fElectedThershold = 0.75f;
		for (auto oCandidate : m_vCandidates) {
			if (bElected) {
				//someone is elected already -> skip the comparision
				CERR(FormatString("%s\t%d\t%lf(%lf)\n"
					, oCandidate.m_sMove.c_str()
					, oCandidate.m_uiVisitCount
					, oCandidate.getWinCount()
					, oCandidate.getWinRate()));
				continue;
			}

			if (static_cast<WinRate_t>(oCandidate.m_uiVote) / getCurrentMctsList().size() >= fElectedThershold) {
				//if vote rate > 75% -> select the move (elect)
				oBestCandidate = oCandidate;
				//dBestWinRate = dWinRate;//currently not necessary
				CERR(FormatString("*%s\t%d\t%lf(%lf)\n"
					, oCandidate.m_sMove.c_str()
					, oCandidate.m_uiVisitCount
					, oCandidate.getWinCount()
					, oCandidate.getWinRate()));
				bElected = true;
				continue;
			}

			CERR(FormatString("%s\t%d\t%lf(%lf)\n"
				, oCandidate.m_sMove.c_str()
				, oCandidate.m_uiVisitCount
				, oCandidate.getWinCount()
				, oCandidate.getWinRate()));

			//compare win rate
			dWinRate = oCandidate.getWinRate();
			if (dWinRate > dBestWinRate) {
				oBestCandidate = oCandidate;
				dBestWinRate = dWinRate;
			}
		}
	}
	else {
		//select by visit count
		for (auto oCandidate : m_vCandidates) {
			//[BUG]oCandidate.winCounts.at(ciDrawCount)? (for meld move is ciDrawCount - 1?)
			CERR(FormatString("%s\t%d\t%lf(%lf)\n"
				, oCandidate.m_sMove.c_str()
				, oCandidate.m_uiVisitCount
				, oCandidate.getWinCount()
				, oCandidate.getWinRate()));
			if (oCandidate.m_uiVisitCount >= oBestCandidate.m_uiVisitCount) {
				oBestCandidate = oCandidate;
			}
		}
	}
	

	//return answer
	Move oBestMove = oBestCandidate.m_oMove;
	return oBestMove;
}

void MctsPlayer::printInfo(const string& sFunctionName)
{
	CERR(FormatString("[%s] PlayerTile: %s\n", sFunctionName.c_str(), m_oPlayerTile.toString().c_str()));
	CERR(FormatString("[%s] RemainTile:\n%s\n", sFunctionName.c_str(), m_oRemainTile.getReadableString().c_str()));
	CERR(FormatString("[%s] MinLack: %d\n", sFunctionName.c_str(), m_oPlayerTile.getMinLack()));
	CERR(FormatString("[%s] TotalRemainCount: %d\n", sFunctionName.c_str(), m_oRemainTile.getRemainDrawNumber()));
	CERR(FormatString("[%s] DrawCount: %d\n", sFunctionName.c_str(), m_iDrawCount));
}

void MctsPlayer::trainExplorationTerm()
{
	//Currently this function will train the term based on g_vMcts[0]

	/*const int ciMinLack = m_oPlayerTile.getMinLack();
	if (Mcts::isOverExploration(g_vMcts.at(0))) {
		CERR("[MctsThrowHandler::throwTile] OverExploration\n");
		ExplorationTerm_t newTerm = ExplorationTermTable::getExplorationTerm(ciMinLack, m_iDrawCount) / 10.0;
		ExplorationTermTable::setExplorationTerm(ciMinLack, m_iDrawCount, newTerm);
		for (int i = 1; i < m_iDrawCount; i++) {
			if (ExplorationTermTable::getExplorationTerm(ciMinLack, i) > newTerm) {
				ExplorationTermTable::setExplorationTerm(ciMinLack, i, newTerm);
			}
		}
		//ExplorationTermTable::g_vExplorationTermTable[ciMinLack][ciRemainDrawCount / PLAYER_COUNT] /= 10.0;
	}
	else if (Mcts::isUnderExploration(g_vMcts.at(0))) {
		CERR("[MctsDarkKongOrUpgradeKongHandler::darkKongOrUpgradeKong] UnderExploration\n");
		ExplorationTerm_t newTerm = ExplorationTermTable::getExplorationTerm(ciMinLack, m_iDrawCount) * 10.0;
		ExplorationTermTable::setExplorationTerm(ciMinLack, m_iDrawCount, newTerm);
		for (int i = m_iDrawCount + 1; i <= AVERAGE_MAX_DRAW_COUNT; i++) {
			if (ExplorationTermTable::getExplorationTerm(ciMinLack, i) < newTerm) {
				ExplorationTermTable::setExplorationTerm(ciMinLack, i, newTerm);
			}
		}
	}*/
}

vector<Move> MctsPlayer::getMeldMoveList(const Tile & oTargetTile, const bool & bCanEat, const bool & bCanKong) const
{
	vector<Move> vMoves;
	vMoves.reserve(6);
	vMoves.push_back(MoveType::Move_Pass);

	//eat left
	if (bCanEat && m_oPlayerTile.canEatLeft(oTargetTile)) {
		vMoves.push_back(Move(MoveType::Move_EatLeft, oTargetTile));
	}
	//eat middle
	if (bCanEat && m_oPlayerTile.canEatMiddle(oTargetTile)) {
		vMoves.push_back(Move(MoveType::Move_EatMiddle, oTargetTile));
	}
	//eat right
	if (bCanEat && m_oPlayerTile.canEatRight(oTargetTile)) {
		vMoves.push_back(Move(MoveType::Move_EatRight, oTargetTile));
	}
	//pong
	if (m_oPlayerTile.canPong(oTargetTile)) {
		vMoves.push_back(Move(MoveType::Move_Pong, oTargetTile));
	}
	//kong
	if (bCanKong && m_oPlayerTile.canKong(oTargetTile)) {
		vMoves.push_back(Move(MoveType::Move_Kong, oTargetTile));
	}
	return vMoves;
}

int MctsPlayer::getDrawCount(const int & iOriginDrawCount, const Move & oMove)
{
	switch (oMove.getMoveType()) {
	case MoveType::Move_EatLeft:
	case MoveType::Move_EatMiddle:
	case MoveType::Move_EatRight:
	case MoveType::Move_Pong:
	case MoveType::Move_DrawUselessTile:
		return iOriginDrawCount - 1;
	case MoveType::Move_DarkKong:
	case MoveType::Move_UpgradeKong:
		return iOriginDrawCount + 1;
	case MoveType::Move_Kong:
	case MoveType::Move_Pass:
	case MoveType::Move_Throw:
	case MoveType::Move_WinByOther:
	case MoveType::Move_WinBySelf:
		return iOriginDrawCount;
	default:
		std::cerr << "[MctsPlayer::getDrawCount] Unexpected move type: " << oMove.toString() << std::endl;
		assert(0);
	}
	return -1;
}

void MctsPlayer::analyzeAloneTileRate(const Tile& oPopTile) const
{
	std::fstream fout("MinLackHandList.txt", std::ios::out | std::ios::app);
	std::fstream fout2("MinLackHandList_detail.txt", std::ios::out | std::ios::app);
	PlayerTile oPlayerTile = m_oPlayerTile;
	oPlayerTile.popTileFromHandTile(oPopTile);
	int iTotalTileTypeCount = 0, iAloneTileTypeCount = 0;

	fout2 << oPlayerTile.toString();
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		if (oPlayerTile.getHandTileNumber(i) == SAME_TILE_COUNT)
			continue;
		iTotalTileTypeCount++;
		if (oPlayerTile.willBeAloneTile(i)) {
			fout2 << "\t1";
			iAloneTileTypeCount++;
		}
		else {
			fout2 << "\t0";
		}
	}

	fout << iAloneTileTypeCount << "\t" << iTotalTileTypeCount << std::endl;
	fout2 << std::endl;
	fout.close();
	fout2.close();
}
