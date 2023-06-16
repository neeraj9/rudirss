#pragma once

#include "FeedElement.h"
#include "FeedData.h"

#include <functional>

class Feed : public FeedElement
{
public:
    virtual ~Feed() = 0 {}

    virtual void ParseFromFile(const std::wstring &file) = 0;
    virtual void ParseFromString(const std::wstring &xmlString) = 0;

    using FN_ON_ITERATE_FEED = std::function<bool(const FeedData& feed)>;
    virtual void IterateFeeds(FN_ON_ITERATE_FEED onIterateFeed) = 0;
    virtual void ClearEntries() = 0;
};
