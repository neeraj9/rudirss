#pragma once

#include "FeedClient.h"
#include <functional>

class RudiRSSClient : public FeedClient
{
protected:
    using FN_ON_FEED_READY = std::function<void(const std::unique_ptr<Feed>& feed)>;
    FN_ON_FEED_READY m_fnOnFeedReady;

    virtual void OnFeedReady(const std::unique_ptr<Feed>& feed);

public:
    RudiRSSClient();
    virtual ~RudiRSSClient();

    bool Initialize(FN_ON_FEED_READY fnOnFeedReady);
};
