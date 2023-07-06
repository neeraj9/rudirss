#include "pch.h"
#include "Timer.h"

TEST(TestTimerCase, TestPeriodTimer)
{
    Timer timerPrint;

    int cnt = 0;
    timerPrint.Create([](PVOID param, BOOLEAN timerOrWaitFired) {
        int *pCnt = reinterpret_cast<int*>(param);
        (*pCnt)++;
        }, &cnt, 0, 1000, WT_EXECUTEDEFAULT);

    Sleep(6000);
    timerPrint.Delete();
    ASSERT_TRUE(cnt >= 5);
}