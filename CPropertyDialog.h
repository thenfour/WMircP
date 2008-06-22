

#include "atlwin.h"
#include "options.h"
#include "CommandDlg.h"

#pragma comment(lib, "comctl32.lib")

class CPropertyDialog :
  public CDialogImpl<CPropertyDialog>
{
public:
  enum { IDD = IDD_PROPERTYDIALOG };

  void Check(UINT id, bool b)
  {
    CheckDlgButton(id, b ? BST_CHECKED : BST_UNCHECKED);
  }
  bool IsChecked(UINT id)
  {
    return IsDlgButtonChecked(id) == BST_CHECKED;
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

  BEGIN_MSG_MAP(CPropertyDialog)
    MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
    NOTIFY_HANDLER(IDC_COMMANDLIST, LVN_ITEMACTIVATE, OnItemActivate)
    NOTIFY_HANDLER(IDC_COMMANDLIST, LVN_ITEMCHANGED, OnItemChanged)
    COMMAND_ID_HANDLER( IDOK, OnOK )
    COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
    COMMAND_ID_HANDLER( IDC_NEWCOMMAND, OnNewCommand )
    COMMAND_ID_HANDLER( IDC_EDITCOMMAND, OnEditCommand )
    COMMAND_ID_HANDLER( IDC_DELETECOMMAND, OnDeleteCommand )
    COMMAND_ID_HANDLER( IDC_MOVEUP, OnMoveUp )
    COMMAND_ID_HANDLER( IDC_MOVEDOWN, OnMoveDown )
  END_MSG_MAP()

  CPropertyDialog(Options& o, IWMPMedia* p) :
    m_realOptions(o),
    m_optionsCopy(o),
    m_changedRealOptions(false),
    m_pMedia(p)
  {
  }

  bool ChangedOptions() const
  {
    return m_changedRealOptions;
  }

  int GetSelectedCommandID()
  {
    return GetLPARAMFromListIndex(ListView_GetNextItem(GetDlgItem(IDC_COMMANDLIST), -1, LVNI_SELECTED));
  }

  LPARAM GetLPARAMFromListIndex(int index)
  {
    HWND h = GetDlgItem(IDC_COMMANDLIST);
    LVITEM i = {0};
    i.iItem = index;
    i.iSubItem = 0;
    i.mask = LVIF_PARAM;
    ListView_GetItem(h, &i);
    return i.lParam;
  }

  void PopulateCommandsList()
  {
    HWND h = GetDlgItem(IDC_COMMANDLIST);
    int selectedIndex = ListView_GetSelectionMark(h);
    bool doSelection = selectedIndex != -1;
    LPARAM selectedID = GetLPARAMFromListIndex(selectedIndex);

    ListView_DeleteAllItems(h);

    LVITEM i = {0};
    i.mask = LVIF_PARAM | LVIF_TEXT;
    std::vector<CommandSpec> commands = m_optionsCopy.Commands();
    for(std::vector<CommandSpec>::iterator it = commands.begin(); it != commands.end(); ++ it)
    {
      std::wstring name = it->Name();
      i.lParam = it->id();
      i.pszText = const_cast<PWSTR>(name.c_str());
      i.iItem = ListView_GetItemCount(h);
      ListView_InsertItem(h, &i);
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

  LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
  {
    CenterWindow();
    LVCOLUMN lvc = {0};
    lvc.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvc.iSubItem = 0;
    lvc.pszText = L"Name";
    lvc.cx = 180;
    ListView_InsertColumn(GetDlgItem(IDC_COMMANDLIST), 0, &lvc);

    ListView_SetExtendedListViewStyle(GetDlgItem(IDC_COMMANDLIST), LVS_EX_FULLROWSELECT);

    // initialize controls.
    SetDlgItemText(IDC_DDESERVER, m_optionsCopy.DDEServer().c_str());
    Check(IDC_SHOWTRAYICON, m_optionsCopy.ShowTrayIcon());
    Check(IDC_ENABLE, m_optionsCopy.Enabled());

    PopulateCommandsList();

    UpdateControlsToListviewSelection();

    return 1;
  }

  LRESULT OnNewCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    CommandSpec c(m_optionsCopy);
    if(CommandDlg::ShowNew(m_hWnd, c, m_pMedia))
    {
      m_optionsCopy.AddOrUpdateCommand(c);
      PopulateCommandsList();
    }
    return 0;
  }

  LRESULT OnEditCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    CommandSpec c;
    m_optionsCopy.GetCommandFromID(GetSelectedCommandID(), c);
    if(CommandDlg::ShowEdit(m_hWnd, c, m_pMedia))
    {
      m_optionsCopy.AddOrUpdateCommand(c);
      PopulateCommandsList();
    }
    return 0;
  }

  LRESULT OnDeleteCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    m_optionsCopy.DeleteCommand(GetSelectedCommandID());
    PopulateCommandsList();
    return 0;
  }

  LRESULT OnMoveUp(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    if(m_optionsCopy.MoveCommand(GetSelectedCommandID(), true))
    {
      PopulateCommandsList();
    }
    return 0;
  }

  LRESULT OnMoveDown(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    if(m_optionsCopy.MoveCommand(GetSelectedCommandID(), false))
    {
      PopulateCommandsList();
    }
    return 0;
  }

  LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    // commands are already populated
    m_optionsCopy.DDEServer(GetDlgItemText(IDC_DDESERVER));
    m_optionsCopy.ShowTrayIcon(IsChecked(IDC_SHOWTRAYICON));
    m_optionsCopy.Enabled(IsChecked(IDC_ENABLE));

    // validate
    if(m_optionsCopy.DDEServer().empty())
    {
      MessageBox(L"DDE server must be filled in.  Use 'mIRC' if you are uncertain.", L"Error", MB_OK | MB_ICONERROR);
      GetDlgItem(IDC_DDESERVER).SetFocus();
      return 0;
    }

    m_realOptions = m_optionsCopy;
    m_changedRealOptions = true;
    m_realOptions.Save();
    EndDialog( IDOK );
    return 0;
  }

  LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
  {
    EndDialog( IDCANCEL );
    return 0;
  }

  void UpdateControlsToListviewSelection()
  {
    HWND h = GetDlgItem(IDC_COMMANDLIST);
    BOOL b = (0 != ListView_GetSelectedCount(h)) ? TRUE : FALSE;
    ::EnableWindow(GetDlgItem(IDC_MOVEUP), b);
    ::EnableWindow(GetDlgItem(IDC_MOVEDOWN), b);
    ::EnableWindow(GetDlgItem(IDC_DELETECOMMAND), b);
    ::EnableWindow(GetDlgItem(IDC_EDITCOMMAND), b);
  }

  LRESULT OnItemChanged(int, LPNMHDR pnmhdr, BOOL& bHandled)
  {
    UpdateControlsToListviewSelection();
    return 0;
  }

  LRESULT OnItemActivate(int, LPNMHDR pnmhdr, BOOL& bHandled)
  {
    OnEditCommand(0, 0, 0, bHandled);
    return 0;
  }

private:
  Options& m_realOptions;
  Options m_optionsCopy;
  bool m_changedRealOptions;
  IWMPMedia* m_pMedia;
};
