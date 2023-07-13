#pragma once

#include "WindowHandle.h"

#include <functional>
#include <CommCtrl.h>

class ListView : public WindowHandle
{
public:
    ListView() : m_width{ 0 }, m_height{ 0 } {}
    virtual ~ListView() {}

    using FN_PROCESS_MESSAGE = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;
    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

    static LPARAM GetLParamFromActivatedItem(LPNMITEMACTIVATE activateItem)
    {
        LVITEM lvItem{};
        lvItem.iItem = activateItem->iItem;
        lvItem.iSubItem = activateItem->iSubItem;
        lvItem.mask = LVIF_PARAM;
        SendMessage(activateItem->hdr.hwndFrom, LVM_GETITEM, 0, (LPARAM)&lvItem);
        return lvItem.lParam;
    }

    static LPARAM GetLParamFromSelectedItem(HWND hwnd, int selectedIndex)
    {
        LVITEM lvItem{};
        lvItem.iItem = selectedIndex;
        lvItem.iSubItem = 0;
        lvItem.mask = LVIF_PARAM;
        SendMessage(hwnd, LVM_GETITEM, 0, (LPARAM)&lvItem);
        return lvItem.lParam;
    }

    const int GetWidth() const { return m_width; }
    const int GetHeight() const { return m_height; }

protected:
    FN_PROCESS_MESSAGE m_fnProcessMessage;
    int m_width;
    int m_height;
};
