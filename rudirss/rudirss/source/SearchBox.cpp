#include "SearchBox.h"
#include "resource.h"

#include <CommCtrl.h>

SearchBox::SearchBox() : m_width{ 0 }, m_height{ 0 }
{

}

SearchBox::~SearchBox()
{

}

void SearchBox::Initialize(int x, int y, int width, int height, HWND hWnd, HINSTANCE hInstance, HMENU searchBoxId)
{
    if (Attach(CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, nullptr, WS_CHILD | WS_VISIBLE, x, y, width, height,
        hWnd, searchBoxId, hInstance, nullptr)))
    {
        m_width = width;
        m_height = height;

        SendMessage(m_hWnd, EM_SETCUEBANNER, FALSE, (LPARAM)L"Search Title");
    }
}

void SearchBox::SetFont(HFONT font)
{
    if (m_hWnd)
        SendMessage(m_hWnd, WM_SETFONT, (WPARAM)font, TRUE);
}
