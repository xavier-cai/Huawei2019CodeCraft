#ifndef RUN_FRAMEWORK_H
#define RUN_FRAMEWORK_H

#include "scheduler.h"

/* for find a best answer */
class RunFramework
{
public:
    RunFramework();
    void Initialize();
    bool HandleTerminateAssert();
    void Run(int argc, char* argv[]);
    void RunImpl(Scheduler* scheduler);

    //implements
    void RunStableVersion();
    void RunFindABetterAnswer();

private:
    bool IsNoMoreTime() const;

    double m_maxTime;
    double m_safeInterval;
    int m_bestAnswer;

    //variables of implements
    bool m_isStableOutputed;
};

#endif