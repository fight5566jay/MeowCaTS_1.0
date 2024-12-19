#include "Mcts2.h"
#include <cmath>
#include <cfloat> //FLT_MAX
#include "../MJLibrary/Base/Tools.h"
#include "../MJLibrary/Base/SGF.h"
#include "../MJLibrary/MJ_Base/MJStateHashKeyCalculator.h"

/*
Mcts2::Mcts2(const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const int & iDrawCount)
	: BaseTree2(SamplingData(Move(), oPlayerTile, oRemainTile, std::max(iDrawCount, oPlayerTile.getMinLack())), BaseTreeConfig()), m_iLeftCandidateCount(0)
{
	if (getConfig().m_bFirstSetup) {
		loadDefaultConfig();
	}

	_initData(MctsConfig());
	setupCandidateNodes(getLegalMoves(oPlayerTile, oRemainTile, true, false, getConfig().m_bMergeAloneTile));
}

Mcts2::Mcts2(const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const int & iDrawCount, const vector<Move>& vLegalMoves)
	: BaseTree2(SamplingData(Move(), oPlayerTile, oRemainTile, std::max(iDrawCount, oPlayerTile.getMinLack())), BaseTreeConfig()), m_iLeftCandidateCount(0)
{
	if (getConfig().m_bFirstSetup) {
		loadDefaultConfig();
	}

	_initData(MctsConfig());
	setupCandidateNodes(vLegalMoves);
}

Mcts2::Mcts2(const SamplingData & oData) : BaseTree2(oData, BaseTreeConfig()), m_iLeftCandidateCount(0)
{
	if (getConfig().m_bFirstSetup) {
		loadDefaultConfig();
	}

	_initData(MctsConfig());
	setupCandidateNodes(getLegalMoves(oData.m_oPlayerTile, oData.m_oRemainTile, true, false, getConfig().m_bMergeAloneTile));
}
*/

Mcts2::Mcts2(const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const int & iDrawCount, const MctsConfig& oConfig, const bool& bStoreConfig)
	: /*BaseTree2(SamplingData(Move(), oPlayerTile, oRemainTile, std::max(iDrawCount, oPlayerTile.getMinLack())), oConfig, false),*/ m_iLeftCandidateCount(0)
{
	_init(SamplingData(Move(), oPlayerTile, oRemainTile, std::max(iDrawCount, oPlayerTile.getMinLack())), oConfig, bStoreConfig);
	setupCandidateNodes(getLegalMoves(oPlayerTile, oRemainTile, true, false, oConfig.m_bMergeAloneTile));
}

Mcts2::Mcts2(const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const int & iDrawCount, const vector<Move>& vLegalMoves, const MctsConfig& oConfig, const bool& bStoreConfig)
	: /*BaseTree2(SamplingData(Move(), oPlayerTile, oRemainTile, std::max(iDrawCount, oPlayerTile.getMinLack())), oConfig, false),*/ m_iLeftCandidateCount(0)
{
	_init(SamplingData(Move(), oPlayerTile, oRemainTile, std::max(iDrawCount, oPlayerTile.getMinLack())), oConfig, bStoreConfig);
	setupCandidateNodes(vLegalMoves);
}

Mcts2::Mcts2(const SamplingData & oData, const MctsConfig& oConfig, const bool& bStoreConfig)/* : BaseTree2(oData, oConfig, false)*/
{
	_init(oData, oConfig, bStoreConfig);
	setupCandidateNodes(getLegalMoves(oData.m_oPlayerTile, oData.m_oRemainTile, true, false, oConfig.m_bMergeAloneTile));
}

void Mcts2::init(const SamplingData & oData)
{
	BaseTree2::init(oData);
	m_vPath.clear();
	m_vPath.reserve(2 * oData.m_oPlayerTile.getMinLack());
	setupCandidateNodes(getLegalMoves(oData.m_oPlayerTile, oData.m_oRemainTile, true, false, getConfig().m_bMergeAloneTile));
}

void Mcts2::reset()
{
	BaseTree2::reset();
	m_vPath.clear();
	m_iLeftCandidateCount = 0;
}

void Mcts2::setupCandidateNodes(const vector<Move>& vMoves)
{
	MoveType oMoveType = vMoves.at(0).getMoveType();
	switch (oMoveType) {
	case MoveType::Move_Pass:
	case MoveType::Move_EatLeft:
	case MoveType::Move_EatMiddle:
	case MoveType::Move_EatRight:
	case MoveType::Move_Pong:
	case MoveType::Move_Kong:
		expandMeldMove(m_pRootNode.get(), vMoves);
		break;
	default:
		expand(m_pRootNode.get(), vMoves);
	}

	m_iLeftCandidateCount = m_pRootNode->getChildrenCount();
}

void Mcts2::sample(const int & iSampleCount)
{
	if (m_pRootNode->getDrawCount() < m_pRootNode->getSamplingData().m_oPlayerTile.getMinLack()) {
		std::cerr << "[Mcts2::sample] Error: draw count (" << m_pRootNode->getDrawCount() << ") is less than min lack (" << m_pRootNode->getSamplingData().m_oPlayerTile.getMinLack() << ")." << std::endl;
	}
	assert(m_pRootNode->getDrawCount() >= m_pRootNode->getSamplingData().m_oPlayerTile.getMinLack());

	int iCounter = 10000;//for debug msg
	int iLeftSampleCount = iSampleCount;
	array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1> vWinCounts;
	const int ciVisitCountWithoutTTBeforeSampling = m_pRootNode->getVisitCountNoTT();//for debugging & checking
	int iSampleCountThisRound;

	while (iLeftSampleCount > 0 && m_iLeftCandidateCount > 1) {
		iSampleCountThisRound = std::min(getConfig().m_iSampleCountPerRound, iLeftSampleCount);

		//selection
		TreeNodePtr pNode = select();
		if (getConfig().m_bKeepSelectionDownThroughTTNode && !pNode->isWin() && pNode->existTTNode()) {//TT leaf node
			pNode = pNode->getTTNode();
		}

		//expansion & simulation
		if (pNode->getMove().getMoveType() == MoveType::Move_WinBySelf) {
			//DebugLogger::writeLine("The selected node is win node.");
			std::fill(vWinCounts.begin(), vWinCounts.end(), 1.0f);
			iLeftSampleCount -= iSampleCountThisRound;
		}
		else if (pNode->isLose()
			|| getConfig().m_bStopSelectionIfRunOutOfDrawCount && m_uiLeftDrawCount == 0 && !pNode->isLeaf()) {
			//run out of draw count
			vWinCounts = simulate(pNode, iSampleCountThisRound);
			iLeftSampleCount -= iSampleCountThisRound;
		}
		else if (getConfig().m_bUseTT && !getConfig().m_bKeepSelectionDownThroughTTNode && pNode->getMove().getMoveType() == MoveType::Move_DrawUselessTile) {
			//[Use TT & Not use select down through] draw useless tile node: don't expand it, just sample
			vWinCounts = simulate(pNode, iSampleCountThisRound);
			iLeftSampleCount -= iSampleCountThisRound;
		}
		else {
			if (!pNode->isLeaf()) {
				std::cerr << "[Mcts2::sample] Error: Try to expand pNode, but pNode is not leaf node." << std::endl;
				assert(pNode->isLeaf());
			}
			TreeNodePtr pChildNode = expand(pNode);
			vWinCounts = simulate(pNode, iSampleCountThisRound);//[discussion] simluate pChild?
			iLeftSampleCount -= iSampleCountThisRound;
		}

		//backpropagation
		backPropagate(pNode, vWinCounts, 1);

		//print operation msg
		if ((iSampleCount - iLeftSampleCount) > iCounter) {
			CERR(FormatString("[Mcts::sample] sampled %d times.\n", iCounter));
			iCounter += 10000;
		}

		//debug
		/*if (m_pRootNode->getVisitCountNoTT() - ciVisitCountWithoutTTBeforeSampling != iSampleCount - iLeftSampleCount) {
			std::cerr << "Root visit count (without TT) = " << m_pRootNode->getVisitCountNoTT() << std::endl;
			std::cerr << "Expected visit count = " << iSampleCount - iLeftSampleCount << std::endl;
			std::cerr << "Path: ";
			for (TreeNodePtr ptr = pNode; ptr != nullptr; ptr = ptr->m_pParent) {//wrong if using vPath
				std::cerr << ptr->getMove().toString() << " <- ";
			}
			std::cerr << std::endl;
			writeToSgf();
		}
		//
		assert(m_pRootNode->getVisitCountNoTT() - ciVisitCountWithoutTTBeforeSampling == iSampleCount - iLeftSampleCount);*/
	}
}

void Mcts2::sampleUntilTimesUp(const time_t & iEndTime)
{
	if (m_pRootNode->getDrawCount() < m_pRootNode->getSamplingData().m_oPlayerTile.getMinLack()) {
		std::cerr << "[Mcts2::sample] Error: draw count (" << m_pRootNode->getDrawCount() << ") is less than min lack (" << m_pRootNode->getSamplingData().m_oPlayerTile.getMinLack() << ")." << std::endl;
	}
	assert(m_pRootNode->getDrawCount() >= m_pRootNode->getSamplingData().m_oPlayerTile.getMinLack());

	int iCounter = 10000;//for debug msg
	array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1> vWinCounts;
	const int ciVisitCountWithoutTTBeforeSampling = m_pRootNode->getVisitCountNoTT();//for debugging & checking
	int iUsedSampleCount = 0;

	while (clock() < iEndTime && m_iLeftCandidateCount > 1) {

		//selection
		TreeNodePtr pNode = select();
		if (getConfig().m_bKeepSelectionDownThroughTTNode && !pNode->isWin() && pNode->existTTNode()) {//TT leaf node
			pNode = pNode->getTTNode();
		}

		//expansion & simulation
		if (pNode->getMove().getMoveType() == MoveType::Move_WinBySelf) {
			//DebugLogger::writeLine("The selected node is win node.");
			std::fill(vWinCounts.begin(), vWinCounts.end(), 1.0f);
		}
		else if (pNode->isLose()
			|| getConfig().m_bStopSelectionIfRunOutOfDrawCount && m_uiLeftDrawCount == 0 && !pNode->isLeaf()) {
			//run out of draw count
			vWinCounts = simulate(pNode, getConfig().m_iSampleCountPerRound);
			iUsedSampleCount += getConfig().m_iSampleCountPerRound;
		}
		else if (getConfig().m_bUseTT && !getConfig().m_bKeepSelectionDownThroughTTNode && pNode->getMove().getMoveType() == MoveType::Move_DrawUselessTile) {
			//[Use TT & Not use select down through] draw useless tile node: don't expand it, just sample
			vWinCounts = simulate(pNode, getConfig().m_iSampleCountPerRound);
			iUsedSampleCount += getConfig().m_iSampleCountPerRound;
		}
		else {
			if (!pNode->isLeaf()) {
				std::cerr << "[Mcts2::sample] Error: Try to expand pNode, but pNode is not leaf node." << std::endl;
				assert(pNode->isLeaf());
			}
			TreeNodePtr pChildNode = expand(pNode);
			vWinCounts = simulate(pNode, getConfig().m_iSampleCountPerRound);
			iUsedSampleCount += getConfig().m_iSampleCountPerRound;
		}

		//backpropagation
		backPropagate(pNode, vWinCounts, 1);

		//print operation msg
		if (iUsedSampleCount > iCounter) {
			CERR(FormatString("[Mcts::sample] sampled %d times.\n", iCounter));
			iCounter += 10000;
		}
	}
}

Move Mcts2::getBestMove() const
{
	if (m_pRootNode->isLeaf())
		return Move();

	int iDrawCount = m_pRootNode->getDrawCount();
	TreeNodePtr pBestChild = m_pRootNode->m_vChildren.at(0).get();
	for (int i = 1; i < m_pRootNode->getChildrenCount(); i++) {
		const TreeNodePtr pChild = m_pRootNode->m_vChildren.at(i).get();
		if (pChild->getVisitCountNoTT() > pBestChild->getVisitCountNoTT()
			|| pChild->getVisitCountNoTT() == pBestChild->getVisitCountNoTT() && pChild->getWinCountNoTT() > pBestChild->getWinCountNoTT())
		{
			pBestChild = pChild;
		}
	}
	return pBestChild->getMove();
}

Move Mcts2::getBestMeldMove() const
{
	if (m_pRootNode->isLeaf())
		return Move(MoveType::Move_Pass);

	int iDrawCount = m_pRootNode->getDrawCount();
	TreeNodePtr pBestChild = m_pRootNode->m_vChildren.at(0).get();
	for (int i = 1; i < m_pRootNode->getChildrenCount(); i++) {
		const TreeNodePtr pChild = m_pRootNode->m_vChildren.at(i).get();
		if (pChild->getVisitCountNoTT() > pBestChild->getVisitCountNoTT()
			|| pChild->getVisitCountNoTT() == pBestChild->getVisitCountNoTT() && pChild->getWinCountNoTT() > pBestChild->getWinCountNoTT())
		{
			pBestChild = pChild;
		}
	}

	MoveType oBestMoveType = pBestChild->getMove().getMoveType();
	if ((oBestMoveType == MoveType::Move_EatLeft || oBestMoveType == MoveType::Move_EatMiddle
		|| oBestMoveType == MoveType::Move_EatRight || oBestMoveType == MoveType::Move_Pong)
		&& pBestChild->m_vChildren.at(0)->getMove().getTargetTile() == pBestChild->getMove().getTargetTile()) {
		return Move(MoveType::Move_Pass);
	}
	return pBestChild->getMove();
}

void Mcts2::writeToSgf(const string sAppendName)
{
	if (!getConfig().m_bLogTreeSgf)
		return;

	CERR("[Mcts2::writeToSgf] Write to sgf file...\n");
	string sSgfName = m_pRootNode->getSamplingData().m_oPlayerTile.toString() + "_" + m_pRootNode->getSamplingData().m_oRemainTile.toString();
	if (!sAppendName.empty()) { sSgfName += "_" + sAppendName; }
	SGF oSgf(sSgfName, Type_TreeSgf);

	//add root command
	oSgf.CreateNewSGF();
	oSgf.addRootToSgf();
	oSgf.addTag("DEALER", "E");

	//add current handtile command
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		Tile oTile(i);
		int iTileCount = m_pRootNode->getSamplingData().m_oPlayerTile.getHandTileNumber(oTile);
		for (int j = 0; j < iTileCount; j++) {
			oSgf.addMoveToSgf(1, "M" + std::to_string(oTile.getSgfId()));
		}
	}

	stringstream ss;
	ss << "RemainTile:" << std::endl << m_pRootNode->getSamplingData().m_oRemainTile.getReadableString() << std::endl;
	//ss << "Exploration constant: " << g_dExploarionTerm << std::endl;
	ExplorationTerm_t dExplorationTerm = ExplorationTermTable::getExplorationTerm(m_pRootNode->getSamplingData().m_oPlayerTile.getMinLack(), m_pRootNode->getDrawCount());
	ss << "Exploration constant: " << dExplorationTerm << std::endl;
	oSgf.addTag("C", ss.str());

	//write node command
	oSgf.addTag(m_pRootNode->getSgfString());

	//end of Sgf
	oSgf.finish();

	CERR("[Mcts2::writeToSgf] Write to sgf file done.\n");
}

void Mcts2::printChildrenResult(std::ostream & out) const
{
	if (m_pRootNode->isLeaf()) {
		out << "[Mcts::printChildrenResult]There is no child." << std::endl;
		return;
	}

	out << "PlayerTile: " << m_pRootNode->getSamplingData().m_oPlayerTile.toString() << std::endl;
	out << "RemainTile: " << std::endl << m_pRootNode->getSamplingData().m_oRemainTile.getReadableString() << std::endl;
	out << "Action\t" << "VisitCount\t" << "WinCount\t" << "WinRate" << std::endl;
	for (int i = 0; i < m_pRootNode->getChildrenCount(); i++) {
		const TreeNodePtr pChild = m_pRootNode->m_vChildren.at(i).get();
		SamplingData oData = pChild->getSamplingData();
		Move oMove = pChild->getMove();
		if (pChild->m_bIsPruned)
			out << "*";
		out << oMove.toString();
		if (oMove.getMoveType() == MoveType::Move_EatLeft
			|| oMove.getMoveType() == MoveType::Move_EatMiddle
			|| oMove.getMoveType() == MoveType::Move_EatRight
			|| oMove.getMoveType() == MoveType::Move_Pong)
			out << pChild->m_vChildren.at(0)->getMove().toString();
		out << "\t" << pChild->getVisitCountNoTT() << "\t\t" << pChild->getWinCountNoTT() << "\t" << pChild->getWinRateNoTT();
		if (getConfig().m_bUsePruning) {
			out << " (" << pChild->getWinRateNoTT() - pChild->getErrorRangeNoTT(g_uiStdDevCount) << " ~ " << pChild->getWinRateNoTT() + pChild->getErrorRangeNoTT(g_uiStdDevCount) << ")";
		}
		out << std::endl;
	}
}

vector<Move> Mcts2::getLegalMoves(const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const bool & bCanDarkKongOrUpgradeKong, const bool & bCanWin, const bool bMergeAloneTile)
{
	vector<Move> vMoves;
	if (oPlayerTile.getHandTileNumber() % 3 == 2) {
		if (bCanWin && oPlayerTile.isWin()) {
			vMoves.emplace_back(MoveType::Move_WinBySelf, Tile());//or Move_WinByOther
		}
		else {
			//generate the states after discard
			vMoves.reserve(oPlayerTile.getHandTileNumber());
			for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
				if (oPlayerTile.getHandTileNumber(i) > 0) {
					vMoves.emplace_back(MoveType::Move_Throw, Tile(i));
				}
				if (bCanDarkKongOrUpgradeKong && oPlayerTile.canDarkKong(i)) {
					vMoves.emplace_back(MoveType::Move_DarkKong, Tile(i));
				}
				if (bCanDarkKongOrUpgradeKong && oPlayerTile.canUpgradeKong(i)) {
					vMoves.emplace_back(MoveType::Move_UpgradeKong, Tile(i));
				}
			}
		}
	}
	else if (oPlayerTile.getHandTileNumber() % 3 == 1) {
		bool bExistAloneTile = false;
		//generate the states after mo (pick up)
		vMoves.reserve(MAX_DIFFERENT_TILE_COUNT);

		
		if (bMergeAloneTile) {
			//Add draw move that draw non-alone tile and draw useless tile move
			for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
				if (oRemainTile.getRemainNumber(i) <= 0)
					continue;

				if (oPlayerTile.willBeAloneTile(i)) {
					bExistAloneTile = true;
				}
				else {
					vMoves.emplace_back(MoveType::Move_Draw, Tile(i));
				}
			}

			/*
			//Method 2: Add all useful draw moves (make minlack decreases) and draw useless tile move
			PlayerTile oNewPlayerTile = oPlayerTile;
			for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
				if (oRemainTile.getRemainNumber(i) > 0) {
					oNewPlayerTile.putTileToHandTile(i);
					if (oNewPlayerTile.getMinLack() < oPlayerTile.getMinLack()) {
						vMoves.emplace_back(MoveType::Move_Draw, Tile(i));
					}
					oNewPlayerTile.popTileFromHandTile(i);
				}
			}
			*/

			//If no useful draw move -> every move is useful move
			if (vMoves.empty()) {//ex: HandTile = 4444z
				for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
					if (oRemainTile.getRemainNumber(i) > 0) {
						vMoves.emplace_back(MoveType::Move_Draw, Tile(i));
					}
				}
			}
			else if (bExistAloneTile) {
				vMoves.emplace_back(MoveType::Move_DrawUselessTile);//draw useless tile
			}
			//else {
				//possible HandTile: 147a 258b 369c 1234567z
				//but do nothing because vMoves will be empty and be proceeded by previous "if" section
			//}
		}
		else {
			//Add all possible draw moves
			for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
				if (oRemainTile.getRemainNumber(i) > 0) {
					vMoves.emplace_back(MoveType::Move_Draw, Tile(i));
				}
			}
		}

	}

	/*for (auto oMove : vMoves) {
		std::cerr << oMove.toString() << std::endl;
	}*/

	assert(!vMoves.empty() && vMoves.at(0).getMoveType() != MoveType::Move_DrawUselessTile);
	return vMoves;
}

TreeNodePtr Mcts2::select()
{
	//init
	clearPath();
	m_uiLeftDrawCount = m_pRootNode->getSamplingData().m_iDrawCount;
	TreeNodePtr pCurrentNode = m_pRootNode.get();

	while (true) {
		pushNodeToPath(pCurrentNode);
		if (pCurrentNode->isLeaf()) {
			//The current node should be complete setup to access correct TT node 
			pCurrentNode->completeSetup(getConfig().m_bUseTT, getConfig().m_uiTTType, true);

			if (!getConfig().m_bKeepSelectionDownThroughTTNode
				|| !pCurrentNode->existTTNode()
				|| pCurrentNode->getMove().getMoveType() == MoveType::Move_WinBySelf
				|| pCurrentNode->getMove().getMoveType() == MoveType::Move_WinByOther) {
				//!getConfig().m_bKeepSelectionDownThroughTTNode: No need to jump to TT node
				//!pCurrentNode->existTTNode(): TT node is not exist
				//win (even though TT node exist): No need to jump to TT node
				break;
			}
			
			if (pCurrentNode->getTTNode()->isLeaf()) {
				//DebugLogger::writeLine("Goes to TT node " + pCurrentNode->getMove().toString() + ", and this is a leaf node.");
				break;
			}

			pCurrentNode = pCurrentNode->getTTNode();
		}

		if (getConfig().m_bStopSelectionIfRunOutOfDrawCount && m_uiLeftDrawCount == 0) {
			break;
		}

		if (isMaxNode(pCurrentNode)) {
			pCurrentNode = getBestChild(pCurrentNode);
		}
		else if (isChanceNode(pCurrentNode)) {
			pCurrentNode = select_chance(pCurrentNode);
			m_uiLeftDrawCount--;
		}
		else {
			CERR("[Mcts::selection] Illegal player tile number: PlayerTile = " + pCurrentNode->getSamplingData().m_oPlayerTile.toString() + "\n");
			exit(1);
		}
	}

	assert(pCurrentNode->isCompleteSetup());
	return pCurrentNode;
}

TreeNodePtr Mcts2::expand(TreeNodePtr pNode, const vector<Move> vLegalMoves)
{
	SamplingData& oData = pNode->getSamplingData();
	//PlayerTile oNewPlayerTile = oData.m_oPlayerTile;
	//RemainTile oNewRemainTile = oData.m_oRemainTile;
	const int ciTotalRemainTileCount = oData.m_oRemainTile.getRemainNumber();
	int iUselessTileCount = ciTotalRemainTileCount;//[Caution]Should make sure that MoveType::Move_DrawUselessTile is the last move in vLegalMoves

	NodeWinRate_t dUselessTileChance = 1.0;
	array<NodeWinRate_t, MAX_DIFFERENT_TILE_COUNT> vChanceConsiderMeldFactor;
	if (getConfig().m_bConsiderMeldFactor) {
		vChanceConsiderMeldFactor = ChanceNode2::getChanceConsideringMeldFactor(oData.m_oPlayerTile, oData.m_oRemainTile);
	}

	//DebugLogger::writeLine(FormatString("[Mcts::expand] Current node: %p", pNode));
	//DebugLogger::writeLine(FormatString("PlayerTile: %s", oData.m_oPlayerTile.toString().c_str()));
	//DebugLogger::writeLine(FormatString("Draw count: %d", pNode->getDrawCount()));
	//DebugLogger::writeLine(FormatString("Sample draw count: %d", pNode->getSampleDrawCount()));

	for (Move oMove : vLegalMoves) {
		//DebugLogger::writeLine(FormatString("Current move: %s", oMove.toString().c_str()));

		//compute draw count
		int iDrawCount = pNode->getDrawCount();
		switch (oMove.getMoveType()) {
		case MoveType::Move_Draw:
			iDrawCount--;
			break;
		case MoveType::Move_EatLeft:
		case MoveType::Move_EatMiddle:
		case MoveType::Move_EatRight:
		case MoveType::Move_Pong:
		case MoveType::Move_DrawUselessTile:
			iDrawCount--;
			break;
		case MoveType::Move_DarkKong:
		case MoveType::Move_UpgradeKong:
			iDrawCount++;
			break;
		case MoveType::Move_Throw:
		case MoveType::Move_Kong:
		case MoveType::Move_Pass:
		case MoveType::Move_WinByOther:
		case MoveType::Move_WinBySelf:
			break;
		default:
			std::cerr << "[Mcts::expand] Unexpected move type: " << oMove.toString() << std::endl;
			assert(0);
		}

		//generate new child node
		if (oMove.getMoveType() == MoveType::Move_Draw
			|| oMove.getMoveType() == MoveType::Move_EatLeft
			|| oMove.getMoveType() == MoveType::Move_EatMiddle
			|| oMove.getMoveType() == MoveType::Move_EatRight
			|| oMove.getMoveType() == MoveType::Move_Pong)
		{
			allocNewMaxNode(pNode, oMove, iDrawCount);

			iUselessTileCount -= oData.m_oRemainTile.getRemainNumber(oMove.getTargetTile());
			if (getConfig().m_bConsiderMeldFactor) {
				dUselessTileChance -= vChanceConsiderMeldFactor.at(oMove.getTargetTile());
			}
		}
		else if (oMove.getMoveType() == MoveType::Move_Throw
			|| oMove.getMoveType() == MoveType::Move_Kong
			|| oMove.getMoveType() == MoveType::Move_DarkKong
			|| oMove.getMoveType() == MoveType::Move_UpgradeKong
			|| oMove.getMoveType() == MoveType::Move_Pass) {

			//new node is a chance node (== pNode is a max node)
			allocNewChanceNode(pNode, oMove, iDrawCount);
		}
		else if (oMove.getMoveType() == MoveType::Move_WinByOther
			|| oMove.getMoveType() == MoveType::Move_WinBySelf) {
			//don't let pTTNode point to other nodes, else selection will loop forever.
			//allocNewChanceNode(pNode, oMove, iDrawCount);
			allocNewMaxNode(pNode, oMove, iDrawCount);
		}
		else if (oMove.getMoveType() == MoveType::Move_DrawUselessTile) {
			TreeNodePtr pTTNode = pNode->existTTNode() ? pNode->getTTNode() : pNode;
			allocNewChanceNode(pNode, oMove, iDrawCount, pTTNode);
		}
		else {
			std::cerr << "[Mcts::expand] Illegal action: " << oMove.toString() << std::endl;
			assert(0);
		}

		//DebugLogger::writeLine(FormatString("Current draw count: %d", pNode->m_vChildren.back()->getDrawCount()));
		//DebugLogger::writeLine(FormatString("Current TT draw count: %d", pNode->m_vChildren.back()->getSampleDrawCount()));

		//if pNode is a chance node, setup chance
		if (isChanceNode(pNode)) {
			ChanceNodePtr pChanceNode = static_cast<ChanceNodePtr>(pNode);
			bool bIsDrawUselessTile = oMove.getMoveType() == MoveType::Move_DrawUselessTile;
			pChanceNode->m_vWeights.push_back(1.0);
			if (getConfig().m_bConsiderMeldFactor) {
				if (bIsDrawUselessTile) {
					assert(dUselessTileChance > 0.0f);
					pChanceNode->m_vChances.push_back(dUselessTileChance);
				}
				else {
					pChanceNode->m_vChances.push_back(vChanceConsiderMeldFactor.at(oMove.getTargetTile()));
				}
			}
			else {
				int iRemainTileCount = bIsDrawUselessTile ? iUselessTileCount : oData.m_oRemainTile.getRemainNumber(oMove.getTargetTile());
				pChanceNode->m_vChances.push_back(static_cast<NodeWinRate_t>(iRemainTileCount) / ciTotalRemainTileCount);
			}
		}
	}

	//setup weights (for important sampling)
	if (getConfig().m_bUseImportanceSampling && isChanceNode(pNode)) {
		uint32_t iChildrenCount = pNode->m_vChildren.size();
		vector<NodeWinRate_t> vNewChance(iChildrenCount);

		std::fill(vNewChance.begin(), vNewChance.end(), 1.0 / iChildrenCount);//uniform distribution

		static_cast<ChanceNodePtr>(pNode)->setWeightWithModifiedChance(vNewChance);
	}

	//DebugLogger::writeLine("[Mcts::expand] end");
	return pNode->m_vChildren.at(0).get();
}

array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1> Mcts2::simulate(TreeNodePtr pNode, const int & iSampleCount)
{
	pNode->clearSamplingData();
	pNode->sample(iSampleCount);

	//convert variable type of win count
	array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1> vWinRates;
	std::fill(vWinRates.begin(), vWinRates.end(), 0.0f);

	//DebugLogger::writeLine("[Mcts::simulate]");
	//DebugLogger::writeLine(FormatString("PlayerTile: %s", pNode->getSamplingData().m_oPlayerTile.toString().c_str()));
	//DebugLogger::writeLine(FormatString("RemainTile: %s", pNode->getSamplingData().m_oRemainTile.toString().c_str()));
	//DebugLogger::writeLine(FormatString("Sampled draw count: %d", pNode->getSamplingData().m_iDrawCount));
	//DebugLogger::writeLine("simulate result:");
	for (int i = 0; i < vWinRates.size(); i++) {
		vWinRates[i] = pNode->getSamplingData().getWinRate(i);//[TODO] apply getSamplingData().getWinRates()?
		//DebugLogger::write(FormatString("%f\t", vWinRates.at(i)));
	}
	//DebugLogger::writeLine("");
	return vWinRates;
}

void Mcts2::backPropagate(TreeNodePtr pNode, const array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1>& vWinCounts, const int & iVisitCount)
{
	//DebugLogger::writeLine("[Mcts::backPropagate]");
	
	int iDrawCountBias = 0;
	if (getConfig().m_bKeepSelectionDownThroughTTNode) {
		TreeNodePtr pCurrentNode;
		NodeWinRate_t dWeight;
		auto vWeightWinCounts = vWinCounts;
		int iPreviousNodeChildId;
		auto iSize = m_vPath.size();

		for (int i = iSize - 1; i >= 0; i--) {
			pCurrentNode = m_vPath[i];

			//[Importance sampling]update weight win counts
			if (getConfig().m_bUseImportanceSampling && (i != m_vPath.size() - 1) && isChanceNode(pCurrentNode)) {
				//[Debug]
				//DebugLogger::writeLine(FormatString("[%d][Update weight]", i));
				//DebugLogger::writeLine("Weight win counts before:");
				//for (int i = 0; i < vWeightWinCounts.size(); i++) {
					//DebugLogger::write(FormatString("%f\t", vWeightWinCounts.at(i)));
				//}

				//compute weighted win count
				dWeight = static_cast<ChanceNodePtr>(pCurrentNode->getTTNode())->getWeight(iPreviousNodeChildId);
				assert(dWeight > 0);
				for (int j = 0; j < vWeightWinCounts.size(); j++) {
					vWeightWinCounts[j] *= dWeight;
				}

				//[Debug]
				//DebugLogger::writeLine("Weight win counts after:");
				//for (int i = 0; i < vWeightWinCounts.size(); i++) {
					//DebugLogger::write(FormatString("%f\t", vWeightWinCounts.at(i)));
				//}
				//DebugLogger::writeLine(FormatString("\n[%d][Update weight end]", i));
			}

			//[Debug]
			//DebugLogger::writeLine(FormatString("[%d][Update win counts and visit count]", i));
			//DebugLogger::writeLine(FormatString("Node pointer: %p", pCurrentNode));
			//DebugLogger::writeLine("Weight win counts:");
			//for (int i = 0; i <= pCurrentNode->getDrawCount(); i++) {
				//DebugLogger::write(FormatString("%f\t", vWeightWinCounts.at(i)));
			//}
			//DebugLogger::writeLine("");

			NodeEquilvalenceInPath2 oNodeEquilaventStateInPath = isEquivalantToAncient(i);
			switch (oNodeEquilaventStateInPath) {
			case NodeEquilvalenceInPath2::EquilaventToAncientNode:
				//no update
				break;
			case NodeEquilvalenceInPath2::TTEquilaventToAncientNode:
				pCurrentNode->addVisitCountNoTT(iVisitCount);
				pCurrentNode->addWinCountNoTT(vWeightWinCounts, iDrawCountBias);
				/*for (auto pChild : m_pRootNode->m_vChildren) {
					if (pChild->isCompleteSetup())
						assert(pChild->getVisitCountNoTT() == pChild->getVisitCountTT());
				}*/
				break;
			case NodeEquilvalenceInPath2::DifferentNode:
				pCurrentNode->addVisitCountNoTT(iVisitCount);
				pCurrentNode->addWinCountNoTT(vWeightWinCounts, iDrawCountBias);
				pCurrentNode->addVisitCountTT(iVisitCount);
				pCurrentNode->addWinCountTT(vWeightWinCounts, iDrawCountBias);
				/*for (auto pChild : m_pRootNode->m_vChildren) {
					if (pChild->isCompleteSetup())
						assert(pChild->getVisitCountNoTT() == pChild->getVisitCountTT());
				}*/
				break;
			default:
				std::cerr << "[Mcts::backPropagate] Unknown oNodeEquilaventStateInPath." << std::endl;
				assert(0);
			}

			//DebugLogger::writeLine(FormatString("\n[%d][Update end]", i));

			//update draw count bias
			if (pCurrentNode->getMove().getMoveType() == MoveType::Move_Draw || pCurrentNode->getMove().getMoveType() == MoveType::Move_DrawUselessTile) {
				iDrawCountBias++;
				//DebugLogger::writeLine(FormatString("Update draw Count Bias: %d", iDrawCountBias));
			}

			iPreviousNodeChildId = pCurrentNode->getChildId();
		}
	}
	else {//not getConfig().m_bKeepSelectionDownThroughTTNode
		TreeNodePtr pCurrentNode;
		NodeWinRate_t dWeight;
		auto vWeightWinCounts = vWinCounts;
		int iPreviousNodeChildId;
		auto iSize = m_vPath.size();

		for (int i = iSize - 1; i >= 0; i--) {
			pCurrentNode = m_vPath[i];

			//update weight win counts
			if (getConfig().m_bUseImportanceSampling && ( i != m_vPath.size() - 1) && isChanceNode(pCurrentNode)) {
				//[Debug]
				//DebugLogger::writeLine(FormatString("[%d][Update weight]", i));
				//DebugLogger::writeLine("Win count before:");
				//for (int j = 0; j < vWeightWinCounts.size(); j++) {
					//DebugLogger::write(FormatString("%f\t", vWeightWinCounts.at(j)));
				//}

				//compute weighted win count
				dWeight = static_cast<ChanceNodePtr>(pCurrentNode->getTTNode())->getWeight(iPreviousNodeChildId);
				for (int j = 0; j < vWeightWinCounts.size(); j++) {
					vWeightWinCounts[j] *= dWeight;
				}

				//[Debug]
				//DebugLogger::writeLine("Win count after:");
				//for (int j = 0; j < vWeightWinCounts.size(); j++) {
					//DebugLogger::write(FormatString("%f\t", vWeightWinCounts.at(j)));
				//}
				//DebugLogger::writeLine(FormatString("\n[%d][Update weight end]", i));
			}

			//update both TT data and NoTT data
			pCurrentNode->addVisitCountNoTT(iVisitCount);
			pCurrentNode->addWinCountNoTT(vWeightWinCounts, iDrawCountBias);
			pCurrentNode->addVisitCountTT(iVisitCount);
			pCurrentNode->addWinCountTT(vWeightWinCounts, iDrawCountBias);

			//update draw count bias
			if (pCurrentNode->getMove().getMoveType() == MoveType::Move_Draw || pCurrentNode->getMove().getMoveType() == MoveType::Move_DrawUselessTile) {
				iDrawCountBias++;
			}

			iPreviousNodeChildId = pCurrentNode->getChildId();
		}
	}

}

TreeNodePtr Mcts2::expandMeldMove(TreeNodePtr pNode, const vector<Move> vLegalMoves)
{
	//[CAUTION]Currently expand from root node only, but can append to more general usage.
	PlayerTile oNewPlayerTile = pNode->getSamplingData().m_oPlayerTile;
	//RemainTile_t oNewRemainTile = pNode->getSamplingData().m_oRemainTile;

	//DebugLogger::writeLine(FormatString("[Mcts::expandMeldMove] Current node: %p", pNode));
	//DebugLogger::writeLine(FormatString("PlayerTile: %s", pNode->getSamplingData().m_oPlayerTile.toString().c_str()));
	//DebugLogger::writeLine(FormatString("Draw count: %d", pNode->getDrawCount()));
	//DebugLogger::writeLine(FormatString("Sample draw count: %d", pNode->getSampleDrawCount()));

	for (Move oMove : vLegalMoves) {
		//start loop
		//DebugLogger::writeLine(FormatString("Current move: %s", oMove.toString().c_str()));
		

		//setup draw count
		int iDrawCount = pNode->getDrawCount();
		if (oMove.getMoveType() == MoveType::Move_EatLeft
			|| oMove.getMoveType() == MoveType::Move_EatMiddle
			|| oMove.getMoveType() == MoveType::Move_EatRight
			|| oMove.getMoveType() == MoveType::Move_Pong) 
		{
			//allocate child node (meld move) and its grand child node (throw move)
			assert(oNewPlayerTile == pNode->getSamplingData().m_oPlayerTile);
			oNewPlayerTile.doAction(oMove);
			auto vLegalNextMoves = getLegalMoves(oNewPlayerTile, pNode->getSamplingData().m_oRemainTile, false, false, getConfig().m_bMergeAloneTile);
			for (Move oNextMove : vLegalNextMoves) {
				allocNewMaxNode(pNode, oMove, iDrawCount - 1);
				TreeNodePtr pChild = pNode->m_vChildren.back().get();
				pChild->completeSetup(getConfig().m_bUseTT, getConfig().m_uiTTType, false);
				allocNewChanceNode(pChild, oNextMove, iDrawCount - 1);
			}
			oNewPlayerTile.undoAction(oMove);
			assert(oNewPlayerTile == pNode->getSamplingData().m_oPlayerTile);
		}
		else if (oMove.getMoveType() == MoveType::Move_Pass
			|| oMove.getMoveType() == MoveType::Move_Kong) {
			//allocate child node
			allocNewChanceNode(pNode, oMove, iDrawCount);
		}
	}

	return pNode->m_vChildren.at(0).get();
}

TreeNodePtr Mcts2::getBestChild(const TreeNodePtr pParent)
{
	if (pParent->isLeaf()) {
		return nullptr;
	}

	//ExplorationTerm_t dExplorationTerm = ExplorationTermTable::getExplorationTerm(m_pRootNode->getSamplingData().m_oPlayerTile.getMinLack(), m_pRootNode->getDrawCount());

	//DebugLog
	//DebugLogger::writeLine("[Mcts::getBestChild]");
	//DebugLogger::writeLine(FormatString("Current Node: %p", pParent));
	//DebugLogger::writeLine(FormatString("Current Move: %s", pParent->getMove().toString().c_str()));
	//DebugLogger::writeLine(FormatString("Current PlayerTile: %s", pParent->getSamplingData().m_oPlayerTile.toString().c_str()));
	//DebugLogger::writeLine(FormatString("Current RemainTile: %s", pParent->getSamplingData().m_oRemainTile.toString().c_str()));
	//DebugLogger::writeLine(FormatString("Current Draw Count: %d", pParent->getDrawCount()));
	//DebugLogger::writeLine(FormatString("ExplorationTerm = %f", g_dExplorationTerm));
	//DebugLogger::writeLine(FormatString("Parent visit count = %d", pParent->getVisitCountNoTT()));
	//

	TreeNodePtr pBestChild = pParent->m_vChildren[0].get();
	NodeWinRate_t dBestValue = getUcbValue(pBestChild);
	int iBestChildMinLack = pBestChild->getMinLack();
	TreeNodePtr pChild;
	NodeWinRate_t dValue;
	int iChildMinLack;

	oReserviorOneSampler.clear();
	oReserviorOneSampler.input(pBestChild);
	for (int i = 1; i < pParent->m_vChildren.size(); i++) {
		pChild = pParent->m_vChildren[i].get();
		//if (pChild->m_bIsPruned)
		//	continue;
		dValue = getUcbValue(pChild);

		
		if (getConfig().m_bUseMoveOrdering) {
			if (dValue > dBestValue) {
				oReserviorOneSampler.clear();
				pBestChild = oReserviorOneSampler.input(pChild);
				dBestValue = dValue;
				iBestChildMinLack = pChild->getMinLack();
			}
			else if (dValue == dBestValue) {
				iChildMinLack = pChild->getMinLack();
				if (iChildMinLack < iBestChildMinLack) {
					oReserviorOneSampler.clear();
					pBestChild = oReserviorOneSampler.input(pChild);
					//dBestValue = dValue;//no need
					iBestChildMinLack = iChildMinLack;
				}
				else if (iChildMinLack == iBestChildMinLack) {
					pBestChild = oReserviorOneSampler.input(pChild);
					//dBestValue = dValue;//no need
					//iBestChildMinLack = iChildMinLack;//no need
				}
			}
			
		}
		else {
			/*/
			if (dValue > dBestValue) {
				pBestChild = pChild;
				dBestValue = dValue;
			}
			/*/
			if (dValue > dBestValue) {
				oReserviorOneSampler.clear();
				pBestChild = oReserviorOneSampler.input(pChild);
				dBestValue = dValue;
			}
			else if (dValue == dBestValue) {
				pBestChild = oReserviorOneSampler.input(pChild);
				//dBestValue = dValue;//no need
			}
			/**/
		}

		if (pChild->isCompleteSetup()) {
			//DebugLogger::writeLine(FormatString("%s\tWinRate = %f\tExplore = %f(visit count = %d)\tUcbScore = %f", pChild->getMove().toString().c_str(), pChild->getWinRateTT(), g_dExplorationTerm * sqrt(log(static_cast<NodeWinRate_t>(pChild->m_pParent->getVisitCountNoTT())) / static_cast<NodeWinRate_t>(pChild->getVisitCountNoTT())), pChild->getVisitCountNoTT(), dValue));
		}
		else {
			//DebugLogger::writeLine("Not selected before");
		}
	}

	//pruning
	/*if (getConfig().m_bUsePruning) {
		NodeWinRate_t dWinRateLowerBound = pBestChild->getWinRateTT() - pBestChild->getErrorRangeNoTT(g_uiStdDevCount);
		for (int i = 0; i < pParent->m_vChildren.size(); i++) {
			const TreeNodePtr pChild = pParent->m_vChildren.at(i).get();
			if (pChild->m_bIsPruned)
				continue;
			NodeWinRate_t dWinRateUpperBound = pChild->getWinRateTT() + pChild->getErrorRangeNoTT(g_uiStdDevCount);
			if (dWinRateUpperBound < dWinRateLowerBound) {
				pChild->m_bIsPruned = true;
				if (pParent == m_pRootNode.get()) {
					m_iLeftCandidateCount--;
				}
			}
		}
	}*/

	//DebugLogger::writeLine(FormatString("[Mcts::getBestChild] Select best child: %p\t%s", pBestChild, pBestChild->getMove().toString().c_str()));
	return pBestChild;
}

NodeWinRate_t Mcts2::getUcbValue(const TreeNodePtr pNode) const
{
	if (!pNode->isCompleteSetup() || pNode->getVisitCountNoTT() == 0)
		return FLT_MAX;

	WinRate_t fExplorationTerm = (pNode->getMinLack() <= 1) ? 0.5 : m_fExplorationTerm;

	switch (getConfig().m_uiUcbFormulaType) {
	//[BUG] m_pParent->getVisitCountTT() may less than the sum of pNode->getVisitCountNoTT()
	//ex: the path is n_p -> n_3 -> n_1.
	//in backpropagation, the visit count of n_p, n_3, n_1 increase 1, therefore the visit count of all child increase 2.
	//However the TT visit count of parent increase 1 only.
	//Therefore, the exploration term is incorrect.
	//But this will happened only on chance node and chance node doesn't use ucb value
	//So, it doesn't affect the correctness of selection
	case 1: //use TT win rate directly
		return pNode->getWinRateTT() + fExplorationTerm * sqrt(log(static_cast<NodeWinRate_t>(pNode->m_pParent->getVisitCountTT())) / static_cast<NodeWinRate_t>(pNode->getVisitCountNoTT()));
	case 2://use noTT win rate with TT win rate as reference
		return pNode->getWinRateNoTT() * (1.0f - getConfig().m_fTTWinRateWeight) + pNode->getWinRateTT() * getConfig().m_fTTWinRateWeight + fExplorationTerm * sqrt(log(static_cast<NodeWinRate_t>(pNode->m_pParent->getVisitCountTT())) / static_cast<NodeWinRate_t>(pNode->getVisitCountNoTT()));
	case 3://use PUCT (currently weak)
		return pNode->getWinRateNoTT() + fExplorationTerm * pNode->getWinRateTT() * sqrt(log(static_cast<NodeWinRate_t>(pNode->m_pParent->getVisitCountTT())) / static_cast<NodeWinRate_t>(pNode->getVisitCountNoTT()));
	default:
		std::cerr << "[Mcts2::getUcbValue] Unknown ucb formula type (1~3)." << std::endl;
		assert(getConfig().m_uiUcbFormulaType >= 1 && getConfig().m_uiUcbFormulaType <= 3);
	}
	return 0.0f;
}

TreeNodePtr Mcts2::select_chance(const TreeNodePtr pNode)
{
	//DebugLog
	//DebugLogger::writeLine("[Mcts::select_chance]");
	//DebugLogger::writeLine(FormatString("Current Node: %p", pNode));
	//DebugLogger::writeLine(FormatString("Current Move: %s", pNode->getMove().toString().c_str()));
	//DebugLogger::writeLine(FormatString("Current PlayerTile: %s", pNode->getSamplingData().m_oPlayerTile.toString().c_str()));
	//DebugLogger::writeLine(FormatString("Current RemainTile: %s", pNode->getSamplingData().m_oRemainTile.toString().c_str()));
	//DebugLogger::writeLine(FormatString("Current Draw Count: %d", pNode->getDrawCount()));
	//for (int i = 0; i < pNode->m_vChildren.size(); i++) {
		//DebugLogger::writeLine(FormatString("%s\tVisit count (without TT) = %d\tChances = %f\tWeight = %f", pNode->m_vChildren.at(i)->getMove().toString(), pNode->m_vChildren.at(i)->getVisitCountNoTT(), static_cast<ChanceNodePtr>(pNode)->getChance(i), static_cast<ChanceNodePtr>(pNode)->getWeight(i)));
	//}
	//

	NodeWinRate_t dLeftChance = 1.0;
	const int iSize = pNode->m_vChildren.size() - 1;
	for (int i = 0; i < iSize; i++) {
		const TreeNodePtr pCurrentNode = pNode->m_vChildren.at(i).get();
		NodeWinRate_t dCurrentChance = static_cast<ChanceNodePtr>(pNode)->getChance(i);
		bool bHit = hitRate(dCurrentChance / dLeftChance);
		if (bHit) {
			//DebugLogger::writeLine(FormatString("[Mcts::select_chance] Select child: %p\t%s", pNode->m_vChildren.at(i), pNode->m_vChildren.at(i)->getMove().toString().c_str()));
			return pCurrentNode;
		}

		dLeftChance -= dCurrentChance;

		//CERR("[Mcts::select_chance] Acummulate chance = " + std::to_string(1.0 - dLeftChance) + "\n");
		/*if (dLeftChance < -1e-6) {
			std::cerr << "[Mcts::select_chance] dLeftChance < -1e-6. Something wrong!" << std::endl;
			assert(dLeftChance >= -1e-6);
		}*/
	}

	return pNode->m_vChildren.back().get();
}

NodeEquilvalenceInPath2 Mcts2::isEquivalantToAncient(const int& iIndex) const
{
	if (iIndex <= 0)
		return NodeEquilvalenceInPath2::DifferentNode;

	NodeEquilvalenceInPath2 oResult = NodeEquilvalenceInPath2::DifferentNode;
	TreeNodePtr pTTNode = m_vPath.at(iIndex)->getTTNode();
	for (int i = iIndex - 1; i >= 0; i--) {
		if (pTTNode == m_vPath.at(i)->getTTNode()) {
			if (m_vPath.at(iIndex) == m_vPath.at(i)) {
				return NodeEquilvalenceInPath2::EquilaventToAncientNode;
			}
			oResult = NodeEquilvalenceInPath2::TTEquilaventToAncientNode;
		}
	}
	return oResult;
}

void  Mcts2::_init(const SamplingData& oData, const MctsConfig& oConfig, const bool& bStoreConfig)
{
	if (bStoreConfig)
		_initConfig(oConfig);
	
	BaseTree2::init(oData);
	m_vPath.clear();
	m_vPath.reserve(2 * oData.m_oPlayerTile.getMinLack());

	//m_fExplorationTerm = ExplorationTermTable::getExplorationTerm(m_pRootNode->getSamplingData().m_oPlayerTile.getMinLack(), m_pRootNode->getDrawCount());
	m_fExplorationTerm = getConfig().m_fExplorationTerm;
	m_vPath.reserve(2 * m_pRootNode->getSamplingData().m_oPlayerTile.getMinLack());
	CERR(FormatString("[Mcts2::_initData] Exploration term: %f\n", m_fExplorationTerm));
}

void Mcts2::_initConfig(const MctsConfig& oConfig)
{
	//cerr << "[Mcts2::_initConfig] start." << endl;
	m_pBaseConfig = std::make_shared<MctsConfig>(oConfig);

	if (m_pBaseConfig->m_bUseTT) {
		_setupTT();
	}
}


void Mcts2::loadDefaultConfig(const bool& bForceLoadConfig)
{
	//[TODO]need to verify if it works correctly.
	getConfig().loadFromDefaultConfigFile(bForceLoadConfig);
}

bool Mcts2::isMaxNode(const TreeNodePtr pNode) const
{
	return pNode->isRoot() || pNode->getSamplingData().m_oPlayerTile.getHandTileNumber() % 3 == 2;
}

bool Mcts2::isChanceNode(const TreeNodePtr pNode) const
{
	return !pNode->isRoot() && pNode->getSamplingData().m_oPlayerTile.getHandTileNumber() % 3 == 1;//may be wrong in some situation?
}

void Mcts2::pushNodeToPath(const TreeNodePtr pNode)
{
	m_vPath.push_back(pNode);
}

void Mcts2::popNodeFromPath()
{
	m_vPath.pop_back();
}

void Mcts2::clearPath()
{
	m_vPath.clear();
}

void Mcts2::allocNewMaxNode(TreeNodePtr pParent, const Move & oMove, const int & iDrawCount)
{
	try {
		pParent->m_vChildren.emplace_back(std::make_shared<BaseTreeNode2>(pParent, oMove, iDrawCount));
	}
	catch (std::bad_alloc& ba) {
		std::cerr << "[Mcts::allocNewMaxNode] Cannot allocate BaseTreeNode: " << ba.what() << std::endl;
		exit(87);
	}

	if (pParent->m_vChildren.back() == nullptr) {
		std::cerr << "[Mcts::allocNewMaxNode] pParent->m_vChildren.back() == nullptr." << std::endl;
		assert(pParent->m_vChildren.back() != nullptr);
	}
	
	auto iChildrenCount = pParent->m_vChildren.size();
	if (iChildrenCount > 1) {
		pParent->m_vChildren.at(iChildrenCount - 2)->m_pSibling = pParent->m_vChildren.back().get();
	}
	pParent->m_vChildren.back()->m_iChildId = iChildrenCount - 1;
}

void Mcts2::allocNewChanceNode(TreeNodePtr pParent, const Move & oMove, const int & iDrawCount)
{
	try {
		pParent->m_vChildren.emplace_back(std::make_shared<ChanceNode2>(pParent, oMove, iDrawCount));
	}
	catch (std::bad_alloc& ba) {
		std::cerr << "[Mcts::allocNewChanceNode] Cannot allocate ChanceChildNode: " << ba.what() << std::endl;
		exit(87);
	}

	if (pParent->m_vChildren.back() == nullptr) {
		std::cerr << "[Mcts::allocNewChanceNode] pParent->m_vChildren.back() == nullptr." << std::endl;
		assert(pParent->m_vChildren.back() != nullptr);
	}

	auto iChildrenCount = pParent->m_vChildren.size();
	if (iChildrenCount > 1) {
		pParent->m_vChildren.at(iChildrenCount - 2)->m_pSibling = pParent->m_vChildren.back().get();
	}
	pParent->m_vChildren.back()->m_iChildId = iChildrenCount - 1;
}

void Mcts2::allocNewChanceNode(TreeNodePtr pParent, const Move & oMove, const int & iDrawCount, const TreeNodePtr pTTNode)
{
	try {
		pParent->m_vChildren.emplace_back(std::make_shared<ChanceNode2>(pParent, oMove, iDrawCount, pTTNode));
	}
	catch (std::bad_alloc& ba) {
		std::cerr << "[Mcts::allocNewChanceNode] Cannot allocate ChanceChildNode: " << ba.what() << std::endl;
		exit(87);
	}

	if (pParent->m_vChildren.back() == nullptr) {
		std::cerr << "[Mcts::allocNewChanceNode] pParent->m_vChildren.back() == nullptr." << std::endl;
		assert(pParent->m_vChildren.back() != nullptr);
	}

	auto iChildrenCount = pParent->m_vChildren.size();
	if (iChildrenCount > 1) {
		pParent->m_vChildren.at(iChildrenCount - 2)->m_pSibling = pParent->m_vChildren.back().get();
	}
	pParent->m_vChildren.back()->m_iChildId = iChildrenCount - 1;
}
