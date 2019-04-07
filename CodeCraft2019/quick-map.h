#ifndef QUICK_MAP_H
#define QUICK_MAP_H

#include <map>
#include "map-array.h"
#include "callback.h"
#include "assert.h"

template <typename _TK, typename _TV>
class QuickMap
{
public:
    typedef typename std::map<_TK, _TV>::iterator iterator;
    typedef typename std::map<_TK, _TV>::const_iterator const_iterator;

    QuickMap() : m_size(0), m_ites(0), m_indexer(0) { }

    QuickMap(const unsigned int& size, const IndexerEnhanced<_TK>* indexer)
        : m_size(size), m_indexer(indexer->CopyObject()) { Initilize(); }

    QuickMap(const unsigned int& size, const std::map<_TK, _TV>& src, const IndexerEnhanced<_TK>* indexer)
        : m_size(size), m_map(src), m_indexer(indexer->CopyObject()) { Initilize(); }

    template <typename _TSRC>
    QuickMap(const unsigned int& size, const std::map<_TK, _TSRC>& src, const IndexerEnhanced<_TK>* indexer)
        : m_size(size), m_indexer(indexer->CopyObject())
    {
        m_ites = m_size > 0 ? (new iterator[m_size]) : 0;
        for (unsigned int i = 0; i < m_size; i++)
            m_ites[i] = m_map.end();
        for (typename std::map<_TK, _TSRC>::const_iterator ite = src.begin(); ite != src.end(); ++ite)
        {
            std::pair<iterator, bool> result = m_map.insert(std::make_pair(ite->first, _TV(ite->second)));
            ASSERT(result.second);
            m_ites[m_indexer->Output(ite->first)] = result.first;
        }
    }
    
    QuickMap(const QuickMap& o)
        : m_size(0), m_ites(0), m_indexer(0) { *this = o; }

    QuickMap& operator = (const QuickMap& o)
    {
        Dispose();
        m_size = o.m_size;
        m_map = o.m_map;
        m_indexer = o.m_indexer->CopyObject();
        Initilize();
        return *this;
    }

    ~QuickMap() { Dispose(); }

    iterator begin() { return m_map.begin(); }
    const_iterator begin() const { return m_map.begin(); }
    iterator end() { return m_map.end(); }
    const_iterator end() const { return m_map.end(); }

//#define ITERATOR_VALID(ite) (ite##._Getcont() != 0)
//#define INVALID_ITERATOR iterator()
#define ITERATOR_VALID(ite) (ite != end())
#define INVALID_ITERATOR end()
    _TV& operator [] (const _TK& index) const
    {
        auto& ret = m_ites[m_indexer->Output(index)];
        ASSERT(ITERATOR_VALID(ret));
        return ret->second;
    }

    iterator find (const _TK& index)
    {
        auto& ret = m_ites[m_indexer->Output(index)];
        return ITERATOR_VALID(ret) ? ret : end();
    }

    const_iterator find (const _TK& index) const
    {
        auto& ret = m_ites[m_indexer->Output(index)];
        return ITERATOR_VALID(ret) ? ret : end();
    }

    iterator erase (const const_iterator& ite)
    {
        m_ites[m_indexer->Output(ite->first)] = INVALID_ITERATOR;
        return m_map.erase(ite);
    }

    std::pair<iterator, bool> insert (const _TK& index, const _TV& v)
    {
        auto result = m_map.insert(std::make_pair(index, v));
        if (result.second)
            m_ites[m_indexer->Output(index)] = result.first;
        return result;
    }

    void clear()
    {
        for (auto ite = m_map.begin(); ite != m_map.end(); ++ite)
            m_ites[m_indexer->Output(ite->first)] = m_map.end();
        m_map.clear();
    }
#undef ITERATOR_VALID

    typename std::map<_TK, _TV>::size_type size() { return m_map.size(); }

private:
    void Initilize()
    {
        m_ites = m_size > 0 ? (new iterator[m_size]) : 0;
        for (unsigned int i = 0; i < m_size; i++)
            m_ites[i] = m_map.end();
        for (auto ite = m_map.begin(); ite != m_map.end(); ++ite)
            m_ites[m_indexer->Output(ite->first)] = ite;
    }

    void Dispose()
    {
        if (m_ites != 0)
            delete m_ites;
        if (m_indexer != 0)
            delete m_indexer;
    }

    unsigned int m_size;
    std::map<_TK, _TV> m_map;
    iterator* m_ites;
    IndexerEnhanced<_TK>* m_indexer;

};//class QuickMap

#endif