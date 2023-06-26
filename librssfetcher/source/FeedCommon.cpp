#include "FeedCommon.h"
#include "libmsxml.h"
#include "RSSFeed.h"
#include "AtomFeed.h"

#include <stdexcept>

using namespace FeedCommon;

void FeedCommon::IterateSiblingElements(const WinMSXML::XMLElement& element, FN_ON_ITERATE_ELEMENT onIterateElement)
{
    if (!element)
        throw std::invalid_argument("Error: empty element.");

    if (!onIterateElement)
        throw std::invalid_argument("Error: invalid FN_ON_ITERATE_ELEMENT callback function.");

    HRESULT result = S_OK;
    WinMSXML::XMLElement elementIter = element;
    do
    {
        CComBSTR name;
        result = elementIter->get_baseName(&name);
        if (S_OK != result)
            break;

        CComBSTR text;
        result = elementIter->get_text(&text);
        if (S_OK != result)
            break;

        if (!onIterateElement(std::wstring_view(name), std::wstring_view(text), elementIter))
            break;

        WinMSXML::XMLElement tmp;
        result = elementIter->get_nextSibling(&tmp);
        if (S_OK != result)
            break;
        elementIter = tmp;
    } while (S_OK == result);
}

FeedSpecification FeedCommon::GetFeedSpecification(const WinMSXML &xml)
{
    auto xmlDoc = xml.GetXMLDocument();
    WinMSXML::XMLElement firstChild;
    if (FAILED(xmlDoc->get_firstChild(&firstChild)) || !firstChild)
        throw std::runtime_error("Error: unable to get first child of root.");

    WinMSXML::XMLElement feed;
    FeedSpecification spec = FeedSpecification::None;
    IterateSiblingElements(firstChild, [&](const std::wstring_view& name, const std::wstring_view& value,
        const WinMSXML::XMLElement& element) -> bool {
            if (L"rss" == name)
            {
                spec = FeedSpecification::RSS;
                return false;
            }
            else if (L"feed" == name)
            {
                spec = FeedSpecification::Atom;
                return false;
            }

            return true;
        });

    return spec;
}

bool FeedCommon::Initialize()
{
    return SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
}

void FeedCommon::Uninitialize()
{
    CoUninitialize();
}

bool FeedCommon::ConvertWideStringToString(const std::wstring& ws, std::string& s)
{
    int len;

    len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.length(), nullptr, 0, nullptr, FALSE);
    if (0 == len)
        return false;

    std::string tmp;
    tmp.resize(len);
    if (0 == WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.length(), tmp.data(), len, nullptr, FALSE))
        return false;

    s = std::move(tmp);

    return true;
}

bool FeedCommon::ConvertStringToWideString(const std::string& s, std::wstring& ws)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), nullptr, 0);
    if (0 == len )
        return false;

    std::wstring tmp;
    tmp.resize(len);
    if (0 == MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), tmp.data(), len))
        return false;

    ws = std::move(tmp);

    return true;
}


std::unique_ptr<Feed> FeedCommon::CreateFeed(const std::wstring& xmlString)
{
    if (xmlString.empty())
        throw std::invalid_argument("Error: buffer is empty.");

    std::unique_ptr<Feed> feed;
    try
    {
        WinMSXML xml;
        xml.Init();
        xml.LoadFromString(xmlString);

        auto spec = GetFeedSpecification(xml);
        switch (spec)
        {
        case FeedSpecification::RSS:
        {
            feed = std::make_unique<RSSFeed>();
            FeedBase* feedBase = reinterpret_cast<FeedBase*>(feed.get());
            feedBase->SetSpec(FeedSpecification::RSS);
            feedBase->Parse(xml);
            break;
        }

        case FeedSpecification::Atom:
        {
            feed = std::make_unique<AtomFeed>();
            FeedBase* feedBase = reinterpret_cast<FeedBase*>(feed.get());
            feedBase->SetSpec(FeedSpecification::Atom);
            feedBase->Parse(xml);
            break;
        }

        default:
            break;
        }
    }
    catch (const std::exception &e)
    {
        throw e;
    }

    return feed;
}

#if 0
template<typename T>
std::unique_ptr<T> FeedCommon::CreateFeedTask(const char *rawFeedData, size_t size)
{
    if (!rawFeedData || 0 == size)
        return {};

    return std::make_unique<T>(rawFeedData, size);
}

template<typename T>
void FeedCommon::DestroyFeedTask(std::unique_ptr<T>& feedTask)
{
    feedTask.reset();
}
#endif
