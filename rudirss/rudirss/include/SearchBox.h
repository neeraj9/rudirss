#pragma once

#include "WindowHandle.h"

#include <string>

class SearchBox : public WindowHandle
{
public:
    static const int DEFAULT_WIDTH = 300;
    static const int DEFAULT_HEIGHT = 24;

    enum class SearchType
    {
        SOURCE_FEEDS,
        FEED_ITEMS,
    };

    SearchBox();
    virtual ~SearchBox();

    void Initialize(int x, int y, int width, int height, HWND hWnd, HINSTANCE hInstance, HMENU searchBoxId, const std::wstring &cueText);
    void SetFont(HFONT font);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    std::string GetSearchText();

protected:
    int m_width;
    int m_height;
    std::string m_lastSearch;
};
