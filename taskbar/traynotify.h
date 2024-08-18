/*
 * Copyright 2003, 2004 Martin Fuchs
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
// Explorer and Desktop clone
//
// traynotify.h
//
// Martin Fuchs, 22.08.2003
//


#define CLASSNAME_TRAYNOTIFY    TEXT("TrayNotifyWnd")
#define TITLE_TRAYNOTIFY        TEXT("")

#define CLASSNAME_CLOCKWINDOW   TEXT("TrayClockWClass")

#define CLASSNAME_SHOWDESKTOPBUTTONWINDOW   TEXT("TrayShowDesktopButtonWClass")

#define ICON_AUTOHIDE_SECONDS   300

#define PM_GETMODULEPATH_CB     (WM_APP+0x21)
#define PM_GET_NOTIFYAREA       (WM_APP+0x22)

#define X86_HWND INT32
#define X64_HWND INT64

#define X86_HICON INT32
#define X64_HICON INT64

#define CHAR_A CHAR
#define CHAR_W WCHAR

#define DECL_PF_NOTIFYICONDATA(pf,chrset)\
    typedef struct pf##_NOTIFYICONDATA##chrset {\
    ULONG32 cbSize;\
    pf##_HWND hWnd;\
    UINT32 uID;\
    UINT32 uFlags;\
    UINT32 uCallbackMessage;\
    pf##_HICON hIcon;\
    CHAR_##chrset   szTip[128];\
    ULONG32 dwState;\
    ULONG32 dwStateMask;\
    CHAR_##chrset   szInfo[256];\
    union {\
        UINT32  uTimeout;\
        UINT32  uVersion;  /* used with NIM_SETVERSION, values 0, 3 and 4 */\
    } DUMMYUNIONNAME;\
    CHAR_##chrset   szInfoTitle[64];\
    ULONG32 dwInfoFlags;\
    GUID guidItem;\
    pf##_HICON hBalloonIcon;\
} pf##_NOTIFYICONDATA##chrset;

DECL_PF_NOTIFYICONDATA(X86, A)
DECL_PF_NOTIFYICONDATA(X86, W)
DECL_PF_NOTIFYICONDATA(X64, A)
DECL_PF_NOTIFYICONDATA(X64, W)

/// NotifyIconIndex is used for maintaining the order of notification icons.
struct NotifyIconIndex {
    NotifyIconIndex(NOTIFYICONDATA *pnid);

    // sort operator
    friend bool operator<(const NotifyIconIndex &a, const NotifyIconIndex &b)
    {return a._hWnd < b._hWnd || (a._hWnd == b._hWnd && a._uID < b._uID);}

    HWND    _hWnd;
    UINT32  _uID;

protected:
    NotifyIconIndex();
};


enum NOTIFYICONMODE {
    NIM_SHOW, NIM_HIDE, NIM_AUTO
};

extern String string_from_mode(NOTIFYICONMODE mode);

/// configuration for the display mode of a notification icon
struct NotifyIconConfig {
    NotifyIconConfig() : _mode(NIM_AUTO) {}

    void    create_name();
    bool    match(const NotifyIconConfig &props) const;

    // properties used to identify a notification icon
    String  _tipText;
    String  _windowTitle;   // To look at the window title and at the window module path of the notify icon owner window
    String  _modulePath;    // to identify notification icons is an extension above XP's behaviour.
    // (XP seems to store icon image data in the registry instead.)
    NOTIFYICONMODE _mode;

    String  _name;          /// unique name used to identify the entry in the configuration file
};

/// list of NotifyIconConfig structures
typedef list<NotifyIconConfig> NotifyIconCfgList;

/// structure for maintaining informations about one notification icon
struct NotifyInfo : public NotifyIconIndex, public NotifyIconConfig {
    NotifyInfo();

    // sort operator
    friend bool operator<(const NotifyInfo &a, const NotifyInfo &b)
    {return a._idx < b._idx;}

    bool    modify(void *pnid);
    template<typename NID_T>
    bool    _modify(NID_T *pnid);

    bool    set_title();
    int     _idx;   // display index
    HICON   _hIcon;
    DWORD   _dwState;
    UINT    _uCallbackMessage;
    UINT    _version;
    GUID    _guid;
    DWORD   _lastChange;    // timer tick value of the last change
};

typedef map<NotifyIconIndex, NotifyInfo> NotifyIconMap;
typedef set<NotifyInfo> NotifyIconSet;

#ifdef USE_NOTIFYHOOK
struct NotifyHook {
    NotifyHook();
    ~NotifyHook();

    void    GetModulePath(HWND hwnd, HWND hwndCallback);
    bool    ModulePathCopyData(LPARAM lparam, HWND *phwnd, String &path);

protected:
    const UINT WM_GETMODULEPATH;
};
#endif

/// tray notification area aka "tray"
struct NotifyArea : public Window {
    typedef Window super;

    NotifyArea(HWND hwnd);
    ~NotifyArea();

    static HWND Create(HWND hwndParent);

    LRESULT ProcessTrayNotification(int notify_code, NOTIFYICONDATA *pnid);

protected:
    WindowHandle _hwndClock;
    int     _clock_width;

    WindowHandle _hwndShowDesktopBtn;
    int     _showdesktopbtn_width;

    ToolTip _tooltip;

#ifdef USE_NOTIFYHOOK
    NotifyHook _hook;
#endif

    bool    _show_hidden;
    bool    _hide_inactive;
    bool    _show_button;

    LRESULT Init(LPCREATESTRUCT pcs);
    LRESULT WndProc(UINT nmsg, WPARAM wparam, LPARAM lparam);
    int     Command(int id, int code);
    int     Notify(int id, NMHDR *pnmh);

    void    UpdateIcons();
    void    Paint();
    void    Refresh(bool do_refresh = false);
    void    CancelModes();

    NotifyIconSet::iterator IconHitTest(const POINT &pos);
    bool    DetermineHideState(NotifyInfo &entry);

    void    read_config();
    void    write_config();
    void    show_clock(bool flag);
    void    show_showdesktopbtn(bool flag);

    friend struct TrayNotifyDlg;

    NotifyIconCfgList _cfg;

    map<HWND, String> _window_modules;

    NotifyIconMap _icon_map;
    NotifyIconSet _sorted_icons;
    int     _next_idx;
    size_t  _last_icon_count;
};


struct NotifyIconDlgInfo : public NotifyIconConfig {
    typedef NotifyIconConfig super;

    NotifyIconDlgInfo(const NotifyInfo &info) : super(info), _lastChange(info._lastChange) {}
    NotifyIconDlgInfo(const NotifyIconConfig &cfg) : super(cfg), _lastChange(0) {}
    NotifyIconDlgInfo() : _lastChange(0) {}

    DWORD   _lastChange;
};

typedef map<int, NotifyIconDlgInfo> NotifyIconDlgInfoMap;


/// configuration dialog for notification icons
struct TrayNotifyDlg : public ResizeController<Dialog> {
    typedef ResizeController<Dialog> super;

    TrayNotifyDlg(HWND hwnd);
    ~TrayNotifyDlg();

protected:
    HWND    _tree_ctrl;
    HACCEL  _haccel;
    HIMAGELIST  _himl;
    NotifyArea *_pNotifyArea;
    NotifyIconDlgInfoMap _info;

    typedef pair<NOTIFYICONMODE, DWORD> IconStatePair;
    typedef map<NotifyIconIndex, IconStatePair> IconStateMap;

    NotifyIconCfgList _cfg_org;
    IconStateMap _icon_states_org;
    bool    _show_hidden_org;

    HTREEITEM _hitemCurrent;
    HTREEITEM _hitemCurrent_visible;
    HTREEITEM _hitemCurrent_hidden;
    HTREEITEM _hitemConfig;
    HTREEITEM _selectedItem;

    virtual LRESULT WndProc(UINT nmsg, WPARAM wparam, LPARAM lparam);
    virtual int Command(int id, int code);
    virtual int Notify(int id, NMHDR *pnmh);

    void    Refresh();
    void    InsertItem(HTREEITEM hparent, HTREEITEM after, const NotifyInfo &, HDC);
    void    InsertItem(HTREEITEM hparent, HTREEITEM after, const NotifyIconDlgInfo &, HDC, HICON, NOTIFYICONMODE);
    void    SetIconMode(NOTIFYICONMODE mode);
    void    RefreshProperties(const NotifyIconDlgInfo &entry);
};


/// window for displaying the time in the tray notification area
struct ClockWindow : public Window {
    typedef Window super;

    ClockWindow(HWND hwnd);

    static HWND Create(HWND hwndParent);

    void    TimerTick();

protected:
    LRESULT WndProc(UINT nmsg, WPARAM wparam, LPARAM lparam);
    int     Notify(int id, NMHDR *pnmh);

    bool    FormatTime();
    void    Paint();

    TCHAR   _time[32];
    ToolTip _tooltip;
};

#define SHOWDESKTOPBUTTON_WIDTH DPI_SX(5)
struct ShowDesktopButtonWindow : public Window {
    typedef Window super;
    ShowDesktopButtonWindow(HWND hwnd);
    static HWND Create(HWND hwndParent);
protected:
    LRESULT WndProc(UINT nmsg, WPARAM wparam, LPARAM lparam);
    void    Paint();
};
