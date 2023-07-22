#pragma once

#include "WindowHandle.h"

#include <string>

class SearchBox : public WindowHandle
{
public:
    static const int DEFAULT_WIDTH = 300;
    static const int DEFAULT_HEIGHT = 20;

    SearchBox();
    virtual ~SearchBox();

    void Initialize(int x, int y, int width, int height, HWND hWnd, HINSTANCE hInstance, HMENU searchBoxId, const std::wstring &cueText);
    void SetFont(HFONT font);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

protected:
    int m_width;
    int m_height;
};
