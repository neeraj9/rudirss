#include "RudiRSSClient.h"

RudiRSSClient::RudiRSSClient()
{

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
    if (m_fnOnFeedReady)
        m_fnOnFeedReady(feed);
}


