#include "pch.h"
#include "Timer.h"

TEST(TestTimerCase, TestPeriodTimer)
{
    Timer timer;

    int cnt = 0;
    timer.Create([](PVOID param, BOOLEAN timerOrWaitFired) {
        int *pCnt = reinterpret_cast<int*>(param);
        (*pCnt)++;
        }, &cnt, 0, 1000, WT_EXECUTEDEFAULT);

    Sleep(6000);
    timer.Delete();
    ASSERT_TRUE(cnt >= 5);
}