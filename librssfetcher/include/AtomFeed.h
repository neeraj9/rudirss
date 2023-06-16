#pragma once

#include "FeedBase.h"
#include "libmsxml.h"

#include <vector>
#include <string>

class AtomFeed : public FeedBase
{
protected:
    virtual void Parse(WinMSXML& xml);

public:
    AtomFeed();
    virtual ~AtomFeed();
};
