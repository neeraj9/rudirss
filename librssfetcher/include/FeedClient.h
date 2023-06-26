#pragma once

#include "FeedFetcher.h"
#include "FeedWorker.h"
#include "FeedTask.h"
#include "Feed.h"

#include <atlutil.h>
#include <atlcore.h>
#include <string>
#include <queue>
#include <memory>

class FeedClient
{
protected:
    FeedFetcher m_feedFetcher;
    CThreadPool<FeedWorker> m_threardPool;
    ATL::CComCriticalSection m_taskLock;
    std::queue<std::unique_ptr<FeedTask>> m_taskQueue;

public:
    FeedClient();
    virtual ~FeedClient();

    bool Initialize();
    void Shutdown();
    void ConsumeFeed(const std::wstring& feedUrl);

    void PushTask(std::unique_ptr<FeedTask> task);
    std::unique_ptr<FeedTask> PopTask();

    virtual void OnFeedReady(const std::unique_ptr<Feed> &feed) = 0;
};

