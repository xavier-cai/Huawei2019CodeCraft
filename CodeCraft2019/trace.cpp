#include "trace.h"
#include "assert.h"

Trace::Trace()
{
    m_current = m_container.begin();
}

Trace::Trace(const Trace& o)
{
    *this = o;
}

Trace& Trace::operator = (const Trace& o)
{
    m_container = o.m_container;
    m_current = m_container.begin();
    for (auto ite = o.m_container.begin(); ite != o.m_current && ite != o.m_container.end(); ite++, m_current++);
    return *this;
}

Trace::Node Trace::Head()
{
    return m_container.begin();
}

Trace::Node Trace::Tail()
{
    return m_container.end();
}

Trace::Node& Trace::Current()
{
    return m_current;
}

Trace::NodeConst Trace::Head() const
{
    return m_container.begin();
}

Trace::NodeConst Trace::Tail() const
{
    return m_container.end();
}

Trace::NodeConst Trace::Current() const
{
    return m_current;
}

std::size_t Trace::Size() const
{
    return m_container.size();
}

void Trace::RemoveFromTail()
{
    ASSERT(m_current != m_container.end());
    bool end = m_current == --m_container.end();
    m_container.erase(--m_container.end());
    if (end)
        m_current = m_container.end();
}

void Trace::AddToTail(int id)
{
    m_container.push_back(id);
    if (m_current == m_container.end())
        m_current--;
}

void Trace::Clear()
{
    ASSERT(m_current == m_container.begin());
    m_container.clear();
    m_current = m_container.begin();
}

Trace::Node Trace::Forward()
{
    ASSERT(m_current != m_container.end());
    m_current++;
    return m_current;
}

Trace::Node Trace::Backward()
{
    ASSERT(m_current != m_container.begin());
    m_current--;
    return m_current;
}
