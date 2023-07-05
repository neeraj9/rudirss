#pragma once

#include "FeedClient.h"
#include "FeedCommon.h"

#include <functional>
#include <vector>

class RudiRSSClient : public FeedClient
{
public:
    RudiRSSClient();
    virtual ~RudiRSSClient();

    struct FeedContainer
    {
        FeedData feedInfo;
        std::vector<FeedData> feedData;
        FeedCommon::FeedSpecification spec;
        FeedContainer() : spec{FeedCommon::FeedSpecification::None} {}
        FeedContainer(const FeedContainer& rhs) = delete;
        FeedContainer(FeedContainer&& rhs) noexcept: feedInfo(std::move(rhs.feedInfo)), feedData(std::move(rhs.feedData)),
            spec{ rhs.spec } {}
    };

    using FN_ON_FEED_READY = std::function<void(const LONG_PTR, const std::unique_ptr<Feed>&)>;
    bool Initialize(FN_ON_FEED_READY fnOnFeedReady);
    void QueryFeedContainer(const LONG_PTR feedId, std::function<void(const FeedContainer*)> fnQuery);
    void QueryFeedData(const LONG_PTR feedId, const LONG_PTR feedDataId, std::function<void(const FeedContainer*, const FeedData*)> fnQuery);

protected:
    FN_ON_FEED_READY m_fnOnFeedReady;

    std::vector<FeedContainer> m_feedCollection;
    ATL::CComCriticalSection m_feedLock;

    virtual void OnFeedReady(const std::unique_ptr<Feed>& feed);

};
