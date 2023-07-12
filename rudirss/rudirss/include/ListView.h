#pragma once

#include "WindowHandle.h"

#include <functional>
#include <CommCtrl.h>

class ListView : public WindowHandle
{
public:
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

protected:
    FN_PROCESS_MESSAGE m_fnProcessMessage;
};
