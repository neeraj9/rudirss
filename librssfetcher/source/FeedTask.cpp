#include "FeedTask.h"
#include "FeedCommon.h"
#include "FeedClient.h"
#include "FeedBase.h"

FeedTask::FeedTask(const char *rawFeedData, size_t size): m_rawFeedData(rawFeedData, size)
{
}

FeedTask::~FeedTask()
{

}

void FeedTask::DoTask(void* param, OVERLAPPED* overlapped)
{
    std::wstring wsRawFeedData;
    if (FeedCommon::ConvertStringToWideString(m_rawFeedData, wsRawFeedData))
    {
        try
        {
            auto feed = FeedCommon::CreateFeed(wsRawFeedData);
            if (feed)
            {
                auto feedBase = reinterpret_cast<FeedBase*>(feed.get());
                feedBase->SetFeedUrl(m_feedUrl);
                auto feedClient = reinterpret_cast<FeedClient*>(param);
                feedClient->OnFeedReady(feed);
            }
        }
        catch (const std::exception& e)
        {

        }
    }
}
