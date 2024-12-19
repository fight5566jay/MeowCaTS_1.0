#include "DrawCountTable.h"
#include "GodMoveSampling_v3.h"
#include <fstream>
using std::ifstream;
using std::cout;
using std::endl;

bool DrawCountTable::g_bFirstSetup = true;
array<array<int, AVERAGE_MAX_DRAW_COUNT + 1>, MAX_MINLACK + 1> DrawCountTable::g_vDrawCountTable;
array<int, 6> DrawCountTable::g_vInfo = { SUIT_COUNT, MAX_SUIT_TILE_RANK, HONOR_COUNT, MAX_HONOR_TILE_RANK, SAME_TILE_COUNT, NEED_GROUP };

DrawCountTable::DrawCountTable()
{
	//memset(m_vTable, 0, (MAX_MINLACK + 1) * (AVERAGE_MAX_DRAW_COUNT + 1));
	init();
}

void DrawCountTable::init()
{
	//default m_vMostPossibleMaxDrawCount
	/*for (int i = 0; i < MAX_MINLACK; i++) {
		m_vMostPossibleMaxDrawCount[i] = i + 3;
	}*/

	//setup
	if (g_bFirstSetup) {
		setupDrawCountTable();
		g_bFirstSetup = false;
	}
}

void DrawCountTable::setupDrawCountTable()
{
	string sDefaultFileName = "drawCountInfo";
	loadWinCountTableFromFile(sDefaultFileName);
	updateMostWinCountList();
	updateDrawCountTable();
}

void DrawCountTable::train(const int& iIterationCount)
{
	//int vTotalWinCount[MAX_MINLACK + 1] = { 0 };
	const int ciSampleCount = 200000;
	const double dTheshold = 0.9;
	const string sFileName = "drawCountInfo";

	loadWinCountTableFromFile(sFileName);

	int iCurrentBestIndex;
	int iMinLack;
	for (int i = 0; i < iIterationCount; i++) {
		initRandomState(MAX_HANDTILE_COUNT);
		while (!m_oPlayerTile.isWin() && m_oRemainTile.getRemainDrawNumber() > 0) {
			//throw tile
			iCurrentBestIndex = 0;
			iMinLack = m_oPlayerTile.getMinLack();
			for (int j = 1; j < AVERAGE_MAX_DRAW_COUNT; j++) {
				if (m_vWinCountTable[iMinLack][j] >= m_vWinCountTable[iMinLack][iCurrentBestIndex]) {
					iCurrentBestIndex = j;
				}
			}

			GodMoveSampling_v3 oSampling(m_oPlayerTile, m_oRemainTile);//set draw count?
			m_oPlayerTile.popTileFromHandTile(oSampling.throwTile(false, false, false));

			//pick tile
			Tile oDrawnTile = m_oRemainTile.randomPopTile();
			m_oPlayerTile.putTileToHandTile(oDrawnTile);

			if (m_oPlayerTile.isWin())
				break;

			CERR("PlayerTile: " + m_oPlayerTile.toString() + "\n");
			CERR("RemainTile:\n" + m_oRemainTile.getReadableString() + "\n");

			SamplingDataType oData(Move(), m_oPlayerTile, m_oRemainTile, AVERAGE_MAX_DRAW_COUNT);
			oData.sample(ciSampleCount);
			for (int j = 0; j <= AVERAGE_MAX_DRAW_COUNT; j++) {
				if (oData.getWinRate(j) > dTheshold) {
					m_vWinCountTable[iMinLack][j]++;
					break;
				}
			}
			if (oData.getWinRate(AVERAGE_MAX_DRAW_COUNT) <= dTheshold)
				m_vWinCountTable[iMinLack][AVERAGE_MAX_DRAW_COUNT + 1]++;
			//m_vTotalWinCount[m_oPlayerTile.getMinLack()]++;
			//vTotalWinCount[m_oPlayerTile.getMinLack()]++;

			//if (i % 100 == 99) {
				updateMostWinCountList();
				updateDrawCountTable();
				writeWinCountTableToFile(sFileName);
			//}


			//show current result
			//system("cls");
			std::cout << "Test Case " << i + 1 << std::endl;
			std::cout << "Current result:" << std::endl;
			for (int j = 0; j <= MAX_MINLACK; j++) {
				//std::cout << "[" << vTotalWinCount[j] << "]";
				for (int k = 0; k <= AVERAGE_MAX_DRAW_COUNT + 1; k++) {
					std::cout << "\t" << m_vWinCountTable[j][k];
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;

			std::cout << "Suit count: " << SUIT_COUNT << ", max rank: " << MAX_SUIT_TILE_RANK << std::endl;
			std::cout << "Honor count: " << HONOR_COUNT << ", max rank: " << MAX_HONOR_TILE_RANK << std::endl;
			std::cout << "Same tile count: " << SAME_TILE_COUNT << std::endl;
			std::cout << "Need group: " << NEED_GROUP << std::endl;
			std::cout << "Non-pickable tile count: " << NONPICKABLE_TILE_COUNT << std::endl;
		}
	}
}

void DrawCountTable::loadWinCountTableFromFile(const string& sFileName)
{
	ifstream fin(getRealFileName(sFileName));
	if (fin.is_open() && checkValidDrawCountTableVersion(fin)) {
		//load from file
		for (int i = 0; i <= MAX_MINLACK; i++) {
			for (int j = 0; j <= AVERAGE_MAX_DRAW_COUNT + 1; j++) {
				fin >> m_vWinCountTable[i][j];
				//m_vTotalWinCount[i] += m_vWinCountTable[i][j];
			}
		}
		fin.close();
		std::cerr << "[DrawCountTable::loadWinCountTableFromFile] Load from the file " << sFileName << "." << std::endl;
	}
	else {
		for (int i = 0; i < m_vWinCountTable.size(); i++) {
			std::fill(m_vWinCountTable[i].begin(), m_vWinCountTable[i].end(), 0);
		}
		std::cerr << "[DrawCountTable::loadWinCountTableFromFile] Cannot find the file " << sFileName << ". The win count table will initialize to zeros." << std::endl;
	}
}

void DrawCountTable::writeWinCountTableToFile(const string& sFileName)
{
	ofstream fout(getRealFileName(sFileName));
	if (fout.is_open()) {
		for (int i = 0; i < g_vInfo.size(); i++) {
			fout << g_vInfo.at(i) << " ";
		}
		fout << std::endl;
		for (int i = 0; i <= MAX_MINLACK; i++) {
			for (int j = 0; j <= AVERAGE_MAX_DRAW_COUNT + 1; j++) {
				fout << m_vWinCountTable[i][j] << " ";
			}
			fout << std::endl;
		}
		fout.close();
		std::cerr << "[DrawCountTable::writeWinCountTableToFile] Wrtie to the file " << sFileName << "." << std::endl;
	}
	else{
		std::cerr << "[DrawCountTable::writeWinCountTableToFile] Cannot wrtie to the file " << sFileName << "." << std::endl;
	}
	
}

void DrawCountTable::updateMostWinCountList()
{
	for (int i = 1; i <= MAX_MINLACK; i++) {
		int iBestIndex = std::min(i * 2, AVERAGE_MAX_DRAW_COUNT);//designer default
		for (int j = 1; j <= AVERAGE_MAX_DRAW_COUNT; j++) {
			if (m_vWinCountTable[i][j] > m_vWinCountTable[i][iBestIndex]) {
				iBestIndex = j;
			}
		}
		m_vMostPossibleMaxDrawCount[i] = iBestIndex;
	}
}

void DrawCountTable::updateDrawCountTable()
{
	//const int ciMaxDrawCount = AVERAGE_MAX_DRAW_COUNT > MAX_MINLACK ? (AVERAGE_MAX_DRAW_COUNT - MAX_MINLACK) : MAX_MINLACK;
	for (int drawCount = 1; drawCount <= AVERAGE_MAX_DRAW_COUNT; drawCount++) {
		g_vDrawCountTable[0][drawCount] = 1;
		for (int minLack = 1; minLack <= MAX_MINLACK; minLack++) {
			if (drawCount <= minLack) {
				g_vDrawCountTable[minLack][drawCount] = minLack;
				continue;
			}
			/*
			0) 1 1 2 3
			1) 1 1 2 3
			2) 1 2 2 3
			3) 1 ? 3 3
			4) 1 ? ? 4
			5) 1 ? ? ?
			6) 1 ? ? ?
			7) 1 ? ? ?
			8) 1 5 6 7
			*/

			if (minLack + 1 == AVERAGE_MAX_DRAW_COUNT) {
				g_vDrawCountTable[minLack][drawCount] = minLack + 1;
			}
			else {
				g_vDrawCountTable[minLack][drawCount] = minLack + 1 + static_cast<int>((m_vMostPossibleMaxDrawCount.at(minLack) - (minLack + 1)) * (drawCount - (minLack + 1)) / (AVERAGE_MAX_DRAW_COUNT - (minLack + 1)));
			}
			
		}
	}

	for (int minLack = 0; minLack <= MAX_MINLACK; minLack++) {
		g_vDrawCountTable[minLack][0] = g_vDrawCountTable[minLack][1];
	}

	/*std::cerr << "DrawCountTable:" << std::endl;
	for (int i = 0; i < MAX_MINLACK; i++) {
		for (int j = 0; j < MAX_DRAW_COUNT; j++) {
			std::cerr << g_vDrawCountTable_v2[i][j] << " ";
		}
		std::cerr << std::endl;
	}*/
}

void DrawCountTable::initRandomState(const int& iHandTileNumber)
{
	m_oPlayerTile.clear();
	m_oRemainTile.init(RemainTileType::Type_Playing);
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
						m_oRemainTile.undoPopTile(i);
						Tile oTile = m_oRemainTile.randomPopTile();
						m_oPlayerTile.putTileToHandTile(oTile);
					}
				}
			}
		}


		int iInitMinLack = m_oPlayerTile.getMinLack();
		if (iInitMinLack == 0)
			continue;

		int iMaxPopTileCount = m_oRemainTile.getRemainDrawNumber() / 5;
		int iRandomThrownTileCount = rand() % iMaxPopTileCount;
		//int iRandomThrownTileCount = rand() % (m_oRemainTile.getRemainDrawNumber() - DIFFERENT_TILE_NUMBER);
		for (int i = 0; i < iRandomThrownTileCount; i++) {
			Tile oTile = m_oRemainTile.randomPopTile();
			if (m_oRemainTile.getRemainNumber(oTile) == 0) {
				m_oRemainTile.undoPopTile(oTile);
				i--;
			}
		}
		break;
	}
}

bool DrawCountTable::checkValidDrawCountTableVersion(ifstream& fin)
{
	int iNumber;
	for (int i = 0; i < g_vInfo.size(); i++) {
		fin >> iNumber;
		if (iNumber != g_vInfo[i]) {
			std::cerr << "[GodMoveSampling_v3::checkValidDrawCountTableVersion] The file is not consistent: vInfo[" << i << "]=" << iNumber
				<< ", but the expected value is " << g_vInfo[i] << "." << std::endl;
			return false;
		}
	}
	return true;
}

void DrawCountTable::printDrawCountTable() const
{
	for (int i = 0; i <= MAX_MINLACK; i++) {
		for (int j = 0; j <= AVERAGE_MAX_DRAW_COUNT; j++) {
			std::cout << g_vDrawCountTable[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

string DrawCountTable::getRealFileName(const string& sFileName) const
{
	stringstream ss;
	ss << sFileName;
	for (int i = 0; i < g_vInfo.size(); i++) {
		ss << "_" << g_vInfo[i];
	}
	return ss.str();
}
