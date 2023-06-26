#include "pch.h"

#include "Task.h"
#include "Worker.h"

#include <atlutil.h>

class TestTask: public Task
{
protected:
    int m_num;

public:
    TestTask() :m_num{ 0 } {}
    explicit TestTask(int num) : m_num{ num } {}
    virtual ~TestTask() {}

    virtual void DoTask(void* param, OVERLAPPED* overlapped)
    {
        printf("Number: %d\n", m_num);
    }
};

class TestWorker: public Worker
{
protected:
    virtual void OnComplete(Worker::RequestType requestType)
    {
        auto task = reinterpret_cast<Task*>(requestType);
        delete task;
    }

public:
    TestWorker() {}
    virtual ~TestWorker() {}

    virtual BOOL Initialize(void* param) { return TRUE; }
    virtual void Terminate(void* param) {}
    virtual void Execute(RequestType requestType, void* param, OVERLAPPED* overlapped)
    {
        auto task = reinterpret_cast<Task*>(requestType);
        task->DoTask(param, overlapped);
        OnComplete(requestType);
    }
};

TEST(TestWorkerCase, TestDoTask)
{
    CThreadPool<TestWorker> testWorker;
    if (SUCCEEDED(testWorker.Initialize(nullptr)))
    {
        for (int i = 0; i < 100; i++)
        {
            testWorker.QueueRequest(reinterpret_cast<Worker::RequestType>(new TestTask(i)));
        }

        testWorker.Shutdown(1000);
    }
}