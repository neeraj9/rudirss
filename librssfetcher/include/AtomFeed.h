#pragma once

#include "FeedBase.h"
#include "libmsxml.h"

#include <vector>
#include <string>

class AtomFeed : public FeedBase
{
public:
    AtomFeed();
    virtual ~AtomFeed();

    virtual void Parse(WinMSXML& xml);
};
