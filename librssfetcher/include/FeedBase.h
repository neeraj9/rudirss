#pragma once

#include "Feed.h"
#include "FeedData.h"
#include "libmsxml.h"
#include "FeedCommon.h"

#include <vector>
#include <string>

using namespace FeedCommon;

class FeedBase : public Feed
{
protected:
    FeedData m_feedInfo;
    std::vector<FeedData> m_feeds;
    FeedSpecification m_spec;
    std::wstring m_feedUrl;

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

    virtual void Parse(WinMSXML& xml) = 0;

    void SetSpec(FeedSpecification spec) { m_spec = spec; }
    FeedSpecification GetSpec() const { return m_spec; }

    const std::vector<FeedData>& GetFeedData() const { return m_feeds; }
    const FeedData& GetFeedInfo() const { return m_feedInfo; }

    void SetFeedUrl(const std::wstring& feedUrl) { m_feedUrl = feedUrl; }
    const std::wstring GetFeedUrl() const { return m_feedUrl; }
};

