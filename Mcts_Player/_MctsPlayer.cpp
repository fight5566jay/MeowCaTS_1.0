#include "TestMode.h"
#include "../MJLibrary/Base/ConfigManager.h"
#ifndef LINUX
#include "../CommunicationLibrary/GameBridge.h"
#endif
#include "../MctsLibrary/MctsPlayer.h"

void init();
void run(int& argc, char* argv[]);
void test(int argc, char* argv[]);
void end();
void writeConfigInfoToFile();

int main(int argc, char* argv[]) {
	init();
	//run(argc, argv);
	test(argc, argv);

	//system("pause");
	return 0;
}

void init()
{
	TileGroup::g_bFirstInit = true;

	//ConfigManager::loadConfig();
	MinLackTable_old::makeTable(MJGameType2::MJGameType_16);
	//MinLackTable::makeTable();
	//WinTable::makeTable();
	ExplorationTermTable::makeTable();
	DebugLogger::start("MctsDebugLog");
	writeConfigInfoToFile();

	/*
	HandTile oHand("123466789a 123b 123c 55A");
	std::unique_ptr<int[]> vIds;
	oHand.getId(vIds);
	std::cout << "Test MinLack = " << oHand.getMinLack() << std::endl;
	std::cout << "Test MinLack = " << MinLackTable_old::getMinLackNumber(vIds.get()) << std::endl;
	*/
}

void run(int& argc, char* argv[])
{
#ifndef LINUX
	std::string sConfigFileName("config_contestMode.ini");
	Ini oIni(sConfigFileName);
	MctsPlayerConfig oPlayerConfig(oIni);
	MctsPlayer oPlayer(oPlayerConfig);
	GameBridge oGameBridge(&oPlayer);
	bool bUseSocket = ConfigManager::g_bUseSocket;
	if (argc == 3) {
		string sIp = argv[1];
		int iPort = toInt(argv[2]);
		oGameBridge = GameBridge(&oPlayer, sIp, iPort);
	}
	else if (bUseSocket) {
		Ini& oIni = Ini::getInstance();
		string sIp = oIni.getStringIni("Socket.ServerIP");
		int iPort = oIni.getIntIni("Socket.ServerPort");
		oGameBridge = GameBridge(&oPlayer, sIp, iPort);
	}


	while (true) {
		vector<string> vCommand = oGameBridge.getCommand();
		bool bContinue = oGameBridge.handleCommand(vCommand);
		if (!bContinue && !bUseSocket) { break; }
	}
#endif
}

void test(int argc, char* argv[]) {
	TestMode oMode;
	oMode.run(argc, argv);
}

void end() {
	DebugLogger::stop();
}

void writeConfigInfoToFile() {
	ofstream fout("MahjongBaseConfigInfo.txt");
	fout << "PLAYER_COUNT=" << PLAYER_COUNT << std::endl;
	fout << "SUIT_COUNT=" << SUIT_COUNT << std::endl;
	fout << "MAX_SUIT_TILE_RANK=" << MAX_SUIT_TILE_RANK << std::endl;
	fout << "HONOR_COUNT=" << HONOR_COUNT << std::endl;
	fout << "MAX_HONOR_TILE_RANK=" << MAX_HONOR_TILE_RANK << std::endl;
	fout << "SAME_TILE_COUNT=" << SAME_TILE_COUNT << std::endl;
	fout << "NEED_GROUP=" << NEED_GROUP << std::endl;
	fout << "NONPICKABLE_TILE_COUNT=" << NONPICKABLE_TILE_COUNT << std::endl;
	fout.close();
}