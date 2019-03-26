#ifndef TRACE_H
#define TRACE_H

#include <list>

class Trace
{
public:
    typedef std::list<int> Container;
    typedef Container::iterator Node;
    typedef Container::const_iterator NodeConst;
    
private:
    Container m_container;
    Node m_end;
    std::size_t m_size;
    
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
    void Clear(NodeConst untill);
    void Clear();
    
};//class Trace

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
