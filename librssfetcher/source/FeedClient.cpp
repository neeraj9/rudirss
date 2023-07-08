#include "FeedClient.h"
#include "FeedCommon.h"

FeedClient::FeedClient()
{
    m_taskLock.Init();
}

FeedClient::~FeedClient()
{
    Shutdown();
}

bool FeedClient::Initialize()
{
    bool result = false;
    if (SUCCEEDED(m_threardPool.Initialize(reinterpret_cast<void*>(this))))
    {
        try
        {
            m_feedFetcher.Initialize([&](const FetchUnit* fetchUnit, bool result, DWORD statusCode, const char* data, size_t size,
                const WINHTTP_ASYNC_RESULT* asyncResult) {
                    auto feedTask = FeedCommon::CreateFeedTask<FeedTask>(data, size);
                    feedTask->SetFeedUrl(fetchUnit->FeedUrl());
                    PushTask(std::move(feedTask));

                    // Pass worker type instead of passing task pointer here, and maintain task queue ourselves because IOCP's internal queue
                    // can't be controlled outside. The main goal of task queue is to easily know whether queue is empty and to prevent the operation from
                    // memory leak in IOCP queue when the thread pool is shutdown and some tasks still remain in IOCP queue.
                    m_threardPool.QueueRequest(static_cast<Worker::RequestType>(FeedWorker::Type::Feed));
                });

            m_feedFetcher.StartFetchRoutine();

            result = true;
        }
        catch (const std::exception& e)
        {
        }
    }

    return result;
}

void FeedClient::Shutdown()
{
    m_feedFetcher.StopFetchRoutine();
    m_threardPool.Shutdown(INFINITE);
}

void FeedClient::ConsumeFeed(const std::wstring& feedUrl)
{
    m_feedFetcher.PushFeedUrl(feedUrl);
}

void FeedClient::PushTask(std::unique_ptr<FeedTask> task)
{
    ATL::CComCritSecLock lock(m_taskLock);
    m_taskQueue.push(std::move(task));
}

std::unique_ptr<FeedTask> FeedClient::PopTask()
{
    ATL::CComCritSecLock lock(m_taskLock);
    if (m_taskQueue.empty())
        return {};

    auto task = std::move(m_taskQueue.front());
    m_taskQueue.pop();
    return task;
}
