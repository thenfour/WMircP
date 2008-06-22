

#pragma once


#include <libcc\ccstr.h>
#include <libcc\result.h>
#include <vector>


#ifdef _DEBUG
# define ENABLE_LOG_FILE 1// allow logging to file
# define ENABLE_LOG_ODSF 1// allow OutputDebugString()
# define ENABLE_LOG_WINDOW 1
#else
# define ENABLE_LOG_FILE 1// allow logging to file
# define ENABLE_LOG_ODSF 0// allow OutputDebugString()
# define ENABLE_LOG_WINDOW 0
#endif

// if any logging features are requested, then use the real log class.
#if (ENABLE_LOG_WINDOW == 1) || (ENABLE_LOG_FILE == 1) || (ENABLE_LOG_ODSF == 1)
# define ENABLE_LOG_AT_ALL 1
#else
# define ENABLE_LOG_AT_ALL 0
#endif


#if ENABLE_LOG_AT_ALL != 1
class LogWindow
{
public:
  template<typename T>
  inline void Message(const T&) { }
  inline void MessageBox_(const TCHAR* s, const TCHAR* caption = 0, HWND hParent = 0)
  {
    if(!caption)
    {
      caption = m_caption.c_str();
    }
    MessageBox(hParent, s, caption, MB_OK | MB_ICONASTERISK);
  }
  inline void MessageBox_(const LibCC::Format& s, const TCHAR* caption = 0, HWND hParent = 0)
  {
    MessageBox_(s.CStr(), caption, hParent);
  }
  inline void MessageBox_(const LibCC::Result& s, const TCHAR* caption = 0, HWND hParent = 0)
  {
    MessageBox_(s.str().c_str(), caption, hParent);
  }
  inline void Create(PCTSTR fileName, PCTSTR caption, HINSTANCE hInstance)
  {
    m_caption = caption;
  }
  inline void Destroy() { }

private:
  _tstring m_caption;
};

extern LogWindow* g_pLog;

class LogScopeMessage
{
public:
  LogScopeMessage() { }
  template<typename T>
  LogScopeMessage(const T&) { }
};

#else

// use this class to help find the right coords for a debug log window.
class LogWindowPlacementHelper
{
public:
  static const DWORD m_width = 300;
  static const DWORD m_height = 300;
  DWORD m_x;
  DWORD m_y;

  static const TCHAR* GetSemaphoreName()
  {
    return _T("WMircPLogWindowCount");
  }

  LogWindowPlacementHelper()
  {
    // determine placement.
    m_hGlobalSemaphore = CreateSemaphore(0, 0, 1000, GetSemaphoreName());
    // increase ref count and get the previous count.
    LONG i;
    ReleaseSemaphore(m_hGlobalSemaphore, 1, &i);
    int screenColumns = GetSystemMetrics(SM_CXSCREEN) / m_width;
    m_x = m_width * (i % screenColumns);
    m_y = m_height * (i / screenColumns);
  }

  ~LogWindowPlacementHelper()
  {
    CloseHandle(m_hGlobalSemaphore);
  }

  HANDLE m_hGlobalSemaphore;
};

// this class "owns" a thread that does all the UI.
class LogWindow
{
public:
  LogWindow();
  ~LogWindow();

  void Create(PCTSTR fileName, PCTSTR caption, HINSTANCE hInstance);
  void Destroy();

  void Indent();
  void Outdent();

  // marshals the string to the other thread
  void Message(const std::basic_string<TCHAR>& s);
  void Message(const LibCC::Format& s);
  void Message(const LibCC::Result& s);
  void Message(const TCHAR* s);

  // if caption = 0, then we just use the internal caption
  void MessageBox_(const LibCC::Format& s, const TCHAR* caption = 0, HWND hParent = 0);
  void MessageBox_(const LibCC::Result& s, const TCHAR* caption = 0, HWND hParent = 0);
  void MessageBox_(const TCHAR* s, const TCHAR* caption = 0, HWND hParent = 0);

  // just a convenience function that appends 2 strings together
  void _Message(const TCHAR* s1, const TCHAR* s2);

private:
  static void __cdecl ThreadProc(void*);
  void ThreadProc();
  static LRESULT CALLBACK MainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  static const UINT WM_LogMessage = WM_APP;
  static const UINT WM_LogExit = WM_APP + 2;
  static const UINT WM_Indent = WM_APP + 3;
  static const UINT WM_Outdent = WM_APP + 4;

  // each thread gets a different edit, tab item, and id.
  struct ThreadInfo
  {
    ThreadInfo() :
      hEdit(0),
      threadID(0),
      tabItem(0),
      indent(0)
    {
    }
    ThreadInfo(const ThreadInfo& rhs) 
    {
      operator =(rhs);
    }
    ThreadInfo& operator =(const ThreadInfo& rhs) 
    {
      hEdit = rhs.hEdit;
      threadID = rhs.threadID;
      tabItem = rhs.tabItem;
      indent = rhs.indent;
      return *this;
    }
    HWND hEdit;
    DWORD threadID;
    int tabItem;
    size_t indent;
  };

  ThreadInfo& GetThreadInfo(DWORD threadID);

  struct MessageInfo
  {
    std::basic_string<TCHAR> s1;
    std::basic_string<TCHAR> s2;
    DWORD threadID;
    SYSTEMTIME localTime;
  };

  HANDLE m_hInitialized;
  HANDLE m_hThread;
  HWND m_hMain;
  HWND m_hEdit;// composite of all threads.
  HFONT m_hFont;

  HWND m_hTab;
  std::vector<ThreadInfo> m_threads;

  HINSTANCE m_hInstance;
  std::basic_string<TCHAR> m_fileName;
  std::basic_string<TCHAR> m_caption;
};

extern LogWindow* g_pLog;

class LogScopeMessage
{
public:
  LogScopeMessage(const std::basic_string<TCHAR>& op, LogWindow* pLog = g_pLog) :
    m_pLog(pLog)
  {
    m_pLog->_Message(_T("{ "), op.c_str());
    m_pLog->Indent();
  }
  LogScopeMessage(const LibCC::Format& op, LogWindow* pLog = g_pLog) :
    m_pLog(pLog)
  {
    m_pLog->_Message(_T("{ "), op.CStr());
    m_pLog->Indent();
  }
  LogScopeMessage(const TCHAR* op, LogWindow* pLog = g_pLog) :
    m_pLog(pLog)
  {
    m_pLog->_Message(_T("{ "), op);
    m_pLog->Indent();
  }
  ~LogScopeMessage()
  {
    if(m_pLog)
    {
      m_pLog->Outdent();
      m_pLog->Message(_T("}"));
    }
  }

  LogWindow* m_pLog;
};
#endif// ENABLE_LOG_AT_ALL
