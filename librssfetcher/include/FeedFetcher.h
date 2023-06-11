#pragma once

#include <string>

class FeedFetcher
{
public:
    virtual ~FeedFetcher() {}

    std::wstring GetFeedData(const std::wstring &feedURL);
};
