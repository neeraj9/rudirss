#include "AtomFeed.h"
#include "libmsxml.h"
#include "FeedCommon.h"

#include <stdexcept>

using namespace FeedCommon;

AtomFeed::AtomFeed()
{

}

AtomFeed::~AtomFeed()
{

}

void AtomFeed::Parse(WinMSXML& xml)
{
    auto xmlDoc = xml.GetXMLDocument();
    WinMSXML::XMLElement firstChild;
    if (FAILED(xmlDoc->get_firstChild(&firstChild)) || !firstChild)
        throw std::runtime_error("Error: unable to get first child of root.");

    WinMSXML::XMLElement feed;
    IterateSiblingElements(firstChild, [&](const std::wstring_view& name, const std::wstring_view& value,
        const WinMSXML::XMLElement& element) -> bool {
            if (L"feed" == name)
            {
                feed = element;
                return false;
            }

            return true;
        });

    if (!feed)
        throw std::runtime_error("Error: unable to get feed element.");

    WinMSXML::XMLElement feedFirstChild;
    if (FAILED(feed->get_firstChild(&feedFirstChild)) || !feedFirstChild)
        throw std::runtime_error("Error: unable to get first child of rss.");

    IterateSiblingElements(feedFirstChild, [&](const std::wstring_view& name, const std::wstring_view& value,
        const WinMSXML::XMLElement& element) -> bool {
            if (L"entry" != name)
            {
                SetValue(std::wstring(name), std::wstring(value));
            }
            else
            {
                WinMSXML::XMLElement entryFirstChild;
                if (SUCCEEDED(element->get_firstChild(&entryFirstChild)))
                {
                    FeedData feedData;
                    IterateSiblingElements(entryFirstChild, [&](const std::wstring_view& name, const std::wstring_view& value,
                        const WinMSXML::XMLElement& elementIter) -> bool {
                            if (L"link" == name)
                            {
                                auto link = xml.GetAttributeValue(elementIter, L"href");
                                feedData.SetValue(std::wstring(name), link);
                            }
                            else
                            {
                                feedData.SetValue(std::wstring(name), std::wstring(value));
                            }
                            return true;
                        });

                    m_feeds.push_back(std::move(feedData));
                }
            }

            return true;
        });
}
