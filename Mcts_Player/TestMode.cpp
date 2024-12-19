#include "TestMode.h"
#include <cfloat> //FLT_MAX
#include "../MctsLibrary/Mcts2.h"
#include "../MctsLibrary/MctsPlayer.h"
#include "../SamplingLibrary/DrawCountTable.h"
#include "../MctsLibrary/ExplorationTermTable.h"
#include "../MJLibrary/MJ_Base/MinLackTable/MinLackTable_old.h"
#include "../MJLibrary/MJ_Base/MinLackTable/MinLackTable.h"
//#include "../MJLibrary/MJ_Base/TranspositionTable.h"
//#include "../MJLibrary/MJ_Base/TranspositionTable.cpp"
#include "../SamplingLibrary/SamplingData/GodMoveSamplingData.h"
#include "../SamplingLibrary/SamplingData/UsefulTileGreedySamplingData.h"
typedef GodMoveSamplingData SamplingData_t;

using std::cout;
using std::cerr;
using std::cin;
using std::endl;

void TestMode::run(int argc, char * argv[])
{
	//testMcts(argc, argv);
	//testMcts2(argc, argv);
	testMctsPlayer(argc, argv);
	//testSamplingData(argc, argv);
	//testUsefulTileGreedySamplingData(argc, argv);
	//testExplorationTermTable(argc, argv);
	//testReserviorOneSampler(argc, argv);
	//testOther(argc, argv);
	//trainExplorationTermTable();
	//trainExplorationTermTable2(argc, argv);
	//testNewMinLackTable();
	//runMctsStep();
	//genTestCase();
}

void TestMode::testMctsPlayer(int argc, char * argv[])
{
	ifstream fin(getCurrentPath() + "\\randomHands_10000.txt");

	const int ciTestCaseCount = 10000;
	DrawCountTable oDrawCountTable;
	oDrawCountTable.printDrawCountTable();
	string sConfigFileName = "config.ini";
	Ini oIni(sConfigFileName);
	MctsPlayerConfig oPlayerConfig(oIni);
	const int iSampleCount = oPlayerConfig.m_uiSampleCount;
	MctsPlayer oPlayer(iSampleCount, oPlayerConfig);

	for (int i = 1; i <= ciTestCaseCount; i++) {
		/*
		m_oPlayerTile = PlayerTile("114588a 3578b 118899c A");
		m_oRemainTile = RemainTile("2343343214434343241444444224322121", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("3589a 23467b 678c 66A ( 333A )");
		m_oRemainTile = RemainTile("4434334334333433444444433243213322", RemainTileType::Type_Playing);
		m_oRemainTile = RemainTile("4424234334333433444444433243213322", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("113334467a 11899b 56c A");
		m_oRemainTile = RemainTile("2212332432444333213234334331021111", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("112a 5699b 235566c A ( 222A )");
		m_oRemainTile = RemainTile("2343421311442334322334224323122021", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("112a 99b 235566c A ( 222A 456b )");
		m_oRemainTile = RemainTile("2343421311442334322334224323122021", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("22223678a 11355b 267c 5A");
		m_oRemainTile = RemainTile("4034433332434244441344433444323334", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("2234679a 5799b 3699c 22A");
		m_oRemainTile = RemainTile("4233433434444343423434434424221444", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("44579a 346b 123678c A ( 999b )");
		m_oRemainTile = RemainTile("3231342323132433113324432223100102", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("3689a 111366789b 2378c A");
		m_oRemainTile = RemainTile("4434434331434423334234443342321342", RemainTileType::Type_Playing);
		initState_random(MAX_HANDTILE_COUNT);
		m_oPlayerTile = PlayerTile("111366789a 3689b 2378c A");
		m_oRemainTile = RemainTile_t("1434423334434434334234443342321342", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("2378a 111366789b 3689c A");
		m_oRemainTile = RemainTile("4234443341434423334434434332321342", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("55a 224579b 123468c A ( 777A )");
		m_oRemainTile = RemainTile("3444244431243333433333434322012421", RemainTileType::Type_Playing);
		*/
		
		//m_oPlayerTile = PlayerTile("12345a 233778b 223788c A");
		//m_oRemainTile = RemainTile_t("3333344444324442333234443234444444", RemainTileType::Type_Playing);
		if (fin.eof())
			break;
		initState_fromFile(fin);

		cerr << "---- Test Case " << i << " ----" << endl;

		/**/
		//Test Method 1: Test one case
		printState();
		cerr << "Minlack: " << m_oPlayerTile.getMinLack() << endl;
		HandTileIdArray vHandTileIds;
		m_oPlayerTile.getHandTile().getId(vHandTileIds);
		//cerr << "oldminlack: " << MinLackTable_old::getMinLackNumber(vHandTileIds.data()) << endl;
		cerr << "Total remain count:" << m_oRemainTile.getRemainDrawNumber() << endl;
		cerr << "Draw count: " << oDrawCountTable.getDrawCount(m_oPlayerTile.getMinLack(), m_oRemainTile.getRemainDrawNumber() / PLAYER_COUNT) << endl;
		
		time_t startTime = clock();
		oPlayer.resetup(m_oPlayerTile, m_oRemainTile);
		Tile oThrownTile = oPlayer.askThrow();
		time_t iUsedTime = clock() - startTime;

		cout << m_oPlayerTile.toString() << "\t" << m_oRemainTile.toString() << "\t" << iUsedTime << endl;
		cerr << "Best move: T" << oThrownTile.toString() << endl;
		cerr << "Used time: " << iUsedTime << " ms." << endl;
		oPlayer.reset();
		cerr << "Player reset." << endl;
		
		/*/
		//Test Method 2: Keep playing until game ends

		while (m_oPlayerTile.getMinLack() > 0 && m_oRemainTile.getRemainDrawNumber() > 0) {
			printState();
			cerr << "Minlack: " << m_oPlayerTile.getMinLack() << endl;
			HandTileIdArray vHandTileIds;
			m_oPlayerTile.getHandTile().getId(vHandTileIds);
			cerr << "oldminlack: " << MinLackTable_old::getMinLackNumber(vHandTileIds.get()) << endl;
			cerr << "Total remain count:" << m_oRemainTile.getRemainDrawNumber() << endl;
			cerr << "Draw count: " << oDrawCountTable.getDrawCount(m_oPlayerTile.getMinLack(), m_oRemainTile.getRemainDrawNumber() / PLAYER_COUNT) << endl;

			time_t startTime = clock();
			oPlayer.resetup(m_oPlayerTile, m_oRemainTile);

			Tile oThrownTile = oPlayer.askThrow();
			cerr << "Best move: T" << oThrownTile.toString() << endl;
			//bool bResponse = oPlayer.askPong(oTargetTile, bCanEat, bCanKong);
			//Tile oTargetTile = Tile("4b");
			//bool bCanEat = true, bCanKong = false;
			//vector<Move> vLegalMoves = getMeldMoveList(oTargetTile, bCanEat, bCanKong);
			//cout << "Best move: " << (bResponse? Move(MoveType::Move_Pong, oTargetTile) : Move(MoveType::Move_Pass)).toString() << endl;
			//MoveType oMoveType = oPlayer.askEat(oTargetTile);
			//cout << "BestMove: " << Move(oMoveType, oTargetTile).toString() << endl;
			//auto oMovePair = oPlayer.askDarkKongOrUpgradeKong();
			//Move oMove(oMovePair.first, oMovePair.second);
			//cout << "Best move: " << oMove.toString() << endl;

			time_t iUsedTime = clock() - startTime;
			cerr << "Used time: " << iUsedTime << " ms." << endl;
			cout << m_oPlayerTile.toString()
				<< "\t" << m_oRemainTile.toString()
				<< "\t" << iUsedTime
				<< "\t" << oPlayer.getUsedSampleCount()
				<< "\t" << m_oPlayerTile.getMinLack()
				<< "\t" << oDrawCountTable.getDrawCount(m_oPlayerTile.getMinLack(), m_oRemainTile.getRemainDrawNumber() / PLAYER_COUNT) << endl;
			oPlayer.reset();
			cerr << "Player reset." << endl;

			oPlayer.throwTile(oThrownTile);
			oPlayer.drawTile(m_oRemainTile.randomPopTile());
		}
		/**/
	}

	system("pause");
}

void TestMode::testMcts2(int argc, char * argv[])
{
	const int iSampleCount = 100000;
	const int iSampleCountPerIteration = 1000;
	const int ciTestCaseCount = 1;
	ifstream fin(getCurrentPath() + "\\TestCase_10000.txt");

	Ini oIni("config1.ini");
	MctsConfig oConfig(oIni);
	DrawCountTable oDrawCountTable;
	oDrawCountTable.printDrawCountTable();

	for (int i = 1; i <= ciTestCaseCount; i++) {
		cerr << "---- Test Case " << i << " ----" << endl;

		//initState1();
		//initState_random(17);
		//initState_worstCase();
		//initState_fromFile(fin);
		//m_oPlayerTile = PlayerTile("34a 124667b 1236c 33A ( 111A )");
		//m_oRemainTile = RemainTile("4322232322240213123334424140222423", RemainTileType::Type_Playing);
		//m_oPlayerTile = PlayerTile("234a 12466b 1236c 33A ( 111A )");
		//m_oRemainTile = RemainTile("4222232322240213123334424140222423", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("3589a 23467b 678c 66A ( 333A )");
		m_oRemainTile = RemainTile_t("4434334334333433444444433243213322", RemainTileType::Type_Playing);

		time_t iStartTime = clock();
		const int iDrawCount = oDrawCountTable.getDrawCount(m_oPlayerTile.getMinLack(), m_oRemainTile.getRemainDrawNumber() / PLAYER_COUNT);
		//const int iDrawCount = 3;
		Mcts2 oTree(m_oPlayerTile, m_oRemainTile, iDrawCount, Mcts2::getLegalMoves(m_oPlayerTile, m_oRemainTile, true, true, false), oConfig, true);

		/*Tile oTargetTile = Tile("1c");
		bool bCanEat = false, bCanKong = true;
		vector<Move> vLegalMoves = getMeldMoveList(oTargetTile, bCanEat, bCanKong);
		Mcts2 oTree(m_oPlayerTile, m_oRemainTile, iDrawCount, vLegalMoves);*/
		//Mcts2::m_fExplorationTerm = 0.1f;

		//oTree.sample(100000);
		/*int iLeftSampleCount;
		while(true) {
			cerr << "Input sample count: ";
			cin >> iLeftSampleCount;
			if (iLeftSampleCount <= 0)
				break;

			while (iLeftSampleCount > 0) {
				oTree.sample(iSampleCountPerIteration);
				iLeftSampleCount -= iSampleCountPerIteration;
			}

			oTree.writeToSgf();
			//system("pause");
		}*/
		

		//cout << "---- Test Case " << i << " ----" << endl;
		printState();
		cout << "Minlack: " << m_oPlayerTile.getMinLack() << endl;
		//cout << "oldminlack: " << MinLackTable_old::getMinLackNumber(m_oPlayerTile.getHandTile().getId().get()) << endl;
		cout << "Total remain count:" << m_oRemainTile.getRemainDrawNumber() << endl;
		cout << "Draw count: " << iDrawCount << endl;

		oTree.sample(iSampleCount);

		cout << "Used time: " << clock() - iStartTime << " ms." << endl;
		oTree.printChildrenResult();
		cout << "Best move: " << oTree.getBestMove().toString() << endl;
		//cout << "Used sample count: " << oTree.getUsedSampleCount() << endl;
		//cout << "TT entry using rate: " << TranspositionTable<BaseTreeNodePtr>::getEntryUsingRate() << endl;
		//cout << "TT collision rate: " << TranspositionTable<BaseTreeNodePtr>::getCollisionRate() << endl;
		//oTree.writeToSgf();
	}

	system("pause");
}

void TestMode::testNewMinLackTable()
{
	MinLackTable_old::makeTable(MJGameType2::MJGameType_16);
	MinLackTable::makeTable();

	int suit_max_id = static_cast<int>(pow(SAME_TILE_COUNT + 1, MAX_SUIT_TILE_RANK)) - 1;
	int honor_max_id = static_cast<int>(pow(SAME_TILE_COUNT + 1, MAX_HONOR_TILE_RANK)) - 1;
	array<int, TILE_TYPE_COUNT> ids;
	bool bLoopEnd = false;

	//init
	std::fill(ids.begin(), ids.end(), 0);

	//run
	while (!bLoopEnd) {
		int iOldMinLack = MinLackTable_old::getMinLackNumber(ids.data());
		int iNewMinLack = MinLackTable::getMinLackNumber(ids.data());
		if (iOldMinLack != iNewMinLack) {
			cout << "--- Found Different MinLack ---" << endl;
			cout << "hand:";
			for (int i = 0; i < SUIT_COUNT; i++) 
				cout << " " << MinLackTable_old::g_eSuit[ids.at(i)].toString();
			for (int i = 0; i < HONOR_COUNT; i++)
				cout << " " << MinLackTable_old::g_eHonor[ids.at(SUIT_COUNT+ i)].toString();
			cout << endl;
		}

		//update ids
		for (int i = 0; i < TILE_TYPE_COUNT; i++) {
			ids[i]++;
			if (i < SUIT_COUNT) {
				if(ids[i] <= suit_max_id)
					break;
				ids[i] = 0;
			}
			else {
				if (ids[i] <= honor_max_id)
					break;
				if (i == TILE_TYPE_COUNT - 1)
					bLoopEnd = true;
			}
		}
	}
}

void TestMode::testSamplingData(int argc, char * argv[])
{
	const int iSamplePerRound = 10000;
	const int iSampleRoundCount = 100;
	const int ciTestCaseCount = 1;

	DrawCountTable oDrawCountTable;
	oDrawCountTable.printDrawCountTable();

	for (int i = 1; i <= ciTestCaseCount; i++) {
		m_oPlayerTile = PlayerTile("234a 24667b 1236c 33A ( 111A )");
		m_oRemainTile = RemainTile_t("4222232322240213123334424140222423", RemainTileType::Type_Playing);

		const int iDrawCount = oDrawCountTable.getDrawCount(m_oPlayerTile.getMinLack(), m_oRemainTile.getRemainDrawNumber() / PLAYER_COUNT);
		//const int iDrawCount = 10;
		cout << "---- Test Case " << i << " ----" << endl;
		cerr << "---- Test Case " << i << " ----" << endl;
		vector<SamplingData_t> vDatas;
		vDatas.reserve(MAX_HANDTILE_COUNT);
		for (int j = 0; j < MAX_DIFFERENT_TILE_COUNT; j++) {
			if (m_oPlayerTile.getHandTileNumber(j) == 0)
				continue;

			m_oPlayerTile.popTileFromHandTile(j);
			Move oMove(MoveType::Move_Throw, Tile(j));
			SamplingData_t oData(oMove, m_oPlayerTile, m_oRemainTile, iDrawCount, GetTileType::GetTileType_FourPlayerMo);
			cout << "Action: " << oMove.toString() << endl;
			for (int k = 1; k <= iSampleRoundCount; k++) {
				oData.sample(iSamplePerRound);
				cout << "Sample " << iSamplePerRound * k << " times." << endl;
			}

			vDatas.push_back(oData);
			m_oPlayerTile.putTileToHandTile(j);
		}

		printState();
		cout << "Minlack: " << m_oPlayerTile.getMinLack() << endl;
		//cout << "oldminlack: " << MinLackTable_old::getMinLackNumber(m_oPlayerTile.getHandTile().getId().get()) << endl;
		cout << "Total remain count:" << m_oRemainTile.getRemainNumber() << endl;
		cout << "Remain draw count of RemainTile:" << m_oRemainTile.getRemainDrawNumber() << endl;
		cout << "Draw count: " << iDrawCount << endl;
		/*cout << "Drawn tiles:" << endl;
		for (int i = 0; i < oData.m_vDrawnTiles_Order.size(); i++) {
			array<Tile, PLAYER_COUNT> vDrawnTiles = oData.m_vDrawnTiles_Order.at(i);
			cout << "(" << i + 1 << ")";
			for (int j = 0; j < vDrawnTiles.size(); j++) {
				cout << " " << vDrawnTiles.at(j).toString();
			}
			cout << endl;
		}
		cout << "Possible PlayerTiles:" << endl;
		for (int i = 0; i < oData.m_vPossiblePlayerTiles.size(); i++) {
			cout << oData.m_vPossiblePlayerTiles.at(i).toString() << " " << oData.m_vPossiblePlayerTiles.at(i).getMinLack(NEED_GROUP) << endl;
		}*/

		int iUsedSampleCount = 0;
		for (auto oData : vDatas) {
			cout << oData.m_oAction.toString();
			for (int j = 0; j <= oData.m_iDrawCount; j++) {
				cout << "\t" << oData.getWinCount(j) << "(" << oData.getWinRate(j) << ")";
			}
			cout << endl;
			iUsedSampleCount += oData.m_iUsedSampleCount;
		}
		cout << "Used sample count: " << iUsedSampleCount << endl;
	}

	system("pause");
}

void TestMode::testUsefulTileGreedySamplingData(int argc, char * argv[])
{
	const int iSamplePerRound = 10000;
	const int iSampleRoundCount = 10;
	const int ciTestCaseCount = 1;

	DrawCountTable oDrawCountTable;
	oDrawCountTable.printDrawCountTable();

	for (int i = 1; i <= ciTestCaseCount; i++) {
		//m_oPlayerTile = PlayerTile("23a 135b c 44A ( 777A 666A 555A )");
		//m_oRemainTile = RemainTile("4004444440404004444444444444440111", RemainTileType::Type_Playing);
		//m_oPlayerTile = PlayerTile("11368a 4b 445999c 1A ( 768b )");
		//m_oRemainTile = RemainTile("2433434344443433344442344413423434", RemainTileType::Type_Playing);
		m_oPlayerTile = PlayerTile("2a 456667789b 1228c A ( 555A )");
		m_oRemainTile = RemainTile_t("4344434434443312333244444341113112", RemainTileType::Type_Playing);

		const int iDrawCount = oDrawCountTable.getDrawCount(m_oPlayerTile.getMinLack(), m_oRemainTile.getRemainDrawNumber() / PLAYER_COUNT);
		//const int iDrawCount = 10;
		cout << "---- Test Case " << i << " ----" << endl;
		cerr << "---- Test Case " << i << " ----" << endl;
		vector<UsefulTileGreedySamplingData> vDatas;
		vDatas.reserve(MAX_HANDTILE_COUNT);
		for (int j = 0; j < MAX_DIFFERENT_TILE_COUNT; j++) {
			if (m_oPlayerTile.getHandTileNumber(j) == 0)
				continue;

			m_oPlayerTile.popTileFromHandTile(j);
			Move oMove(MoveType::Move_Throw, Tile(j));
			UsefulTileGreedySamplingData oData(oMove, m_oPlayerTile, m_oRemainTile, iDrawCount, SamplingType::SamplingType_FourPlayers);
			cout << "Action: " << oMove.toString() << endl;
			for (int k = 1; k <= iSampleRoundCount; k++) {
				oData.sample(iSamplePerRound);
				cout << "Sample " << iSamplePerRound * k << " times." << endl;
			}

			vDatas.push_back(oData);
			m_oPlayerTile.putTileToHandTile(j);
		}

		printState();
		cout << "Minlack: " << m_oPlayerTile.getMinLack() << endl;
		//cout << "oldminlack: " << MinLackTable_old::getMinLackNumber(m_oPlayerTile.getHandTile().getId().get()) << endl;
		cout << "Total remain count:" << m_oRemainTile.getRemainNumber() << endl;
		cout << "Remain draw count of RemainTile:" << m_oRemainTile.getRemainDrawNumber() << endl;
		cout << "Draw count: " << iDrawCount << endl;
		/*cout << "Drawn tiles:" << endl;
		for (int i = 0; i < oData.m_vDrawnTiles_Order.size(); i++) {
			array<Tile, PLAYER_COUNT> vDrawnTiles = oData.m_vDrawnTiles_Order.at(i);
			cout << "(" << i + 1 << ")";
			for (int j = 0; j < vDrawnTiles.size(); j++) {
				cout << " " << vDrawnTiles.at(j).toString();
			}
			cout << endl;
		}
		cout << "Possible PlayerTiles:" << endl;
		for (int i = 0; i < oData.m_vPossiblePlayerTiles.size(); i++) {
			cout << oData.m_vPossiblePlayerTiles.at(i).toString() << " " << oData.m_vPossiblePlayerTiles.at(i).getMinLack(NEED_GROUP) << endl;
		}*/

		int iUsedSampleCount = 0;
		for (auto oData : vDatas) {
			cout << oData.m_oAction.toString();
			for (int j = 0; j <= oData.m_iDrawCount; j++) {
				cout << "\t" << oData.getWinCount(j) << "(" << oData.getWinRate(j) << ")";
			}
			cout << endl;
			iUsedSampleCount += oData.m_iUsedSampleCount;
		}
		cout << "Used sample count: " << iUsedSampleCount << endl;
	}

	system("pause");
}

void TestMode::testReserviorOneSampler(int argc, char * argv[])
{
	ReserviorOneSampler<int> oSampler;
	int iTestCount = 1000000;
	const int iInputCount = 100;
	int *vSampleResult = new int[iInputCount];
	std::fill(vSampleResult, vSampleResult + iInputCount, 0);

	for (int c = 0; c < iTestCount; c++) {
		oSampler.clear();
		for (int i = 0; i < iInputCount; i++) {
			oSampler.input(i);
		}
		vSampleResult[oSampler.getData()]++;
	}

	for (int i = 0; i < iInputCount; i++) {
		cout << "Index " << i << ": " << vSampleResult[i] << endl;
	}
	
	delete[] vSampleResult;
	system("pause");
}

void TestMode::testExplorationTermTable(int argc, char* argv[])
{
	/*/
	if (argc == 1) {
		cout << "usage: " << argv[0] << " <command>\n  'c'   create\n  'r'   remove\n  'a'   attach" << endl;
		return;
	}

	//const string sShmSegmentName = "ExplorationTermTable";
	const char* sShmSegmentName = "ExplorationTermTable";
	//managed_shared_memory segment;

	switch (tolower(argv[1][0])) {
	case 'c':
		shared_memory_object::remove(sShmSegmentName);
		try {
			ExplorationTermTable::m_oExplorationTermTable_segment = managed_shared_memory
			(
				create_only
				, sShmSegmentName
				, EXPLORATION_TERM_TABLE_SIZE * sizeof(ExplorationTerm_t) + 256 //Some memory is used by segment itself.
			);
			cout << "shm " << sShmSegmentName << " has been created." << endl;
			ExplorationTermTable::m_pExplorationTermTable = ExplorationTermTable::m_oExplorationTermTable_segment.construct<ExplorationTerm_t>("TermTable")[EXPLORATION_TERM_TABLE_SIZE](1.0);
			cout << "table has been constructed." << endl;
			ExplorationTermTable::loadFromFile();
			cout << "table has been load from file." << endl;
			ExplorationTermTable::printTable();
		}
		catch (interprocess_exception& ex) {
			cerr << "[ExplorationTermTable::init] ex = " << ex.what() << endl;
			cerr << "[ExplorationTermTable::init] get_error_code = " << ex.get_error_code() << endl;
		}
		break;
	case 'r':
		shared_memory_object::remove(sShmSegmentName);
		cout << "shm " << sShmSegmentName << " has been removed." << endl;
		break;
	case 'a':
		cout << "try to attach shm " << sShmSegmentName << "." << endl;
		try {
			ExplorationTermTable::m_oExplorationTermTable_segment = managed_shared_memory
			(
				open_only
				, sShmSegmentName
			);
			cout << "shm " << sShmSegmentName << " has been attached." << endl;
			ExplorationTermTable::m_pExplorationTermTable = ExplorationTermTable::m_oExplorationTermTable_segment.find<ExplorationTerm_t>("TermTable").first;
			cout << "table has been found." << endl;
			ExplorationTermTable::printTable();
		}
		catch (interprocess_exception& ex) {
			cerr << "[ExplorationTermTable::init] ex = " << ex.what() << endl;
			cerr << "[ExplorationTermTable::init] get_error_code = " << ex.get_error_code() << endl;
		}
		break;
	case 'w':
		cout << "try to attach shm " << sShmSegmentName << "." << endl;
		try {
			ExplorationTermTable::m_oExplorationTermTable_segment = managed_shared_memory
			(
				open_only
				, sShmSegmentName
			);
			cout << "shm " << sShmSegmentName << " has been attached." << endl;
			ExplorationTermTable::m_pExplorationTermTable = ExplorationTermTable::m_oExplorationTermTable_segment.find<ExplorationTerm_t>("TermTable").first;
			cout << "table has been found." << endl;
			ExplorationTermTable::setExplorationTerm(2, 2, 99.0);
			cout << "value has been set to " << ExplorationTermTable::getExplorationTerm(2, 2) << endl;
			ExplorationTermTable::printTable();
		}
		catch (interprocess_exception& ex) {
			cerr << "[ExplorationTermTable::init] ex = " << ex.what() << endl;
			cerr << "[ExplorationTermTable::init] get_error_code = " << ex.get_error_code() << endl;
		}
		break;
	default:
		cerr << "unknown command." << endl;
		break;
	}
	
	/*/

	//ExplorationTermTable::makeTable();
	ExplorationTermTable::printTable();

	cout << ExplorationTermTable::getExplorationTerm(2, 3) << endl;
	ExplorationTermTable::setExplorationTerm(2, 2, 99.0);
	ExplorationTermTable::printTable();

	//system("pause");
	//oTable.writeToFile()
	/**/
}

void TestMode::moTile(const Tile& oTile)
{
	m_oRemainTile.popTile(oTile);
	m_oPlayerTile.putTileToHandTile(oTile);
}

void TestMode::initState_random(const int& iHandTileNumber, const bool& bTooManyMelds)
{
	//m_oPlayerTile.clear();
	//m_oRemainTile.init(RemainTileType::Type_Playing);
	while (true) {
		//init
		m_oPlayerTile.clear();
		m_oRemainTile.init(RemainTileType::Type_Playing);
		for (int i = 0; i < iHandTileNumber; i++) {
			Tile oTile = m_oRemainTile.randomPopTile();
			m_oPlayerTile.putTileToHandTile(oTile);
		}


		//pop alone honor tile
		bool bNoAloneHonor = false;
		while (!bNoAloneHonor) {
			bNoAloneHonor = true;
			for (int i = DIFF_SUIT_TILE_COUNT; i < MAX_DIFFERENT_TILE_COUNT; i++) {
				if (m_oPlayerTile.getHandTile().getTileNumber(i) == 1) {
					bNoAloneHonor = false;
					break;
				}
			}

			if (!bNoAloneHonor) {
				for (int i = DIFF_SUIT_TILE_COUNT; i < MAX_DIFFERENT_TILE_COUNT; i++) {
					while (m_oPlayerTile.getHandTile().getTileNumber(i) == 1) {
						m_oPlayerTile.popTileFromHandTile(i);
						m_oRemainTile.addTile(i);
						Tile oTile = m_oRemainTile.randomPopTile();
						m_oPlayerTile.putTileToHandTile(oTile);
					}
				}
			}
		}


		int iInitMinLack = m_oPlayerTile.getMinLack();
		if (iInitMinLack == 0)
			continue;

		//need too many melds handtile?
		if (bTooManyMelds && m_oPlayerTile.getMinLack(NEED_GROUP) != m_oPlayerTile.getMinLack(NEED_GROUP + 1) - 1) {
			continue;
		}

		int iMaxPopTileCount = MAX_HANDTILE_COUNT * 2;
		int iRandomThrownTileCount = rand() % iMaxPopTileCount;
		//int iRandomThrownTileCount = rand() % (m_oRemainTile.getRemainDrawNumber() - DIFFERENT_TILE_NUMBER);
		for (int i = 0; i < iRandomThrownTileCount; i++) {
			Tile oTile = m_oRemainTile.randomPopTile();
			if (m_oRemainTile.getRemainNumber(oTile) == 0) {
				m_oRemainTile.addTile(oTile);
				i--;
			}
		}
		break;
	}
}

void TestMode::initState_fromFile(ifstream & fin)
{
	//m_oPlayerTile.clear();
	//m_oRemainTile.init(RemainTileType::Type_Playing);
	string sPlayerTile, sRemainTile;
	std::getline(fin, sPlayerTile, '\t');
	std::getline(fin, sRemainTile, '\n');
	m_oPlayerTile = PlayerTile(sPlayerTile);
	m_oRemainTile = RemainTile_t(sRemainTile, RemainTileType::Type_Playing);
}

void TestMode::initState1()
{
	m_oPlayerTile.clear();
	m_oRemainTile.init(RemainTileType::Type_Playing);
	/**/
	moTile(Tile("7a"));
	moTile(Tile("9a"));
	moTile(Tile("1b"));
	moTile(Tile("2b"));
	moTile(Tile("3b"));
	moTile(Tile("3b"));
	moTile(Tile("4b"));
	moTile(Tile("2c"));
	moTile(Tile("2c"));
	moTile(Tile("6c"));
	moTile(Tile("9c"));
	moTile(Tile("1A"));
	moTile(Tile("1A"));
	moTile(Tile("2A"));
	moTile(Tile("2A"));
	moTile(Tile("4A"));
	//moTile(Tile("2A"));
	m_oPlayerTile.eatMiddle(Tile("2b"));
	/*/
	moTile(Tile("1a"));
	moTile(Tile("2a"));
	moTile(Tile("3a"));
	moTile(Tile("4a"));
	moTile(Tile("5a"));
	/**/
}

void TestMode::initState2()
{
	m_oPlayerTile.clear();
	m_oRemainTile.init(RemainTileType::Type_Playing);
	moTile(Tile("7a"));
	moTile(Tile("9a"));
	moTile(Tile("4b"));
	moTile(Tile("5b"));
	moTile(Tile("7b"));
	moTile(Tile("8b"));
	moTile(Tile("9b"));
	moTile(Tile("2c"));
	moTile(Tile("3c"));
	moTile(Tile("4c"));
	moTile(Tile("7c"));
	moTile(Tile("7c"));
	moTile(Tile("7c"));
	moTile(Tile("9c"));
	moTile(Tile("9c"));
	moTile(Tile("4A"));
	moTile(Tile("4A"));
}

void TestMode::initState_worstCase()
{
	m_oPlayerTile.clear();
	m_oRemainTile.init(RemainTileType::Type_Playing);
	moTile(Tile("1a"));
	moTile(Tile("5a"));
	moTile(Tile("9a"));
	moTile(Tile("1b"));
	moTile(Tile("5b"));
	moTile(Tile("9b"));
	moTile(Tile("1c"));
	moTile(Tile("5c"));
	moTile(Tile("9c"));
	moTile(Tile("1A"));
	moTile(Tile("2A"));
	moTile(Tile("3A"));
	moTile(Tile("4A"));
	moTile(Tile("5A"));
	moTile(Tile("6A"));
	moTile(Tile("7A"));
	moTile(Tile("7A"));
}

void TestMode::initState_greatCase()
{
	m_oPlayerTile.clear();
	m_oRemainTile.init(RemainTileType::Type_Playing);
	moTile(Tile("3a"));
	moTile(Tile("4a"));
	moTile(Tile("5a"));
	moTile(Tile("6a"));
	moTile(Tile("7a"));
	moTile(Tile("1b"));
	moTile(Tile("2b"));
	moTile(Tile("3b"));
	moTile(Tile("4b"));
	moTile(Tile("5b"));
	moTile(Tile("6b"));
	moTile(Tile("7b"));
	moTile(Tile("8b"));
	moTile(Tile("9b"));
	moTile(Tile("5c"));
	moTile(Tile("6c"));
	moTile(Tile("7c"));
}

void TestMode::printState() const
{
	cerr << "PlayerTile: " << m_oPlayerTile.toString() << endl;
	cerr << "RemainTile:" << endl << m_oRemainTile.getReadableString() << endl;
}

void TestMode::genTestCase()
{
	const int ciTestCaseCount = 10000;
	ofstream fout(getCurrentPath() + "\\TestCase_" + std::to_string(ciTestCaseCount) + ".txt");
	for (int i = 0; i < ciTestCaseCount; i++) {
		initState_random(17);
		fout << m_oPlayerTile.toString() << "\t" << m_oRemainTile.toString() << endl;
	}
	fout.close();
}

vector<Move> TestMode::getMeldMoveList(const Tile & oTargetTile, const bool & bCanEat, const bool & bCanKong) const
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
