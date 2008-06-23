

#pragma once

#include "..\libcc\libcc\registry.hpp"
#include <map>
#include <list>
#include <string>
#include <algorithm>

#ifdef TARGET_FOOBAR
#	define APP_NAME L"foobarMircP"
#else
#	define APP_NAME L"WMircP"
#endif


class IOptionsID
{
public:
  virtual int IssueNewID() = 0;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////
class FilterSpec
{
public:
  FilterSpec(IOptionsID& parent)
  {
    m_id = parent.IssueNewID();
  }

  FilterSpec() :
    m_id(-1)
  {
  }

  FilterSpec(LibCC::RegistryKey& myk)
  {
    myk[L"m_id"].GetValue(m_id);
    myk[L"m_attribute"].GetValue(m_attribute);
    myk[L"m_matches"].GetValue(m_matches);
  }

  void Save(LibCC::RegistryKey& parent) const
  {
    LibCC::RegistryKey myk( parent.SubKey( LibCC::Format().i(m_id).Str() ) );
    myk.Create();
    myk[L"m_id"] = m_id;
    myk[L"m_attribute"] = m_attribute;
    myk[L"m_matches"] = m_matches;
  }

  int id() const { return m_id; }
  std::wstring Attribute() const { return m_attribute; }
  std::wstring Matches() const { return m_matches; }

  void Attribute(const std::wstring& s) { m_attribute = s; }
  void Matches(const std::wstring& s) { m_matches = s; }

private:
  int m_id;
  std::wstring m_attribute;
  std::wstring m_matches;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////
class CommandSpec :
  public IOptionsID
{
public:
  CommandSpec(IOptionsID& parent)
  {
    m_nextFilterID = 0;
    m_id = parent.IssueNewID();

    // defaults
    m_enabled = true;
    m_order = 0;
    m_name = L"Default";
    m_autoName = false;
    m_commands.push_back(L"/describe #aoeu is playing $(Author) - $(Title) [ $(Duration) / $(Composer) ]");
    m_allowRepeats = false;
    m_allowInterval = true;
    m_interval = 20 * 60;// 20 minutes
    m_delay = 5;
    m_allowFilter = false;
  }

  CommandSpec(LibCC::RegistryKey& myk)
  {
    myk[L"m_enabled"].GetValue(m_enabled);
    myk[L"m_nextFilterID"].GetValue(m_nextFilterID);
    myk[L"m_id"].GetValue(m_id);
    myk[L"m_order"].GetValue(m_order);
    myk[L"m_name"].GetValue(m_name);
    myk[L"m_autoName"].GetValue(m_autoName);
    myk[L"m_allowRepeats"].GetValue(m_allowRepeats);
    myk[L"m_allowInterval"].GetValue(m_allowInterval);
    myk[L"m_interval"].GetValue(m_interval);
    myk[L"m_delay"].GetValue(m_delay);
    myk[L"m_allowFilter"].GetValue(m_allowFilter);
    std::wstring cmd;
    myk[L"m_commands"].GetValue(cmd);
    LibCC::StringSplitByString(cmd, L"\r\n", std::back_inserter(m_commands));

    // now load filters
    for(LibCC::RegistryKeyW::SubKeyIterator it = myk.EnumSubKeysBegin(); it != myk.EnumSubKeysEnd(); ++ it)
    {
      AddOrUpdateFilter(FilterSpec(*it));
    }
  }

  CommandSpec() :
    m_id(-1)
  {
  }

  // this filter has its own child key, named after our ID.
  void Save(LibCC::RegistryKey& parent) const
  {
    LibCC::RegistryKey myk( parent.SubKey( LibCC::Format().i(m_id).Str() ) );
    myk.Create();
    myk[L"m_enabled"] = m_enabled;
    myk[L"m_nextFilterID"] = m_nextFilterID;
    myk[L"m_id"] = m_id;
    myk[L"m_order"] = m_order;
    myk[L"m_name"] = m_name;
    myk[L"m_autoName"] = m_autoName;
    myk[L"m_allowRepeats"] = m_allowRepeats;
    myk[L"m_allowInterval"] = m_allowInterval;
    myk[L"m_interval"] = m_interval;
    myk[L"m_delay"] = m_delay;
    myk[L"m_allowFilter"] = m_allowFilter;
    myk[L"m_commands"] = LibCC::StringJoin(m_commands.begin(), m_commands.end(), L"\r\n");
    // delete all subkeys.
    for(LibCC::RegistryKeyW::SubKeyIterator it = myk.EnumSubKeysBegin(); it != myk.EnumSubKeysEnd(); ++ it)
    {
      it->Delete();
    }
    // now save filters
    for(std::map<int, FilterSpec>::const_iterator it = m_filters.begin(); it != m_filters.end(); ++ it)
    {
      it->second.Save(myk);
    }
  }

  void GenerateName()
  {
    // #jazz;#pop / genre=jazz;pop / type=audio (disabled)

    // make a list of channels.
    std::list<std::wstring> chans;
    for(std::list<std::wstring>::iterator it = m_commands.begin(); it != m_commands.end(); ++ it)
    {
      std::wstring chan;
      if(ExtractChannelName(*it, chan))
      {
        chans.push_back(chan);
      }
    }

    std::wstring filterText;

    if(m_allowFilter)
    {
      // make a list of filters
      std::list<std::wstring> filters;
      for(std::map<int, FilterSpec>::iterator it = m_filters.begin(); it != m_filters.end(); ++ it)
      {
        filters.push_back(LibCC::Format("%=%").s(it->second.Attribute()).s(it->second.Matches()).Str());
      }
      filterText = LibCC::Format(" / %").s(LibCC::StringJoin(filters.begin(), filters.end(), L" / ")).Str();
    }

    m_name = LibCC::Format("%%%")
      .s(LibCC::StringJoin(chans.begin(), chans.end(), L";"))
      .s(filterText)
      .s(m_enabled ? L"" : L" (disabled)")
      .Str()
      ;
  }

  int id() const { return m_id; }
  bool Enabled() const { return m_enabled; }
  int Order() const { return m_order; }
  std::wstring Name() const { return m_name; }
  bool AutoName() const { return m_autoName; }
  bool AllowRepeats() const { return m_allowRepeats; }
  bool AllowInterval() const { return m_allowInterval; }
  int Interval() const { return m_interval; }
  int Delay() const { return m_delay; }
  bool AllowFilter() const { return m_allowFilter; }
  std::list<std::wstring> Commands() const { return m_commands; }

  void Enabled(bool e)
  {
    m_enabled = e;
    GenerateNameIfNeeded();
  }
  void Order(int i) { m_order = i; }
  void Name(const std::wstring& s) { m_name = s; }
  void AutoName(bool b)
  {
    m_autoName = b;
    GenerateNameIfNeeded();
  }
  void AllowRepeats(bool b) { m_allowRepeats = b; }
  void AllowInterval(bool b) { m_allowInterval = b; }
  void Interval(int i) { m_interval = i; }
  void Delay(int i) { m_delay = i; }
  void AllowFilter(bool b)
  {
    m_allowFilter = b;
    GenerateNameIfNeeded();
  }
  void Commands(std::list<std::wstring>& r)
  {
    m_commands = r;
    GenerateNameIfNeeded();
  }

  // m_filters
  std::vector<FilterSpec> Filters() const
  {
    std::vector<FilterSpec> ret;
    for(std::map<int, FilterSpec>::const_iterator it = m_filters.begin(); it != m_filters.end(); ++ it)
    {
      ret.push_back(it->second);
    }
    return ret;
  }
  void AddOrUpdateFilter(const FilterSpec& f)
  {
    m_filters[f.id()] = f;
    GenerateNameIfNeeded();
  }
  bool DeleteFilter(int id)
  {
    bool ret = m_filters.erase(id) != 0;
    GenerateNameIfNeeded();
    return ret;
  }
  bool GetFilterSpecByID(int id, FilterSpec& out)
  {
    std::map<int, FilterSpec>::const_iterator it = m_filters.find(id);
    if(it == m_filters.end())
    {
      return false;
    }
    out = it->second;
    return true;
  }

  int IssueNewID()
  {
    return m_nextFilterID ++;
  }

private:

  void GenerateNameIfNeeded()
  {
    if(m_autoName) GenerateName();
  }

  static bool ExtractChannelName(const std::wstring& in, std::wstring& out)
  {
    // find the first '#'
    std::wstring::size_type pound = in.find('#');
    if(pound == std::wstring::npos) return false;
    out = in.substr(pound);

    std::wstring::size_type ws = out.find_first_of(L" \t\n\r");
    if(ws == std::wstring::npos) return true;
    // we found whitespace; truncate
    out.resize(ws);
    return true;
  }

  int m_nextFilterID;

  int m_id;// unique identifier so it can be stored in the registry nicely.  i would use
           // m_name, but i don't want to have to guarantee that it's unique because:
           // 1) can be auto-generated, and
           // 2) can be edited.  id cannot be edited; saves some logic.
  bool m_enabled;
  int m_order;
  std::wstring m_name;
  bool m_autoName;
  std::list<std::wstring> m_commands;
  bool m_allowRepeats;
  bool m_allowInterval;
  int m_interval;
  int m_delay;
  bool m_allowFilter;
  std::map<int, FilterSpec> m_filters;
};

inline bool __CommandOrderCompare(const CommandSpec& lhs, const CommandSpec& rhs)
{
  return lhs.Order() < rhs.Order();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
class Options :
  public IOptionsID
{
public:
  Options()
  {
    m_nextCommandID = 0;
    // default values
    m_DDEServer = L"mIRC";
    AddOrUpdateCommand(CommandSpec(*this));// create one command with defaults
    m_showTrayIcon = true;
    m_enabled = true;
  }

  void Load()
  {
    LibCC::RegistryKey k(L"HKCU\\Software\\Carl Corcoran\\" APP_NAME);
    if(k.Exists())
    {
      k[L"m_nextCommandID"].GetValue(m_nextCommandID);
      k[L"m_DDEServer"].GetValue(m_DDEServer);
      k[L"m_showTrayIcon"].GetValue(m_showTrayIcon);
      k[L"m_enabled"].GetValue(m_enabled);
      for(LibCC::RegistryKeyW::SubKeyIterator it = k.EnumSubKeysBegin(); it != k.EnumSubKeysEnd(); ++ it)
      {
        AddOrUpdateCommand(CommandSpec(*it));
      }
    }
  }

  // this filter has its own child key, named after our ID.
  void Save() const
  {
    LibCC::RegistryKey k(L"HKCU\\Software\\Carl Corcoran\\" APP_NAME);
    k.Create();
    k[L"m_nextCommandID"] = m_nextCommandID;
    k[L"m_showTrayIcon"] = m_showTrayIcon;
    k[L"m_DDEServer"] = m_DDEServer;
    k[L"m_enabled"] = m_enabled;
    // delete all subkeys.
    for(LibCC::RegistryKeyW::SubKeyIterator it = k.EnumSubKeysBegin(); it != k.EnumSubKeysEnd(); ++ it)
    {
      it->Delete();
    }
    for(std::map<int, CommandSpec>::const_iterator it = m_commands.begin(); it != m_commands.end(); ++ it)
    {
      it->second.Save(k);
    }
  }

  bool Enabled() const { return m_enabled; }
  void Enabled(bool b) { m_enabled = b; }

  bool ShowTrayIcon() const { return m_showTrayIcon; }
  void ShowTrayIcon(bool b) { m_showTrayIcon = b; }

  std::wstring DDEServer() const { return m_DDEServer; }
  void DDEServer(const std::wstring& s) { m_DDEServer = s; }

  // m_commands
  std::vector<CommandSpec> Commands() const// returns them in the correct order.
  {
    std::vector<CommandSpec> ret;
    for(std::map<int, CommandSpec>::const_iterator it = m_commands.begin(); it != m_commands.end(); ++ it)
    {
      ret.push_back(it->second);
    }
    std::sort(ret.begin(), ret.end(), __CommandOrderCompare);
    return ret;
  }
  bool GetCommandFromID(int id, CommandSpec& out) const
  {
    std::map<int, CommandSpec>::const_iterator it = m_commands.find(id);
    if(it == m_commands.end())
    {
      return false;
    }
    out = it->second;
    return true;
  }
  void AddOrUpdateCommand(const CommandSpec& f)
  {
    m_commands[f.id()] = f;
  }
  bool DeleteCommand(int id)
  {
    return m_commands.erase(id) != 0;
  }
  bool MoveCommand(int id, bool up)
  {
    CommandSpec c;
    if(!GetCommandFromID(id, c))
    {
      return false;
    }
    // reconstruct all the order numbers by creating a temp container.
    std::vector<CommandSpec> sorted = Commands();
    bool bChanged = false;
    for(std::vector<CommandSpec>::iterator it = sorted.begin(); it != sorted.end(); ++ it)
    {
      std::vector<CommandSpec>::iterator next = (it + 1);
      if(next == sorted.end())
      {
        return false;
      }
      if(up)
      {
        if(next->id() == c.id())
        {
          // ok it = the one we are bumping down.  next = the one we are bumping up.  swap them.
          std::swap(*it, *next);
          bChanged = true;
          break;
        }
      }
      else
      {
        if(it->id() == c.id())
        {
          // ok it = the one we are bumping down.  next = the one we are bumping up.  swap them.
          std::swap(*it, *next);
          bChanged = true;
          break;
        }
      }
    }
    // now we have a vector of them sorted properly.  set their orders in our main container.
    int order = 0;
    for(std::vector<CommandSpec>::iterator it = sorted.begin(); it != sorted.end(); ++ it)
    {
      m_commands[it->id()].Order(order ++);
    }
    return bChanged;
  }

  int IssueNewID()
  {
    return m_nextCommandID ++;
  }

private:
  int m_nextCommandID;
  std::map<int, CommandSpec> m_commands;
  std::wstring m_DDEServer;
  bool m_showTrayIcon;
  bool m_enabled;
};

