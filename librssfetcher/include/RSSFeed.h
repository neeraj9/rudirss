#pragma once

#include "FeedBase.h"
#include "libmsxml.h"

#include <vector>
#include <string>

class RSSFeed : public FeedBase
{
protected:
    std::wstring m_version;

public:
    RSSFeed();
    virtual ~RSSFeed();

    virtual void Parse(WinMSXML &xml);
    const std::wstring GetVersion() const { return m_version; }
};
