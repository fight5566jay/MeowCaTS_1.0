#pragma once
#include "../MJLibrary/MJ_Base/MahjongBaseConfig.h"
#include <array>
#include <string>
#define EXPLORATION_TERM_TABLE_SIZE ((MAX_MINLACK + 1) * (AVERAGE_MAX_DRAW_COUNT + 1))
using std::array;
using std::string;
typedef float ExplorationTerm_t;

//#define USE_SHM_EXPLR_TERM_TABLE
#ifdef USE_SHM_EXPLR_TERM_TABLE
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
using namespace boost::interprocess;
#endif



class ExplorationTermTable //TODO: convert to static class for more convenient usage (ex: in MctsPlayer)
{
public:
	ExplorationTermTable();
	~ExplorationTermTable();

	static void makeTable();
	static uint32_t getIndex(const uint32_t& uiMinLack, const uint32_t& uiDrawCount) { return uiMinLack * (AVERAGE_MAX_DRAW_COUNT + 1) + uiDrawCount; };
	static ExplorationTerm_t getExplorationTerm(const uint32_t& uiMinLack, const uint32_t& uiDrawCount);
	static void setExplorationTerm(const uint32_t& uiMinLack, const uint32_t& uiDrawCount, const ExplorationTerm_t oTerm);
	static void loadFromFile(const string& sFileName);
	static void writeToFile(const string& sFileName);
	static void loadFromFile();
	static void writeToFile();
	static void printTable();

#ifdef USE_SHM_EXPLR_TERM_TABLE
	static ExplorationTerm_t* m_pExplorationTermTable;
	static managed_shared_memory m_oExplorationTermTable_segment;
	static string g_sShmSegmentName;
	static string g_sShmTableName;
	static bool m_bIsShmCreater;
#else
	static array<ExplorationTerm_t, EXPLORATION_TERM_TABLE_SIZE> m_pExplorationTermTable;
#endif
	

	static uint32_t m_iSetCounter;
};
