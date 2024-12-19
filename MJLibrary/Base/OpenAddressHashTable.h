#pragma once
//#include "boost/smart_ptr/make_shared_array.hpp"
#include <memory>

typedef unsigned long long HashKey;
typedef unsigned int HashTableIndexType;

template<class _data> class OpenAddressHashTable;
template<class _data> class OpenAddressHashTableEntry {
	friend class OpenAddressHashTable<_data>;
private:
	bool m_bIsFree;
	HashKey m_key;

public:
	_data m_data;
	OpenAddressHashTableEntry() : m_bIsFree(true) {}
	inline void setEntry(HashKey key) { m_key = key; }
	inline void clear()
	{
		//m_data.clear();
		m_bIsFree = true;
	}
	inline HashKey getHashKey() const { return m_key; }
};

template<class _data> class OpenAddressHashTable {
private:
	HashTableIndexType m_count;
	const size_t m_size;

public:
	//OpenAddressHashTableEntry<_data>* m_entry;
	std::unique_ptr<OpenAddressHashTableEntry<_data>[]> m_entry;
	const HashTableIndexType m_mask;

	OpenAddressHashTable(int bitSize = 21)
		: m_mask((1 << bitSize) - 1)
		, m_count(0)
		, m_size(1 << bitSize)
		//, m_entry(new OpenAddressHashTableEntry<_data>[1ULL << bitSize])
	{
		m_entry =std::make_unique<OpenAddressHashTableEntry<_data>[]>(1ULL << bitSize);
	}

	~OpenAddressHashTable() { /*delete[] m_entry;*/ }

	HashTableIndexType getCount() const { return m_count; }

	bool isFull() const { return m_count >= m_size; }

	HashTableIndexType lookup(const HashKey& key) const
	{
		HashTableIndexType index = static_cast<HashTableIndexType>(key)&m_mask;
		HashTableIndexType lastIndex = (index - 1)&m_mask;

		while (index != lastIndex) {
			const OpenAddressHashTableEntry<_data>& entry = m_entry[index];

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

	_data getData(const HashTableIndexType& index) const
	{
		assert(index >= 0 && index < m_size);
		return m_entry[index].m_data;
	}

	void store(const HashKey& key, const _data& data)
	{
		HashTableIndexType index = static_cast<HashTableIndexType>(key)&m_mask;
		HashTableIndexType lastIndex = (index - 1)&m_mask;

		while (index != lastIndex) {
			OpenAddressHashTableEntry<_data>& entry = m_entry[index];

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

	void clear()
	{
		m_count = 0;
		for (uint i = 0; i < m_size; ++i) { m_entry[i].clear(); }
	}
};