#include "FeedBase.h"
#include "libmsxml.h"
#include "FeedCommon.h"

#include <stdexcept>

using namespace FeedCommon;

FeedBase::FeedBase()
{

}

FeedBase::~FeedBase()
{

}

std::wstring FeedBase::GetTitle() const
{
    return GetValue(L"title");
}

std::wstring FeedBase::GetLink() const
{
    return GetValue(L"link");
}

std::wstring FeedBase::GetDescription() const
{
    return GetValue(L"description");
}

std::wstring FeedBase::GetAuthor() const
{
    return GetValue(L"author");
}

std::wstring FeedBase::GetID() const
{
    return GetValue(L"id");
}

std::wstring FeedBase::GetUpdated() const
{
    return GetValue(L"updated");
}

void FeedBase::SetValue(const std::wstring& name, const std::wstring& value)
{
    m_feedInfo.SetValue(name, value);
}

std::wstring FeedBase::GetValue(const std::wstring& name) const 
{
    return m_feedInfo.GetValue(name);
}

void FeedBase::ParseFromFile(const std::wstring& file)
{
    WinMSXML xml;
    xml.Init();
    xml.Load(file);
    Parse(xml);
}

void FeedBase::ParseFromString(const std::wstring& xmlString)
{
    if (xmlString.empty())
        throw std::runtime_error("Error: XML string is empty.");

    WinMSXML xml;
    xml.Init();
    xml.LoadFromString(xmlString);
    Parse(xml);
}

void FeedBase::IterateFeeds(FN_ON_ITERATE_FEED onIterateFeed)
{
    if (!onIterateFeed)
        throw std::runtime_error("Error: invalid FN_ON_ITERATE_FEED callback function.");

    for (const auto& entry : m_feeds)
    {
        if (!onIterateFeed(entry))
            break;
    }
}

void FeedBase::ClearEntries()
{
    m_feeds.clear();
}
