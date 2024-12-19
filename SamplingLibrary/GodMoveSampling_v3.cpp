#include "GodMoveSampling_v3.h"
#include <future>
#ifdef LINUX
#include "Tools.h"
#include "Ini.h"
#else
#include "..\MJLibrary\Base\Tools.h"
#include "..\MJLibrary\Base\Ini.h"
#endif



using std::cout;
using std::endl;
using std::shared_ptr;
using std::fstream;

bool GodMoveSampling_v3::m_bFirstSetup = true;
GodMoveSampling_v3::SampleMode GodMoveSampling_v3::m_oSampleMode;
uint32_t GodMoveSampling_v3::m_uiSampleParameter;
uint32_t GodMoveSampling_v3::m_uiSampleFirstRound;
uint32_t GodMoveSampling_v3::m_uiSamplePerRound;
bool GodMoveSampling_v3::m_bUseMultiThread;
bool GodMoveSampling_v3::m_bPruning;
bool GodMoveSampling_v3::m_bUseTooManyMeldsStrategy;
bool GodMoveSampling_v3::m_bUseRule2;
bool GodMoveSampling_v3::m_bUseRule2FastReturn;
bool GodMoveSampling_v3::m_bUseDepth1FastReturn;
bool GodMoveSampling_v3::m_bPrintDebugMsg;
int GodMoveSampling_v3::m_iConsideringChunkType;
bool GodMoveSampling_v3::m_bNoConsideringChunkAtBeginning;
GetTileTypeTmp GodMoveSampling_v3::m_oGetTileType;
array<array<int, MAX_MINLACK + 1>, MAX_DRAW_COUNT + 1> GodMoveSampling_v3::g_vDrawCountTable;
array<array<int, MAX_MINLACK + 1>, MAX_DRAW_COUNT + 1> GodMoveSampling_v3::g_vDrawCountTable_v2;

GodMoveSampling_v3::GodMoveSampling_v3(
	const PlayerTile& oPlayerTile
	, const RemainTile_t& oRemainTile
	, const int& iLeftDrawCount)
	: m_oPlayerTile(oPlayerTile), m_oRemainTile(oRemainTile), m_iLeftDrawCount(iLeftDrawCount), m_iUsedSDCount(3)
{
	init();
	//printSetting();
}

GodMoveSampling_v3::GodMoveSampling_v3(
	const PlayerTile& oPlayerTile
	, const RemainTile_t& oRemainTile
	, const SampleMode& oSampleMode
	, const uint32_t& ullSampleParameter
	, const int& iLeftDrawCount)
	: m_oPlayerTile(oPlayerTile), m_oRemainTile(oRemainTile), m_iLeftDrawCount(iLeftDrawCount), m_iUsedSDCount(3)
{
	init();
	m_oSampleMode = oSampleMode;
	m_uiSampleParameter = ullSampleParameter;
	m_bFirstSetup = true;//make sure it can be set again when calling GodMoveSampling_v3(const PlayerTile&, const RemainTile&, const int&)
	//printSetting();
}

void GodMoveSampling_v3::init()
{
	Ini& oIni = Ini::getInstance();
	if (m_bFirstSetup) {
		//m_oDrawCountTable.setupDrawCountTable(); //already done at DrawCountTable()
		m_bUseTooManyMeldsStrategy = oIni.getIntIni("TooManyMeldsStrategy.UseTooManyMeldsStrategy") > 0;
		m_bUseRule2 = oIni.getIntIni("TooManyMeldsStrategy.UseRule2") > 0;
		m_bUseRule2FastReturn = oIni.getIntIni("TooManyMeldsStrategy.UseRule2FastReturn") > 0;
		m_bUseDepth1FastReturn = oIni.getIntIni("TooManyMeldsStrategy.UseDepth1FastReturn") > 0;
		m_iConsideringChunkType = oIni.getIntIni("BaseConfig.ConsideringChunkType");
		m_bNoConsideringChunkAtBeginning = oIni.getIntIni("BaseConfig.NoConsideringChunkAtBeginning") > 0;
		m_bUseMultiThread = oIni.getIntIni("Multithread.ThreadNum") > 0;
		m_bPruning = oIni.getIntIni("BaseConfig.DoPruning") > 0;
		m_oGetTileType = static_cast<GetTileTypeTmp>(oIni.getIntIni("BaseConfig.GetTileType"));
		m_oSampleMode = oIni.getIntIni("BaseConfig.UseTime") > 0 ? SampleMode::Mode_Time : SampleMode::Mode_Count;
		if (m_oSampleMode == SampleMode::Mode_Time) {
			m_uiSampleParameter = static_cast<uint32_t>(oIni.getIntIni("BaseConfig.TimeLimitMs"));
		}
		else /* if (m_oSampleMode == SampleMode::Mode_Count)*/ {
			m_uiSampleParameter = static_cast<uint32_t>(oIni.getIntIni("BaseConfig.SampleCount"));
			m_uiSampleFirstRound = static_cast<uint32_t>(oIni.getIntIni("BaseConfig.SampleFirstRound"));
			m_uiSamplePerRound = static_cast<uint32_t>(oIni.getIntIni("BaseConfig.SamplePerRound"));
		}
		m_bPrintDebugMsg = oIni.getIntIni("Debug.PrintDebugMsg");
		m_bFirstSetup = false;
	}

	if (m_iLeftDrawCount == -1) {
		//int iDrawTableVersion = oIni.getIntIni("BaseConfig.DrawTableVersion");
		//if (iDrawTableVersion == 2){
		assert(m_oRemainTile.getRemainDrawNumber() / 4 >= 0 && m_oRemainTile.getRemainDrawNumber() / 4 <= AVERAGE_MAX_DRAW_COUNT);
		assert(m_oPlayerTile.getMinLack() >= 0 && m_oPlayerTile.getMinLack() <= MAX_MINLACK);
		/*std::cerr << "WinCountTable:" << std::endl;
		for (int i = 0; i <= MAX_MINLACK; i++) {
			for (int j = 0; j <= AVERAGE_MAX_DRAW_COUNT + 1; j++) {
				std::cerr << m_oDrawCountTable.m_vWinCountTable.at(i).at(j) << " ";
			}
			std::cerr << std::endl;
		}

		std::cerr << "MostPossibleMaxDrawCount:" << std::endl;
		for (int i = 0; i <= MAX_MINLACK; i++) {
			std::cerr << m_oDrawCountTable.m_vMostPossibleMaxDrawCount.at(i) << " ";
		}
		std::cerr << std::endl;

		std::cerr << "DrawCountTable:" << std::endl;
		std::cerr << m_oRemainTile.getRemainDrawNumber() / 4 << " " << m_oPlayerTile.getMinLack() << std::endl;
		for (int i = 0; i <= AVERAGE_MAX_DRAW_COUNT; i++) {
			for (int j = 0; j <= MAX_MINLACK; j++) {
				std::cerr << m_oDrawCountTable.getDrawCount(j, i) << " ";
			}
			std::cerr << std::endl;
		}*/
		m_iLeftDrawCount = m_oDrawCountTable.getDrawCount(m_oPlayerTile.getMinLack(), m_oRemainTile.getRemainDrawNumber() / 4);
		assert(m_iLeftDrawCount > 0 && m_iLeftDrawCount <= m_oRemainTile.getRemainNumber() / PLAYER_COUNT);
		//}else/* if(iDrawTableVersion == 1)*/
		//	m_iLeftDrawCount = g_vDrawCountTable[m_oRemainTile.getRemainDrawNumber() / 4][m_oPlayerTile.getMinLack()];
	}
}

void GodMoveSampling_v3::reset()
{
	m_vCandidates.clear();
	m_uiUsedSampleCount = 0;
	m_iSampleCountG1D1 = m_iSampleCountG1D2 = m_iSampleCountG2D1 = m_iSampleCountG2D2 = 0;
	m_uiStartTime = clock();
}

Tile GodMoveSampling_v3::throwTile(const bool& bUseTooManyMeldStrategy, const bool& bConsideringChunk, const bool& bUseUcb)
{
	time_t startTime = clock();
	reset();
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		Tile oThrownTile(i);
		if (m_oPlayerTile.getHandTileNumber(oThrownTile) > 0) {
			Move oMove(MoveType::Move_Throw, oThrownTile);
			m_oPlayerTile.popTileFromHandTile(oThrownTile);
			SamplingDataType oData(oMove, m_oPlayerTile, m_oRemainTile, m_iLeftDrawCount, m_oGetTileType);
			m_vCandidates.push_back(oData);
			m_oPlayerTile.putTileToHandTile(oThrownTile);
		}
	}

	if (m_bUseTooManyMeldsStrategy && bUseTooManyMeldStrategy && isTooManyMelds()) {
		Tile oBestThrownTile = tooManyMeldsStrategy().getTargetTile();
		/*if (clock() - startTime > 2500) {
			fstream fout("TimeoutLog.txt", std::ios::out | std::ios::app);
			fout << clock() - startTime << "\t" << m_oPlayerTile.toString() << "\t" << m_oRemainTile.toString() << std::endl;
			fout.close();
		}*/
		return oBestThrownTile;
	}

	SamplingDataType oBestData;
	if (bUseUcb) {
		oBestData = makeMoveDecision_UCB();
	}
	else if (bConsideringChunk && getAvoidingChunkType() == 1) {
		oBestData = makeMoveDecisionWithChunkAvoidingType1();
	}
	else if (bConsideringChunk && getAvoidingChunkType() == 2) {
		oBestData = makeMoveDecisionWithChunkAvoidingType2();
	}
	else {
		oBestData = makeMoveDecision();
	}

	Tile oBestThrownTile = oBestData.m_oAction.getTargetTile();
	if (oBestThrownTile.isNull()) {
		std::cerr << "[GodMoveSampling_v3::throwTile] Invalid oBestThrownTile: The best move is " << oBestData.m_oAction.toString() << ", but the valid move should be a throw move!!" << std::endl;
		printCurrentCandidate();
	}
	assert(!oBestThrownTile.isNull());


	/*time_t usedTime = clock() - startTime;
	if (usedTime > 2500) {
		fstream fout("TimeoutLog.txt", std::ios::out | std::ios::app);
		fout << clock() - startTime << "\t" << m_oPlayerTile.toString() << "\t" << m_oRemainTile.toString() << std::endl;
		fout.close();
	}*/

	if (m_bPrintDebugMsg)
		printCurrentCandidate();
	return oBestThrownTile;
}

vector<SamplingDataType> GodMoveSampling_v3::throwTile_detail(/*const bool & bUseTooManyMeldStrategy*/)
{
	//if (m_bUseTooManyMeldsStrategy && bUseTooManyMeldStrategy && m_iLeftDrawCount > m_oPlayerTile.getMinLack()) {
		//Use too meny meld strategy
	//}
	time_t startTime = clock();
	reset();
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		Tile oThrownTile(i);
		if (m_oPlayerTile.getHandTileNumber(oThrownTile) > 0) {
			Move oMove(MoveType::Move_Throw, oThrownTile);
			m_oPlayerTile.popTileFromHandTile(oThrownTile);
			SamplingDataType oData(oMove, m_oPlayerTile, m_oRemainTile, m_iLeftDrawCount, m_oGetTileType);
			m_vCandidates.push_back(oData);
			m_oPlayerTile.putTileToHandTile(oThrownTile);
		}
	}
	time_t genTime = clock() - startTime;
	//CERR(FormatString("[GodMoveSampling_v3::throwTile_detail]CandidateGeneration time: %d ms.\n", clock() - startTime));

	if (m_iConsideringChunkType == 1) {
		makeMoveDecisionWithChunkAvoidingType1();//NOTICE: The result "win rate - chunk rate" doesn't update into m_vCandidates
	}
	else if (m_iConsideringChunkType == 2) {
		makeMoveDecisionWithChunkAvoidingType2();
	}
	else {
		makeMoveDecision();
	}
	time_t sampleTime = clock() - startTime - genTime;
	//CERR(FormatString("[GodMoveSampling_v3::throwTile_detail]Used sample count: %u\n", m_uiUsedSampleCount));
	//CERR(FormatString("[GodMoveSampling_v3::throwTile_detail]Sampling time: %d ms.\n", sampleTime));
	selectionSortCandidate(m_iLeftDrawCount);
	time_t sortTime = clock() - startTime - genTime - sampleTime;
	//CERR(FormatString("[GodMoveSampling_v3::throwTile_detail]Sorting time: %d ms. %s\n", sortTime, m_oPlayerTile.toString().c_str()));
	return m_vCandidates;
}

vector<SamplingDataType> GodMoveSampling_v3::throwTile_detail(vector<Tile>& vThrownTiles/*, const bool & bUseTooManyMeldStrategy*/)
{
	//if (m_bUseTooManyMeldsStrategy && bUseTooManyMeldStrategy && m_iLeftDrawCount > m_oPlayerTile.getMinLack()) {
		//Use too meny meld strategy
	//}

	reset();
	for (Tile oThrownTile : vThrownTiles) {
		if (m_oPlayerTile.getHandTileNumber(oThrownTile) > 0) {
			Move oMove(MoveType::Move_Throw, oThrownTile);
			m_oPlayerTile.popTileFromHandTile(oThrownTile);
			SamplingDataType oData(oMove, m_oPlayerTile, m_oRemainTile, m_iLeftDrawCount, m_oGetTileType);
			m_vCandidates.push_back(oData);
			m_oPlayerTile.putTileToHandTile(oThrownTile);
		}
	}

	if (m_iConsideringChunkType == 1) {
		makeMoveDecisionWithChunkAvoidingType1();//NOTICE: The result "win rate - chunk rate" doesn't update into m_vCandidates
	}
	else if (m_iConsideringChunkType == 2) {
		makeMoveDecisionWithChunkAvoidingType2();
	}
	else {
		makeMoveDecision();
	}

	selectionSortCandidate(m_iLeftDrawCount);
	return m_vCandidates;
}

vector<SamplingDataType> GodMoveSampling_v3::sampleTheseMove(vector<Move>& vMoves)
{
	bool bAllThrowMove = true;//need better method to deal with this
	reset();

	//generate data after move
	for (Move oMove : vMoves) {
		switch (oMove.getMoveType()) {
		case MoveType::Move_Throw:
			m_oPlayerTile.popTileFromHandTile(oMove.getTargetTile());
			m_vCandidates.push_back(SamplingDataType(oMove, m_oPlayerTile, m_oRemainTile, m_iLeftDrawCount, m_oGetTileType));
			m_oPlayerTile.putTileToHandTile(oMove.getTargetTile());
			break;
		case MoveType::Move_Pass:
		case MoveType::Move_EatLeft:
		case MoveType::Move_EatMiddle:
		case MoveType::Move_EatRight:
		case MoveType::Move_Pong:
		case MoveType::Move_Kong:
		case MoveType::Move_DarkKong:
		case MoveType::Move_UpgradeKong:
			makeMeldSamplingData(oMove.getTargetTile(), oMove.getMoveType());
			bAllThrowMove = false;
			break;
		default:
			std::cerr << "[GodMoveSampling_v3::getBestMove] Illegal move type: " << oMove.toString() << std::endl;
			system("pause");
			exit(87);
		}
	}

	//compute
	if (getAvoidingChunkType() == 1 && bAllThrowMove) {
		makeMoveDecisionWithChunkAvoidingType1();//NOTICE: The result "win rate - chunk rate" doesn't update into m_vCandidates
	}
	else if (getAvoidingChunkType() == 2) {
		makeMoveDecisionWithChunkAvoidingType2();
	}
	else {
		makeMoveDecision();
	}

	selectionSortCandidate();
	return m_vCandidates;
}

MoveType GodMoveSampling_v3::meld(const Tile& oTile, const bool& bCanEat, const bool& bCanKong, const bool& bUseTooManyMeldStrategy, const bool& bConsideringChunk)
{
	reset();
	makeMeldSamplingData(Tile(), MoveType::Move_Pass);
	if (bCanEat) {
		if (m_oPlayerTile.canEatLeft(oTile))
			makeMeldSamplingData(oTile, MoveType::Move_EatLeft);
		if (m_oPlayerTile.canEatMiddle(oTile))
			makeMeldSamplingData(oTile, MoveType::Move_EatMiddle);
		if (m_oPlayerTile.canEatRight(oTile))
			makeMeldSamplingData(oTile, MoveType::Move_EatRight);
	}
	if (m_oPlayerTile.canPong(oTile))
		makeMeldSamplingData(oTile, MoveType::Move_Pong);
	if (bCanKong && m_oPlayerTile.canKong(oTile))
		makeMeldSamplingData(oTile, MoveType::Move_Kong);

	if (m_bUseTooManyMeldsStrategy && bUseTooManyMeldStrategy && isTooManyMelds()) {
		return tooManyMeldsStrategy().getMoveType();
	}

	SamplingDataType oBestData;
	if (bConsideringChunk && getAvoidingChunkType() == 2) {
		oBestData = makeMoveDecisionWithChunkAvoidingType2();
	}
	else {
		oBestData = makeMoveDecision();
	}

	if (m_bPrintDebugMsg)
		printCurrentCandidate();
	return oBestData.m_oAction.getMoveType();
}

vector<SamplingDataType> GodMoveSampling_v3::meld_detail(const Tile& oTile, const bool& bCanEat, const bool& bCanKong)
{
	reset();
	makeMeldSamplingData(Tile(), MoveType::Move_Pass);
	if (bCanEat) {
		if (m_oPlayerTile.canEatLeft(oTile))
			makeMeldSamplingData(oTile, MoveType::Move_EatLeft);
		if (m_oPlayerTile.canEatMiddle(oTile))
			makeMeldSamplingData(oTile, MoveType::Move_EatMiddle);
		if (m_oPlayerTile.canEatRight(oTile))
			makeMeldSamplingData(oTile, MoveType::Move_EatRight);
	}
	if (m_oPlayerTile.canPong(oTile))
		makeMeldSamplingData(oTile, MoveType::Move_Pong);
	if (bCanKong && m_oPlayerTile.canKong(oTile))
		makeMeldSamplingData(oTile, MoveType::Move_Kong);

	if (getAvoidingChunkType() == 2) {
		makeMoveDecisionWithChunkAvoidingType2();
	}
	else {
		makeMoveDecision();
	}
	return m_vCandidates;
}

MoveType GodMoveSampling_v3::meld_v2(const Tile& oTile, const bool& bCanEat, const bool& bCanKong, const bool& bUseTooManyMeldStrategy, const bool& bConsideringChunk)
{
	reset();
	makeMeldSamplingData_method2(Tile(), MoveType::Move_Pass);
	if (bCanEat) {
		if (m_oPlayerTile.canEatLeft(oTile))
			makeMeldSamplingData_method2(oTile, MoveType::Move_EatLeft);
		if (m_oPlayerTile.canEatMiddle(oTile))
			makeMeldSamplingData_method2(oTile, MoveType::Move_EatMiddle);
		if (m_oPlayerTile.canEatRight(oTile))
			makeMeldSamplingData_method2(oTile, MoveType::Move_EatRight);
	}
	if (m_oPlayerTile.canPong(oTile))
		makeMeldSamplingData_method2(oTile, MoveType::Move_Pong);
	if (bCanKong && m_oPlayerTile.canKong(oTile))
		makeMeldSamplingData_method2(oTile, MoveType::Move_Kong);

	if (m_bUseTooManyMeldsStrategy && bUseTooManyMeldStrategy && isTooManyMelds()) {
		return tooManyMeldsStrategy().getMoveType();
	}

	SamplingDataType oBestData;
	if (bConsideringChunk && getAvoidingChunkType() == 2) {
		oBestData = makeMoveDecisionWithChunkAvoidingType2();
	}
	else {
		oBestData = makeMoveDecision();
	}

	return oBestData.m_oAction.getMoveType();
}

vector<SamplingDataType> GodMoveSampling_v3::meld_v2_detail(const Tile& oTile, const bool& bCanEat, const bool& bCanKong)
{
	reset();
	makeMeldSamplingData_method2(Tile(), MoveType::Move_Pass);
	if (bCanEat) {
		if (m_oPlayerTile.canEatLeft(oTile))
			makeMeldSamplingData_method2(oTile, MoveType::Move_EatLeft);
		if (m_oPlayerTile.canEatMiddle(oTile))
			makeMeldSamplingData_method2(oTile, MoveType::Move_EatMiddle);
		if (m_oPlayerTile.canEatRight(oTile))
			makeMeldSamplingData_method2(oTile, MoveType::Move_EatRight);
	}
	if (m_oPlayerTile.canPong(oTile))
		makeMeldSamplingData_method2(oTile, MoveType::Move_Pong);
	if (bCanKong && m_oPlayerTile.canKong(oTile))
		makeMeldSamplingData_method2(oTile, MoveType::Move_Kong);

	makeMoveDecision();
	return m_vCandidates;
}

void GodMoveSampling_v3::makeMeldSamplingData_method2(const Tile& oTile, const MoveType& oMakeMeldType)
{
	//Generate all throw moves after meld
	//set player tile
	PlayerTile oPlayerTile = m_oPlayerTile;
	switch (oMakeMeldType) {
	case MoveType::Move_EatRight:
		oPlayerTile.eatRight(oTile);
		break;

	case MoveType::Move_EatMiddle:
		oPlayerTile.eatMiddle(oTile);
		break;

	case MoveType::Move_EatLeft:
		oPlayerTile.eatLeft(oTile);
		break;

	case MoveType::Move_Pong:
		oPlayerTile.pong(oTile);
		break;

	case MoveType::Move_Kong:
		oPlayerTile.kong(oTile);
		break;

	case MoveType::Move_DarkKong:
		oPlayerTile.darkKong(oTile);
		break;

	case MoveType::Move_UpgradeKong:
		oPlayerTile.upgradeKong(oTile);
		break;
	}

	//set draw count
	int iDrawCount;
	if (oMakeMeldType == MoveType::Move_Pass || oMakeMeldType == MoveType::Move_Kong) { iDrawCount = m_iLeftDrawCount; }
	else if (oMakeMeldType == MoveType::Move_DarkKong || oMakeMeldType == MoveType::Move_UpgradeKong) { iDrawCount = m_iLeftDrawCount + 1; }
	else { iDrawCount = m_iLeftDrawCount - 1; }//pong, eat

	//set SamplingData
	if (oMakeMeldType == MoveType::Move_EatRight
		|| oMakeMeldType == MoveType::Move_EatMiddle
		|| oMakeMeldType == MoveType::Move_EatLeft
		|| oMakeMeldType == MoveType::Move_Pong)
	{
		for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
			if (oPlayerTile.getHandTileNumber(i) > 0) {
				oPlayerTile.popTileFromHandTile(i);
				//SamplingDataType oMeld(Move(oMakeMeldType, oTile), oPlayerTile, m_oRemainTile, iDrawCount, m_oGetTileType);
				//m_vCandidates.push_back(oMeld);
				m_vCandidates.emplace_back(Move(oMakeMeldType, oTile), oPlayerTile, m_oRemainTile, iDrawCount, m_oGetTileType);
				oPlayerTile.putTileToHandTile(i);
			}
		}
	}
	else {
		//SamplingDataType oMeld(Move(oMakeMeldType, oTile), oPlayerTile, m_oRemainTile, iDrawCount, m_oGetTileType);
		//m_vCandidates.push_back(oMeld);
		m_vCandidates.emplace_back(Move(oMakeMeldType, oTile), oPlayerTile, m_oRemainTile, iDrawCount, m_oGetTileType);
	}
}

void GodMoveSampling_v3::testTime_PP()
{
	time_t iStartTime = clock();
	reset();
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		Tile oThrownTile(i);
		if (m_oPlayerTile.getHandTileNumber(oThrownTile) > 0) {
			Move oMove(MoveType::Move_Throw, oThrownTile);
			m_oPlayerTile.popTileFromHandTile(oThrownTile);
			SamplingDataType oData(oMove, m_oPlayerTile, m_oRemainTile, m_iLeftDrawCount, m_oGetTileType);
			m_vCandidates.push_back(oData);
			m_oPlayerTile.putTileToHandTile(oThrownTile);
		}
	}
	makeMoveDecision();
	std::cout << clock() - iStartTime << "\t";
	cout.flush();

	iStartTime = clock();
	reset();
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		Tile oThrownTile(i);
		if (m_oPlayerTile.getHandTileNumber(oThrownTile) > 0) {
			Move oMove(MoveType::Move_Throw, oThrownTile);
			m_oPlayerTile.popTileFromHandTile(oThrownTile);
			SamplingDataType oData(oMove, m_oPlayerTile, m_oRemainTile, m_iLeftDrawCount, m_oGetTileType);
			m_vCandidates.push_back(oData);
			m_oPlayerTile.putTileToHandTile(oThrownTile);
		}
	}
	while (m_uiUsedSampleCount < (m_uiSampleParameter - m_vCandidates.size())) {
		sampleAllCandidates();
	}
	std::cout << clock() - iStartTime << "\t";
	cout.flush();

	iStartTime = clock();
	reset();
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		Tile oThrownTile(i);
		if (m_oPlayerTile.getHandTileNumber(oThrownTile) > 0) {
			Move oMove(MoveType::Move_Throw, oThrownTile);
			m_oPlayerTile.popTileFromHandTile(oThrownTile);
			SamplingDataType oData(oMove, m_oPlayerTile, m_oRemainTile, m_iLeftDrawCount, m_oGetTileType);
			m_vCandidates.push_back(oData);
			m_oPlayerTile.putTileToHandTile(oThrownTile);
		}
	}
	sampleAllCandidatesNoPruning();
	std::cout << clock() - iStartTime << std::endl;
}

std::pair<MoveType, Tile> GodMoveSampling_v3::darkKongOrUpgradeKong(const bool& bUseTooManyMeldStrategy, const bool& bConsideringChunk)
{
	reset();
	//generate throw action candidate
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		Tile oThrownTile(i);
		if (m_oPlayerTile.getHandTileNumber(oThrownTile) > 0) {
			Move oMove(MoveType::Move_Throw, oThrownTile);
			m_oPlayerTile.popTileFromHandTile(oThrownTile);
			SamplingDataType oData(oMove, m_oPlayerTile, m_oRemainTile, m_iLeftDrawCount, m_oGetTileType);
			m_vCandidates.push_back(oData);
			m_oPlayerTile.putTileToHandTile(oThrownTile);
		}
	}

	//generate dark kong action candidate
	vector<Tile> vTile = m_oPlayerTile.getHandTile().getCanDarkKongTile();
	for (int i = 0; i < vTile.size(); ++i) {
		makeMeldSamplingData(vTile[i], MoveType::Move_DarkKong);
	}

	//generate upgrade kong action candidate
	vTile = m_oPlayerTile.getUpgradeKongTile();
	for (int i = 0; i < vTile.size(); ++i) {
		makeMeldSamplingData(vTile[i], MoveType::Move_UpgradeKong);
	}

	Move oBestMove;
	if (m_bUseTooManyMeldsStrategy && bUseTooManyMeldStrategy && isTooManyMelds()) {
		oBestMove = tooManyMeldsStrategy();
	}
	else if (bConsideringChunk && getAvoidingChunkType() == 2) {
		oBestMove = makeMoveDecisionWithChunkAvoidingType2().m_oAction;
	}
	else {
		oBestMove = makeMoveDecision().m_oAction;
	}

	if (m_bPrintDebugMsg)
		printCurrentCandidate();

	if (oBestMove.getMoveType() == MoveType::Move_Throw) {
		return std::pair<MoveType, Tile>(MoveType::Move_Pass, Tile());
	}
	return std::pair<MoveType, Tile>(oBestMove.getMoveType(), oBestMove.getTargetTile());
}

vector<SamplingDataType> GodMoveSampling_v3::darkKongOrUpgradeKong_detail()
{
	reset();
	//generate throw action candidate
	for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		Tile oThrownTile(i);
		if (m_oPlayerTile.getHandTileNumber(oThrownTile) > 0) {
			Move oMove(MoveType::Move_Throw, oThrownTile);
			m_oPlayerTile.popTileFromHandTile(oThrownTile);
			SamplingDataType oData(oMove, m_oPlayerTile, m_oRemainTile, m_iLeftDrawCount, m_oGetTileType);
			m_vCandidates.push_back(oData);
			m_oPlayerTile.putTileToHandTile(oThrownTile);
		}
	}

	//generate dark kong action candidate
	vector<Tile> vTile = m_oPlayerTile.getHandTile().getCanDarkKongTile();
	for (int i = 0; i < vTile.size(); ++i) {
		makeMeldSamplingData(vTile[i], MoveType::Move_DarkKong);
	}

	//generate upgrade kong action candidate
	vTile = m_oPlayerTile.getUpgradeKongTile();
	for (int i = 0; i < vTile.size(); ++i) {
		makeMeldSamplingData(vTile[i], MoveType::Move_UpgradeKong);
	}

	if (m_iConsideringChunkType == 2) {
		makeMoveDecisionWithChunkAvoidingType2();
	}
	else {
		makeMoveDecision();
	}
	return m_vCandidates;
}

Move GodMoveSampling_v3::tooManyMeldsStrategy()
{
	m_iSampleCountG1D1 = m_iSampleCountG1D2 = m_iSampleCountG2D1 = m_iSampleCountG2D2 = 0;//for experiment
	//split into 2 groups
	vector<Move> vMoveList_Group1;//Melds count is decrease after the throw move
	vector<Move> vMoveList_Group2;//Melds count is not decrease after the throw move = still too many melds

	//compute max uncompleted melds count
	//if getMinLack(MAX_NEED_GROUP + 1) > getMinLack(MAX_NEED_GROUP) + 1, throwTile_TooManyMelds() should not be executed.
	int iMaxUncompletedGroupCount = NEED_GROUP + 1;
	while (m_oPlayerTile.getMinLack(iMaxUncompletedGroupCount + 1) <= m_oPlayerTile.getMinLack(iMaxUncompletedGroupCount) + 1) {
		iMaxUncompletedGroupCount++;
	}
	CERR(FormatString("Max uncompleted meld count: %d\n", iMaxUncompletedGroupCount));

	//setup 2 groups
	for (int i = 0; i < m_vCandidates.size(); i++) {
		bool bTooManyMeld = m_vCandidates.at(i).m_oPlayerTile.getMinLack(iMaxUncompletedGroupCount) <= m_vCandidates.at(i).m_oPlayerTile.getMinLack(iMaxUncompletedGroupCount - 1) + 1;
		if (bTooManyMeld) {//still too many meld
			vMoveList_Group2.push_back(m_vCandidates.at(i).m_oAction);
		}
		else
			vMoveList_Group1.push_back(m_vCandidates.at(i).m_oAction);
		CERR(m_vCandidates.at(i).m_oAction.toString() + FormatString("(%d)\n", bTooManyMeld));
	}

	int iGroup2Count = vMoveList_Group2.size();
	if (vMoveList_Group1.size() == 0 || vMoveList_Group2.size() == 0) {
		//These situations don't need to use too many melds strategy -> use original sampling method:
		//(1) All moves are breaking meld moves (ex: 1289m1289s)
		//(2) All moves are non-breaking meld moves (ex: 678m11334668899s555z)
		return makeMoveDecision(false).m_oAction;
	}

	time_t dStartTime = clock();
	uint32_t ullLeftParam = m_uiSampleParameter;
	int iLeftSearchCount = m_bUseMultiThread ? 3 : (iGroup2Count + 2);
	//int iLeftSearchCount = 3;//used for tournament version (with time limit)
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] ullLeftParam = %llu , iLeftSearchCount = %d\n", ullLeftParam, iLeftSearchCount));
	const uint32_t cullSampleParam_Group1Depth1 = ullLeftParam / iLeftSearchCount;
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] cullSampleParam_Group1Depth1 = %llu \n", cullSampleParam_Group1Depth1));

	//Search group 1 depth 1
	CERR("[Group 1 Depth 1]\n");
	GodMoveSampling_v3 oSampling_Group1Depth1(m_oPlayerTile, m_oRemainTile, m_oSampleMode, cullSampleParam_Group1Depth1, m_iLeftDrawCount);
	vector<SamplingDataType> vData_Group1Depth1 = oSampling_Group1Depth1.sampleTheseMove(vMoveList_Group1);
	m_uiUsedSampleCount += oSampling_Group1Depth1.getUsedSampleCount();
	m_iSampleCountG1D1 = oSampling_Group1Depth1.getUsedSampleCount();

	//Get best data in group 1 depth 1
	int iBestIndex_Group1Depth1 = 0;
	for (int i = 1; i < vData_Group1Depth1.size(); i++) {
		if (vData_Group1Depth1[i].getWinRate() > vData_Group1Depth1[iBestIndex_Group1Depth1].getWinRate()) {
			iBestIndex_Group1Depth1 = i;
		}
	}
	SamplingDataType oBestData_Group1Depth1 = vData_Group1Depth1[iBestIndex_Group1Depth1];
	Move oBestMove_Group1Depth1 = oBestData_Group1Depth1.m_oAction;
	WinRate_t dBestWinRate_Group1Depth1 = oBestData_Group1Depth1.getWinRate();
	for (SamplingDataType oData : vData_Group1Depth1) {
		CERR(oData.m_oAction.getTargetTile().toString() + FormatString("(%lf)\n", oData.getWinRate()));
	}
	CERR("Best Data 1: " + oBestMove_Group1Depth1.toString() + FormatString("(%lf)\n\n", dBestWinRate_Group1Depth1));
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] Group1Depth1 used time:  %d ms.\n", clock() - dStartTime));
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] G1D1 used %u sample count.\n", oSampling_Group1Depth1.getUsedSampleCount()));


	//update search param
	uint32_t ullUsedParam = m_oSampleMode == SampleMode::Mode_Time ? (clock() - dStartTime) : oSampling_Group1Depth1.getUsedSampleCount();
	ullLeftParam -= ullUsedParam;
	iLeftSearchCount--;

	//Search group 1 depth 2
	CERR("[Group 1 Depth 2]\n");
	dStartTime = clock();
	m_oPlayerTile.doAction(oBestMove_Group1Depth1);

	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] ullLeftParam = %llu , iLeftSearchCount = %d\n", ullLeftParam, iLeftSearchCount));
	const uint32_t cullSampleParam_Group1Depth2 = ullLeftParam / iLeftSearchCount;//need bigger? (candidate count may too much, ex: 9 for 345668m 3566p 1346889s)
	//const uint32_t cullSampleParam_Group1Depth2 = max(ullLeftParam / iLeftSearchCount, 50000ui32);
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] cullSampleParam_Group1Depth2 = %llu \n", cullSampleParam_Group1Depth2));

	GodMoveSampling_v3 oSampling_Group1Depth2(m_oPlayerTile, m_oRemainTile, m_oSampleMode, cullSampleParam_Group1Depth2, m_iLeftDrawCount);
	vector<SamplingDataType> vData_Group1Depth2 = oSampling_Group1Depth2.throwTile_detail();
	vector<Move> vGoodEnoughThrow_Group1 = oSampling_Group1Depth2.getGoodEnoughMove();
	m_oPlayerTile.undoAction(oBestMove_Group1Depth1);
	m_uiUsedSampleCount += oSampling_Group1Depth2.getUsedSampleCount();
	m_iSampleCountG1D2 = oSampling_Group1Depth2.getUsedSampleCount();
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] Group1Depth2 used time:  %d ms.\n", clock() - dStartTime));
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] G1D2 used %u sample count.\n", oSampling_Group1Depth2.getUsedSampleCount()));

	//update search param
	ullUsedParam = m_oSampleMode == SampleMode::Mode_Time ? (clock() - dStartTime) : oSampling_Group1Depth2.getUsedSampleCount();
	ullLeftParam -= ullUsedParam;
	iLeftSearchCount--;

	//Search group 2 depth 1
	CERR("[Group 2 Depth 1]\n");
	dStartTime = clock();
	uint32_t uiUsedSampleCountBeforeG2D1 = m_uiUsedSampleCount;//for print debug msg
	//const uint32_t ullSampleParam_Group2Depth1 = 5000;//Mode_Sample
	//const uint32_t ullSampleParam_Group2Depth1 = min(static_cast<uint32_t>(5000), ullLeftParam * 0.5 / iLeftSearchCount);
	uint32_t ullSampleParam_Group2Depth1 = static_cast<uint32_t>(ullLeftParam / iLeftSearchCount);
	if (m_oSampleMode == SampleMode::Mode_Count) {
		ullSampleParam_Group2Depth1 = std::min(ullSampleParam_Group2Depth1 / 2, 5000U);
	}
	else if (m_bUseMultiThread) {
		//SampleMode::Mode_Time && m_bUseMultiThread -> able to sample simultaneously
		ullSampleParam_Group2Depth1 = ullLeftParam / 2;
	}

	const int ciGroup2Size = vMoveList_Group2.size();
	shared_ptr<SamplingDataType[]> vData_Group2Depth1(new SamplingDataType[ciGroup2Size], std::default_delete<SamplingDataType[]>());
	vector<std::future<void>> vSampleFuture_Group2Depth1;
	vSampleFuture_Group2Depth1.reserve(ciGroup2Size);
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] ullLeftParam = %llu , iLeftSearchCount = %d\n", ullLeftParam, iLeftSearchCount));
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] ullSampleParam_Group2Depth1 = %llu \n", ullSampleParam_Group2Depth1));

	for (int i = 0; i < ciGroup2Size; i++) {
		Move oMove = vMoveList_Group2.at(i);
		m_oPlayerTile.doAction(oMove);

		vData_Group2Depth1[i] = SamplingDataType(oMove, m_oPlayerTile, m_oRemainTile, m_iLeftDrawCount, m_oGetTileType);
		if (m_bUseMultiThread) {
			if (m_iConsideringChunkType == 2)
				vSampleFuture_Group2Depth1.push_back(std::async(std::launch::async, sampleCandidateWithChunkAvoiding, std::ref(vData_Group2Depth1[i]), ullSampleParam_Group2Depth1));
			else
				vSampleFuture_Group2Depth1.push_back(std::async(std::launch::async, sampleCandidate, std::ref(vData_Group2Depth1[i]), ullSampleParam_Group2Depth1));
		}
		else {
			vData_Group2Depth1[i].sample(ullSampleParam_Group2Depth1);
		}
		m_oPlayerTile.undoAction(oMove);

	}

	//wait for all thread is finished
	if (m_bUseMultiThread) {
		for (int i = 0; i < ciGroup2Size; i++) {
			vSampleFuture_Group2Depth1[i].get();

		}
	}

	for (int i = 0; i < ciGroup2Size; i++) {
		m_uiUsedSampleCount += vData_Group2Depth1[i].m_iUsedSampleCount;
	}
	m_iSampleCountG2D1 = m_uiUsedSampleCount - uiUsedSampleCountBeforeG2D1;
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] Group2Depth1 used time:  %d ms.\n", clock() - dStartTime));
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] G2D1 used %u sample count.\n", m_uiUsedSampleCount - uiUsedSampleCountBeforeG2D1));

	//update search param
	ullUsedParam = m_oSampleMode == SampleMode::Mode_Time ? (clock() - dStartTime) : (m_uiUsedSampleCount - uiUsedSampleCountBeforeG2D1);
	ullLeftParam -= ullUsedParam;

	//get the best one of group 2 depth 1
	int iBestIndex_Group2Depth1 = 0;
	WinRate_t dBestWinRate_Group2Depth1 = 0.0;
	for (int i = 0; i < ciGroup2Size; i++) {
		WinRate_t dWinRate = vData_Group2Depth1[i].getWinRate();
		if (dWinRate > dBestWinRate_Group2Depth1) {
			iBestIndex_Group2Depth1 = i;
			dBestWinRate_Group2Depth1 = dWinRate;
		}
		CERR("[Action " + vData_Group2Depth1[i].m_oAction.toString() + "] " + FormatString("%lf\n", vData_Group2Depth1[i].getWinRate()));
	}

	//[Extra Rule] If G1D1 is better than G2D1, choose G1D1
	//fast return
	if (m_bUseDepth1FastReturn && dBestWinRate_Group2Depth1 < dBestWinRate_Group1Depth1) {
		//Highest depth 1 win rate although breaking meld [group 1]
		CERR("FinalAnswer: " + oBestMove_Group1Depth1.toString() + ". (Highest depth 1 win rate although breaking meld [group 1])\n");
		return oBestMove_Group1Depth1;
	}

	//get good enough move in group 2 depth 1
	vector<Move> vGoodEnoughThrow_Group2;
	WinRate_t dBestMove_LowerBound_Group2Depth1 = dBestWinRate_Group2Depth1 - m_iUsedSDCount * vData_Group2Depth1[iBestIndex_Group2Depth1].getSD();
	CERR(FormatString("Best lower bound: %s(%lf)\n", vData_Group2Depth1[iBestIndex_Group2Depth1].m_oAction.toString().c_str(), dBestMove_LowerBound_Group2Depth1));
	CERR("Upper bound:\n");
	for (int i = 0; i < ciGroup2Size; i++) {
		WinRate_t dUpperBound = vData_Group2Depth1[i].getWinRate() + m_iUsedSDCount * vData_Group2Depth1[i].getSD();
		if (dUpperBound >= dBestMove_LowerBound_Group2Depth1) {
			vGoodEnoughThrow_Group2.push_back(vData_Group2Depth1[i].m_oAction);
		}
		CERR(FormatString("%s(%lf)\n", vData_Group2Depth1[i].m_oAction.toString().c_str(), dUpperBound));
	}

	//check if exist a tile in vGoodEnoughThrow_Group1 & vGoodEnoughThrow_Group2
	vector<Move> vGoodEnoughInBothGroup;
	for (Move oMove_Group1 : vGoodEnoughThrow_Group1) {
		for (Move oMove_Group2 : vGoodEnoughThrow_Group2) {
			if (oMove_Group1 == oMove_Group2) {
				vGoodEnoughInBothGroup.push_back(oMove_Group1);
			}
		}
	}


	//[Rule 2] If exists, return the move with best group 2 depth 1 win rate.
	//fast return
	if (m_bUseRule2 && m_bUseRule2FastReturn && vGoodEnoughInBothGroup.size() > 0) {
		//find best throw in group 2 good enough list
		CERR("FinalGoodEnoughList:\n");
		Move oBestMove_Final = vGoodEnoughInBothGroup.at(0);
		WinRate_t dBestWinRate_Final = 0.0;
		for (Move oGoodEnoughMove : vGoodEnoughInBothGroup) {
			for (int i = 0; i < ciGroup2Size; i++) {
				WinRate_t dWinRate = vData_Group2Depth1[i].getWinRate();
				if (oGoodEnoughMove == vData_Group2Depth1[i].m_oAction) {
					if (dWinRate >= dBestWinRate_Final) {
						oBestMove_Final = oGoodEnoughMove;
						dBestWinRate_Final = dWinRate;
					}
					CERR(FormatString("%s (%lf)\n", oGoodEnoughMove.toString().c_str(), dWinRate));
				}
			}
		}
		CERR("Final answer: " + oBestMove_Final.toString() + " (Good enough).\n");
		return oBestMove_Final;
	}

	/*
	//[optional] If group 2 move is discarding alone tile, return group 2 move
	//else skip group2 depth2, return group 1 answer
	if (oBestMove_Group1Depth1.getMoveType() == MoveType::Move_Throw) {
		Move oBestMove_Group2Depth1 = vData_Group2Depth1[iBestIndex_Group2Depth1].m_oAction;
		Tile oBestThrownTile_Group2Depth1 = oBestMove_Group2Depth1.getTargetTile();//[bug]how about non-throwing action?
		if (m_oPlayerTile.isAloneTile(oBestThrownTile_Group2Depth1)) {
			return vData_Group2Depth1[iBestIndex_Group2Depth1].m_oAction;
		}
		return oBestMove_Group1Depth1;
	}*/


	//Search group 2 depth 2
	CERR("[Group 2 Depth 2]\n");
	Move oBestMove_Group2 = vMoveList_Group2.at(0);
	double dBestWinRate_Group2Depth2 = 0.0;
	uint32_t ullSampleParam_Group2Depth2;
	//vector<std::future<SamplingDataType>> vSampleFuture_Group2Depth2;
	vector<std::future<std::pair<SamplingDataType, uint32_t>>> vSampleFuture_Group2Depth2;
	vSampleFuture_Group2Depth2.reserve(vMoveList_Group2.size());
	uint32_t uiUsedSampleCountBeforeG2D2 = m_uiUsedSampleCount;

	for (int i = 0; i < vMoveList_Group2.size(); i++) {
		Move oMove = vMoveList_Group2.at(i);

		if (m_bUseMultiThread) {
			if (i == 0) {
				ullSampleParam_Group2Depth2 = ullLeftParam;
				dStartTime = clock();
				CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] ullLeftParam = %llu , iLeftSearchCount = %d\n", ullLeftParam, iLeftSearchCount));
				CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] ullSampleParam_Group2Depth2 = %llu \n", ullSampleParam_Group2Depth2));
			}
			m_oPlayerTile.doAction(oMove);
			vSampleFuture_Group2Depth2.push_back(std::async(std::launch::async, getBestThrowWinRate, m_oPlayerTile, m_oRemainTile, m_oSampleMode, ullSampleParam_Group2Depth2, m_iLeftDrawCount));
			m_oPlayerTile.undoAction(oMove);
			//update search param
			if (m_oSampleMode == SampleMode::Mode_Time) {
				ullSampleParam_Group2Depth2 -= (clock() - dStartTime);
			}
		}
		else {
			ullSampleParam_Group2Depth2 = ullLeftParam / iLeftSearchCount;
			CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] ullLeftParam = %llu , iLeftSearchCount = %d\n", ullLeftParam, iLeftSearchCount));
			CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] cullSampleParam_Group2Depth2 = %llu \n", ullSampleParam_Group2Depth2));
			dStartTime = clock();

			CERR("[Action " + vData_Group2Depth1[i].m_oAction.toString() + "] " + FormatString("%lf\n", vData_Group2Depth1[i].getWinRate()));

			m_oPlayerTile.doAction(oMove);
			GodMoveSampling_v3 oSampling_Group2Depth2(m_oPlayerTile, m_oRemainTile, m_oSampleMode, ullSampleParam_Group2Depth2, m_iLeftDrawCount);
			vector<SamplingDataType> vData_Group2Depth2 = oSampling_Group2Depth2.throwTile_detail();
			WinRate_t dWinRate = vData_Group2Depth2.at(0).getWinRate();
			m_oPlayerTile.undoAction(oMove);
			m_uiUsedSampleCount += oSampling_Group2Depth2.getUsedSampleCount();


			CERR("[Second throw: " + vData_Group2Depth2.at(0).m_oAction.toString() + "] " + FormatString("%lf\n", dWinRate));
			//update best throw of group2 depth 2
			if (dWinRate >= dBestWinRate_Group2Depth2) {
				oBestMove_Group2 = oMove;
				dBestWinRate_Group2Depth2 = dWinRate;
			}
			CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] Group2Depth2(%d) used time:  %d ms.\n", i + 1, clock() - dStartTime));

			//update search param
			uint32_t ullUsedParam = m_oSampleMode == SampleMode::Mode_Time ? (clock() - dStartTime) : oSampling_Group2Depth2.getUsedSampleCount();
			ullLeftParam -= ullUsedParam;
			iLeftSearchCount--;
		}
	}

	//wait for all thread is finished
	if (m_bUseMultiThread) {
		vector<std::pair<SamplingDataType, uint32_t>> vData_Group2Depth2(vMoveList_Group2.size());
		for (int i = 0; i < vSampleFuture_Group2Depth2.size(); i++) {
			vData_Group2Depth2[i] = vSampleFuture_Group2Depth2[i].get();
			m_uiUsedSampleCount += vData_Group2Depth2.at(i).second;
		}


		time_t usedTime = clock() - dStartTime;
		for (int i = 0; i < vSampleFuture_Group2Depth2.size(); i++) {
			WinRate_t dWinRate = vData_Group2Depth2.at(i).first.getWinRate();
			//debug msg
			CERR("[Action " + vData_Group2Depth1[i].m_oAction.toString() + "] " + FormatString("%lf\n", vData_Group2Depth1[i].getWinRate()));
			CERR("[Second throw: " + vData_Group2Depth2.at(i).first.m_oAction.toString() + "] " + FormatString("%lf\n", dWinRate));
			//update best throw of group2 depth 2
			if (dWinRate >= dBestWinRate_Group2Depth2) {
				oBestMove_Group2 = vMoveList_Group2.at(i);
				dBestWinRate_Group2Depth2 = dWinRate;
			}
		}
		CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] Group2Depth2 used time:  %d ms.\n", usedTime));
	}

	m_iSampleCountG2D2 = m_uiUsedSampleCount - uiUsedSampleCountBeforeG2D2;
	CERR(FormatString("[GodMoveSampling_v3::throwTile_TooManyMelds] G2D1 used %u sample count.\n", m_uiUsedSampleCount - uiUsedSampleCountBeforeG2D2));
	CERR("[Best depth 2 throw: " + oBestMove_Group2.toString() + "] " + FormatString("%lf\n", dBestWinRate_Group2Depth2));

	//[Extra Rule] If G1D1 is better than G2D1, choose G1D1
	//entirely search return
	if (!m_bUseDepth1FastReturn && dBestWinRate_Group2Depth1 < dBestWinRate_Group1Depth1) {
		//Highest depth 1 win rate although breaking meld [group 1]
		CERR("FinalAnswer: " + oBestMove_Group1Depth1.toString() + ". (Highest depth 1 win rate although breaking meld [group 1])\n");
		return oBestMove_Group1Depth1;
	}

	//[Rule 2] If exists, return the move with best group 2 depth 1 win rate.
	//entirely search return
	if (m_bUseRule2 && !m_bUseRule2FastReturn && vGoodEnoughInBothGroup.size() > 0) {
		//find best throw in group 2 good enough list
		CERR("FinalGoodEnoughList:\n");
		Move oBestMove_Final = vGoodEnoughInBothGroup.at(0);
		WinRate_t dBestWinRate_Final = 0.0;
		for (Move oGoodEnoughMove : vGoodEnoughInBothGroup) {
			for (int i = 0; i < ciGroup2Size; i++) {
				WinRate_t dWinRate = vData_Group2Depth1[i].getWinRate();
				if (oGoodEnoughMove == vData_Group2Depth1[i].m_oAction) {
					if (dWinRate >= dBestWinRate_Final) {
						oBestMove_Final = oGoodEnoughMove;
						dBestWinRate_Final = dWinRate;
					}
					CERR(FormatString("%s (%lf)\n", oGoodEnoughMove.toString().c_str(), dWinRate));
				}
			}
		}
		CERR("Final answer: " + oBestMove_Final.toString() + " (Good enough).\n");
		return oBestMove_Final;
	}


	//[Rule 1] Final move decision: throw higher 2nd throw win rate
	if (vData_Group1Depth2.at(0).getWinRate() > dBestWinRate_Group2Depth2) {//about 80%
		CERR("Final answer: " + oBestMove_Group1Depth1.toString() + " (Highest 2nd throw win rate (group 1)).\n");
		return oBestMove_Group1Depth1;
	}
	//about 20%
	CERR("Final answer: " + oBestMove_Group2.toString() + " (Highest 2nd throw win rate (group 2)).\n");
	return oBestMove_Group2;
}

vector<Move> GodMoveSampling_v3::getGoodEnoughMove() const
{
	//NOTICE: This function should be called after throwTile, meld or darkKongOrUpgradeKong.
	vector<Move> vAns;

	//find the best
	int iBestIndex = 0;
	WinRate_t dBestWinRate = 0.0;
	CERR("[GoodEnoughMove]vCandidate:\n");
	for (int i = 0; i < m_vCandidates.size(); i++) {
		WinRate_t dWinRate = m_vCandidates.at(i).getWinRate();
		CERR(FormatString("%s(%lf)\n", m_vCandidates.at(i).m_oAction.toString().c_str(), m_vCandidates.at(i).getWinRate()));
		if (dWinRate > dBestWinRate) {
			iBestIndex = i;
			dBestWinRate = dWinRate;
		}
	}

	//check if the range of the candidate overlaps the range of the best candidate
	WinRate_t dBestCandidateLowerBound = m_vCandidates.at(iBestIndex).getWinRate() - m_iUsedSDCount * m_vCandidates.at(iBestIndex).getSD();
	CERR(FormatString("\nBest move: %s(LowerBound = %lf)\n\n", m_vCandidates.at(iBestIndex).m_oAction.toString().c_str(), dBestCandidateLowerBound));
	CERR("UpperBound:\n");
	for (SamplingDataType oCandidate : m_vCandidates) {
		WinRate_t dUpperBound = oCandidate.getWinRate() + m_iUsedSDCount * oCandidate.getSD();
		if (dUpperBound >= dBestCandidateLowerBound) {
			vAns.push_back(oCandidate.m_oAction);
			CERR("*");
		}
		CERR(FormatString("%s(UpperBound = %lf)\n", oCandidate.m_oAction.toString().c_str(), dUpperBound));
	}

	CERR("vAns:\n");
	for (Move oAns : vAns) {
		CERR(FormatString("%s\n", oAns.toString().c_str()));
	}
	return vAns;
}

SamplingDataType GodMoveSampling_v3::makeMoveDecision(const bool& bConsideringChunk)
{
	//---debug msg---
	CERR("[makeMoveDecision]\n");
	CERR(FormatString("mo count: %d\n", m_iLeftDrawCount));
	if (m_oSampleMode == SampleMode::Mode_Count) {
		CERR(FormatString("Sample mode: UseCount\n"));
		CERR(FormatString("Sample count : %llu\n", m_uiSampleParameter));
	}
	else {
		CERR(FormatString("Sample mode: UseTime\n"));
		CERR(FormatString("Sample time : %llu ms\n", m_uiSampleParameter));
	}
	CERR(FormatString("PlayerTile: %s\n", m_oPlayerTile.toString().c_str()));
	CERR(FormatString("RemainTile:\n%s\n", m_oRemainTile.getReadableString().c_str()));
	//---end debug msg---

	if (m_vCandidates.size() == 1) {
		sampleOneCandidate();
		return getBestCandidate();
	}
	/*if (!m_bPruning) {
		sampleAllCandidatesNoPruning();
		return getBestCandidate();
	}*/

	m_bFirstRound = true;
	m_iLeftCandidateCount = m_vCandidates.size();
	while (m_iLeftCandidateCount > 1) {

		if (m_oSampleMode == SampleMode::Mode_Count && m_uiUsedSampleCount >= (m_uiSampleParameter - m_vCandidates.size())) {//sample count
			break;
		}
		if (m_oSampleMode == SampleMode::Mode_Time && clock() - m_uiStartTime >= m_uiSampleParameter) {//time's up
			break;
		}

		sampleAllCandidates();
		CERR("sampleAllCandidates done.\n");
		CERR(FormatString("[makeMoveDecision]Used sample count: %d.\n", m_uiUsedSampleCount));
		CERR(FormatString("[makeMoveDecision]Used time: %d ms.\n", clock() - m_uiStartTime));
	}

	return getBestCandidate();
}

SamplingDataType GodMoveSampling_v3::makeMoveDecisionWithChunkAvoidingType1()
{
	//---debug msg---
	CERR("[makeMoveDecisionWithChunkAvoidingType1]\n");
	CERR(FormatString("mo count: %d\n", m_iLeftDrawCount));
	if (m_oSampleMode == SampleMode::Mode_Count) {
		CERR(FormatString("Sample mode: UseCount\n"));
		CERR(FormatString("Sample count : %llu\n", m_uiSampleParameter));
	}
	else {
		CERR(FormatString("Sample mode: UseTime\n"));
		CERR(FormatString("Sample time : %llu ms\n", m_uiSampleParameter));
	}
	CERR(FormatString("PlayerTile: %s\n", m_oPlayerTile.toString().c_str()));
	CERR(FormatString("RemainTile:\n%s\n", m_oRemainTile.getReadableString().c_str()));
	//---end debug msg---

	m_bFirstRound = true;
	m_iLeftCandidateCount = m_vCandidates.size();
	while (m_iLeftCandidateCount > 1) {
		if (m_oSampleMode == SampleMode::Mode_Count && m_uiUsedSampleCount >= (m_uiSampleParameter - m_vCandidates.size())) {//sample count
			break;
		}
		if (m_oSampleMode == SampleMode::Mode_Time && clock() - m_uiStartTime >= m_uiSampleParameter) {//time's up
			break;
		}

		sampleAllCandidates();
	}

	return getBestCandidateWithChunkAvoiding();
}

SamplingDataType GodMoveSampling_v3::makeMoveDecisionWithChunkAvoidingType2()
{
	//---debug msg---
	CERR("[makeMoveDecisionWithChunkAvoidingType2]\n");
	CERR(FormatString("mo count: %d\n", m_iLeftDrawCount));
	if (m_oSampleMode == SampleMode::Mode_Count) {
		CERR("Sample mode: UseCount\n");
		CERR(FormatString("Sample count : %llu\n", m_uiSampleParameter));
	}
	else {
		CERR("Sample mode: UseTime\n");
		CERR(FormatString("Sample time : %llu ms\n", m_uiSampleParameter));
	}
	CERR("PlayerTile: " + m_oPlayerTile.toString() + "\n");
	CERR("RemainTile:\n" + m_oRemainTile.getReadableString() + "\n");
	//---end debug msg---

	m_bFirstRound = true;
	m_iLeftCandidateCount = m_vCandidates.size();
	while (m_iLeftCandidateCount > 1) {
		if (m_oSampleMode == SampleMode::Mode_Count && m_uiUsedSampleCount >= (m_uiSampleParameter - m_vCandidates.size())) {//sample count
			break;
		}
		if (m_oSampleMode == SampleMode::Mode_Time && clock() - m_uiStartTime >= m_uiSampleParameter) {//time's up
			break;
		}

		sampleAllCandidatesWithChunkAvoiding();
	}

	return getBestCandidate();
}

SamplingDataType GodMoveSampling_v3::makeMoveDecision_UCB(const bool& bConsideringChunk)
{
	//---debug msg---
	CERR("[makeMoveDecision_UCB]\n");
	CERR(FormatString("mo count: %d\n", m_iLeftDrawCount));
	if (m_oSampleMode == SampleMode::Mode_Count) {
		CERR(FormatString("Sample mode: UseCount\n"));
		CERR(FormatString("Sample count : %llu\n", m_uiSampleParameter));
	}
	else {
		CERR(FormatString("Sample mode: UseTime\n"));
		CERR(FormatString("Sample time : %llu ms\n", m_uiSampleParameter));
	}
	CERR(FormatString("PlayerTile: %s\n", m_oPlayerTile.toString().c_str()));
	CERR(FormatString("RemainTile:\n%s\n", m_oRemainTile.getReadableString().c_str()));
	//---end debug msg---
	assert(m_vCandidates.size() > 0);

	m_iLeftCandidateCount = m_vCandidates.size();
	while (true) {
		int iSelectedIndex = getUcbBestDataIndex();
		CERR(FormatString("select data %d (action: %s)\n", iSelectedIndex, m_vCandidates[iSelectedIndex].m_oAction.toString()));
		m_vCandidates[iSelectedIndex].sample(m_uiSamplePerRound);
		m_uiUsedSampleCount += m_uiSamplePerRound;

		if (m_oSampleMode == SampleMode::Mode_Count && m_uiUsedSampleCount >= (m_uiSampleParameter - m_vCandidates.size())) {//sample count
			break;
		}
		if (m_oSampleMode == SampleMode::Mode_Time && clock() - m_uiStartTime >= m_uiSampleParameter) {//time's up
			break;
		}
		if (m_iLeftCandidateCount == 1)
			break;


		/*CERR("sampleAllCandidates done.\n");
		CERR(FormatString("[makeMoveDecision]Used sample count: %d.\n", m_ullUsedSampleCount));
		CERR(FormatString("[makeMoveDecision]Used time: %d ms.\n", clock() - m_uiStartTime));*/
	}

	return getBestCandidate();
}

int GodMoveSampling_v3::getUcbBestDataIndex() const
{
	assert(m_vCandidates.size() > 0);
	int iBestIndex = 0;
	CERR(FormatString("%s ", m_vCandidates.at(0).m_oAction.toString()));
	double dBestUcbScore = getUcbScore(m_vCandidates.at(0));


	auto iCandidateCount = m_vCandidates.size();
	for (int i = 1; i < iCandidateCount; i++) {
		CERR(FormatString("%s ", m_vCandidates.at(i).m_oAction.toString()));
		double dUcbScore = getUcbScore(m_vCandidates.at(i));
		if (dUcbScore > dBestUcbScore) {
			iBestIndex = i;
			dBestUcbScore = dUcbScore;
		}
	}
	return iBestIndex;
}

WinRate_t GodMoveSampling_v3::getUcbScore(const SamplingDataType& oData) const
{
	if (oData.m_iUsedSampleCount == 0) {
		CERR(" first time.\n");
		return 1024.0;
	}
	int iVisitCount = oData.m_iUsedSampleCount;
	const WinRate_t dExplorationFactor = 0.2;
	CERR(FormatString("%lf = %lf + %lf * %lf (sqrt(log(%d) / %d))\n"
		, oData.getWinRate() + dExplorationFactor * sqrt(log(m_uiUsedSampleCount) / iVisitCount)
		, oData.getWinRate()
		, dExplorationFactor
		, sqrt(log(m_uiUsedSampleCount) / iVisitCount)
		, m_uiUsedSampleCount
		, iVisitCount));
	return oData.getWinRate() + dExplorationFactor * sqrt(log(m_uiUsedSampleCount) / iVisitCount);
}

void GodMoveSampling_v3::sampleAllCandidates()
{
	uint32_t uiSampleCountPerRound;

	if (m_bFirstRound) {
		uiSampleCountPerRound = m_uiSampleFirstRound;
		m_bFirstRound = false;
	}
	else {
		uiSampleCountPerRound = m_uiSamplePerRound;
	}
	if (m_oSampleMode == SampleMode::Mode_Count) {
		uiSampleCountPerRound = std::min(uiSampleCountPerRound, static_cast<uint32_t>((m_uiSampleParameter - m_uiUsedSampleCount) / m_vCandidates.size()));
		//uiSampleCountPerRound = max(1u, uiSampleCountPerRound);
	}
	CERR(FormatString("[GodMoveSampling_v3::sampleAllCandidates] m_uiSampleParameter = %u\n", m_uiSampleParameter));
	CERR(FormatString("[GodMoveSampling_v3::sampleAllCandidates] m_vCandidates size = %d\n", m_vCandidates.size()));
	CERR(FormatString("[GodMoveSampling_v3::sampleAllCandidates] max uiSampleCountPerRound = %d\n", m_uiSampleParameter / m_vCandidates.size()));
	CERR(FormatString("[GodMoveSampling_v3::sampleAllCandidates] uiSampleCountPerRound = %u\n", uiSampleCountPerRound));
	if (m_uiSampleParameter == 0) {
		CERR("Error!\n");
	}

	//start sampling
	if (m_bUseMultiThread) {//multi-thread
		vector<std::future<void>> vSampleFuture;
		vSampleFuture.reserve(m_vCandidates.size());

		for (int i = 0; i < m_vCandidates.size(); i++) {
			if (m_vCandidates[i].m_bIsPruned)
				continue;
			vSampleFuture.push_back(std::async(std::launch::async, sampleCandidate, std::ref(m_vCandidates[i]), uiSampleCountPerRound));
			m_uiUsedSampleCount += uiSampleCountPerRound;
		}
		for (int i = 0; i < vSampleFuture.size(); i++) {
			vSampleFuture[i].get();
		}
	}
	else {//single thread
		for (int i = 0; i < m_vCandidates.size(); i++) {
			if (m_vCandidates[i].m_bIsPruned)
				continue;
			sampleCandidate(m_vCandidates[i], uiSampleCountPerRound);
			m_uiUsedSampleCount += uiSampleCountPerRound;
		}
	}

	//find best data
	/*double dMaxScore = 0.0;
	int iBestIndex = 0;
	for (int i = 0; i < m_vCandidates.size(); i++) {
		if (m_vCandidates[i].m_bIsPruned)
			continue;
		double dScore = m_vCandidates[i].getWinRate();
		if (dScore >= dMaxScore) {
			iBestIndex = i;
			dMaxScore = dScore;
		}
	}

	//[Special case]avoid all zero win rate case -> give one more draw
	if (dMaxScore == 0.0) {
		CERR("[GodMoveSampling_v3::sampleAllCandidates]zero win rate appeared. Add one more draw count.\n");
		for (int i = 0; i < m_vCandidates.size(); i++) {
			m_vCandidates[i].m_iDrawCount++;
		}
	}

	//pruning
	if (m_bPruning) {
		double dLowerBound = dMaxScore - m_iUsedSDCount * m_vCandidates[iBestIndex].getSD();
		//start pruning
		for (int i = 0; i < m_vCandidates.size(); i++) {
			if (m_vCandidates[i].m_bIsPruned)
				continue;
			double dUpperBound = m_vCandidates[i].getWinRate() + m_iUsedSDCount * m_vCandidates[i].getSD();
			if (dUpperBound < dLowerBound) {
				m_vCandidates[i].m_bIsPruned = true;
				m_iLeftCandidateCount--;
			}
		}
	}*/
	prune();

	//debug
	//printCurrentCandidate();
	CERR(FormatString("[GodMoveSampling_v3::sampleAllCandidates] m_uiUsedSampleCount = %u\n", m_uiUsedSampleCount));
	/*if (m_uiUsedSampleCount > (m_uiSampleParameter + m_vCandidates.size())) {
		CERR("Used too many sample!\n");
	}*/
	//system("pause");
}

void GodMoveSampling_v3::sampleAllCandidatesWithChunkAvoiding()
{
	uint32_t uiSampleCountPerRound;

	if (m_bFirstRound) {
		uiSampleCountPerRound = m_uiSampleFirstRound;
		m_bFirstRound = false;
	}
	else {
		uiSampleCountPerRound = m_uiSamplePerRound;
	}
	if (m_oSampleMode == SampleMode::Mode_Count) {
		uiSampleCountPerRound = std::min(uiSampleCountPerRound, static_cast<uint32_t>(m_uiSampleParameter / m_vCandidates.size()));
		uiSampleCountPerRound = std::max(1u, uiSampleCountPerRound);
	}

	//start sampling
	if (m_bUseMultiThread) {
		vector<std::future<void>> vSampleFuture;
		vSampleFuture.reserve(m_vCandidates.size());

		for (int i = 0; i < m_vCandidates.size(); i++) {
			if (m_vCandidates[i].m_bIsPruned)
				continue;
			vSampleFuture.push_back(std::async(std::launch::async, sampleCandidateWithChunkAvoiding, std::ref(m_vCandidates[i]), uiSampleCountPerRound));
			m_uiUsedSampleCount += uiSampleCountPerRound;
		}
		for (int i = 0; i < vSampleFuture.size(); i++) {
			vSampleFuture[i].get();
		}
	}
	else {
		for (int i = 0; i < m_vCandidates.size(); i++) {
			if (m_vCandidates[i].m_bIsPruned)
				continue;
			sampleCandidate(m_vCandidates[i], uiSampleCountPerRound);
		}
		m_uiUsedSampleCount += uiSampleCountPerRound;
	}

	//find best data
	WinRate_t dMaxScore = 0.0;
	int iBestIndex = 0;
	for (int i = 0; i < m_vCandidates.size(); i++) {
		if (m_vCandidates[i].m_bIsPruned)
			continue;
		WinRate_t dScore = m_vCandidates[i].getWinRate();
		if (dScore >= dMaxScore) {
			iBestIndex = i;
			dMaxScore = dScore;
		}
	}

	//[Special case]avoid all zero win rate case -> give one more draw
	if (dMaxScore == 0.0) {
		for (int i = 0; i < m_vCandidates.size(); i++) {
			m_vCandidates[i].m_iDrawCount++;
		}
	}

	//pruning
	if (m_bPruning) {
		WinRate_t dLowerBound = dMaxScore - m_iUsedSDCount * m_vCandidates[iBestIndex].getSD();
		//start pruning
		for (int i = 0; i < m_vCandidates.size(); i++) {
			if (m_vCandidates[i].m_bIsPruned)
				continue;
			WinRate_t dUpperBound = m_vCandidates[i].getWinRate() + m_iUsedSDCount * m_vCandidates[i].getSD();
			if (dUpperBound < dLowerBound) {
				m_vCandidates[i].m_bIsPruned = true;
				m_iLeftCandidateCount--;
			}
		}
	}

	//debug
	//printCurrentCandidate();
	//system("pause");
}

void GodMoveSampling_v3::sampleOneCandidate()
{
	//only called when m_vCandidates.size() == 1
	assert(m_vCandidates.size() == 1);
	if (m_oSampleMode == SampleMode::Mode_Time) {
		time_t startTime = clock();
		while (clock() - startTime < m_uiSampleParameter) {
			sampleCandidate(m_vCandidates[0], 500);
		}
	}
	else {
		sampleCandidate(m_vCandidates[0], m_uiSampleParameter);
	}
}

void GodMoveSampling_v3::sampleAllCandidatesNoPruning()
{
	if (m_oSampleMode == SampleMode::Mode_Count) {
		uint32_t iSampleCountPerCandidate = m_uiSampleParameter / m_vCandidates.size();
		if (m_bUseMultiThread) {
			//TODO
		}
		else {
			for (int i = 0; i < m_vCandidates.size(); i++) {
				m_vCandidates[i].sample(iSampleCountPerCandidate);
				m_uiUsedSampleCount += iSampleCountPerCandidate;
			}
		}

	}
	else /*if (m_oSampleMode == SampleMode::Mode_Time)*/ {
		//TODO
		if (m_bUseMultiThread) {
			//TODO
		}
		else {
			//TODO
		}
	}
}

void GodMoveSampling_v3::sampleCandidate(SamplingDataType& oData, const unsigned int& cuiSampleParameter)
{
	if (m_oSampleMode == SampleMode::Mode_Count) {
		oData.sample(cuiSampleParameter);
		return;
	}

	//SampleMode::Mode_Time
	/*time_t startTime = clock();
	while (clock() - startTime < cuiSampleParameter) {
		oData.sample(500);
	}*/
	oData.sample(m_uiSamplePerRound);
}

void GodMoveSampling_v3::sampleCandidateWithChunkAvoiding(SamplingDataType& oData, const unsigned int& cuiSampleParameter)
{
	if (m_oSampleMode == SampleMode::Mode_Count) {
		oData.sample_withChunk(cuiSampleParameter);
		return;
	}

	//SampleMode::Mode_Time
	time_t startTime = clock();
	while (clock() - startTime < cuiSampleParameter) {
		oData.sample_withChunk(m_uiSamplePerRound);
	}
}

void GodMoveSampling_v3::prune()
{
	if (!m_bPruning) {//better to be controlled by caller
		return;
	}

	//find best data
	WinRate_t dMaxScore = 0.0;
	int iBestIndex = 0;
	for (int i = 0; i < m_vCandidates.size(); i++) {
		if (m_vCandidates[i].m_bIsPruned)
			continue;
		WinRate_t dScore = m_vCandidates[i].getWinRate();
		if (dScore >= dMaxScore) {
			iBestIndex = i;
			dMaxScore = dScore;
		}
	}

	//[Special case]avoid all zero win rate case -> give one more draw
	if (dMaxScore == 0.0) {
		CERR("[GodMoveSampling_v3::sampleAllCandidates]zero win rate appeared. Add one more draw count.\n");
		for (int i = 0; i < m_vCandidates.size(); i++) {
			m_vCandidates[i].m_iDrawCount++;
		}
	}

	if (!m_bPruning) {//better to be controlled by caller
		return;
	}

	//pruning
	WinRate_t dLowerBound = dMaxScore - m_iUsedSDCount * m_vCandidates[iBestIndex].getSD();
	//start pruning
	for (int i = 0; i < m_vCandidates.size(); i++) {
		if (m_vCandidates[i].m_bIsPruned)
			continue;
		WinRate_t dUpperBound = m_vCandidates[i].getWinRate() + m_iUsedSDCount * m_vCandidates[i].getSD();
		if (dUpperBound < dLowerBound) {
			m_vCandidates[i].m_bIsPruned = true;
			m_iLeftCandidateCount--;
		}
	}
}

SamplingDataType GodMoveSampling_v3::getBestCandidate()
{
	if (m_vCandidates.size() <= 0) {
		std::cerr << "[GodMoveSampling_v3::getBestCandidateWithChunkAvoiding] Empty candidate!!" << std::endl;
		printCurrentCandidate();
	}
	assert(m_vCandidates.size() > 0);
	int iBestIndex = 0;
	for (int i = 1; i < m_vCandidates.size(); i++) {
		if (!m_vCandidates[i].m_bIsPruned && m_vCandidates[i].getWinRate() >= m_vCandidates[iBestIndex].getWinRate()) {
			iBestIndex = i;
		}
	}
	return m_vCandidates[iBestIndex];
}

SamplingDataType GodMoveSampling_v3::getBestCandidateWithChunkAvoiding()
{
	if (m_vCandidates.size() <= 0) {
		std::cerr << "[GodMoveSampling_v3::getBestCandidateWithChunkAvoiding] Empty candidate!!" << std::endl;
		printCurrentCandidate();
	}
	assert(m_vCandidates.size() > 0);
	int iBestIndex = 0;
	for (int i = 1; i < m_vCandidates.size(); i++) {
		if (m_vCandidates[i].m_oAction.getMoveType() != MoveType::Move_Throw) {
			std::cerr << "[GodMoveSampling_v3::getBestCandidateWithChunkAvoiding] Invalid move type: The move is " << m_vCandidates[i].m_oAction.toString() << ", but the legal type should be MoveType::Move_Throw!!" << std::endl;
			printCurrentCandidate();
		}
		assert(m_vCandidates[i].m_oAction.getMoveType() == MoveType::Move_Throw);
		if (!m_vCandidates[i].m_bIsPruned && m_vCandidates[i].getWinRate() - m_oRemainTile.getDangerousFactor(m_vCandidates[i].m_oAction.getTargetTile())
			>= m_vCandidates[iBestIndex].getWinRate() - m_oRemainTile.getDangerousFactor(m_vCandidates[iBestIndex].m_oAction.getTargetTile())) {
			iBestIndex = i;
		}
	}
	return m_vCandidates[iBestIndex];
}

void GodMoveSampling_v3::makeMeldSamplingData(const Tile& oTile, const MoveType& oMakeMeldType)
{
	//set player tile
	PlayerTile oPlayerTile = m_oPlayerTile;
	switch (oMakeMeldType) {
	case MoveType::Move_EatRight:
		oPlayerTile.eatRight(oTile);
		break;

	case MoveType::Move_EatMiddle:
		oPlayerTile.eatMiddle(oTile);
		break;

	case MoveType::Move_EatLeft:
		oPlayerTile.eatLeft(oTile);
		break;

	case MoveType::Move_Pong:
		oPlayerTile.pong(oTile);
		break;

	case MoveType::Move_Kong:
		oPlayerTile.kong(oTile);
		break;

	case MoveType::Move_DarkKong:
		oPlayerTile.darkKong(oTile);
		break;

	case MoveType::Move_UpgradeKong:
		oPlayerTile.upgradeKong(oTile);
		break;
	}

	//set draw count
	int iDrawCount;
	if (oMakeMeldType == MoveType::Move_Pass) { iDrawCount = m_iLeftDrawCount; }
	else if (oMakeMeldType == MoveType::Move_Kong) { iDrawCount = m_iLeftDrawCount; }
	else if (oMakeMeldType == MoveType::Move_DarkKong) { iDrawCount = m_iLeftDrawCount + 1; }
	else if (oMakeMeldType == MoveType::Move_UpgradeKong) { iDrawCount = m_iLeftDrawCount + 1; }
	else { iDrawCount = m_iLeftDrawCount - 1; }//pong, eat

	//set SamplingData
	SamplingDataType oMeld(Move(oMakeMeldType, oTile), oPlayerTile, m_oRemainTile, iDrawCount, m_oGetTileType);
	m_vCandidates.push_back(oMeld);
}

void GodMoveSampling_v3::selectionSortCandidate()
{
	for (int i = 0; i < m_vCandidates.size() - 1; i++) {
		WinRate_t dMaxWinRate = m_vCandidates[i].getWinRate();
		int iBestIndex = i;
		for (int j = i + 1; j < m_vCandidates.size(); j++) {
			if (m_vCandidates[j].getWinRate() > dMaxWinRate) {
				iBestIndex = j;
				dMaxWinRate = m_vCandidates[j].getWinRate();
			}
		}

		//swap to the front(i)
		SamplingDataType tmpData = m_vCandidates[i];
		m_vCandidates[i] = m_vCandidates[iBestIndex];
		m_vCandidates[iBestIndex] = tmpData;
	}
}

void GodMoveSampling_v3::selectionSortCandidate(const int& iMoCount)
{
	for (int i = 0; i < m_vCandidates.size() - 1; i++) {
		double dMaxWinRate = m_vCandidates[i].getWinRate(iMoCount);
		int iBestIndex = i;
		for (int j = i + 1; j < m_vCandidates.size(); j++) {
			if (m_vCandidates[j].getWinRate(iMoCount) > dMaxWinRate) {
				iBestIndex = j;
				dMaxWinRate = m_vCandidates[j].getWinRate(iMoCount);
			}
		}

		//swap to the front(i)
		SamplingDataType tmpData = m_vCandidates[i];
		m_vCandidates[i] = m_vCandidates[iBestIndex];
		m_vCandidates[iBestIndex] = tmpData;
	}
}

bool GodMoveSampling_v3::isTooManyMelds() const
{
	if (!m_bUseTooManyMeldsStrategy) {
		//manually control
		return false;
	}

	if (m_iLeftDrawCount <= m_oPlayerTile.getMinLack()) {
		//no more effort to search too many meld strategy
		return false;
	}

	if (m_oPlayerTile.getMinLack(NEED_GROUP + 1) > m_oPlayerTile.getMinLack(NEED_GROUP) + 1) {
		//not too many melds
		return false;
	}

	//[optional] Don't use too many meld strategy if there is an alone tile
	/*for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; i++) {
		if (m_oPlayerTile.getHandTile().isAloneTile(i)) {
			return false;
		}
	}*/

	return true;
}

int GodMoveSampling_v3::getAvoidingChunkType() const
{
	if (m_bNoConsideringChunkAtBeginning && m_oRemainTile.getRemainNumber() > 90) {
		//Don't considering chunk at the beginning of the game.
		return 0;
	}
	return m_iConsideringChunkType;
}

void GodMoveSampling_v3::printCurrentCandidate() const
{
	std::cerr << "Action count: " << m_vCandidates.size() << std::endl;
	for (SamplingDataType oCandidate : m_vCandidates) {
		if (oCandidate.m_bIsPruned)
			std::cerr << "^";
		std::cerr << oCandidate.m_oAction.toString();
		std::cerr << " " << oCandidate.m_oPlayerTile.toString();
		for (int i = 0; i <= m_iLeftDrawCount + 1; i++) {
			std::cerr << "\t" << oCandidate.getWinRate(i);
			if (i == oCandidate.m_iDrawCount) {
				std::cerr << " (" << oCandidate.m_iUsedSampleCount << ")*";
				if (!oCandidate.m_bIsPruned)
					std::cerr << " +- " << m_iUsedSDCount * oCandidate.getSD();
			}
		}
		std::cerr << oCandidate.m_iUsedTime << " ms.";
		std::cerr << std::endl;
	}
}

void GodMoveSampling_v3::printSetting() const
{
	std::cerr << "[GodMoveSampling_v3]Draw count: " << m_iLeftDrawCount << std::endl;
	std::cerr << "[GodMoveSampling_v3]Sample mode: " << (m_oSampleMode == SampleMode::Mode_Time ? "SampleMode::Mode_Time" : "Mode_Sample") << std::endl;
	if (m_oSampleMode == SampleMode::Mode_Time) {
		std::cerr << "[GodMoveSampling_v3]Time limit: " << m_uiSampleParameter << " ms" << std::endl;
	}
	else {
		std::cerr << "[GodMoveSampling_v3]Sample count: " << m_uiSampleParameter << std::endl;
	}
	std::cerr << "[GodMoveSampling_v3]Sample count in first round: " << m_uiSampleFirstRound << std::endl;
	std::cerr << "[GodMoveSampling_v3]Sample count per round: " << m_uiSamplePerRound << std::endl;
	std::cerr << "[GodMoveSampling_v3]Use TooManyMeldsStrategy: " << m_bUseTooManyMeldsStrategy << std::endl;
	std::cerr << "[GodMoveSampling_v3]ConsideringChunkType: " << m_iConsideringChunkType << std::endl;
	std::cerr << "[GodMoveSampling_v3]NoConsideringChunkAtBeginning: " << m_bNoConsideringChunkAtBeginning << std::endl;
	std::cerr << "[GodMoveSampling_v3]Use Multi-thread: " << m_bUseMultiThread << std::endl;
	std::cerr << "[GodMoveSampling_v3]Use Pruning: " << m_bPruning << std::endl;
}

/*SamplingDataType GodMoveSampling_v3::getBestThrowWinRate(const PlayerTile& oPlayerTile, const RemainTile oRemainTile, const SampleMode oSampleMode, const uint32_t& cullSampleParam, const int& iLeftDrawCount)
{
	GodMoveSampling_v3 oSampling_Group2Depth2(oPlayerTile, oRemainTile, oSampleMode, cullSampleParam, iLeftDrawCount);
	vector<SamplingDataType> vData_Group2Depth2 = oSampling_Group2Depth2.throwTile_detail();
	return vData_Group2Depth2.at(0);
}*/

std::pair<SamplingDataType, uint32_t> GodMoveSampling_v3::getBestThrowWinRate(const PlayerTile& oPlayerTile, const RemainTile_t oRemainTile, const SampleMode oSampleMode, const uint32_t& cullSampleParam, const int& iLeftDrawCount)
{
	GodMoveSampling_v3 oSampling_Group2Depth2(oPlayerTile, oRemainTile, oSampleMode, cullSampleParam, iLeftDrawCount);
	vector<SamplingDataType> vData_Group2Depth2 = oSampling_Group2Depth2.throwTile_detail();
	return std::pair<SamplingDataType, uint32_t>(vData_Group2Depth2.at(0), oSampling_Group2Depth2.getUsedSampleCount());
}

void GodMoveSampling_v3::makeWinRateTimeStamp()
{
	stringstream ss;
	static bool bIsFirst20000 = true;
	static Tile oAnsTile_2w;
	static vector<vector<double>> vWinRates;
	static vector<uint32_t> vSampleCount;

	if (m_uiUsedSampleCount == 0) {
		bIsFirst20000 = true;
		vSampleCount.clear();
		vWinRates.clear();
		return;
	}

	vSampleCount.push_back(m_uiUsedSampleCount);
	vector<double> vWinRate(m_vCandidates.size());
	for (int i = 0; i < m_vCandidates.size(); i++) {
		vWinRate[i] = m_vCandidates.at(i).getWinRate();
	}
	vWinRates.push_back(vWinRate);

	if (bIsFirst20000 && m_uiUsedSampleCount > 20000) {
		oAnsTile_2w = getBestCandidate().m_oAction.getTargetTile();
		bIsFirst20000 = false;
	}
	if (m_uiUsedSampleCount > 40000) {
		Tile oAnsTile_4w = getBestCandidate().m_oAction.getTargetTile();
		if (oAnsTile_2w != oAnsTile_4w && (!m_oPlayerTile.isAloneTile(oAnsTile_2w) || !m_oPlayerTile.isAloneTile(oAnsTile_4w))) {
			fstream fout("diffAnsList.txt", std::ios::out | std::ios::app);
			fout << oAnsTile_2w.toString() << "\t" << oAnsTile_4w.toString() << "\t" << m_oPlayerTile.toString() << "\t" << m_oRemainTile.getReadableString() << std::endl;
			fout.close();
			fstream fout2("winRateTimeStamp" + m_oPlayerTile.toString() + ".txt", std::ios::out | std::ios::app);
			fout2 << "SampleCount";
			for (int i = 0; i < m_vCandidates.size(); i++) {
				fout2 << "\t" << m_vCandidates.at(i).m_oAction.toString();
			}
			fout2 << std::endl;
			for (int i = 0; i < vWinRates.size(); i++) {
				fout2 << vSampleCount.at(i);
				for (int j = 0; j < vWinRates.at(i).size(); j++) {
					fout2 << "\t" << vWinRates.at(i).at(j);
				}
				fout2 << std::endl;
			}
			fout2.close();
		}
	}
}