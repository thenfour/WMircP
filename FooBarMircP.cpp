
#include "stdafx.h"
#include "SDK-2008-05-27\foobar2000\SDK\foobar2000.h"
//#pragma comment(lib, "SDK-2008-05-27\\foobar2000\\shared\\shared.lib")
//
//
//bool g_showWindow = false;
//
//
//DECLARE_COMPONENT_VERSION
//(
//	"foobarMircP",
//	"0.0",
//	"foobarMircP, by tenfour"
//);
//
//
//
//class mainmenu_commands_tutorial1 :
//	public mainmenu_commands
//{
//	// Return the number of commands we provide.
//	virtual t_uint32 get_command_count()
//	{
//		return 1;
//	}
//
//	// All commands are identified by a GUID.
//	virtual GUID get_command(t_uint32 p_index)
//	{
//		// {C818DE52-3F2C-459B-811D-D104C60C9384}
//		static const GUID guid_main_tutorial1 =
//		{ 0xc818de52, 0x3f2c, 0x459b, { 0x81, 0x1d, 0xd1, 0x04, 0xc6, 0x0c, 0x93, 0x84 } };
//
//		if (p_index == 0)
//			return guid_main_tutorial1;
//		return pfc::guid_null;
//	}
//
//	// Set p_out to the name of the n-th command.
//	// This name is used to identify the command and determines
//	// the default position of the command in the menu.
//	virtual void get_name(t_uint32 p_index, pfc::string_base & p_out)
//	{
//		if (p_index == 0)
//			p_out = "Show foobarMircP";
//	}
//
//	// Set p_out to the description for the n-th command.
//	virtual bool get_description(t_uint32 p_index, pfc::string_base & p_out)
//	{
//		if (p_index == 0)
//			p_out = "Show foobarMircP";
//		else
//			return false;
//		return true;
//	}
//
//	// Every set of commands needs to declare which group it belongs to.
//	virtual GUID get_parent()
//	{
//		return mainmenu_groups::view;
//	}
//
//	// Execute n-th command.
//	// p_callback is reserved for future use.
//	virtual void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback)
//	{
//		if (p_index == 0 && core_api::assert_main_thread()) {
//			if (g_showWindow)
//				// Hide and disable the window.
//				//CTutorialWindow::HideWindow();
//				int a = 1;
//			else
//				// Show and enable the window.
//				//CTutorialWindow::ShowWindow();
//				int a = 1;
//		}
//	}
//
//	// The standard version of this command does not support checked or disabled
//	// commands, so we use our own version.
//	virtual bool get_display(t_uint32 p_index, pfc::string_base & p_text, t_uint32 & p_flags)
//	{
//		p_flags = 0;
//		if (is_checked(p_index))
//			p_flags |= flag_checked;
//		get_name(p_index,p_text);
//		return true;
//	}
//
//	// Return whether the n-th command is checked.
//	bool is_checked(t_uint32 p_index)
//	{
//		if (p_index == 0)
//			return g_showWindow;
//		return false;
//	}
//};
//
//// We need to create a service factory for our menu item class,
//// otherwise the menu commands won't be known to the system.
//static mainmenu_commands_factory_t< mainmenu_commands_tutorial1 > foo_menu;
//
//
//
//class foobar2000CallBack :
//	private play_callback
//{
//public:
//};

























































// stuff that can help:

	//// Register ourselves as message_filter, so we can process keyboard shortcuts
	//// and (possibly) dialog messages.
	//static_api_ptr_t<message_loop>()->add_message_filter(this);

	//// Register ourselves as play_callback to get notified about playback events.
	//static_api_ptr_t<play_callback_manager>()->register_callback(this,
	//	flag_on_playback_new_track |
	//	flag_on_playback_dynamic_info_track |
	//	flag_on_playback_stop,
	//	false);

	//static_api_ptr_t<message_loop>()->remove_message_filter(this);

	//static_api_ptr_t<play_callback_manager>()->unregister_callback(this);



//
//void CTutorialWindow::OnLButtonDown(UINT nFlags, CPoint point) {
//	// Get currently playing track.
//	static_api_ptr_t<play_control> pc;
//	metadb_handle_ptr handle;
//
//	// If some track is playing...
//	if (pc->get_now_playing(handle)) {
//		POINT pt;
//		GetCursorPos(&pt);
//
//		// ...detect a drag operation.
//		if (DragDetect(m_hWnd, pt)) {
//			metadb_handle_list items;
//			items.add_item(handle);
//
//			// Create an IDataObject that contains the dragged track.
//			static_api_ptr_t<playlist_incoming_item_filter> piif;
//			// create_dataobject_ex() returns a smart pointer unlike create_dataobject()
//			// which returns a raw COM pointer. The less chance we have to accidentally
//			// get the reference counting wrong, the better.
//			pfc::com_ptr_t<IDataObject> pDataObject = piif->create_dataobject_ex(items);
//
//			// Create an IDropSource.
//			// The constructor of IDropSource_tutorial1 is hidden by design; we use the
//			// provided factory method which returns a smart pointer.
//			pfc::com_ptr_t<IDropSource> pDropSource = IDropSource_tutorial1::g_create(m_hWnd);
//
//			DWORD effect;
//			// Perform drag&drop operation.
//			DoDragDrop(pDataObject.get_ptr(), pDropSource.get_ptr(), DROPEFFECT_COPY, &effect);
//		}
//	}
//}
//
//void CTutorialWindow::OnPaint(HDC hdc) {
//	CPaintDC dc(m_hWnd);
//	PaintContent(dc.m_ps);
//}
//
//void CTutorialWindow::OnPrintClient(HDC hdc, UINT uFlags) {
//	PAINTSTRUCT ps = { 0 };
//	ps.hdc = hdc;
//	GetClientRect(m_hWnd, &ps.rcPaint);
//	ps.fErase = FALSE;
//	PaintContent(ps);
//}
//
//void CTutorialWindow::PaintContent(PAINTSTRUCT &ps) {
//	if (GetSystemMetrics(SM_REMOTESESSION)) {
//		// Do not use double buffering, if we are running on a Remote Desktop Connection.
//		// The system would have to transfer a bitmap everytime our window is painted.
//		Draw(ps.hdc, ps.rcPaint);
//	} else if (!IsRectEmpty(&ps.rcPaint)) {
//		// Use double buffering for local drawing.
//		CMemoryDC dc(ps.hdc, ps.rcPaint);
//		Draw(dc, ps.rcPaint);
//	}
//}
//
//void CTutorialWindow::Draw(HDC hdc, CRect rcPaint) {
//	// We will paint the background in the default window color.
//	HBRUSH hBrush = GetSysColorBrush(COLOR_WINDOW);
//	FillRect(hdc, rcPaint, hBrush);
//
//	HICON hIcon = static_api_ptr_t<ui_control>()->get_main_icon();
//	if (hIcon != NULL) {
//		DrawIconEx(hdc, 2, 2, hIcon, 32, 32, 0, hBrush, DI_NORMAL);
//	}
//
//	try
//	{
//		static_api_ptr_t<play_control> pc;
//		metadb_handle_ptr handle;;
//		if (pc->get_now_playing(handle)) {
//			pfc::string8 format;
//			g_advconfig_string_format.get_static_instance().get_state(format);
//			service_ptr_t<titleformat_object> script;
//			static_api_ptr_t<titleformat_compiler>()->compile_safe(script, format);
//
//			pfc::string_formatter text;
//			pc->playback_format_title_ex(handle, NULL, text, script, NULL, play_control::display_level_titles);
//			HFONT hFont = m_font;
//			if (hFont == NULL)
//				hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
//			SelectObject(hdc, hFont);
//			SetTextAlign(hdc, TA_TOP | TA_LEFT);
//			uExtTextOut(hdc, 32+4, 2, ETO_CLIPPED, rcPaint, text, text.length(), 0);
//		}
//	}
//	catch (const std::exception & exc) {
//		console::formatter() << "Exception occurred while drawing " TUTORIAL " window:\n" << exc;
//	}
//}
//
//void CTutorialWindow::OnContextMenu(HWND hWnd, CPoint point) {
//	// We need some IDs for the context menu.
//	enum {
//		// ID for "Choose font..."
//		ID_FONT = 1,
//		// The range ID_CONTEXT_FIRST through ID_CONTEXT_LAST is reserved
//		// for menu entries from menu_manager.
//		ID_CONTEXT_FIRST,
//		ID_CONTEXT_LAST = ID_CONTEXT_FIRST + 1000,
//	};
//
//	// Create new popup menu.
//	HMENU hMenu = CreatePopupMenu();
//
//	// Add our "Choose font..." command.
//	AppendMenu(hMenu, MF_STRING, ID_FONT, TEXT("Choose font..."));
//
//	// Get the currently playing track.
//	metadb_handle_list items;
//	static_api_ptr_t<play_control> pc;
//	metadb_handle_ptr handle;
//	if (pc->get_now_playing(handle)) {
//		// Insert it into a list.
//		items.add_item(handle);
//	}
//
//	// Create a menu_manager that will build the context menu.
//	service_ptr_t<contextmenu_manager> cmm;
//	contextmenu_manager::g_create(cmm);
//	// Query setting for showing keyboard shortcuts.
//	const bool show_shortcuts = config_object::g_get_data_bool_simple(standard_config_objects::bool_show_keyboard_shortcuts_in_menus, false);
//	// Set up flags for contextmenu_manager::init_context.
//	unsigned flags = show_shortcuts ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0;
//	// Initialize menu_manager for using a context menu.
//	cmm->init_context(items, flags);
//	// If the menu_manager has found any applicable commands,
//	// add them to our menu (after a separator).
//	if (cmm->get_root()) {
//		uAppendMenu(hMenu, MF_SEPARATOR, 0, 0);
//		cmm->win32_build_menu(hMenu, ID_CONTEXT_FIRST, ID_CONTEXT_LAST);
//	}
//
//	// Use menu helper to gnereate mnemonics.
//	menu_helpers::win32_auto_mnemonics(hMenu);
//
//	// Get the location of the mouse pointer.
//	// WM_CONTEXTMENU provides position of mouse pointer in argument lp,
//	// but this isn't reliable (for example when the user pressed the
//	// context menu key on the keyboard).
//	CPoint pt;
//	GetCursorPos(pt);
//	// Show the context menu.
//	int cmd = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON, 
//		pt.x, pt.y, 0, m_hWnd, 0);
//
//	// Check what command has been chosen. If cmd == 0, then no command
//	// was chosen.
//	if (cmd == ID_FONT) {
//		// Show font configuration.
//		t_font_description font = cfg_font;
//		if (font.popup_dialog(m_hWnd)) {
//			cfg_font = font;
//			m_font = font.create();
//			::RedrawWindow(m_hWnd, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);
//		}
//	} else if (cmd >= ID_CONTEXT_FIRST && cmd <= ID_CONTEXT_LAST ) {
//		// Let the menu_manager execute the chosen command.
//		cmm->execute_by_id(cmd - ID_CONTEXT_FIRST);
//	}
//
//	// contextmenu_manager instance is released automatically, as is the metadb_handle we used.
//
//	// Finally, destroy the popup menu.
//	DestroyMenu(hMenu);
//}
