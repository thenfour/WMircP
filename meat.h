// the meat.

#pragma once

#include "mirc.h"
#include "DebugLog.h"
#include "options.h"
#include <mmsystem.h>

#pragma comment(lib, "Winmm.lib")


// this contains a CommandSpec, and does the runtime work on it.
class Command
{
public:
  Command(CriticalSection& cs, const CommandSpec& c) :
    m_spec(c),
    m_firstPlay(true),
    m_timerID(0),
    m_cs(cs)
  {
  }

  ~Command()
  {
    if(m_timerID)
    {
      timeKillEvent(m_timerID);
    }
  }

  void OnPlay(IWMPMedia* pMedia, const std::wstring& URL, const std::wstring& DDEServer)
  {
    if(!m_spec.Enabled())
    {
      g_pLog->Message(_T("Command is disabled; ignoring."));
      return;
    }

    mirc::MediaInfoWrapper mi(pMedia);

    if(m_spec.AllowFilter())
    {
      // filter.  only allow if it meets the criteria
      std::vector<FilterSpec> filters = m_spec.Filters();
      for(std::vector<FilterSpec>::iterator it = filters.begin(); it != filters.end(); ++ it)
      {
        std::wstring val = mi.GetAttribute(it->Attribute());
        if(FALSE == PathMatchSpec(val.c_str(), it->Matches().c_str()))
        {
          g_pLog->Message(LibCC::Format("Bailing because of filtering.  Attribute % is %, and does not meet your criteria: %")
            .qs(it->Attribute())
            .qs(val)
            .qs(it->Matches()));
          return;
        }
      }
    }

    m_currentURL = URL;
    m_currentCommands = m_spec.Commands();
    for(std::list<std::wstring>::iterator it = m_currentCommands.begin(); it != m_currentCommands.end(); ++ it)
    {
      bool ignore = false;
      if((it->size() > 2) && (it->substr(0,2) == L"--"))
      {
        ignore = true;
      }
      if(ignore)
      {
        g_pLog->Message(LibCC::Format("Ignoring commented command %").qs(*it));
      }
      else
      {
        g_pLog->Message(LibCC::Format("Transforming command %").qs(*it));
        *it = mirc::ProcessCommandString(mi, *it);
      }
    }
    // queue it up for processing.
    UINT ms = m_spec.Delay() * 1000;
    g_pLog->Message(LibCC::Format("Setting the timer for % milliseconds").ui(ms+1));
    m_timerID = timeSetEvent(ms+1,
      max(100,ms+1), DelayProc, (DWORD_PTR)this, TIME_ONESHOT | TIME_KILL_SYNCHRONOUS | TIME_CALLBACK_FUNCTION);
  }

  CommandSpec m_spec;

private:

  // called after the delay.
  static void CALLBACK DelayProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
  {
    Command& This(*((Command*)dwUser));
    CriticalSection::ScopeLock l(This.m_cs);

    g_pLog->Message(LibCC::Format("Timer proc for %").qs(This.m_spec.Name()));
    g_pLog->Indent();

    // skip it if it's happening too fast.
    if(This.m_spec.AllowInterval() && !This.m_firstPlay && (This.m_timer.GetTimeSinceLastTick() < This.m_spec.Interval()))
    {
      g_pLog->Message(_T("Minimum interval not set; bailing."));
      g_pLog->Outdent();
      return;
    }

    // no-repeat?
    if(!This.m_spec.AllowRepeats() && (This.m_currentURL == This.m_lastURL))
    {
      g_pLog->Message(_T("This is a repeat; bailing."));
      g_pLog->Outdent();
      return;
    }

    for(std::list<std::wstring>::iterator it = This.m_currentCommands.begin(); it != This.m_currentCommands.end(); ++ it)
    {
      g_pLog->Message(LibCC::Format("Sending mIRC command %").qs(*it));
      mirc::SendMircCommand(*it);
    }
    This.m_timer.Tick();
    This.m_firstPlay = false;
    This.m_lastURL = This.m_currentURL;

    g_pLog->Outdent();
  }

  CriticalSection& m_cs;

  MMRESULT m_timerID;// timeSetEvent() result.
  LibCC::Timer m_timer;// for minimum interval
  bool m_firstPlay;// first play we ignore the minimum interval.
  std::wstring m_lastURL;// for no-repeat

  std::list<std::wstring> m_currentCommands;// this will get picked up by the delay proc
  std::wstring m_currentURL;// this will get picked up by the delay proc
};

class Meat
{
public:
  Meat(CriticalSection& cs) :
    m_cs(cs)
  {
  }

  void Rebuild(const Options& o)
  {
    m_commands.clear();
    m_options = o;
    // for each command, create a new command object.
    std::vector<CommandSpec> cmd = m_options.Commands();
    for(std::vector<CommandSpec>::iterator it = cmd.begin(); it != cmd.end(); ++ it)
    {
      m_commands.push_back(Command(m_cs, *it));
    }
  }

  void OnPlay(IWMPMedia* pMedia, const std::wstring& URL)
  {
    if(!m_options.Enabled())
    {
      g_pLog->Message(_T("WMircP is disabled; Ignoring the event completely."));
    }
    else
    {
      g_pLog->Message(LibCC::Format("Starting to process % commands").i((int)m_commands.size()));
      for(std::list<Command>::iterator it = m_commands.begin(); it != m_commands.end(); ++ it)
      {
        g_pLog->Message(LibCC::Format("Processing %").qs(it->m_spec.Name()));
        g_pLog->Indent();
        it->OnPlay(pMedia, URL, m_options.DDEServer());
        g_pLog->Outdent();
      }
    }
  }

private:
  Options m_options;
  CriticalSection& m_cs;
  std::list<Command> m_commands;
};

