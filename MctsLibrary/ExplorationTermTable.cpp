#include "ExplorationTermTable.h"
#include <iostream>
#include <fstream>
#include "../MJLibrary/Base/Ini.h"
#include "../MJLibrary/Base/Tools.h"
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;
#ifdef USE_SHM_EXPLR_TERM_TABLE
ExplorationTerm_t* ExplorationTermTable::m_pExplorationTermTable;
managed_shared_memory ExplorationTermTable::m_oExplorationTermTable_segment;
string ExplorationTermTable::g_sShmSegmentName = "Mcts2_ExplorationTermTableSgm";
string ExplorationTermTable::g_sShmTableName = "Mcts2_ExplorationTermTable";
bool ExplorationTermTable::m_bIsShmCreater = false;
#else
array<ExplorationTerm_t, EXPLORATION_TERM_TABLE_SIZE> ExplorationTermTable::m_pExplorationTermTable = 
{
	-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0,
	0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
	0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
	0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
	0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
	0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
	0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
	0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
	0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
	0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
	0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
	0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1
};
#endif
uint32_t ExplorationTermTable::m_iSetCounter = 0;

ExplorationTermTable::ExplorationTermTable()
{
	makeTable();
}

ExplorationTermTable::~ExplorationTermTable()
{
#ifdef USE_SHM_EXPLR_TERM_TABLE
	if(m_bIsShmCreater)
		shared_memory_object::remove(g_sShmSegmentName.c_str());
#endif
}

void ExplorationTermTable::makeTable()
{
#ifdef USE_SHM_EXPLR_TERM_TABLE
	try {
		//try to create a segment
		m_oExplorationTermTable_segment = managed_shared_memory
		(
			create_only
			, g_sShmSegmentName.c_str()
			, EXPLORATION_TERM_TABLE_SIZE * sizeof(ExplorationTerm_t) + 256 //Some memory is used by segment itself.
		);

		//construct table in the segment
		m_pExplorationTermTable = m_oExplorationTermTable_segment.construct<ExplorationTerm_t>(g_sShmTableName.c_str())[EXPLORATION_TERM_TABLE_SIZE](1.0);

		//load table from file
		loadFromFile();

		//other setup
		m_bIsShmCreater = true;
	}
	catch (interprocess_exception &ex) {
		if (ex.get_error_code() == 9) {//segment is existed
			std::cerr << "[ExplorationTermTable::init] shm_segment is already created." << std::endl;
			try {
				//attach to the segment
				m_oExplorationTermTable_segment = managed_shared_memory
				(
					open_only
					, g_sShmSegmentName.c_str()
				);
				std::cerr << "[ExplorationTermTable::init] shm_segment opened." << std::endl;

				//setup table pointer
				m_pExplorationTermTable = m_oExplorationTermTable_segment.find<ExplorationTerm_t>(g_sShmTableName.c_str()).first;

				//load table from file
				loadFromFile();
				//[CAUTION] The table from file (which might be old) will overwrite the current table
				//Please make sure this step will not cause wrong behavior, or just remove this step if you need.

				//other setup
				m_bIsShmCreater = false;
			}
			catch (interprocess_exception &ex) {
				std::cerr << "[ExplorationTermTable::init] ex = " << ex.what() << std::endl;
				std::cerr << "[ExplorationTermTable::init] get_error_code = " << ex.get_error_code() << std::endl;
			}
		}
		else {
			std::cerr << "[ExplorationTermTable::init] ex = " << ex.what() << std::endl;
			std::cerr << "[ExplorationTermTable::init] get_error_code = " << ex.get_error_code() << std::endl;
		}
	}
#else
	//load table from file
	loadFromFile();
	//printTable();
#endif
	
}

ExplorationTerm_t ExplorationTermTable::getExplorationTerm(const uint32_t & uiMinLack, const uint32_t & uiDrawCount)
{
	return m_pExplorationTermTable[getIndex(uiMinLack, uiDrawCount)];
}

void ExplorationTermTable::setExplorationTerm(const uint32_t & uiMinLack, const uint32_t & uiDrawCount, const ExplorationTerm_t oTerm)
{
	m_pExplorationTermTable[getIndex(uiMinLack, uiDrawCount)] = oTerm;

	//[optional]Dynamic update
	m_iSetCounter++;
	//if (m_iSetCounter % 100 == 0) {
		writeToFile();
		CERR("[ExplorationTermTable::setExplorationTerm] Update to file.\n");
		//m_iSetCounter = 0;
	//}
}

void ExplorationTermTable::loadFromFile(const string& sFileName)
{
#ifdef USE_SHM_EXPLR_TERM_TABLE
	assert(m_pExplorationTermTable != nullptr);
#endif
	
	CERR("[ExplorationTermTable::loadFromFile] sFileName = " + sFileName + ".\n");
	ifstream fin(sFileName);
	ExplorationTerm_t oTerm;
	if (fin.is_open()) {
		//load from the file
		for (int i = 0; i <= MAX_MINLACK; i++) {
			for (int j = 0; j <= AVERAGE_MAX_DRAW_COUNT; j++) {
				fin >> oTerm;
				m_pExplorationTermTable[getIndex(i, j)] = oTerm;
			}
		}
		fin.close();
		CERR("[ExplorationTermTable::loadFromFile] Load from the file " + sFileName + ".\n");
	}
	else {
		std::cerr << "[ExplorationTermTable::loadFromFile] Cannot load from the file " << sFileName << "." << std::endl;
		assert(fin.is_open());
	}
	
}

void ExplorationTermTable::writeToFile(const string& sFileName)
{
	ofstream fout(sFileName);
	if (fout.is_open()) {
		//write to the file
		for (int i = 0; i <= MAX_MINLACK; i++) {
			for (int j = 0; j <= AVERAGE_MAX_DRAW_COUNT; j++) {
				fout << m_pExplorationTermTable[getIndex(i, j)] << " ";
			}
			fout << std::endl;
		}
		fout.close();
		std::cerr << "[ExplorationTermTable::writeToFile] Write to the file " << sFileName << "." << std::endl;
	}
	else {
		std::cerr << "[ExplorationTermTable::writeToFile] Cannot write to the file " << sFileName << "." << std::endl;
	}
	
}

void ExplorationTermTable::loadFromFile()
{
	Ini& oIni = Ini::getInstance();
	string sPath = oIni.getStringIni("MCTS.ExplorationTermTableFileName");
	loadFromFile(getCurrentPath() + sPath);
}

void ExplorationTermTable::writeToFile()
{
	Ini& oIni = Ini::getInstance();
	string sPath = oIni.getStringIni("MCTS.ExplorationTermTableFileName");
	writeToFile(getCurrentPath() + sPath);
}

void ExplorationTermTable::printTable()
{
	for (int i = 0; i <= MAX_MINLACK; i++) {
		for (int j = 0; j <= AVERAGE_MAX_DRAW_COUNT; j++) {
			std::cout << m_pExplorationTermTable[getIndex(i, j)] << "\t";
		}
		std::cout << std::endl;
	}
}
