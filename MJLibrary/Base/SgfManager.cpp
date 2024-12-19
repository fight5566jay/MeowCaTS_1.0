#include "SgfManager.h"
#include "Tools.h"
#include "Ini.h"

using std::cout;
using std::endl;

bool SgfManager::g_bFirstSetup = true;
bool SgfManager::g_bAvailable;

SgfManager::SgfManager()
{
	init("log\\MJServerLog_" + getTime());
}

SgfManager::SgfManager(const string& sSgfName)
{
	init(sSgfName);
}

SgfManager::~SgfManager()
{
}

void SgfManager::init(const string& sSgfName)
{
	m_sSgfFileName = sSgfName;
	m_pCurrentNode = m_pRootNode.get();

	if (g_bFirstSetup) {
		Ini &oIni = Ini::getInstance();
		g_bAvailable = oIni.getIntIni("Sgf.LogGameSgf") || oIni.getIntIni("Sgf.LogTreeSgf");
		g_bFirstSetup = false;
	}
}

void SgfManager::writeToSgf()
{
	if (!g_bAvailable)
		return;
	//debug
	/*
	ofstream fout("sgfManager_debug_log_" + m_pRootNode->m_oPlayerTile.toString() + ".txt", std::ios::out);
	printStructure(fout);
	fout.close();
	*/

	SGF oSgf(m_sSgfFileName, Type_TreeSgf);

	//add root command
	oSgf.CreateNewSGF();
	oSgf.addRootToSgf();
	oSgf.addTag("DEALER", "E");

	//add current handtile command
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		Tile oTile(i);
		int iTileCount = m_oPlayerTile.getHandTileNumber(oTile);
		for (int j = 0; j < iTileCount; j++) {
			oSgf.addMoveToSgf(1, "M" + std::to_string(oTile.getSgfId()));
		}
	}

	oSgf.addTag("C", "RemainTile:\n" + m_oRemainTile.getReadableString());
	
	//write node command
	oSgf.addTag(m_pRootNode->getSgfString());

	//end of Sgf
	oSgf.finish();
}

void SgfManager::addRootNode(const PlayerTile & oPlayerTile, const RemainTile_t& oRemainTile)
{
	if (!g_bAvailable)
		return;
	m_oPlayerTile = oPlayerTile;
	m_oRemainTile = oRemainTile;
	m_pRootNode = std::make_shared<SgfNode>(nullptr, 1, Move(MoveType::Move_Init));
	m_pCurrentNode = m_pRootNode.get();
	//printStructure();
}

/*int SgfManager::addNode(const int & iPlayer, const MoveType & oMoveType, const Tile & oTile)
{
	if (!g_bAvailable)
		return -1;

	//findNode(m_pCurrentNode);
	SgfNode oNode(m_pCurrentNode, iPlayer, oMoveType, oTile);
	m_pCurrentNode->m_vChildren.push_back(oNode);
	for (int i = 0; i < m_pCurrentNode->m_vChildren.size(); i++) {
		m_pCurrentNode->m_vChildren[i].m_pParent = m_pCurrentNode;//vector reallocation, plz update the node pointer after push_back()
	}
	//printStructure();
	return m_pCurrentNode->m_vChildren.size() - 1;//node id
}*/

int SgfManager::addNode(const int & iPlayer, const Move & oMove)
{
	if (!g_bAvailable)
		return -1;

	//findNode(m_pCurrentNode);
	m_pCurrentNode->m_vChildren.emplace_back(m_pCurrentNode, iPlayer, oMove);
	for (int i = 0; i < m_pCurrentNode->m_vChildren.size(); i++) {
		m_pCurrentNode->m_vChildren[i].m_pParent = m_pCurrentNode;//vector reallocation, plz update the node pointer after emplace_back()
	}
	//printStructure();
	return m_pCurrentNode->m_vChildren.size() - 1;//node id
}

void SgfManager::switchToParentNode()
{
	if (!g_bAvailable)
		return;
	if (m_pCurrentNode->isRoot()) {
		std::cerr << "[SgfManager::switchToParentNode]Current node is already root!!" << std::endl;
		assert(!m_pCurrentNode->isRoot());
	}
	m_pCurrentNode = m_pCurrentNode->m_pParent;
	
}

void SgfManager::switchToChildNode(const int & iChildId)
{
	if (!g_bAvailable)
		return;
	if (iChildId >= m_pCurrentNode->m_vChildren.size()) {
		std::cerr << "[SgfManager::switchToChildNode]iChildId = " << iChildId << ", size of children = " << m_pCurrentNode->m_vChildren.size()  << std::endl;
		assert(iChildId < m_pCurrentNode->m_vChildren.size());
	}
	m_pCurrentNode = &(m_pCurrentNode->m_vChildren[iChildId]);
}

/*void SgfManager::printStructure() 
{
	printStructure(cout);
	std::cout << "End" << std::endl;
}*/

/*void SgfManager::printStructure(ostream & fout)
{
	vector<SgfNode*> vQueue;
	vQueue.push_back(m_pRootNode.get());

	while (!vQueue.empty()) {
		SgfNode* pCurrentNode = vQueue.at(0);

		//print node info
		fout << "Node: " << pCurrentNode << std::endl;
		fout << "Move: " << pCurrentNode->m_oMove.toString() << std::endl;
		fout << "PlayerTile: " << pCurrentNode->m_oPlayerTile.toString() << std::endl;
		fout << "ParentNode: " << pCurrentNode->m_pParent << std::endl;
		fout << "Children: " << std::endl;
		for (int i = 0; i < pCurrentNode->m_vChildren.size(); i++) {
			fout << &pCurrentNode->m_vChildren.at(i) << " (" << pCurrentNode->m_vChildren.at(i).m_oMove.toString() << ")" << std::endl;
			vQueue.push_back(&pCurrentNode->m_vChildren.at(i));
		}

		//delete first node
		vQueue.erase(vQueue.begin());
	}
	fout << "Current node: " << m_pCurrentNode << std::endl;
}*/

void SgfManager::findNode(const SgfNode * pNode)
{
	if (!g_bAvailable)
		return;
	vector<SgfNode*> vQueue;
	vQueue.push_back(m_pRootNode.get());

	while (!vQueue.empty()) {
		SgfNode* pCurrentNode = vQueue.at(0);

		if (pCurrentNode == pNode) {
			std::cout << "Find node." << std::endl;
			return;
		}
		for (int i = 0; i < pCurrentNode->m_vChildren.size(); i++) {
			vQueue.push_back(&pCurrentNode->m_vChildren.at(i));
		}

		//delete first node
		vQueue.erase(vQueue.begin());
	}

	//cannot find the node
	std::cout << "Cannot find node " << pNode << " !!" << std::endl;
	/*ofstream fout("sgfManager_debug_log_2.txt", std::ios::out);
	printStructure(fout);
	fout.close();
	std::cout << "End" << std::endl;*/
}

/* ---SgfNode--- */
/*SgfNode::SgfNode(SgfNode* pParentNode, const int& iPlayer, const MoveType& oMoveType, const Tile& oTile) : m_pParent(pParentNode), m_iDepth(pParentNode->m_iDepth + 1) 
{
	m_oMove = Move(oMoveType, oTile, iPlayer);
	//m_oPlayerTile = m_oMove.getPlayerTileAfterMove(pParentNode->m_oPlayerTile);//old
	//m_oRemainTile = m_oMove.getRemainTileAfterMove(pParentNode->m_oRemainTile);//old
	m_oPlayerTile = pParentNode->m_oPlayerTile;
	m_oPlayerTile.doAction(m_oMove);
	if (oMoveType == Move_Draw && !oTile.isNull()) {
		m_oRemainTile = pParentNode->m_oRemainTile;
		m_oRemainTile.popTile(oTile);
	}
	
	m_vChildren.reserve(DIFFERENT_TILE_NUMBER);
};*/

SgfNode::SgfNode(SgfNode * pParentNode, const int & iPlayer, const Move & oMove) : m_pParent(pParentNode), m_oMove(oMove)
{
	/*m_oPlayerTile = pParentNode->m_oPlayerTile;
	m_oPlayerTile.doAction(m_oMove);
	if (oMove.getMoveType() == MoveType::Move_Draw && !oMove.getTargetTile().isNull()) {
		m_oRemainTile = pParentNode->m_oRemainTile;
		m_oRemainTile.popTile(oMove.getTargetTile(), 1, true);
	}*/

	m_iDepth = pParentNode ? (pParentNode->m_iDepth + 1) : 0;
	m_vChildren.reserve(MAX_DIFFERENT_TILE_COUNT);
};

string SgfNode::getSgfString() const
{
	string str;
	string sMoveTypeString;
	if (!isRoot()) {
		const string sPlayerWindString[5] = { "Unknown", "E", "S", "W", "N" };
		str.append(";(;" + sPlayerWindString[m_oMove.getMainPlayer()] + "[");

		//write move type of node
		switch (m_oMove.getMoveType()) {
		case MoveType::Move_Throw:
			sMoveTypeString = "D";
			break;
		case MoveType::Move_Draw:
			sMoveTypeString = "M";
			break;
		case MoveType::Move_EatLeft:
				sMoveTypeString = "EL";
				break;
		case MoveType::Move_EatMiddle:
			sMoveTypeString = "EM";
			break;
		case MoveType::Move_EatRight:
			sMoveTypeString = "ER";
			break;
		case MoveType::Move_Pong:
			sMoveTypeString = "P";
			break;
		case MoveType::Move_Kong:
			sMoveTypeString = "G";
			break;
		case MoveType::Move_DarkKong:
			sMoveTypeString = "HG";
			break;
		case MoveType::Move_UpgradeKong:
			sMoveTypeString = "UG";
			break;
		case MoveType::Move_WinByOther:
		case MoveType::Move_WinBySelf:
			sMoveTypeString = "H";
			break;
		default:
			sMoveTypeString = "UNKNOWN";
			break;
		}
		str.append(sMoveTypeString);

		//write (target) tile of move
		str.append(std::to_string(m_oMove.getTile().at(0).getSgfId()) + "]");
	}
	

	//write children's node string
	for (int i = 0; i < m_vChildren.size(); i++) {
		str.append(m_vChildren.at(i).getSgfString());
	}
	
	//write command & node's end
	if(!isRoot())
		str.append(")C[" + m_sCommand + "]");

	return str;
}

void SgfNode::checkLegal(const SgfNode* pRootNode) 
{
	SgfNode* pMyRootNode = this;
	for (int i = 0; i < m_iDepth; i++) {
		if (pMyRootNode->isRoot()){
			std::cerr << "[SgfNode::checkLegal]Current node is already root!!" << std::endl;
			std::cerr << "i: " << i << std::endl;
			std::cerr << m_iDepth << std::endl;
			break;
		}
		pMyRootNode = pMyRootNode->m_pParent;
	}

	if (pMyRootNode != pRootNode) {
		std::cerr << "Error: This node's root is not correct!!" << std::endl;
	}
}
