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
    Node m_current;
    
public:
    Trace();
    Trace(const Trace& o);
    Trace& operator = (const Trace& o);
    Node Head();
    Node Tail();
    Node& Current();
    NodeConst Head() const;
    NodeConst Tail() const;
    NodeConst Current() const;
    std::size_t Size() const;
    void RemoveFromTail();
    void AddToTail(int id);
    void Clear();
    Node Forward();
    Node Backward();
    
};//class Trace

#endif
