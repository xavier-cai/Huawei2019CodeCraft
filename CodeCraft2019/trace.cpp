#include "trace.h"
#include "assert.h"

Trace::Trace()
    : m_size(0)
{
    m_container.push_back(-1);
    m_end = m_container.begin();
}

Trace::Trace(const Trace& o)
{
    *this = o;
}

Trace& Trace::operator = (const Trace& o)
{
    for (auto ite = o.m_container.begin(); ite != o.m_end; ++ite)
        m_container.push_back(*ite);
    m_container.push_back(-1);
    m_end = --m_container.end();
    return *this;
}

Trace::Node Trace::Head()
{
    return m_container.begin();
}

Trace::Node Trace::Tail()
{
    return m_end;
}

Trace::NodeConst Trace::Head() const
{
    return m_container.begin();
}

Trace::NodeConst Trace::Tail() const
{
    return m_end;
}

const std::size_t& Trace::Size() const
{
    return m_size;
}

void Trace::RemoveFromTail()
{
    ASSERT(m_end != m_container.begin());
    --m_end;
    *m_end = -1;
    --m_size;
}

void Trace::AddToTail(int id)
{
    *m_end = id;
    ++m_end;
    if (m_end == m_container.end())
    {
        m_container.push_back(-1);
        --m_end;
    }
    ++m_size;
}

void Trace::Clear(NodeConst untill)
{
    while (m_end != untill)
    {
        RemoveFromTail();
    }
}

void Trace::Clear()
{
    Clear(m_container.begin());
}
