#include "FeedCommon.h"
#include "libmsxml.h"
#include "RSSFeed.h"
#include "AtomFeed.h"

#include <stdexcept>
#include <rpcdce.h>
#include <atltime.h>
#include <fstream>
#include <winhttp.h>

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

std::wstring FeedCommon::GetUUID()
{
    std::wstring sUUID;
    do
    {
        UUID uuid{};
        if (RPC_S_OK != UuidCreate(&uuid))
            break;

        RPC_WSTR uuidString{};
        if (RPC_S_OK != UuidToString(&uuid, &uuidString))
            break;

        sUUID.assign((wchar_t*)uuidString, wcslen((wchar_t*)uuidString));
        RpcStringFree(&uuidString);
    } while (0);

    return sUUID;
}

long long FeedCommon::ConvertDatetimeToTimestamp(FeedSpecification spec, const std::string& datetime)
{
    long long timestamp = 0;
    if (spec == FeedSpecification::RSS)
    {
        const std::map<std::string, int> months{ {"Jan", 1}, { "Feb", 2 }, { "Mar", 3 }, { "Apr", 4 }, { "May", 5 }, { "Jun", 6 },
            { "Jul", 7 }, { "Aug", 8 }, { "Sep", 9 }, { "Oct", 10 }, { "Nov", 11 }, { "Dec", 12 } };
        int year;
        char month[4]{};
        int day;
        int hour;
        int minute;
        int second;
        if (6 == sscanf_s(datetime.c_str(), "%*[^,], %d %s %d %d:%d:%d", &day, month, static_cast<unsigned int>(sizeof(month)),
            &year, &hour, &minute, &second))
        {
            auto it = months.find(month);
            if (it != months.end())
            {
                CTime tmpTimestamp(year, it->second, day, hour, minute, second);
                timestamp = tmpTimestamp.GetTime();
            }
        }
    }
    else if (spec == FeedSpecification::Atom)
    {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        if (6 == sscanf_s(datetime.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second))
        {
            CTime tmpTimestamp(year, month, day, hour, minute, second);
            timestamp = tmpTimestamp.GetTime();
        }
    }

    return timestamp;
}

bool FeedCommon::LoadFeedUrlsFromOPML(const std::wstring& opml, std::vector<std::wstring>& feedUrls)
{
    bool result = false;
    try
    {
        WinMSXML xml;
        xml.Init();
        xml.Load(opml);

        auto xmlDoc = xml.GetXMLDocument();
        CComBSTR xmlText;
        xmlDoc->get_xml(&xmlText);

        WinMSXML::XMLElement firstChild;
        if (FAILED(xmlDoc->get_firstChild(&firstChild)) || !firstChild)
            throw std::runtime_error("Error: unable to get first child of root.");

        WinMSXML::XMLElement opml;
        FeedCommon::IterateSiblingElements(firstChild, [&](const std::wstring_view& name, const std::wstring_view& value,
            const WinMSXML::XMLElement& element) -> bool {
                if (L"opml" == name)
                {
                    opml = element;
                    return false;
                }

                return true;
            });

        // To get all elements with xmlUrl attribute regarless their hierarchy
        CComBSTR query(L"//outline[@xmlUrl]");
        WinMSXML::XMLElementList elementList;
        if (FAILED(opml->selectNodes(query, &elementList)))
            throw std::runtime_error("Error: unable to get outline.");

        long length = 0;
        if (SUCCEEDED(elementList->get_length(&length)))
        {
            elementList->reset();
            for (long i = 0; i < length; i++)
            {
                WinMSXML::XMLElement element;
                if (SUCCEEDED(elementList->get_item(i, &element)));
                {
                    auto xmlUrl = xml.GetAttributeValue(element, L"xmlUrl");
                    feedUrls.push_back(xmlUrl);
                }
            }

            result = true;
        }

    }
    catch (const std::exception& e)
    {
    }

    return result;
}

bool FeedCommon::LoadFeedUrlsFromListFile(const std::wstring& listFile, std::vector<std::wstring>& feedUrls)
{
    std::wfstream f(listFile, std::iostream::in);
    if (f)
    {
        for (std::wstring feedUrl; std::getline(f, feedUrl);)
        {
            WCHAR host[BUFSIZ]{};
            URL_COMPONENTS urlComponents{};
            urlComponents.dwStructSize = sizeof(urlComponents);
            urlComponents.lpszHostName = host;
            urlComponents.dwHostNameLength = _countof(host);
            urlComponents.dwUrlPathLength = -1;
            if (WinHttpCrackUrl(feedUrl.c_str(), 0, 0, &urlComponents))
            {
                feedUrls.push_back(feedUrl);
            }
        }
    }

    return !feedUrls.empty();
}
