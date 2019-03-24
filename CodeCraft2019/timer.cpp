#include "timer.h"
#include "stdlib.h"

TimerHandle::TimerHandle(clock_t record)
    : m_record(record)
{ }

double TimerHandle::GetSpendTime() const
{
    return Timer::Instance.DoGetSpendTime(m_record);
}



Timer Timer::Instance;

Timer::Timer()
    : m_start(clock())
{ }

double Timer::DoGetSpendTime(clock_t t) const
{
    return (double)(clock() - t) / CLOCKS_PER_SEC;
}

const TimerHandle Timer::DoRecord() const
{
    return TimerHandle(clock());
}

double Timer::DoGetSpendTime() const
{
    return DoGetSpendTime(m_start);
}

double Timer::DoGetLeftTime(double max) const
{
    return max - DoGetSpendTime();
}

const TimerHandle Timer::Record()
{
    return Instance.DoRecord();
}

double Timer::GetSpendTime()
{
    return Instance.DoGetSpendTime();
}

double Timer::GetLeftTime(double max)
{
    return Instance.DoGetLeftTime(max);
}
