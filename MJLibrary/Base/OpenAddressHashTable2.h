#pragma once
//#include "boost/smart_ptr/make_shared_array.hpp"
#include <memory>

typedef unsigned long long HashKey;
typedef unsigned int HashTableIndexType;

template<class _data> class OpenAddressHashTable2;
template<class _data> class OpenAddressHashTableEntry2 {
	friend class OpenAddressHashTable2<_data>;
private:
	bool m_bIsFree;
	HashKey m_key;

public:
	_data m_data;
	OpenAddressHashTableEntry2() : m_bIsFree(true) {}
	inline void setEntry(HashKey key) { m_key = key; }
	inline void clear()
	{
		//m_data.clear();
		m_bIsFree = true;
	}
	inline HashKey getHashKey() const { return m_key; }
};

template<class _data> class OpenAddressHashTable2 {
private:
	thread_local static HashTableIndexType m_count;
	thread_local static size_t m_size;

public:
	thread_local static std::unique_ptr<OpenAddressHashTableEntry2<_data>[]> m_entry;
	thread_local static HashTableIndexType m_mask;
	thread_local static bool g_bFirstSetup;

	OpenAddressHashTable2() {};
	~OpenAddressHashTable2() {};

	static void makeTable(const uint32_t& bitSize = 21)
	{
		if (!g_bFirstSetup)
			return;

		m_mask = (1 << bitSize) - 1;
		m_count = 0;
		m_size = (1 << bitSize);
		m_entry = std::make_unique<OpenAddressHashTableEntry2<_data>[]>(1ULL << bitSize);
		g_bFirstSetup = false;
		std::cerr << "[OpenAddressHashTable2::makeTable] m_size = " << m_size << std::endl;
	}

	static HashTableIndexType getCount() { return m_count; }

	static bool isFull() { return m_count >= m_size; }

	static HashTableIndexType lookup(const HashKey& key)
	{
		HashTableIndexType index = static_cast<HashTableIndexType>(key)&m_mask;
		HashTableIndexType lastIndex = (index - 1)&m_mask;

		while (index != lastIndex) {
			const OpenAddressHashTableEntry2<_data>& entry = m_entry[index];

			if (!entry.m_bIsFree) {
				if (entry.m_key == key) { return index; }
				else { index = (index + 1)&m_mask; }
			}
			else {
				return -1;
			}
		}
		return -1;
	}

	static void store(const HashKey& key, const _data& data)
	{
		HashTableIndexType index = static_cast<HashTableIndexType>(key)&m_mask;
		HashTableIndexType lastIndex = (index - 1)&m_mask;

		while (index != lastIndex) {
			OpenAddressHashTableEntry2<_data>& entry = m_entry[index];

			if (entry.m_bIsFree) {
				entry.m_key = key;
				entry.m_data = data;
				entry.m_bIsFree = false;
				m_count++;
				break;
			}
			else {
				index = (index + 1)&m_mask;
			}
		}
	}

	static void clear()
	{
		m_count = 0;
		for (uint32_t i = 0; i < m_size; ++i) { m_entry[i].clear(); }
	}
};

template<class _data>
thread_local HashTableIndexType OpenAddressHashTable2<_data>::m_count = 0;
template<class _data>
thread_local size_t OpenAddressHashTable2<_data>::m_size;
template<class _data>
thread_local std::unique_ptr<OpenAddressHashTableEntry2<_data>[]> OpenAddressHashTable2<_data>::m_entry;
template<class _data>
thread_local HashTableIndexType OpenAddressHashTable2<_data>::m_mask;
template<class _data>
thread_local bool OpenAddressHashTable2<_data>::g_bFirstSetup = true;