#pragma once

#include "Worker.h"

class FeedWorker: public Worker
{
public:
    enum class Type
    {
        Feed,
        None,
    };

    FeedWorker();
    virtual ~FeedWorker();

    virtual BOOL Initialize(void* param);
    virtual void Terminate(void* param);
    virtual void Execute(Worker::RequestType requestType, void* param, OVERLAPPED* overlapped);
};
