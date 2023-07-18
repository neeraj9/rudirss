#pragma once

#include "libmsxml.h"
#include "Feed.h"
#include "FeedTask.h"

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace FeedCommon
{
    enum class FeedSpecification
    {
        RSS,
        Atom,
        None
    };

    using FN_ON_ITERATE_ELEMENT = std::function<bool(const std::wstring_view&, const std::wstring_view&,
        const WinMSXML::XMLElement& element)>;

    void IterateSiblingElements(const WinMSXML::XMLElement& element, FN_ON_ITERATE_ELEMENT onIterateElement);
    FeedSpecification GetFeedSpecification(const WinMSXML &xml);

    bool Initialize();
    void Uninitialize();

    bool ConvertWideStringToString(const std::wstring& ws, std::string &s);
    bool ConvertStringToWideString(const std::string& s, std::wstring &ws);

    std::unique_ptr<Feed> CreateFeed(const std::wstring &xmlString);

    template<typename T>
    std::unique_ptr<T> CreateFeedTask(const char* rawFeedData, size_t size)
    {
        if (!rawFeedData || 0 == size)
            return {};

        return std::make_unique<T>(rawFeedData, size);
    }

    template<typename T>
    void DestroyFeedTask(std::unique_ptr<T>& feedTask)
    {
        feedTask.reset();
    }

    std::wstring GetUUID();

    long long ConvertDatetimeToTimestamp(FeedSpecification spec, const std::string& datetime);

    bool LoadFeedUrlsFromOPML(const std::wstring &opml, std::vector<std::wstring> &feedUrls);
    bool LoadFeedUrlsFromListFile(const std::wstring &listFile, std::vector<std::wstring> &feedUrls);

    bool CopyToClipboard(const std::wstring& data);
};
