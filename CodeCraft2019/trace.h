#ifndef TRACE_H
#define TRACE_H

#include <vector>

class Trace
{
public:
    typedef std::vector<int> Container;
    typedef Container::iterator Node;
    typedef Container::const_iterator NodeConst;
    
private:
    Container m_container;
    std::size_t m_end;
    
public:
    Trace();
    Trace(const Trace& o);
    Trace& operator = (const Trace& o);
    Node Head();
    Node Tail();
    NodeConst Head() const;
    NodeConst Tail() const;
    const std::size_t& Size() const;
    void RemoveFromTail();
    void AddToTail(int id);
    void Clear(const std::size_t& untill);
    void Clear();

    int& operator [] (const std::size_t& index);
    const int& operator [] (const std::size_t& index) const;
    
};//class Trace

#include <list>

template <typename _T>
std::_List_iterator<_T> operator + (const std::_List_iterator<_T>& ite, int n)
{
    auto copy = ite;
    bool opposite = n < 0;
    if (opposite) n = -n;
    while(--n >= 0)
        opposite ? --copy : ++copy;
    return copy;
}

template <typename _T>
std::_List_iterator<_T> operator - (const std::_List_iterator<_T>& ite, int n)
{
    return ite + -n;
}

#endif
