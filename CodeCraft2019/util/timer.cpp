#include "timer.h"
#include "stdlib.h"
#include "log.h"
#include "assert.h"

TimerHandle::TimerHandle(const clock_t& record)
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

double Timer::DoGetSpendTime(const clock_t& t) const
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

double Timer::DoGetLeftTime(const double& max) const
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

double Timer::GetLeftTime(const double& max)
{
    return Instance.DoGetLeftTime(max);
}

Timer::CostStatistic::CostStatistic()
    : UpdateCount(0), UpdateAverage(0), UpdateMax(0), UpdateMin(0), Handle(Timer::Record())
{ }

void Timer::DoUpdateTimeCostBegin(const std::string& id)
{
    m_statistic[id].Handle = Timer::Record();
}

void Timer::DoUpdateTimeCostEnd(const std::string& id)
{
    auto find = m_statistic.find(id);
    ASSERT(find != m_statistic.end());
    double time = find->second.Handle.GetSpendTime();
    if (find->second.UpdateCount == 0)
    {
        find->second.UpdateAverage = time;
        find->second.UpdateMax = time;
        find->second.UpdateMin = time;
    }
    else
    {
        find->second.UpdateAverage = find->second.UpdateAverage / (find->second.UpdateCount + 1) * find->second.UpdateCount + time / (find->second.UpdateCount + 1);
        if (time > find->second.UpdateMax) find->second.UpdateMax = time;
        if (time < find->second.UpdateMin) find->second.UpdateMin = time;
    }
    ++(find->second.UpdateCount);
}

void Timer::DoPrint() const
{
    if (LOG_IS_ENABLE)
    {
        for (auto ite = m_statistic.begin(); ite != m_statistic.end(); ++ite)
        {
            LOG("Scheduler statistic : " << ite->first);
            LOG("\t Update count : " << ite->second.UpdateCount);
            if (ite->second.UpdateCount > 0)
            {
                LOG("\t  Total  cost : " << ite->second.UpdateAverage * ite->second.UpdateCount * 1000 << " ms");
                LOG("\t Average cost : " << ite->second.UpdateAverage * 1000 << " ms");
                LOG("\t Minimum cost : " << ite->second.UpdateMin * 1000 << " ms");
                LOG("\t Maximum cost : " << ite->second.UpdateMax * 1000 << " ms");
            }
        }
    }
}

void Timer::UpdateTimeCostBegin(const std::string& id)
{
    Instance.DoUpdateTimeCostBegin(id);
}

void Timer::UpdateTimeCostEnd(const std::string& id)
{
    Instance.DoUpdateTimeCostEnd(id);
}

void Timer::Print()
{
    Instance.DoPrint();
}