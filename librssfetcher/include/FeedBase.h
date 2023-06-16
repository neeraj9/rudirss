#pragma once

#include "Feed.h"
#include "FeedData.h"
#include "libmsxml.h"

#include <vector>
#include <string>

class FeedBase : public Feed
{
protected:
    FeedData m_feedInfo;
    std::vector<FeedData> m_feeds;

    virtual void Parse(WinMSXML& xml) = 0;

public:
    FeedBase();
    virtual ~FeedBase();

    virtual std::wstring GetTitle() const;
    virtual std::wstring GetLink() const;
    virtual std::wstring GetDescription() const;
    virtual std::wstring GetAuthor() const;
    virtual std::wstring GetID() const;
    virtual std::wstring GetUpdated() const;
    virtual void SetValue(const std::wstring& name, const std::wstring& value);
    virtual std::wstring GetValue(const std::wstring& name) const ;

    virtual void ParseFromFile(const std::wstring& file);
    virtual void ParseFromString(const std::wstring& xmlString);
    virtual void IterateFeeds(FN_ON_ITERATE_FEED onIterateFeed);
    virtual void ClearEntries();
};

