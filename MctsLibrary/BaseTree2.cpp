#include "BaseTree2.h"
#include "../MJLibrary/Base/SGF.h"

BaseTreeNodeBasicData::BaseTreeNodeBasicData() : m_iNoTTVisitCount(0)
{
	std::fill(m_vNoTTWinCounts.begin(), m_vNoTTWinCounts.end(), 0.0f);
}

BaseTreeNodeTTData::BaseTreeNodeTTData() :m_iTTVisitCount(0)
{
	std::fill(m_vTTWinCounts.begin(), m_vTTWinCounts.end(), 0.0f);
}

BaseTreeNodeTTData::BaseTreeNodeTTData(const SamplingData & oData)
	: m_oData(oData)
	, m_iTTVisitCount(0)
{
	std::fill(m_vTTWinCounts.begin(), m_vTTWinCounts.end(), 0.0f);
}

BaseTreeNode2::BaseTreeNode2()
{

}

BaseTreeNode2::BaseTreeNode2(const PlayerTile & oPlayerTile, const RemainTile_t & oRemainTile, const Move & oMove, const int & iDrawCount, const bool& bUseTT, const uint32_t& uiTTType)
	: m_pParent(nullptr)
	, m_pSibling(nullptr)
	, m_bIsPruned(false)
	, m_oMove(oMove)
	, m_iDrawCount(iDrawCount)
	, m_iChildId(0)
	, m_pTTNode(this)
	, m_bCompleteSetup(true)
{
	//Currently only for root node construction
	m_pBasicData = std::make_shared<BaseTreeNodeBasicData>();
	SamplingData oData(m_oMove, oPlayerTile, oRemainTile, iDrawCount);
	bool bStoreToTT = oPlayerTile.getHandTileNumber() % 3 == 2;
	_initTTData(oData, bUseTT, uiTTType, bStoreToTT);
	m_iMinLack = oPlayerTile.getMinLack();
}

BaseTreeNode2::BaseTreeNode2(BaseTreeNode2 * pParent, const Move & oMove, const int & iDrawCount)
	: m_pParent(pParent)
	, m_pSibling(nullptr)
	, m_bIsPruned(false)
	, m_oMove(oMove)
	, m_iDrawCount(iDrawCount)
	, m_pTTNode(this)
	, m_bCompleteSetup(false)
{
	_setupMinLack(pParent->getSamplingData().m_oPlayerTile, oMove);
}

BaseTreeNode2::BaseTreeNode2(BaseTreeNode2* pParent, const Move & oMove, const int & iDrawCount, BaseTreeNode2* pTTNode)
	: m_pParent(pParent)
	, m_pSibling(nullptr)
	, m_bIsPruned(false)
	, m_oMove(oMove)
	, m_iDrawCount(iDrawCount)
	, m_pTTNode(pTTNode)
	, m_bCompleteSetup(false)
{
	//_setupMinLack(pParent->getSamplingData().m_oPlayerTile, oMove);
	m_iMinLack = pTTNode->m_iMinLack;
}


BaseTreeNode2::~BaseTreeNode2()
{

}

void BaseTreeNode2::completeSetup(const bool& bUseTT, const uint32_t uiTTType, const bool& bStoreToTT)
{
	//This function setups m_pBasicData and m_pTTData
	if (m_bCompleteSetup)
		return;

	//setup m_pBasicData
	m_pBasicData = std::make_shared<BaseTreeNodeBasicData>();

	//setup m_pTTData
	if (!existTTNode()) {
		PlayerTile oNewPlayerTile = m_pParent->getSamplingData().m_oPlayerTile;
		RemainTile_t oNewRemainTile = m_pParent->getSamplingData().m_oRemainTile;
		oNewPlayerTile.doAction(m_oMove);
		if (m_oMove.getMoveType() == MoveType::Move_Draw) {
			oNewRemainTile.popTile(m_oMove.getTargetTile());
		}

		SamplingData oNewData(m_oMove, oNewPlayerTile, oNewRemainTile, m_iDrawCount);
		_initTTData(oNewData, bUseTT, uiTTType, bStoreToTT);
	}

	m_bCompleteSetup = true;
}

string BaseTreeNode2::getSgfString() const
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
		case MoveType::Move_DrawUselessTile:
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
		case MoveType::Move_Pass:
			sMoveTypeString = "PASS";//incorrect, just avoiding program crash
			break;
		default:
			sMoveTypeString = "UNKNOWN";
			break;
		}
		str.append(sMoveTypeString);

		//write (target) tile of move
		/*if (m_oMove.getMoveType() != MoveType::Move_Pass
			&& m_oMove.getMoveType() != MoveType::Move_WinByOther
			&& m_oMove.getMoveType() != MoveType::Move_WinBySelf)*/
		if (m_oMove.getMoveType() == MoveType::Move_Pass || m_oMove.getMoveType() == MoveType::Move_DrawUselessTile)
			str.append("]");
		else
			str.append(std::to_string(m_oMove.getTile().at(0).getSgfId()) + "]");
	}


	//write children's node string
	for (int i = 0; i < m_vChildren.size(); i++) {
		str.append(m_vChildren.at(i)->getSgfString());
	}

	//write command & node's end
	if (!isRoot()) {
		str.append(")C[" + getSgfCommand() + getMyCommand() + "]");
	}

	return str;
}

string BaseTreeNode2::getSgfCommand() const
{
	stringstream ss;
	ss << "Action: " << m_oMove.toString() << std::endl;
	ss << "Draw count: " << m_iDrawCount << std::endl;

	if (existTTNode() || m_pTTData) {
		ss << "Sample Draw count: " << getSampleDrawCount() << std::endl;
		ss << "Visit count : " << getVisitCountTT() << std::endl;
		ss << "Win count (Win rate):" << std::endl;
		int iSampleDrawCount = getSampleDrawCount();
		for (int i = 0; i <= iSampleDrawCount; i++) {
			ss << "(" << i << ") " << getWinCountTT(i) << " (" << getWinRateTT(i) << ")" << std::endl;
		}
	}
	else {
		ss << "---No TT Data---" << std::endl;
	}

	if (m_pBasicData) {
		ss << "Visit count Without TT: " << getVisitCountNoTT() << std::endl;
		ss << "Win count (Win rate) without TT:" << std::endl;
		for (int i = 0; i <= m_iDrawCount; i++) {
			ss << "(" << i << ") " << getWinCountNoTT(i) << " (" << getWinRateNoTT(i) << ")" << std::endl;
		}
	}
	else{
		ss << "---No Basic Data---" << std::endl;
	}
	
	ss << "Pointer: " << this << std::endl;
	ss << "TTNode: " << m_pTTNode << std::endl;
	if (existTTNode()) {
		ss << "TTNodePath: " << m_pTTNode->getPathString() << std::endl;
	}
	else {
		ss << "Other equilavent nodes:" << std::endl;
		if (m_vOtherTTNodes.empty()) {
			ss << "No equilavent node." << std::endl;
		}
		else {
			for (BaseTreeNode2* pNode : m_vOtherTTNodes) {
				ss << pNode << " (" << pNode->getPathString() << ")" << std::endl;
				ss << "(" << pNode->getWinRateNoTT(0);
				for (int i = 1; i <= m_iDrawCount; i++) {
					ss << ", " << pNode->getWinRateNoTT(i);
				}
				ss << ")" << std::endl;
				ss << "(" << pNode->getWinCountNoTT(0);
				for (int i = 1; i <= m_iDrawCount; i++) {
					ss << ", " << pNode->getWinCountNoTT(i);
				}
				ss << ") / " << pNode->getVisitCountNoTT() << std::endl;
			}
		}
	}

	return ss.str();
}

void BaseTreeNode2::addWinCountTT(const array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1>& vWinCounts, const int & iBias)
{
	int iDrawCount = getSampleDrawCount();

	//[Debug]
	//DebugLogger::writeLine("[BaseTreeNode2::addWinCountTT] m_vTTWinCounts:");
	//for (int i = 0; i < iDrawCount; i++) {
		//DebugLogger::write(FormatString("%f\t", vWinCounts.at(i)));
	//}
	//DebugLogger::writeLine(FormatString("\nBias=%d", iBias));
	//DebugLogger::writeLine("Win count (before):");

	//for (int i = 0; i <= iDrawCount; i++) {
		//DebugLogger::write(FormatString("%f\t", getWinCountsTT().at(i)));
	//}
	//DebugLogger::writeLine("");

	for (int i = 0; i <= iDrawCount - iBias; i++) {
		//getWinCountsTT().at(i + iBias) += vWinCounts.at(i);
		getWinCountsTT()[i + iBias] += vWinCounts[i];
	}

	//[Debug]
	//DebugLogger::writeLine("Win count (after):");
	//for (int i = 0; i <= iDrawCount; i++) {
		//DebugLogger::write(FormatString("%f\t", getWinCountsTT().at(i)));
	//}
	//DebugLogger::writeLine("");
	/*if (!isRoot()) {
		for (int i = 0; i <= iDrawCount - 1; i++) {
			if (getWinCountsTT()[i + 1] < getWinCountsTT()[i]) {
				std::cerr << "[BaseTreeNode2::addWinCountTT] Error: win counts no TT at " << i + 1 << " = " << getWinCountsTT().at(i + 1) << " is less than at " << i << " = " << getWinCountsTT().at(i) << "." << std::endl;
				assert(getWinCountsTT().at(i + 1) >= getWinCountsTT().at(i));
			}
		}
	}*/
}

void BaseTreeNode2::addWinCountNoTT(const array<NodeWinCount_t, AVERAGE_MAX_DRAW_COUNT + 1>& vWinCounts, const int & iBias)
{
	int iDrawCount = m_iDrawCount;
	//int iDrawCount = getSampleDrawCount();

	//[Debug]
	//DebugLogger::writeLine("[BaseTreeNode2::addWinCountNoTT] m_vNoTTWinCounts:");
	//for (int i = 0; i < iDrawCount; i++) {
		//DebugLogger::write(FormatString("%f\t", vWinCounts.at(i)));
	//
	//DebugLogger::writeLine(FormatString("\nBias=%d", iBias));
	//DebugLogger::writeLine("Win count (before):");

	//for (int i = 0; i <= iDrawCount; i++) {
		//DebugLogger::write(FormatString("%f\t", getWinCountsNoTT().at(i)));
	//}
	//DebugLogger::writeLine("");

	for (int i = 0; i <= iDrawCount - iBias; i++) {
		getWinCountsNoTT()[i + iBias] += vWinCounts[i];
	}

	//[Debug]
	//DebugLogger::writeLine("Win count (after):");
	//for (int i = 0; i <= iDrawCount; i++) {
		//DebugLogger::write(FormatString("%f\t", getWinCountsNoTT().at(i)));
	//}
	//DebugLogger::writeLine("");
	/*if (!isRoot()) {
		for (int i = 0; i <= iDrawCount - 1; i++) {
			assert(getWinCountsNoTT().at(i + 1) >= getWinCountsNoTT().at(i));
		}
	}*/
}

NodeWinRate_t BaseTreeNode2::getErrorRangeNoTT(const uint32_t& uiStdDevCount) const
{
	if (getVisitCountNoTT() == 0)
		return 1.0;
	NodeWinRate_t fWinRate = getWinRateNoTT();
	return sqrt((1 - fWinRate) * fWinRate / getVisitCountNoTT()) * uiStdDevCount;
}

string BaseTreeNode2::getPathString() const
{
	stringstream ss;
	ss << getMove().toString();
	for (BaseTreeNode2* p = m_pParent; p != nullptr; p = p->m_pParent) {
		ss << " <- " << p->getMove().toString();
	}
	return ss.str();
}

void BaseTreeNode2::_initTTData(const SamplingData & oData, const bool& bUseTT, const uint32_t uiTTType, const bool& bStoreToTT)
{
	HashKey_t llHashKey;
	//m_pTTNode = this;//already done.
	if (bUseTT) {
		llHashKey = CalHashKey(oData, uiTTType);
		auto iIndex = TT::lookup(llHashKey);
		if (iIndex != -1) {
			assert(TT::m_entry[iIndex].m_data);
			assert(TT::m_entry[iIndex].m_data->m_pTTData);
			if (isEquilaventState(oData, TT::m_entry[iIndex].m_data->getSamplingData())) {
				m_pTTNode = TT::m_entry[iIndex].m_data;
			}
		}
		else if (bStoreToTT) {
			TT::store(llHashKey, this);
		}
		addMyCommand(FormatString("HashIndex: %u\n", static_cast<HashTableIndexType>(llHashKey) & TT::m_mask));
	}

	//if no TT node is found, make new TT data
	if (m_pTTNode == this) {
		m_pTTData = std::make_shared<BaseTreeNodeTTData>(oData);

		if (bUseTT) {
			_setupSampleDrawCount();
		}
	}
}

void BaseTreeNode2::_setupSampleDrawCount()
{
	if (isRoot()) {
		m_pTTData->m_oData.m_iDrawCount = m_iDrawCount;
		return;
	}

	_setupBitset();

	//setup child's TT draw count
	int iSampleDrawCount = _computeSampleDrawCount();
	if (m_oMove.getMoveType() == MoveType::Move_DrawUselessTile)
		iSampleDrawCount++;//Assume DrawUselessTile node never has child.

	assert(iSampleDrawCount >= m_iDrawCount);

	if (!existTTNode()) {
		m_pTTData->m_oData.m_iDrawCount = iSampleDrawCount;
	}
	else {
		//for debugging
		assert(iSampleDrawCount == m_pTTData->m_oData.m_iDrawCount);
	}
}

void BaseTreeNode2::_setupMinLack(PlayerTile& oPlayerTile, const Move& oMove)
{
	//In order to access min lack before PlayerTile initialize. (stored in TT Data, which may not be initialize yet)
	//we compute the min lack at the constructor of BaseTreeNode2
	oPlayerTile.doAction(oMove);
	m_iMinLack = oPlayerTile.getMinLack();
	oPlayerTile.undoAction(oMove);
}

void BaseTreeNode2::_initPathBitset()
{
	const int iSize = getDrawnTileBitsets().size();
	for (int i = 0; i < iSize; i++) {
		getDrawnTileBitset(i).reset();
		getThrownTileBitset(i).reset();
	}

	if (m_oMove.getMoveType() == MoveType::Move_Throw) {
		getThrownTileBitset(0).set(m_oMove.getTargetTile());
	}
	else if (m_oMove.getMoveType() == MoveType::Move_Draw) {
		getDrawnTileBitset(0).set(m_oMove.getTargetTile());
	}
}

void BaseTreeNode2::_setupBitset()
{
	assert(m_pParent != nullptr);

	//setup bitsets to only 1 bit is set.
	_initPathBitset();

	//setup m_vDrawnTileBitsets
	const int iBitsetSize = getDrawnTileBitsets().size();
	if (m_oMove.getMoveType() == MoveType::Move_Draw) {
		bitset<MAX_DIFFERENT_TILE_COUNT> vDrawnTileCarryBitsets = getDrawnTileBitset(0) & m_pParent->getDrawnTileBitset(0);
		getDrawnTileBitset(0) = getDrawnTileBitset(0) | m_pParent->getDrawnTileBitset(0);
		//DebugLogger::writeLine(FormatString("Parent's vDrawnTileBitset[0]: %s", m_pParent->getDrawnTileBitset(0).to_string().c_str()));
		//DebugLogger::writeLine(FormatString("Child's vDrawnTileBitset[0]: %s", getDrawnTileBitset(0).to_string().c_str()));
		for (int i = 1; i < iBitsetSize; i++) {
			getDrawnTileBitset(i) = m_pParent->getDrawnTileBitset(i) | vDrawnTileCarryBitsets;
			//DebugLogger::writeLine(FormatString("Parent's vDrawnTileBitset[%d]: %s", i, m_pParent->getDrawnTileBitset(i).to_string().c_str()));
			//DebugLogger::writeLine(FormatString("Child's vDrawnTileBitset[%d]: %s", i, getDrawnTileBitset(i).to_string().c_str()));

			if (getDrawnTileBitset(i).none())
				break;
			vDrawnTileCarryBitsets = m_pParent->getDrawnTileBitset(i) & vDrawnTileCarryBitsets;
		}
	}
	else {
		getDrawnTileBitsets() = m_pParent->getDrawnTileBitsets();
		//for (int i = 0; i < iBitsetSize; i++) {
			//DebugLogger::writeLine(FormatString("Parent's vDrawnTileBitset[%d]: %s", i, m_pParent->getDrawnTileBitset(i).to_string().c_str()));
			//DebugLogger::writeLine(FormatString("Child's vDrawnTileBitset[%d]: %s", i, getDrawnTileBitset(i).to_string().c_str()));
		//}
	}

	//setup m_vThrownTileBitsets
	if (m_oMove.getMoveType() == MoveType::Move_Throw) {
		bitset<MAX_DIFFERENT_TILE_COUNT> vThrownTileCarryBitsets = getThrownTileBitset(0) & m_pParent->getThrownTileBitset(0);
		getThrownTileBitset(0) = getThrownTileBitset(0) | m_pParent->getThrownTileBitset(0);
		//DebugLogger::writeLine(FormatString("Parent's vThrownTileBitset[0]: %s", m_pParent->getThrownTileBitset(0).to_string().c_str()));
		//DebugLogger::writeLine(FormatString("Child's vThrownTileBitset[0]: %s", getThrownTileBitset(0).to_string().c_str()));
		for (int i = 1; i < iBitsetSize; i++) {
			getThrownTileBitset(i) = m_pParent->getThrownTileBitset(i) | vThrownTileCarryBitsets;
			//DebugLogger::writeLine(FormatString("Parent's vThrownTileBitset[%d]: %s", i, m_pParent->getThrownTileBitset(i).to_string().c_str()));
			//DebugLogger::writeLine(FormatString("Child's vThrownTileBitset[%d]: %s", i, getThrownTileBitset(i).to_string().c_str()));

			if (getThrownTileBitset(i).none())
				break;
			vThrownTileCarryBitsets = m_pParent->getThrownTileBitset(i) & vThrownTileCarryBitsets;
		}
	}
	else {
		getThrownTileBitsets() = m_pParent->getThrownTileBitsets();
		//for (int i = 0; i < iBitsetSize; i++) {
			//DebugLogger::writeLine(FormatString("Parent's vThrownTileBitset[%d]: %s", i, m_pParent->getThrownTileBitset(i).to_string().c_str()));
			//DebugLogger::writeLine(FormatString("Child's vThrownTileBitset[%d]: %s", i, getThrownTileBitset(i).to_string().c_str()));
		//}
	}
}

int BaseTreeNode2::_computeSampleDrawCount()
{
	//compute child's sample draw count
	int iSampleDrawCount = m_iDrawCount;
	int iDuplicatedDrawThrowCount;
	int iSize = getDrawnTileBitsets().size();
	for (int i = 0; i < iSize; i++) {
		iDuplicatedDrawThrowCount = (getDrawnTileBitset(i) & getThrownTileBitset(i)).count();
		if (iDuplicatedDrawThrowCount == 0)
			break;
		iSampleDrawCount += iDuplicatedDrawThrowCount;
	}
	return iSampleDrawCount;
}

BaseTree2::BaseTree2(const SamplingData & oData, const BaseTreeConfig& oConfig, const bool& bStoreConfig)
{
	_init(oData, oConfig, bStoreConfig);
}

void BaseTree2::init(const SamplingData & oData)
{
	TT::clear();
	m_pRootNode = std::make_shared<BaseTreeNode2>(oData.m_oPlayerTile, oData.m_oRemainTile, MoveType::Move_Init, oData.m_iDrawCount, m_pBaseConfig->m_bUseTT, m_pBaseConfig->m_uiTTType);
}

void BaseTree2::writeToSgf(const string sAppendName)
{
	if (!m_pBaseConfig->m_bLogTreeSgf)
		return;

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

	oSgf.addTag("C", "RemainTile:\n" + m_pRootNode->getSamplingData().m_oRemainTile.getReadableString());

	//write node command
	oSgf.addTag(m_pRootNode->getSgfString());

	//end of Sgf
	oSgf.finish();
}

void BaseTree2::_init(const SamplingData & oData, const BaseTreeConfig & oConfig, const bool& bStoreConfig)
{
	if (bStoreConfig) {
		_initConfig(oConfig);
	}
	init(oData);
}

void BaseTree2::_initConfig(const BaseTreeConfig& oConfig)
{
	cerr << "[BaseTree2::_initConfig] start." << endl;
	m_pBaseConfig = std::make_shared<BaseTreeConfig>(oConfig);
	//m_pBaseConfig->loadFromConfig(*pConfig);
	if (m_pBaseConfig->m_bUseTT) {
		_setupTT();
	}
}

void BaseTree2::_setupTT()
{
	TT::makeTable(m_pBaseConfig->m_uiTTSize);
	MJStateHashKeyCalculator::makeTable();
}
