#include "FeedWorker.h"
#include "FeedTask.h"
#include "FeedCommon.h"
#include "FeedClient.h"

FeedWorker::FeedWorker()
{

}

FeedWorker::~FeedWorker()
{

}

BOOL FeedWorker::Initialize(void* param)
{
    return TRUE;
}

void FeedWorker::Terminate(void* param)
{

}

void FeedWorker::Execute(Worker::RequestType requestType, void* param, OVERLAPPED* overlapped)
{
    auto feedClient = reinterpret_cast<FeedClient*>(param);
    auto task = feedClient->PopTask();
    if (task)
        task->DoTask(param, overlapped);
}
