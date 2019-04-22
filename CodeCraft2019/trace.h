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

    inline Node Head();
    inline Node Tail();
    inline NodeConst Head() const;
    inline NodeConst Tail() const;

    const std::size_t& Size() const;
    void RemoveFromTail();
    void AddToTail(int id);
    void Clear(const std::size_t& untill);
    void Clear();

    inline int& operator [] (const std::size_t& index);
    inline const int& operator [] (const std::size_t& index) const;
    
};//class Trace

#include <list>

template <typename _T> inline
std::_List_iterator<_T> operator + (const std::_List_iterator<_T>& ite, int n)
{
    auto copy = ite;
    bool opposite = n < 0;
    if (opposite) n = -n;
    while(--n >= 0)
        opposite ? --copy : ++copy;
    return copy;
}

template <typename _T> inline
std::_List_iterator<_T> operator - (const std::_List_iterator<_T>& ite, int n)
{
    return ite + -n;
}





/* 
 * [inline functions]
 *   it's not good to write code here, but we really need inline!
 */

inline Trace::Node Trace::Head()
{
    return m_container.begin();
}

inline Trace::Node Trace::Tail()
{
    return m_container.begin() + m_end;
}

inline Trace::NodeConst Trace::Head() const
{
    return m_container.begin();
}

inline Trace::NodeConst Trace::Tail() const
{
    return m_container.begin() + m_end;
}

inline const std::size_t& Trace::Size() const
{
    return m_end;
}

inline int& Trace::operator [] (const std::size_t& index)
{
    return m_container[index];
}

inline const int& Trace::operator [] (const std::size_t& index) const
{
    return m_container[index];
}

#endif
