#pragma once
#include <vector>
#include <string>
#include "SGF.h"
#include "../MJ_Base/Move.h"
#include "../MJ_Base/PlayerTile.h"
#include "../MJ_Base/MJBaseTypedef.h"
using std::string;

class SgfNode {
public:
	SgfNode() {};
	//SgfNode(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile) : m_pParent(nullptr), m_oPlayerTile(oPlayerTile), m_oRemainTile(oRemainTile), m_iDepth(0) { m_vChildren.reserve(MAX_DIFFERENT_TILE_COUNT); };
	//SgfNode(SgfNode* pParentNode, const int& iPlayer, const MoveType& oMoveType, const Tile& oTile);//old
	SgfNode(SgfNode* pParentNode, const int& iPlayer, const Move& oMove);
	~SgfNode() {};

public:
	inline bool isRoot() const { return m_pParent == nullptr; };
	inline void addCommand(const string& sCommand) { m_sCommand.append(sCommand + "\n"); };
	string getSgfString() const;
	void checkLegal(const SgfNode* pRootNode);

	//inline SgfNode* getParentNodePtr() const { return m_pParent; };
	//inline SgfNode* getChildNodePtr(const int& iChildId) const { return &(m_vChildren[iChildId]); };//compiler error

public:
	Move m_oMove;
	//PlayerTile m_oPlayerTile;
	//RemainTile_t m_oRemainTile;
	string m_sCommand;
	int m_iDepth;
	SgfNode* m_pParent;
	vector<SgfNode> m_vChildren;//[TODO]change to shared_ptr?

private:
	
};

class SgfManager
{
public:
	SgfManager();
	SgfManager(const string& sSgfName);
	~SgfManager();

public:
	void init(const string& sSgfName);
	inline void reset() { m_pRootNode.reset(); };
	void writeToSgf();
	void addRootNode(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile);
	//int addNode(const int& iPlayer, const MoveType& oMoveType, const Tile& oTile);//add to current node's child//old
	int addNode(const int& iPlayer, const Move& oMove);//add to current node's child
	void switchToParentNode();
	void switchToChildNode(const int& iChildId);
	inline void switchToLastChildNode() {	switchToChildNode(m_pCurrentNode->m_vChildren.size() - 1); };
	inline void switchToSiblingNode(const int& iChildId) {
		switchToParentNode(); 
		switchToChildNode(iChildId); 
	};
	inline void addCommand(const string& sCommand) { 
		if (!g_bAvailable)
			return;
		m_pCurrentNode->addCommand(sCommand); 
	};
	inline void setFileName(const string& sFileName) { m_sSgfFileName = "log\\" + sFileName + ".sgf"; };//call this function before writeToSgf()

	inline SgfNode getCurrentNode() const { return *m_pCurrentNode; };
	inline SgfNode* getRootNode() const { return m_pRootNode.get(); };
	inline int getChildCount() const { return m_pCurrentNode->m_vChildren.size(); };

private:
	//void printStructure();
	//void printStructure(ostream& fout);
	void findNode(const SgfNode* pNode);

private:
	string m_sSgfFileName;
	PlayerTile m_oPlayerTile;
	RemainTile_t m_oRemainTile;
	std::shared_ptr<SgfNode> m_pRootNode;
	SgfNode* m_pCurrentNode;

	static bool g_bFirstSetup;
	static bool g_bAvailable;
};
