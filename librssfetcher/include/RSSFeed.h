#pragma once

#include "FeedBase.h"
#include "libmsxml.h"

#include <vector>
#include <string>

class RSSFeed : public FeedBase
{
protected:
    std::wstring m_version;

    virtual void Parse(WinMSXML &xml);

public:
    RSSFeed();
    virtual ~RSSFeed();

    const std::wstring GetVersion() const { return m_version; }
};
