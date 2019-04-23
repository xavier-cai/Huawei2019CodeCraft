#ifndef MAP_ARRAY_H
#define MAP_ARRAY_H

#include <map>
#include "copy-object.h"

template <typename TK>
class Indexer : public CopyableTemplate< Indexer<TK> >
{
public:
    virtual ~Indexer();
    const static int NoneIndex; //return NoneIndex means invalid key
    virtual void Input(const TK& key, const int& index); //input
    virtual int Output(const TK& key) const; //output

};//class Indexer

template <typename TK>
class IndexerDefault : public CopyableTemplate< IndexerDefault<TK>, Indexer<TK> >
{
public:
    virtual void Input(const TK& key, const int& index) override;
    virtual int Output(const TK& key) const override;

protected:
    std::map<TK, int> m_map;
    
};//class IndexerDefault

template <typename TK, typename TV>
class MapArray
{
public:
    typedef Indexer<TK> IndexerType;
    MapArray();
    MapArray(const int& size);
    MapArray(const int& size, const bool& isCreate);
    MapArray(const int& size, const IndexerType& indexer);
    virtual ~MapArray();

    void ReplaceDataByIndex(const int& index, TV* data);
    void ReplaceIndexer(const IndexerType& indexer);
    const IndexerType& GetIndexer() const;
    const int& Size() const;
    void Map(const TK& key, const int& index);
    TV* Find(const TK& key) const;

    TV& operator [] (const TK& key);
    const TV& operator [] (const TK& key) const;

private:
    void InitializeData();

    bool m_isManageMemory;
    TV** m_datas;
    int m_size;
    IndexerType* m_indexer;

};//class MapArray

template <typename TK>
class IndexerEnhanced : public CopyableTemplate< IndexerEnhanced<TK>, IndexerDefault<TK> >
{
public:
    virtual void Input(const TK& key, const int& index) override;
    virtual int Output(const TK& key) const override;
    int GetDiffer() const;
    bool GetIsConstantDiffer() const;

protected:
    bool m_isConstantDiffer;
    int m_differ;

};//class IndexerEnhanced

template <typename TK>
class IndexerMirror : public CopyableTemplate< IndexerMirror<TK>, Indexer<TK> >
{
public:
    IndexerMirror(const Indexer<TK>& mirror);
    virtual void Input(const TK& key, const int& index) override;
    virtual int Output(const TK& key) const override;

protected:
    const Indexer<TK>& m_mirror;

};//class IndexerMirror

#include "map-array.cpp"

#endif