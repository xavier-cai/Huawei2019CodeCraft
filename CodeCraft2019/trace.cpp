#include "trace.h"
#include "assert.h"

Trace::Trace()
{
    m_container.push_back(-1);
    m_end = 0;
}

Trace::Trace(const Trace& o)
{
    *this = o;
}

Trace& Trace::operator = (const Trace& o)
{
    m_container = o.m_container;
    m_container.push_back(-1);
    m_end = o.m_end;
    return *this;
}

void Trace::RemoveFromTail()
{
    ASSERT(m_end != 0);
    --m_end;
    m_container[m_end] = -1;
}

void Trace::AddToTail(int id)
{
    ASSERT(id >= 0);
    m_container[m_end] = id;
    ++m_end;
    if (m_end == m_container.size())
    {
        m_container.push_back(-1);
    }
}

void Trace::Clear(const std::size_t& untill)
{
    while (m_end != untill)
    {
        RemoveFromTail();
    }
}

void Trace::Clear()
{
    Clear(0);
}
