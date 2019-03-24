#ifndef MAP_ARRAY_CPP
#define MAP_ARRAY_CPP

#include "map-array.h"
#include "assert.h"

template <typename TK>
const int Indexer<TK>::NoneIndex(-1);

template <typename TK>
Indexer<TK>::~Indexer()
{ }

template <typename TK>
void Indexer<TK>::Input(const TK& key, const int& index)
{ }

template <typename TK>
int Indexer<TK>::Output(const TK& key) const
{
    return NoneIndex;
}



template <typename TK>
void IndexerDefault<TK>::Input(const TK& key, const int& index)
{
    m_map[key] = index;
}

template <typename TK>
int IndexerDefault<TK>::Output(const TK& key) const
{
    auto find = m_map.find(key);
    if (find == m_map.end())
        return Indexer<TK>::NoneIndex;
    return find->second;
}




template <typename TK, typename TV>
MapArray<TK, TV>::MapArray()
{
    ASSERT(false);
}

template <typename TK, typename TV>
MapArray<TK, TV>::MapArray(const int& size)
    : m_size(size)
{
    InitializeData();
    m_indexer = new IndexerDefault<TK>();
}

template <typename TK, typename TV>
MapArray<TK, TV>::MapArray(const int& size, const bool& isCreate)
    : m_isManageMemory(isCreate), m_size(size)
{
    InitializeData();
    m_indexer = new IndexerDefault<TK>();
}

template <typename TK, typename TV>
MapArray<TK, TV>::MapArray(const int& size, const typename MapArray<TK, TV>::IndexerType& indexer)
    : m_isManageMemory(true), m_size(size)
{
    InitializeData();
    m_indexer = indexer.CopyObject(); 
}

template <typename TK, typename TV>
MapArray<TK, TV>::~MapArray()
{
    if (m_isManageMemory)
        for (int i = 0; i < m_size; i++)
            delete m_datas[i];
    delete[] m_datas;
    delete m_indexer;
}

template <typename TK, typename TV>
void MapArray<TK, TV>::InitializeData()
{
    m_datas = new TV*[m_size];
    if (m_isManageMemory)
        for (int i = 0; i < m_size; i++)
            m_datas[i] = new TV();
}

template <typename TK, typename TV>
void MapArray<TK, TV>::ReplaceDataByIndex(const int& index, TV* data)
{
    ASSERT(!m_isManageMemory);
    ASSERT(index != IndexerType::NoneIndex && index >= 0 && index < m_size);
    m_datas[index] = data;
}

template <typename TK, typename TV>
void MapArray<TK, TV>::ReplaceIndexer(const typename MapArray<TK, TV>::IndexerType& indexer)
{
    delete m_indexer;
    m_indexer = indexer.CopyObject(); 
}

template <typename TK, typename TV>
const typename MapArray<TK, TV>::IndexerType& MapArray<TK, TV>::GetIndexer() const
{
    return *m_indexer;
}

template <typename TK, typename TV>
const int& MapArray<TK, TV>::Size() const
{
    return m_size;
}

template <typename TK, typename TV>
void MapArray<TK, TV>::Map(const TK& key, const int& index)
{
    ASSERT(index != IndexerType::NoneIndex && index >= 0 && index < m_size);
    m_indexer->Input(key, index);
}

template <typename TK, typename TV>
TV* MapArray<TK, TV>::Find(const TK& key) const
{
    auto index = m_indexer->Output(key);
    if (index != IndexerType::NoneIndex && index >= 0 && index < m_size)
        return m_datas[index];
    return 0;
}

template <typename TK, typename TV>
TV& MapArray<TK, TV>::operator [] (const TK& key)
{
    int index = m_indexer->Output(key);
    ASSERT(index != IndexerType::NoneIndex && index >= 0 && index < m_size);
    return m_datas[index];
}

template <typename TK, typename TV>
const TV& MapArray<TK, TV>::operator [] (const TK& key) const
{
    int index = m_indexer->Output(key);
    ASSERT(index != IndexerType::NoneIndex && index >= 0 && index < m_size);
    return m_datas[index];
}



template <typename TK>
void IndexerEnhanced<TK>::Input(const TK& key, const int& index)
{
    if (IndexerDefault<TK>::m_map.size() == 0)
    {
        m_isConstantDiffer = true;
        m_differ = key - index;
    }
    if (m_isConstantDiffer)
    {
        m_isConstantDiffer = (key - index) == m_differ;
    }
    IndexerDefault<TK>::Input(key, index);
}

template <typename TK>
int IndexerEnhanced<TK>::Output(const TK& key) const
{
    if (m_isConstantDiffer)
    {
        return key - m_differ;
    }
    return IndexerDefault<TK>::Output(key);
}

template <typename TK>
IndexerMirror<TK>::IndexerMirror(const Indexer<TK>& mirror)
    : m_mirror(mirror)
{ }

template <typename TK>
void IndexerMirror<TK>::Input(const TK& key, const int& index)
{
    ASSERT(false);
}

template <typename TK>
int IndexerMirror<TK>::Output(const TK& key) const
{
    return m_mirror.Output(key);
}

#endif