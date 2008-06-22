

#include "stdafx.h"
#include "debuglog.h"
#include <process.h>
#include <commctrl.h>

#if ENABLE_LOG_AT_ALL == 1

LogWindow::LogWindow() :
  m_hMain(0),
  m_hEdit(0),
  m_hThread(0),
  m_hInitialized(0),
  m_hTab(0)
{
}

LogWindow::~LogWindow()
{
}

inline std::wstring GetFullAppPathX(PCWSTR extra)
{
  WCHAR sz[MAX_PATH];
  WCHAR sz2[MAX_PATH];
  GetModuleFileNameW(0, sz, MAX_PATH);
  wcscpy(PathFindFileNameW(sz), extra);
  PathCanonicalizeW(sz2, sz);
  return sz2;
}

template<typename Char>
inline std::basic_string<Char> GetFullAppPathX(const Char* extra)
{
  std::wstring extraW;
  LibCC::StringCopy<wchar_t>(extraW, extra);
  std::wstring retW = GetFullAppPathX(extraW.c_str());
  return LibCC::StringCopy<Char>(retW);
}

inline std::basic_string<TCHAR> GetFullAppPath(PCTSTR extra)
{
  return GetFullAppPathX<TCHAR>(extra);
}

void LogWindow::Create(PCTSTR fileName, PCTSTR caption, HINSTANCE hInstance)
{
  m_caption = caption;
  m_fileName = GetFullAppPath(fileName);
  m_fileName.append(_T(".log"));
  m_hInstance = hInstance;
  m_hInitialized = CreateEvent(0, FALSE, FALSE, 0);
  m_hThread = (HANDLE)_beginthread(LogWindow::ThreadProc, 0, this);
  WaitForSingleObject(m_hInitialized, INFINITE);

  Message(_T("-------------------------------"));
  Message(LibCC::Format("Starting % log").qs(m_caption));

  CloseHandle(m_hInitialized);
  m_hInitialized = 0;
}

void LogWindow::Destroy()
{
  Message(LibCC::Format("Stopping % log").qs(m_caption));

  // wait.
  if(m_hThread)
  {
    PostMessage(m_hMain, WM_LogExit, 0, 0);
    WaitForSingleObject(m_hThread, INFINITE);
  }
}

void LogWindow::Indent()
{
  SendMessage(m_hMain, WM_Indent, 0, GetCurrentThreadId());
}

void LogWindow::Outdent()
{
  SendMessage(m_hMain, WM_Outdent, 0, GetCurrentThreadId());
}

void LogWindow::MessageBox_(const LibCC::Format& s, const TCHAR* caption, HWND hParent)
{
  MessageBox_(s.CStr(), caption, hParent);
}

void LogWindow::MessageBox_(const LibCC::Result& s, const TCHAR* caption, HWND hParent)
{
  MessageBox_(s.str().c_str(), caption, hParent);
}

void LogWindow::MessageBox_(const TCHAR* s, const TCHAR* caption, HWND hParent)
{
  Message(s);
  if(!caption)
  {
    caption = m_caption.c_str();
  }
  MessageBox(hParent, s, caption, MB_OK | MB_ICONASTERISK);
}

void LogWindow::Message(const std::basic_string<TCHAR>& s)
{
  Message(s.c_str());
}

void LogWindow::Message(const LibCC::Format& s)
{
  Message(s.CStr());
}

void LogWindow::Message(const LibCC::Result& s)
{
  Message(s.str());
}

/*
  the reason for s1 and s2 is for LogScopeMessage and indentation
*/
void LogWindow::_Message(const TCHAR* s1, const TCHAR* s2)
{
  if(m_hThread)
  {
    MessageInfo* pNew = new MessageInfo();
    GetLocalTime(&pNew->localTime);
    pNew->s1 = s1;
    pNew->s2 = s2;
    pNew->threadID = GetCurrentThreadId();

    SendMessage(m_hMain, WM_LogMessage, 0, reinterpret_cast<LPARAM>(pNew));// doesnt return until it's done.
    delete pNew;
  }
}

// marshals the string to the other thread, and makes sure it was delivered.
void LogWindow::Message(const TCHAR* s)
{
  _Message(_T(""), s);
}

void __cdecl LogWindow::ThreadProc(void* p)
{
  static_cast<LogWindow*>(p)->ThreadProc();
}

// this is *always* called by the gui thread.
LogWindow::ThreadInfo& LogWindow::GetThreadInfo(DWORD threadID)
{
  for(std::vector<ThreadInfo>::iterator it = m_threads.begin(); it != m_threads.end(); ++ it)
  {
    if(it->threadID == threadID)
    {
      return *it;
    }
  }

  // omg; not found.  create a new one.
  HWND hNewEdit = 0;
  int newTabItem = 0;
#if (ENABLE_LOG_WINDOW == 1)
  newTabItem = TabCtrl_GetItemCount(m_hTab);
  std::basic_string<TCHAR> text(LibCC::Format("%").ul(threadID).Str());
  TCITEM tci = {0};
  tci.mask = TCIF_TEXT;
  tci.pszText = const_cast<PTSTR>(text.c_str());// i hate these damn non-const structs
  TabCtrl_InsertItem(m_hTab, newTabItem, &tci);

  hNewEdit = CreateWindowEx(0, _T("EDIT"), _T(""),
    WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY | WS_CHILD,
    27, 74, 3, 2, m_hMain, 0, m_hInstance, 0);
  SendMessage(hNewEdit, WM_SETFONT, (WPARAM)m_hFont, TRUE);
  SendMessage(hNewEdit, EM_SETLIMITTEXT, (WPARAM)0, 0);
  PostMessage(m_hMain, WM_SIZE, 0, 0);
#endif
  m_threads.push_back(ThreadInfo());
  ThreadInfo& ret(m_threads.back());
  ret.tabItem = newTabItem;
  ret.threadID = threadID;
  ret.hEdit = hNewEdit;
  
  return ret;
}

void LogWindow::ThreadProc()
{
  // you *cannot* call Message() from this thread.  this thread needs to pump messages 24/7
  WNDCLASS wc = {0};
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = LogWindow::MainProc;
  wc.hInstance = m_hInstance;
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wc.lpszClassName =  m_fileName.c_str();
  RegisterClass(&wc);

  LogWindowPlacementHelper placement;

  m_hMain = CreateWindowEx(0, m_fileName.c_str(), PathFindFileName(m_fileName.c_str()), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW,
    placement.m_x, placement.m_y, placement.m_width, placement.m_height, 0, 0, m_hInstance, this);

#if (ENABLE_LOG_WINDOW == 1)
  ShowWindow(m_hMain, SW_SHOW);
#endif
  SetEvent(m_hInitialized);

  MSG msg;
  while(GetMessage(&msg, 0, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return;
}

LRESULT CALLBACK LogWindow::MainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  LogWindow* pThis = static_cast<LogWindow*>(GetProp(hWnd, _T("WMircPClass")));
  switch(uMsg)
  {
  case WM_CLOSE:
    return 0;
#if (ENABLE_LOG_WINDOW == 1)
  case WM_SIZE:
    {
      RECT rc;
      GetClientRect(hWnd, &rc);

      HDWP hdwp = BeginDeferWindowPos(static_cast<int>(pThis->m_threads.size() + 2));// + tabctrl + composite edit
      // tab ctrl
      DeferWindowPos(hdwp, pThis->m_hTab, 0, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
      TabCtrl_AdjustRect(pThis->m_hTab, FALSE, &rc);
      // all tab edit box
      DeferWindowPos(hdwp, pThis->m_hEdit, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
      for(std::vector<ThreadInfo>::const_iterator it = pThis->m_threads.begin(); it != pThis->m_threads.end(); ++ it)
      {
        // thread edit box
        DeferWindowPos(hdwp, it->hEdit, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
      }

      EndDeferWindowPos(hdwp);
      break;
    }
#endif
  case WM_DESTROY:
    {
      PostQuitMessage(0);
      return 0;
    }
  case WM_LogMessage:
    {
      MessageInfo& mi = *(MessageInfo*)lParam;
      ThreadInfo& ti = pThis->GetThreadInfo(mi.threadID);

      static const size_t IndentSize = 2;

      // convert all newline chars into something else
      for(std::basic_string<TCHAR>::iterator it = mi.s1.begin(); it != mi.s1.end(); ++ it)
      {
        if(*it == '\r') *it = '~';
        if(*it == '\n') *it = '~';
      }
      for(std::basic_string<TCHAR>::iterator it = mi.s2.begin(); it != mi.s2.end(); ++ it)
      {
        if(*it == '\r') *it = '~';
        if(*it == '\n') *it = '~';
      }

      // create indent string
      std::basic_string<TCHAR> indent;
      for(size_t i = 0; i < ti.indent * IndentSize; i ++)
      {
        indent.push_back(' ');
      }

      SYSTEMTIME st;
      GetLocalTime(&st);

      std::basic_string<char> file(LibCC::FormatA("[%-%-%;%:%:%][%] %%%|")
        .ul<10,4>(st.wYear)
        .ul<10,2>(st.wMonth)
        .ul<10,2>(st.wDay)
        .ul<10,2>(st.wHour)
        .ul<10,2>(st.wMinute)
        .ul<10,2>(st.wSecond)
        .ul<16,8,'0'>(mi.threadID)
        .s(indent)
        .s(mi.s1)
        .s(mi.s2)
        .Str());
      std::basic_string<TCHAR> gui(LibCC::Format("%%%|").s(indent).s(mi.s1).s(mi.s2).Str());

      // do ods
#if (ENABLE_LOG_ODSF == 1)
      OutputDebugStringA(file.c_str());
#endif

      // do file
#if (ENABLE_LOG_FILE == 1)
      HANDLE h = CreateFile(pThis->m_fileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0);
      if(h && h != INVALID_HANDLE_VALUE)
      {
        SetFilePointer(h, 0, 0, FILE_END);
        DWORD br;
        WriteFile(h, file.c_str(), (DWORD)file.size(), &br, 0);
        CloseHandle(h);
      }
#endif

      // do gui
#if (ENABLE_LOG_WINDOW == 1)
      int ndx = GetWindowTextLength(pThis->m_hEdit);
      SendMessage(pThis->m_hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
      SendMessage(pThis->m_hEdit, EM_REPLACESEL, 0, (LPARAM)gui.c_str());

      ndx = GetWindowTextLength(ti.hEdit);
      SendMessage(ti.hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
      SendMessage(ti.hEdit, EM_REPLACESEL, 0, (LPARAM)gui.c_str());
#endif
      return 0;
    }
  case WM_Indent:
    {
      ThreadInfo& ti = pThis->GetThreadInfo(static_cast<DWORD>(lParam));
      ti.indent ++;
      return 0;
    }
  case WM_Outdent:
    {
      ThreadInfo& ti = pThis->GetThreadInfo(static_cast<DWORD>(lParam));
      ti.indent --;
      return 0;
    }
  case WM_LogExit:
    {
      DestroyWindow(hWnd);
      return 0;
    }
  case WM_NOTIFY:
    {
#if (ENABLE_LOG_WINDOW == 1)
      NMHDR& h = *(NMHDR*)lParam;
      if(h.hwndFrom == pThis->m_hTab)
      {
        if(h.code == TCN_SELCHANGE)
        {
          int iItem = TabCtrl_GetCurSel(pThis->m_hTab);
          if(iItem == 0)
          {
            // show the ALL tab, hide all others.
            SetWindowPos(pThis->m_hEdit, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            for(std::vector<ThreadInfo>::const_iterator it = pThis->m_threads.begin(); it != pThis->m_threads.end(); ++ it)
            {
              ShowWindow(it->hEdit, SW_HIDE);
            }
          }
          else
          {
            // hide the ALL tab, hide all others, show the selected one.
            ShowWindow(pThis->m_hEdit, SW_HIDE);
            for(std::vector<ThreadInfo>::const_iterator it = pThis->m_threads.begin(); it != pThis->m_threads.end(); ++ it)
            {
              if(it->tabItem == iItem)
              {
                SetWindowPos(it->hEdit, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
              }
              else
              {
                ShowWindow(it->hEdit, SW_HIDE);
              }
            }
          }

          return 0;
        }
      }
#endif
      break;
    }
  case WM_CREATE:
    {
      CREATESTRUCT* pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
      pThis = static_cast<LogWindow*>(pcs->lpCreateParams);
      SetProp(hWnd, _T("WMircPClass"), static_cast<HANDLE>(pThis));
      pThis->m_hMain = hWnd;
#if (ENABLE_LOG_WINDOW == 1)
      HDC dc = GetDC(hWnd);
      pThis->m_hFont = CreateFont(-MulDiv(10, GetDeviceCaps(dc, LOGPIXELSY), 72),0,0,0,0,0,0,0,0,0,0,0,0,_T("Courier"));
      ReleaseDC(hWnd, dc);

      pThis->m_hTab = CreateWindowEx(0, WC_TABCONTROL, _T(""), WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE, 27, 74, 3, 2, hWnd, 0, pThis->m_hInstance, 0);
      SendMessage(pThis->m_hTab, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
      TabCtrl_SetItemSize(pThis->m_hTab, 16, 16);

      // create the first tab for the composite
      TCITEM tci = {0};
      tci.mask = TCIF_TEXT;
      tci.pszText = _T("All");
      TabCtrl_InsertItem(pThis->m_hTab, 0, &tci);

      pThis->m_hEdit = CreateWindowEx(0, _T("EDIT"), _T(""), WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY | WS_CHILD | WS_VISIBLE,
        27, 74, 3, 2, pThis->m_hMain, 0, pThis->m_hInstance, 0);

      SetWindowPos(pThis->m_hEdit, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

      SendMessage(pThis->m_hEdit, WM_SETFONT, (WPARAM)pThis->m_hFont, TRUE);
      SendMessage(pThis->m_hEdit, EM_SETLIMITTEXT, (WPARAM)0, 0);
#endif

      return 0;
    }
  case WM_PAINT:
    PAINTSTRUCT ps;
    BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
    return 0;
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#endif// ENABLE_LOG_AT_ALL


