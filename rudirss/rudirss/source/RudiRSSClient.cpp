#include "RudiRSSClient.h"
#include "FeedBase.h"

RudiRSSClient::RudiRSSClient()
{
    m_feedLock.Init();
}

RudiRSSClient::~RudiRSSClient()
{

}

bool RudiRSSClient::Initialize(FN_ON_FEED_READY fnOnFeedReady)
{
    m_fnOnFeedReady = fnOnFeedReady;
    return FeedClient::Initialize();
}

void RudiRSSClient::OnFeedReady(const std::unique_ptr<Feed>& feed)
{
    if (!feed)
        return;

    FeedBase* feedBase = reinterpret_cast<FeedBase*>(feed.get());
    FeedContainer feedContainer;
    feedContainer.feedInfo = feedBase->GetFeedInfo();
    feedContainer.feedData = feedBase->GetFeedData();
    feedContainer.spec = feedBase->GetSpec();
    std::wstring title = feed->GetTitle();

    ATL::CComCritSecLock lock(m_feedLock);
    m_feedCollection.push_back(std::move(feedContainer));
    if (m_fnOnFeedReady)
        m_fnOnFeedReady(static_cast<LONG_PTR>(m_feedCollection.size() - 1), feed);
}

void RudiRSSClient::QueryFeedContainer(const LONG_PTR feedId, std::function<void(const FeedContainer*)> fnQuery)
{
    if (!fnQuery)
        return;

    size_t idx = static_cast<size_t>(feedId);
    ATL::CComCritSecLock lock(m_feedLock);
    fnQuery(idx < m_feedCollection.size() ? &m_feedCollection[idx]: nullptr);
}

void RudiRSSClient::QueryFeedData(const LONG_PTR feedId, const LONG_PTR feedDataId,
    std::function<void(const FeedContainer*, const FeedData*)> fnQuery)
{
    if (!fnQuery)
        return;

    QueryFeedContainer(feedId, [feedDataId, &fnQuery](const FeedContainer *feedContainer) {
        if (feedContainer)
        {
            size_t idx = static_cast<size_t>(feedDataId);
            fnQuery(feedContainer, idx < feedContainer->feedData.size() ? &feedContainer->feedData[idx]: nullptr);
        }
        });
}
