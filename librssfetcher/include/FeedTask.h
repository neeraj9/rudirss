#pragma once

#include "Task.h"
#include <string>

class FeedTask: public Task
{
protected:
    std::string m_rawFeedData;

public:
    FeedTask() = delete;
    FeedTask(const char *rawFeedData, size_t size);
    virtual ~FeedTask();

    virtual void DoTask(void* param, OVERLAPPED* overlapped);
};
