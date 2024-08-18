/*
 * Copyright 2003, 2004, 2005 Martin Fuchs
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


//
// Explorer clone
//
// quicklaunch.cpp
//
// Martin Fuchs, 22.08.2003
//


#include <precomp.h>

#include "../resource.h"

#include "quicklaunch.h"


QuickLaunchEntry::QuickLaunchEntry()
{
    _hbmp = 0;
}

QuickLaunchMap::~QuickLaunchMap()
{
    while (!empty()) {
        iterator it = begin();
        DeleteBitmap(it->second._hbmp);
        erase(it);
    }
}


QuickLaunchBar::QuickLaunchBar(HWND hwnd)
    :  super(hwnd)
{
    CONTEXT("QuickLaunchBar::QuickLaunchBar()");

    _dir = NULL;
    _next_id = IDC_FIRST_QUICK_ID;
    _btn_dist = 20;
    _size = 0;

    HWND hwndToolTip = (HWND) SendMessage(hwnd, TB_GETTOOLTIPS, 0, 0);

    SetWindowStyle(hwndToolTip, GetWindowStyle(hwndToolTip) | TTS_ALWAYSTIP);

    // delay refresh to some time later
    PostMessage(hwnd, PM_REFRESH, 0, 0);
}

QuickLaunchBar::~QuickLaunchBar()
{
    delete _dir;
}

HWND QuickLaunchBar::Create(HWND hwndParent)
{
    CONTEXT("QuickLaunchBar::Create()");

    ClientRect clnt(hwndParent);

    HWND hwnd = CreateToolbarEx(hwndParent,
                                WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
                                CCS_TOP | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE |
                                TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE | TBSTYLE_FLAT,
                                IDW_QUICKLAUNCHBAR, 0, 0, 0, NULL, 0, 0, 0, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, sizeof(TBBUTTON));

    if (hwnd) {
        new QuickLaunchBar(hwnd);
    }
    return hwnd;
}

void QuickLaunchBar::ReloadShortcuts()
{
    int cnt = 0;
    static ShellDirectory *shelldir = NULL;
    try {
        static TCHAR path[MAX_PATH];
        SpecialFolderFSPath app_data(CSIDL_APPDATA, _hwnd); ///@todo perhaps also look into CSIDL_COMMON_APPDATA ?
        _stprintf(path, TEXT("%s\\")QUICKLAUNCH_FOLDER, (LPCTSTR)app_data);
        RecursiveCreateDirectory(path);
        if (!shelldir) {
            shelldir = new ShellDirectory(GetDesktopFolder(), path, _hwnd);
        }
        shelldir->_scanned = false;
        shelldir->smart_scan(SORT_NAME, SCAN_DONT_EXTRACT_ICONS | SCAN_DONT_ACCESS | SCAN_NO_DIRECTORY);

        // immediatelly extract the shortcut icons
        for (Entry *entry = shelldir->_down; entry; entry = entry->_next) {
            if (entry->_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
                continue;
            if (lstrcmpi(entry->_display_name, TEXT("Shows Desktop")) == 0)
                continue;
            if (lstrcmpi(entry->_display_name, TEXT("Window Switcher")) == 0)
                continue;
            if (!(entry->_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                cnt++;
        }
    } catch (COMException &) {
        cnt = 0;
    }

    if (_entries.size() == cnt + 2) {
        return;
    }

    _next_id = IDC_FIRST_QUICK_ID;
    _entries.clear();
    int btns = (int)SendMessage(_hwnd, TB_BUTTONCOUNT, 0, 0);
    int i = 0;
    for (i = btns;i >= 0;i--) {
        SendMessage(_hwnd, TB_DELETEBUTTON, i, 0);
    }

    AddShortcuts();
}

void QuickLaunchBar::AddShortcuts()
{
    CONTEXT("QuickLaunchBar::AddShortcuts()");

    WaitCursor wait;

    try {
        TCHAR path[MAX_PATH];

        SpecialFolderFSPath app_data(CSIDL_APPDATA, _hwnd); ///@todo perhaps also look into CSIDL_COMMON_APPDATA ?

        _stprintf(path, TEXT("%s\\")QUICKLAUNCH_FOLDER, (LPCTSTR)app_data);

        RecursiveCreateDirectory(path);

        if (!_dir) {
            _dir = new ShellDirectory(GetDesktopFolder(), path, _hwnd);
        }
        _dir->_scanned = false;
        _dir->smart_scan(SORT_NAME, SCAN_DONT_ACCESS | SCAN_NO_DIRECTORY);

        // immediatelly extract the shortcut icons
        for (Entry *entry = _dir->_down; entry; entry = entry->_next)
            entry->_icon_id = entry->safe_extract_icon(ICF_LARGE | ICF_NOLINKOVERLAY);
    } catch (COMException &) {
        return;
    }


    ShellFolder desktop_folder;
    WindowCanvas canvas(_hwnd);

    COLORREF bk_color = TASKBAR_TEXTCOLOR();
    HBRUSH bk_brush = TASKBAR_BRUSH(); //GetSysColorBrush(COLOR_BTNFACE);

    AddButton(ID_MINIMIZE_ALL, g_Globals._icon_cache.get_icon(ICID_MINIMIZE).create_bitmap(bk_color, bk_brush, canvas, TASKBAR_ICON_SIZE), ResString(IDS_MINIMIZE_ALL), NULL);
    AddButton(ID_EXPLORE, g_Globals._icon_cache.get_icon(ICID_EXPLORER).create_bitmap(bk_color, bk_brush, canvas, TASKBAR_ICON_SIZE), ResString(IDS_TITLE), NULL);

    TBBUTTON sep = { 0, -1, TBSTATE_ENABLED, BTNS_SEP, { 0, 0 }, 0, 0 };
    SendMessage(_hwnd, TB_INSERTBUTTON, INT_MAX, (LPARAM)&sep);

    int ignore = 0;
    for (Entry *entry = _dir->_down; entry; entry = entry->_next) {
        // hide files like "desktop.ini"
        if (entry->_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
            continue;

        if (lstrcmpi(entry->_display_name, TEXT("Shows Desktop")) == 0) {
            ignore++;
            continue;
        }
        if (lstrcmpi(entry->_display_name, TEXT("Window Switcher")) == 0) {
            ignore++;
            continue;
        }
        // hide subfolders
        if (!(entry->_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            HBITMAP hbmp = g_Globals._icon_cache.get_icon(entry->_icon_id).create_bitmap(bk_color, bk_brush, canvas, TASKBAR_ICON_SIZE);

            AddButton(_next_id++, hbmp, entry->_display_name, entry);   //entry->_etype==ET_SHELL? desktop_folder.get_name(static_cast<ShellEntry*>(entry)->_pidl): entry->_display_name);
        }
    }

    _btn_dist = LOWORD(SendMessage(_hwnd, TB_GETBUTTONSIZE, 0, 0));
    _size = (int)(_entries.size() * _btn_dist + 5 + 3); // 3 for BTNS_SEP
    _size -= ignore;
    if (JCFG_TB(2, "userebar").ToBool() == TRUE) _size += 20;

    //adjust QuickLaunchBar width
    REBARBANDINFO rbBand;
    rbBand.cbSize = sizeof(REBARBANDINFO);
    rbBand.wID = IDW_QUICKLAUNCHBAR;
    rbBand.fMask = RBBIM_ID | RBBIM_SIZE;
    rbBand.cx = _size;
    SendMessage(GetParent(_hwnd), RB_SETBANDINFO, (WPARAM)0, (LPARAM)&rbBand);
    SendMessage(GetParent(_hwnd), PM_RESIZE_CHILDREN, 0, 0);
}

void QuickLaunchBar::AddButton(int id, HBITMAP hbmp, LPCTSTR name, Entry *entry, int flags)
{
    TBADDBITMAP ab = {0, (UINT_PTR)hbmp};
    int bmp_idx = (int)SendMessage(_hwnd, TB_ADDBITMAP, 1, (LPARAM)&ab);

    QuickLaunchEntry qle;

    qle._hbmp = hbmp;
    qle._title = name;
    qle._entry = entry;

    _entries[id] = qle;

    TBBUTTON btn = {0, 0, flags, BTNS_BUTTON | BTNS_NOPREFIX, {0, 0}, 0, 0};

    btn.idCommand = id;
    btn.iBitmap = bmp_idx;

    SendMessage(_hwnd, TB_INSERTBUTTON, INT_MAX, (LPARAM)&btn);
}

LRESULT QuickLaunchBar::WndProc(UINT nmsg, WPARAM wparam, LPARAM lparam)
{
    switch (nmsg) {
    case PM_REFRESH:
        AddShortcuts();
        break;
    case PM_RELOAD_BUTTONS: {
        ReloadShortcuts();
        break;
    }
    case PM_GET_WIDTH: {
        // take line wrapping into account
        int btns = (int)SendMessage(_hwnd, TB_BUTTONCOUNT, 0, 0);
        int rows = (int)SendMessage(_hwnd, TB_GETROWS, 0, 0);

        static int maxbtns = JCFG_QL(2, "maxiconsinrow").ToInt();
        if (maxbtns < 0) maxbtns = 0;
        if (maxbtns > 0 && maxbtns < 2) maxbtns = 2; // miniconsinrow = 2 Show Desktop & Explorer
        if (maxbtns == 0 || rows == btns) return _size;
        if (btns - 1 <= maxbtns) return _size; // BTNS_SEP

        RECT rect;
        int max_cx = 2 * _btn_dist + 5;

        for (QuickLaunchMap::const_iterator it = _entries.begin(); it != _entries.end(); ++it) {
            SendMessage(_hwnd, TB_GETRECT, it->first, (LPARAM)&rect);
            if (rect.right > max_cx) max_cx = rect.right;
        }

        if (maxbtns > 0) {
            int maxbtns_cx = maxbtns * _btn_dist + 5;  // no BTNS_SEP
            if ((btns - 2) > maxbtns || max_cx > maxbtns_cx) max_cx = maxbtns_cx;
        }
        return max_cx;
    }
    case WM_CONTEXTMENU: {
        TBBUTTON btn;
        QuickLaunchMap::iterator it;
        Point screen_pt(lparam), clnt_pt = screen_pt;
        ScreenToClient(_hwnd, &clnt_pt);

        Entry *entry = NULL;
        int idx = (int)SendMessage(_hwnd, TB_HITTEST, 0, (LPARAM)&clnt_pt);

        if (idx >= 0 &&
            SendMessage(_hwnd, TB_GETBUTTON, idx, (LPARAM)&btn) != -1 &&
            (it = _entries.find(btn.idCommand)) != _entries.end()) {
            entry = it->second._entry;
        }

        if (entry) {    // entry is NULL for desktop switch buttons
            HRESULT hr = entry->do_context_menu(_hwnd, screen_pt, _cm_ifs);

            if (!SUCCEEDED(hr))
                CHECKERROR(hr);
        } else
            goto def;
        break;
    }

default: def:
        return super::WndProc(nmsg, wparam, lparam);
    }

    return 0;
}

int QuickLaunchBar::Command(int id, int code)
{
    CONTEXT("QuickLaunchBar::Command()");

    if ((id & ~0xFF) == IDC_FIRST_QUICK_ID) {
        QuickLaunchEntry &qle = _entries[id];

        if (qle._entry) {
            qle._entry->launch_entry(_hwnd);
            return 0;
        }
    }

    return 0; // Don't return 1 to avoid recursion with DesktopBar::Command()
}

int QuickLaunchBar::Notify(int id, NMHDR *pnmh)
{
    switch (pnmh->code) {
    case TTN_GETDISPINFO: {
        NMTTDISPINFO *ttdi = (NMTTDISPINFO *) pnmh;

        int id = (int)ttdi->hdr.idFrom;
        ttdi->lpszText = _entries[id]._title.str();
#ifdef TTF_DI_SETITEM
        ttdi->uFlags |= TTF_DI_SETITEM;
#endif

        // enable multiline tooltips (break at CR/LF and for very long one-line strings)
        SendMessage(pnmh->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 400);

        break;
    }

    return super::Notify(id, pnmh);
    }

    return 0;
}
