#include <iostream>
#include <fstream>
#include <ctime>
#include "../MJLibrary/Base/Tools.h"
#include "../ServerLibrary/GameServer_V2.h"
#include "../MJLibrary/MJ_Base/MinLackTable/MinLackTable.h"
#include "../ServerLibrary/Wall.h"
//#include "../DummyPlayer/DummyPlayer.h"
#include "../SamplingLibrary/FlatMCPlayer.h"
#include "../MctsLibrary/MctsPlayer.h"
#include "../MctsLibrary/MctsPlayer.cpp"
using std::string;
using std::cout;
using std::endl;
typedef GameServer_V2 Server_t;

void init();
int setupPlayers_mcts_vs_flatMC(int argc, char* argv[], int& iGameCount, PlayerList_t& vPlayers, int& iSeed);
int setupPlayers_mcts_vs_mcts(int argc, char* argv[], int& iGameCount, PlayerList_t& vPlayers, int& iSeed);

int main(int argc, char* argv[])
{
	init();
	Server_t oGameServer;
	PlayerList_t vGamePlayers;
	const bool bTwoPlayerContestMode = true;
	int iGameCount = 1;
	int iSeed = -1;

	//setupPlayers_mcts_vs_flatMC(argc, argv, iGameCount, vGamePlayers, iSeed);
	setupPlayers_mcts_vs_mcts(argc, argv, iGameCount, vGamePlayers, iSeed);
	oGameServer.init(vGamePlayers);


	if (!bTwoPlayerContestMode) {
		for (int i = 0; i < iGameCount; ++i) {
#ifdef WINDOWS
			//string sCommand = "title Port: " + std::to_string(iPort) + "    Game: " + std::to_string(i + 1) + "    Seed: " + std::to_string(iSeed);
			string sCommand = "title Game: " + std::to_string(i + 1) + "    Seed: " + std::to_string(iSeed);
			system(sCommand.c_str());
#endif

			Wall oWall(iSeed);
			oGameServer.start(oWall);
			//string sLog = oGameServer.getGameLog();
			//oLogStream << sLog << std::endl;
			iSeed++;
		}
	}
	else {
		for (int i = 0; i < iGameCount; ++i) {
#ifdef WINDOWS
			//string sCommand = "title TwoPlayerContestMode Port: " + std::to_string(iPort) + "    Game: " + std::to_string(i + 1) + "    Seed: " + std::to_string(iSeed);
			string sCommand = "title TwoPlayerContestMode Game: " + std::to_string(i + 1) + "    Seed: " + std::to_string(iSeed);
			system(sCommand.c_str());
#endif

			Wall oWall(iSeed);

			oGameServer.switchSeat({ 2, 3, 4, 1 });//ABAB -> BABA
			if (i % 2 == 1) {
				iSeed++;//for next game
			}
			/*
			switch (i % 6) {
			case 0:
				//ABAB
				if (i != 0) {
					oGameServer.switchSeat(2, 1, 3, 4);//BAAB -> ABAB
					oGameServer.nextGame();
				}
				break;
			case 1:
				oGameServer.switchSeat(2, 3, 4, 1);//ABAB -> BABA
				break;
			case 2:
				oGameServer.switchSeat(4, 2, 3, 1);//BABA - > AABB
				break;
			case 3:
				oGameServer.switchSeat(2, 3, 4, 1);//AABB -> ABBA
				break;
			case 4:
				oGameServer.switchSeat(2, 3, 4, 1);//ABBA -> BBAA
				break;
			case 5:
				oGameServer.switchSeat(2, 3, 4, 1);//BBAA -> BAAB
				iSeed++;//for next game
				break;
			default:
				std::cerr << "switch unknown errror!!: " << i % 6 << std::endl;
				assert(0);
			}
			*/

			oGameServer.start(oWall, false);
			//string sLog = oGameServer.getGameLog();
			//oLogStream << sLog << std::endl;
		}
	}

	//oLogStream.close();
	std::cout << "Finish " << iGameCount << " game..." << std::endl;
	system("pause");
	return 0;
}

void init()
{
	MinLackTable_old::makeTable(MJGameType2::MJGameType_16);
	//WinTable::makeTable();
}

int setupPlayers_mcts_vs_flatMC(int argc, char* argv[], int& iGameCount, PlayerList_t& vPlayers, int& iSeed)
{
	/*/
	if (argc != 3 && argc != 4) {
		std::cout << "./[Program] [Game Count] [Config file name] ([Random seed])" << std::endl;
		system("pause");
		return 0;
	}
	iGameCount = toInt(string(argv[1]));
	std::string sConfigFileName1(argv[2]);
	iSeed = (argc == 4) ? toInt(string(argv[3])) : -1;
	/*/
	iGameCount = 20000;
	std::string sConfigFileName1("config1.ini");
	iSeed = 234567;
	/**/

	string sPath = getCurrentPath() + "Server_Log";
	if (!isDirExists(sPath)) {
		createFolder(sPath.c_str());
	}
	//ofstream oLogStream(sPath + getTime() + ".txt", std::ios::out);
	ofstream oInitSeedStream(sPath + "/initSeed.txt");
	oInitSeedStream << iSeed;
	oInitSeedStream.close();

	/*if (!oLogStream.is_open()) {
		std::cout << "Open Failed: " + sPath << std::endl;
		system("pause");
		return 0;
	}*/

	Server_t oGameServer;
	Ini oIni1(sConfigFileName1);
	MctsPlayerConfig oPlayerConfig1(oIni1);


	for (int i = 0; i < vPlayers.size(); i++) {
		std::unique_ptr<BasePlayer> pPlayer;
		if (i % 2 == 0) {
			pPlayer = std::make_unique<MctsPlayer>(oPlayerConfig1);
		}
		else {
			//pPlayer = std::make_unique<MctsPlayer>(oPlayerConfig2);
			pPlayer = std::make_unique<FlatMCPlayer>();
		}
		vPlayers[i] = std::make_unique<Player_t>(pPlayer);
	}
}

int setupPlayers_mcts_vs_mcts(int argc, char* argv[], int& iGameCount, PlayerList_t& vPlayers, int& iSeed)
{
	/**/
	if (argc != 4 && argc != 5) {
		std::cout << "./[Program] [Game Count] [Config 1 file name] [Config 2 file name] ([Random seed])" << std::endl;
		system("pause");
		return 0;
	}
	iGameCount = toInt(string(argv[1]));
	std::string sConfigFileName1(argv[2]);
	std::string sConfigFileName2(argv[3]);
	iSeed = (argc == 5) ? toInt(string(argv[4])) : -1;
	/*/
	iGameCount = 20000;
	std::string sConfigFileName1("config1.ini");
	std::string sConfigFileName2("config2.ini");
	iSeed = 234567;
	/**/


	string sPath = getCurrentPath() + "Server_Log";
	if (!isDirExists(sPath)) {
		createFolder(sPath.c_str());
	}
	//ofstream oLogStream(sPath + getTime() + ".txt", std::ios::out);
	ofstream oInitSeedStream(sPath + "/initSeed.txt");
	oInitSeedStream << iSeed;
	oInitSeedStream.close();

	/*if (!oLogStream.is_open()) {
		std::cout << "Open Failed: " + sPath << std::endl;
		system("pause");
		return 0;
	}*/

	Server_t oGameServer;
	Ini oIni1(sConfigFileName1);
	MctsPlayerConfig oPlayerConfig1(oIni1);
	Ini oIni2(sConfigFileName2);
	MctsPlayerConfig oPlayerConfig2(oIni2);


	for (int i = 0; i < vPlayers.size(); i++) {
		std::unique_ptr<BasePlayer> pPlayer;
		if (i % 2 == 0) {
			pPlayer = std::make_unique<MctsPlayer>(oPlayerConfig1);
		}
		else {
			pPlayer = std::make_unique<MctsPlayer>(oPlayerConfig2);
			//pPlayer = std::make_unique<FlatMCPlayer>();
		}
		vPlayers[i] = std::make_unique<Player_t>(pPlayer);
	}
}
