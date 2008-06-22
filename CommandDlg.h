

#include "atlwin.h"
#include "options.h"

class CommandDlg :
  public CDialogImpl<CommandDlg>
{
public:
  enum { IDD = IDD_COMMAND };

  void Check(UINT id, bool b)
  {
    CheckDlgButton(id, b ? BST_CHECKED : BST_UNCHECKED);
  }
  bool IsChecked(UINT id)
  {
    return IsDlgButtonChecked(id) == BST_CHECKED;
  }
  void Enable(UINT id, bool b)
  {
    ::EnableWindow(GetDlgItem(id), b ? TRUE : FALSE);
  }
  std::wstring GetDlgItemText(UINT id)
  {
    HWND h = GetDlgItem(id);
    DWORD len = ::GetWindowTextLength(h) + 1;
    PWSTR sz = new wchar_t[len + 2];
    ::GetWindowText(h, sz, len);
    std::wstring out(sz);
    delete [] sz;
    return out;
  }

  BEGIN_MSG_MAP(CommandDlg)
    MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
    NOTIFY_HANDLER(IDC_FILTERS, LVN_ITEMCHANGED, OnItemChanged)
    COMMAND_HANDLER(IDC_COMMAND, EN_CHANGE, OnCommandChanged)
    COMMAND_ID_HANDLER( IDOK, OnOK )
    COMMAND_ID_HANDLER( IDC_ENABLED, OnEnabled)
    COMMAND_ID_HANDLER( IDC_NEWFILTER, OnNewFilter )
    COMMAND_ID_HANDLER( IDC_NAMEITFORME, OnNameItForMe)
    COMMAND_ID_HANDLER( IDC_ALLOWINTERVAL, OnAllowInterval )
    COMMAND_ID_HANDLER( IDC_ALLOWFILTER, OnAllowFilter )
    COMMAND_ID_HANDLER( IDC_DELETEFILTER, OnDeleteFilter )
    COMMAND_ID_HANDLER( IDC_APPLYFILTER, OnApplyFilter )
    COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
  END_MSG_MAP()

  CommandDlg(CommandSpec& c, IWMPMedia* p) :
    m_real(c),
    m_copy(c),
    m_pMedia(p)
  {
  }

  static bool ShowEdit(HWND parent, CommandSpec& c, IWMPMedia* p)// returns true if the thing was edited.
  {
    CommandDlg dlg(c, p);
    INT_PTR res = dlg.DoModal(parent);
    return res == 1;
  }

  static bool ShowNew(HWND parent, CommandSpec& c, IWMPMedia* p)// returns true if the thing should be added.
  {
    CommandDlg dlg(c, p);
    INT_PTR res = dlg.DoModal(parent);
    return res == 1;
  }


  int GetSelectedFilterID()
  {
    return GetLPARAMFromListIndex(ListView_GetNextItem(GetDlgItem(IDC_FILTERS), -1, LVNI_SELECTED));
  }

  LPARAM GetLPARAMFromListIndex(int index)
  {
    HWND h = GetDlgItem(IDC_FILTERS);
    LVITEM i = {0};
    i.iItem = index;
    i.iSubItem = 0;
    i.mask = LVIF_PARAM;
    ListView_GetItem(h, &i);
    return i.lParam;
  }

  void PopulateFiltersList()
  {
    HWND h = GetDlgItem(IDC_FILTERS);
    int selectedIndex = ListView_GetSelectionMark(h);
    bool doSelection = selectedIndex != -1;
    LPARAM selectedID = GetLPARAMFromListIndex(selectedIndex);

    ListView_DeleteAllItems(h);

    LVITEM i = {0};
    i.mask = LVIF_PARAM | LVIF_TEXT;
    std::vector<FilterSpec> items = m_copy.Filters();
    for(std::vector<FilterSpec>::iterator it = items.begin(); it != items.end(); ++ it)
    {
      std::wstring attr = it->Attribute();
      std::wstring matches = it->Matches();
      i.lParam = it->id();
      i.pszText = const_cast<PWSTR>(attr.c_str());
      i.iItem = ListView_GetItemCount(h);
      int iItem = ListView_InsertItem(h, &i);
      ListView_SetItemText(h, iItem, 1, const_cast<PWSTR>(matches.c_str()));
    }

    if(doSelection)
    {
      LVFINDINFO f;
      f.flags = LVFI_PARAM;
      f.lParam = selectedID;
      int iSel = ListView_FindItem(h, -1, &f);
      ListView_SetSelectionMark(h, iSel);
      // make it selected state
      ListView_SetItemState(h, iSel, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    }
  }

  void PopulateAttributeList()
  {
    HWND h = GetDlgItem(IDC_ATTRIBUTELIST);

    LVCOLUMN lvc = {0};
    lvc.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvc.iSubItem = 0;
    lvc.pszText = L"Attribute Name";
    lvc.cx = 130;
    ListView_InsertColumn(h, 0, &lvc);
    lvc.iSubItem = 1;
    lvc.pszText = L"Value";
    ListView_InsertColumn(h, 1, &lvc);

    if(!m_pMedia)
    {
    }
    else
    {
      long count = 0;
      m_pMedia->get_attributeCount(&count);
      if(count > 1)
      {
        std::map<std::wstring, std::wstring> attributes;
        // build map of attributes
        for(long i = 0; i < count; i ++)
        {
          CComBSTR name;
          if(SUCCEEDED(m_pMedia->getAttributeName(i, &name)))
          {
            CComBSTR value;
            if(SUCCEEDED(m_pMedia->getItemInfo(name, &value)))
            {
              // now i have both name & value.
              attributes[(PCWSTR)name] = (PCWSTR)value;
            }
          }
        }
        for(std::map<std::wstring, std::wstring>::iterator it = attributes.begin(); it != attributes.end(); ++ it)
        {
          LVITEM i = {0};
          i.mask = LVIF_TEXT;
          i.pszText = const_cast<PWSTR>(it->first.c_str());
          i.iItem = ListView_GetItemCount(h);
          int iItem = ListView_InsertItem(h, &i);
          ListView_SetItemText(h, iItem, 1, const_cast<PWSTR>(it->second.c_str()));
        }
      }
    }
    ListView_SetColumnWidth(h, 0, LVSCW_AUTOSIZE);
  }

  LRESULT OnCommandChanged(WORD /*msg*/, WORD /*id*/, HWND hctrl, BOOL& handled)
  {
    HWND h = GetDlgItem(IDC_EXAMPLEOUTPUT);

    ListView_DeleteAllItems(h);

    handled = TRUE;
    std::list<std::wstring> cmd;
    LibCC::StringSplit(GetDlgItemText(IDC_COMMAND), L"\r\n", std::back_inserter(cmd));
    m_copy.Commands(cmd);

    if(m_pMedia)
    {
      mirc::MediaInfoWrapper mi(m_pMedia);
      for(std::list<std::wstring>::iterator it = cmd.begin(); it != cmd.end(); ++ it)
      {
        LVITEM i = {0};
        std::wstring out = mirc::ProcessCommandString(mi, *it);
        i.mask = LVIF_TEXT;
        i.pszText = const_cast<PWSTR>(out.c_str());
        i.iItem = ListView_GetItemCount(h);
        int iItem = ListView_InsertItem(h, &i);
      }
    }
    ListView_SetColumnWidth(h, 0, LVSCW_AUTOSIZE_USEHEADER);
    return 0;
  }

  LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
  {
    CenterWindow();
    // colums for filter
    LVCOLUMN lvc = {0};
    lvc.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvc.iSubItem = 0;
    lvc.pszText = L"Attribute";
    lvc.cx = 130;
    ListView_InsertColumn(GetDlgItem(IDC_FILTERS), 0, &lvc);
    lvc.iSubItem = 1;
    lvc.pszText = L"Matches";
    ListView_InsertColumn(GetDlgItem(IDC_FILTERS), 1, &lvc);

    // columns for example command
    lvc.iSubItem = 0;
    lvc.pszText = L"Command";
    ListView_InsertColumn(GetDlgItem(IDC_EXAMPLEOUTPUT), 0, &lvc);

    // blah
    ListView_SetExtendedListViewStyle(GetDlgItem(IDC_FILTERS), LVS_EX_FULLROWSELECT);
    ListView_SetExtendedListViewStyle(GetDlgItem(IDC_EXAMPLEOUTPUT), LVS_EX_FULLROWSELECT);
    ListView_SetExtendedListViewStyle(GetDlgItem(IDC_ATTRIBUTELIST), LVS_EX_FULLROWSELECT);

    // initialize controls.
    SetDlgItemText(IDC_NAME, m_copy.Name().c_str());

    PopulateFiltersList();

    std::list<std::wstring> cmd = m_copy.Commands();
    std::wstring cmdStr = LibCC::StringJoin(cmd.begin(), cmd.end(), L"\r\n");
    SetDlgItemText(IDC_COMMAND, cmdStr.c_str());

    Check(IDC_ENABLED, m_copy.Enabled());
    Check(IDC_NAMEITFORME, m_copy.AutoName());
    Check(IDC_REPEAT, m_copy.AllowRepeats());
    Check(IDC_ALLOWINTERVAL, m_copy.AllowInterval());
    SetDlgItemText(IDC_MINIMUMINTERVAL, LibCC::Format("%").i(m_copy.Interval()).CStr());
    SetDlgItemText(IDC_DELAY, LibCC::Format("%").i(m_copy.Delay()).CStr());
    Check(IDC_ALLOWFILTER, m_copy.AllowFilter());

    PopulateAttributeList();
    UpdateEnabledWindows();
    BOOL bTemp;
    OnCommandChanged(0, 0, 0, bTemp);
    return 1;
  }

  void UpdateEnabledWindows()
  {
    if(m_copy.AutoName())
    {
      SetDlgItemText(IDC_NAME, m_copy.Name().c_str());
    }
    if(m_copy.Enabled())
    {
      Enable(IDC_NAMEITFORME, true);
      Enable(IDC_COMMAND, true);
      Enable(IDC_ALLOWINTERVAL, true);
      Enable(IDC_REPEAT, true);
      Enable(IDC_ALLOWFILTER, true);
      Enable(IDC_DELAY, true);

      Enable(IDC_NAME, !IsChecked(IDC_NAMEITFORME));
      Enable(IDC_MINIMUMINTERVAL, IsChecked(IDC_ALLOWINTERVAL));
      bool filter = IsChecked(IDC_ALLOWFILTER);
      Enable(IDC_FILTERS, filter);
      Enable(IDC_FILTERKEY, filter);
      Enable(IDC_FILTER, filter);
      Enable(IDC_NEWFILTER, filter);
      bool filterSelected = 0 != ListView_GetSelectedCount(GetDlgItem(IDC_FILTERS));
      Enable(IDC_APPLYFILTER, filter && filterSelected);
      Enable(IDC_DELETEFILTER, filter && filterSelected);
      FilterSpec f;
      if(m_copy.GetFilterSpecByID(GetSelectedFilterID(), f))
      {
        SetDlgItemText(IDC_FILTERKEY, f.Attribute().c_str());
        SetDlgItemText(IDC_FILTER, f.Matches().c_str());
      }
    }
    else
    {
      Enable(IDC_NAME, false);
      Enable(IDC_NAMEITFORME, false);
      Enable(IDC_COMMAND, false);
      Enable(IDC_REPEAT, false);
      Enable(IDC_ALLOWINTERVAL, false);
      Enable(IDC_MINIMUMINTERVAL, false);
      Enable(IDC_DELAY, false);
      Enable(IDC_ALLOWFILTER, false);
      Enable(IDC_FILTERS, false);
      Enable(IDC_FILTERKEY, false);
      Enable(IDC_FILTER, false);
      Enable(IDC_NEWFILTER, false);
      Enable(IDC_APPLYFILTER, false);
      Enable(IDC_DELETEFILTER, false);
    }
  }

  bool ValidateFilterFields()
  {
    if(GetDlgItemText(IDC_FILTERKEY).empty())
    {
      MessageBox(L"You must enter a valid attribute name.  Use the examples on the right.", L"Error", MB_OK | MB_ICONERROR);
      GetDlgItem(IDC_FILTERKEY).SetFocus();
      return false;
    }
    if(GetDlgItemText(IDC_FILTER).empty())
    {
      MessageBox(L"You must enter a valid filter criteria.  Wildcards '?' and '*' are supported.  Separate multiple criteria by semicolon (';').", L"Error", MB_OK | MB_ICONERROR);
      GetDlgItem(IDC_FILTERKEY).SetFocus();
      return false;
    }
    return true;
  }

  LRESULT OnApplyFilter(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    FilterSpec f;
    if(!ValidateFilterFields())
    {
      return 0;
    }
    if(!m_copy.GetFilterSpecByID(GetSelectedFilterID(), f))
    {
      return 0;
    }
    f.Attribute(GetDlgItemText(IDC_FILTERKEY));
    f.Matches(GetDlgItemText(IDC_FILTER));
    m_copy.AddOrUpdateFilter(f);
    PopulateFiltersList();
    UpdateEnabledWindows();
    return 0;
  }

  LRESULT OnNewFilter(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    FilterSpec f(m_copy);
    if(!ValidateFilterFields())
    {
      return 0;
    }
    f.Attribute(GetDlgItemText(IDC_FILTERKEY));
    f.Matches(GetDlgItemText(IDC_FILTER));
    m_copy.AddOrUpdateFilter(f);
    PopulateFiltersList();
    UpdateEnabledWindows();
    return 0;
  }

  LRESULT OnEnabled(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    m_copy.Enabled(IsChecked(IDC_ENABLED));
    UpdateEnabledWindows();
    return 0;
  }

  LRESULT OnNameItForMe(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    m_copy.AutoName(IsChecked(IDC_NAMEITFORME));
    UpdateEnabledWindows();
    return 0;
  }

  LRESULT OnAllowInterval(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    UpdateEnabledWindows();
    return 0;
  }

  LRESULT OnAllowFilter(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    m_copy.AllowFilter(IsChecked(IDC_ALLOWFILTER));
    UpdateEnabledWindows();
    return 0;
  }

  LRESULT OnDeleteFilter(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    if(!m_copy.DeleteFilter(GetSelectedFilterID()))
    {
      return 0;
    }
    PopulateFiltersList();
    UpdateEnabledWindows();
    return 0;
  }

  LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    // filters are already populated
    m_copy.Name(GetDlgItemText(IDC_NAME));
    // Commands already populated
    // auto name already done.
    m_copy.AllowRepeats(IsChecked(IDC_REPEAT));
    m_copy.AllowInterval(IsChecked(IDC_ALLOWINTERVAL));
    m_copy.Interval(wcstol(GetDlgItemText(IDC_MINIMUMINTERVAL).c_str(), 0, 10));
    m_copy.Delay(wcstol(GetDlgItemText(IDC_DELAY).c_str(), 0, 10));
    // allow filter already done

    if(m_copy.Name().empty())
    {
      MessageBox(L"You must fill in a name.", L"Error", MB_OK | MB_ICONERROR);
      GetDlgItem(IDC_NAME).SetFocus();
      return 0;
    }

    m_real = m_copy;
    EndDialog( 1 );
    return 0;
  }

  LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    EndDialog( 0 );
    return 0;
  }

  LRESULT OnItemChanged(int, LPNMHDR pnmhdr, BOOL& bHandled)
  {
    UpdateEnabledWindows();
    return 0;
  }

private:
  CommandSpec& m_real;
  CommandSpec m_copy;
  IWMPMedia* m_pMedia;
};

