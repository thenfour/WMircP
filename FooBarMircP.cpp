

#include "stdafx.h"

#ifdef TARGET_FOOBAR

#include "0811 wmp2mirc.h"
#include "SDK-2008-05-27\foobar2000\SDK\foobar2000.h"
#include <comutil.h>

#pragma comment(lib, "SDK-2008-05-27\\foobar2000\\shared\\shared.lib")
#pragma comment(lib, "comsuppw.lib")


// {865DCA0D-4493-4392-9B52-09D2986008EF}
static const IID IID_FooBarMirc =
{ 0x865dca0d, 0x4493, 0x4392, { 0x9b, 0x52, 0x09, 0xd2, 0x98, 0x60, 0x08, 0xef } };


DECLARE_COMPONENT_VERSION
(
	"foobarMircP",
	"0.0",
	"foobarMircP, by tenfour"
);

// exposes foobar media as IWMPMedia.
struct FooBarMedia :
	public IWMPMedia
{
	ULONG m_refcount;
	std::wstring m_url;
	std::vector<std::pair<std::wstring, std::wstring> > m_attributes;

	FooBarMedia() :
		m_refcount(1)
	{
	}

	HRESULT __stdcall QueryInterface(const IID &iid, void ** out)
	{
		if(iid == __uuidof(IWMPMedia))
		{
			*((IWMPMedia**)out) = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	ULONG __stdcall AddRef(void)
	{
		return ++ m_refcount;
	}
	ULONG __stdcall Release(void)
	{
		m_refcount --;
		if(m_refcount > 0)
			return m_refcount;
		delete this;
		return 0;
	}
	// IDispatch
	HRESULT __stdcall GetTypeInfoCount(UINT *){ return E_NOTIMPL; }
	HRESULT __stdcall GetTypeInfo(UINT,LCID,ITypeInfo **){ return E_NOTIMPL; }
	HRESULT __stdcall GetIDsOfNames(const IID &,LPOLESTR *,UINT,LCID,DISPID *){ return E_NOTIMPL; }
	HRESULT __stdcall Invoke(DISPID,const IID &,LCID,WORD,DISPPARAMS *,VARIANT *,EXCEPINFO *,UINT *){ return E_NOTIMPL; }
	// IWMPMedia
	HRESULT __stdcall get_isIdentical(IWMPMedia *,VARIANT_BOOL *){ return E_NOTIMPL; }

	HRESULT __stdcall get_sourceURL(BSTR * out)// REQUIRED
	{
		*out = SysAllocString(m_url.c_str());
		return S_OK;
	}
	HRESULT __stdcall get_name(BSTR *){ return E_NOTIMPL; }
	HRESULT __stdcall put_name(BSTR){ return E_NOTIMPL; }
	HRESULT __stdcall get_imageSourceWidth(long *){ return E_NOTIMPL; }
	HRESULT __stdcall get_imageSourceHeight(long *){ return E_NOTIMPL; }
	HRESULT __stdcall get_markerCount(long *){ return E_NOTIMPL; }
	HRESULT __stdcall getMarkerTime(long,double *){ return E_NOTIMPL; }
	HRESULT __stdcall getMarkerName(long,BSTR *){ return E_NOTIMPL; }
	HRESULT __stdcall get_duration(double *){ return E_NOTIMPL; }
	HRESULT __stdcall get_durationString(BSTR *){ return E_NOTIMPL; }
	HRESULT __stdcall get_attributeCount(long * out)// REQUIRED
	{
		*out = (long)m_attributes.size();
		return S_OK;
	}
	HRESULT __stdcall getAttributeName(long i,BSTR * out)// REQUIRED
	{
		*out = SysAllocString(m_attributes[i].first.c_str());
		return S_OK;
	}
	HRESULT __stdcall getItemInfo(BSTR attrName, BSTR * out)// REQUIRED
	{
		_bstr_t name(attrName, true);

		for(std::vector<std::pair<std::wstring, std::wstring> >::iterator it = m_attributes.begin(); it != m_attributes.end(); ++ it)
		{
			if(LibCC::StringEqualsI(it->first, static_cast<PCWSTR>(name)))
			{
				// found it.
				*out = SysAllocString(it->second.c_str());
				return S_OK;
			}
		}
		return E_FAIL;
	}
	HRESULT __stdcall setItemInfo(BSTR,BSTR){ return E_NOTIMPL; }
	HRESULT __stdcall getItemInfoByAtom(long,BSTR *){ return E_NOTIMPL; }
	HRESULT __stdcall isMemberOf(IWMPPlaylist *,VARIANT_BOOL *){ return E_NOTIMPL; }
	HRESULT __stdcall isReadOnlyItem(BSTR,VARIANT_BOOL *){ return E_NOTIMPL; }

};

// this exposes foobar's core somewhat like WMP exposes it's core
struct FooBarCore :
	public IFooBarMirc,
	public IWMPCore,
	private play_callback
{
	LibCC::LogReference m_log;

	FooBarMedia m;
	FooBarCore() : m_events(0)
	{
#ifdef _DEBUG
		bool debug = true;
#else
		bool debug = false;
#endif
		m_log.Create(L"foobarMircP.log", core_api::get_my_instance(), debug, debug, true, true, false, true);

		static_api_ptr_t<play_callback_manager>()->register_callback(this,
			flag_on_playback_new_track |
			flag_on_playback_dynamic_info_track |
			flag_on_playback_stop,
			false);
	}

	~FooBarCore()
	{
		static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
	}

	HRESULT __stdcall QueryInterface(const IID &iid, void ** out)
	{
		if(iid == IID_FooBarMirc)
		{
			*((IFooBarMirc**)out) = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	ULONG __stdcall AddRef(void)
	{
		return 1;
	}
	ULONG __stdcall Release(void)
	{
		return 1;
	}
	HRESULT __stdcall GetTypeInfoCount(UINT *)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall GetTypeInfo(UINT,LCID,ITypeInfo **)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall GetIDsOfNames(const IID &,LPOLESTR *,UINT,LCID,DISPID *)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall Invoke(DISPID,const IID &,LCID,WORD,DISPPARAMS *,VARIANT *,EXCEPINFO *,UINT *)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall close(void)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_URL(BSTR *)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall put_URL(BSTR)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_openState(WMPOpenState *)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_playState(WMPPlayState *)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_controls(IWMPControls **)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_settings(IWMPSettings **)
	{
		return E_NOTIMPL;
	}
	void HandleAliases(FooBarMedia* n, const std::wstring& originalName, const std::wstring& val)
	{
		if(LibCC::StringEqualsI(originalName, "artist"))
		{
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"WM/AlbumArtist", val));
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"DisplayArtist", val));
		}
		else if(LibCC::StringEqualsI(originalName, "composer"))
		{
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"Author", val));
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"WM/Composer", val));
		}
		else if(LibCC::StringEqualsI(originalName, "comment"))
		{
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"Description", val));
		}
		else if(LibCC::StringEqualsI(originalName, "album"))
		{
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"HMEAlbumTitle", val));
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"WM/AlbumTitle", val));
		}
		else if(LibCC::StringEqualsI(originalName, "genre"))
		{
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"WM/Genre", val));
		}
		else if(LibCC::StringEqualsI(originalName, "genre"))
		{
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"WM/Genre", val));
		}
		else if(LibCC::StringEqualsI(originalName, "publisher"))
		{
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"WM/Publisher", val));
		}
		else if(LibCC::StringEqualsI(originalName, "tracknumber"))
		{
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"WM/Track", val));
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"WM/TrackNumber", val));
		}
		else if(LibCC::StringEqualsI(originalName, "date"))
		{
			n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"WM/Year", val));
		}
	}
	// Return whether first element is greater than the second
	static bool AttributeLT(const std::pair<std::wstring, std::wstring>& lhs, const std::pair<std::wstring, std::wstring>& rhs)
	{
		 return lhs.first < rhs.first;
	}

	HRESULT __stdcall get_currentMedia(IWMPMedia ** out)
	{
		FooBarMedia* n = new FooBarMedia();
		*out = n;

		static_api_ptr_t<play_control> pc;
		metadb_handle_ptr handle;

		if(!pc->get_now_playing(handle))
		{
			m_log->Message("get_now_playing failed.");
		}
		else
		{
			file_info_impl info;
			if(!handle->get_info(info))
			{
				m_log->Message("get_info failed.");
			}
			else
			{
				LibCC::StringConvert(handle->get_path(), n->m_url);

				n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"SourceURL", n->m_url));
				n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"Duration", LibCC::FormatW("")(info.get_length()).Str()));
				n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(L"FileSize", LibCC::FormatW("")(handle->get_filestats().m_size).Str()));

				t_size count = info.info_get_count();
				for(t_size i = 0; i < count; i ++)
				{
					std::wstring name = LibCC::ToUTF16(info.info_enum_name(i));
					std::wstring value = LibCC::ToUTF16(info.info_enum_value(i));
					n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(name, value));
					HandleAliases(n, name, value);
				}

				count = info.meta_get_count();
				for(t_size i = 0; i < count; i ++)
				{
					std::wstring name = LibCC::ToUTF16(info.meta_enum_name(i));
					t_size valueCount = info.meta_enum_value_count(i);

					std::vector<std::wstring> values;
					for(t_size ival = 0; ival < valueCount; ++ ival)
					{
						values.push_back(LibCC::ToUTF16(info.meta_enum_value(i, ival)));
					}

					std::wstring value = LibCC::StringJoin(values.begin(), values.end(), L", ");
					n->m_attributes.push_back(std::pair<std::wstring, std::wstring>(name, value));
					HandleAliases(n, name, value);
				}

				std::sort(n->m_attributes.begin(), n->m_attributes.end(), AttributeLT);
			}
		}

		return S_OK;
	}
	HRESULT __stdcall put_currentMedia(IWMPMedia *)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_mediaCollection(IWMPMediaCollection **)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_playlistCollection(IWMPPlaylistCollection **)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_versionInfo(BSTR *)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall launchURL(BSTR)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_network(IWMPNetwork **)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_currentPlaylist(IWMPPlaylist **)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall put_currentPlaylist(IWMPPlaylist *)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_cdromCollection(IWMPCdromCollection **)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_closedCaption(IWMPClosedCaption **)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_isOnline(VARIANT_BOOL *)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_error(IWMPError **)
	{
		return E_NOTIMPL;
	}
	HRESULT __stdcall get_status(BSTR *)
	{
		return E_NOTIMPL;
	}

	// IFooBarMirc
	IWMPEvents * m_events;

	void SetEventHandler(IWMPEvents * p)
	{
		m_events = p;
	}

	// playback_events
	void on_playback_starting(playback_control::t_track_command,bool)
	{
		if(m_events)
		{
			m_events->PlayStateChange(wmppsPlaying);
		}
		LibCC::g_pLog->Message(L"on_playback_starting");
	}
	void on_playback_new_track(metadb_handle_ptr)
	{
		if(m_events)
		{
			m_events->PlayStateChange(wmppsPlaying);
		}
		LibCC::g_pLog->Message(L"on_playback_new_track");
	}
	void on_playback_stop(playback_control::t_stop_reason)
	{
		LibCC::g_pLog->Message(L"on_playback_stop");
	}
	void on_playback_seek(double)
	{
		LibCC::g_pLog->Message(L"on_playback_seek");
	}
	void on_playback_pause(bool)
	{
		LibCC::g_pLog->Message(L"on_playback_pause");
	}
	void on_playback_edited(metadb_handle_ptr)
	{
		LibCC::g_pLog->Message(L"on_playback_edited");
	}
	void on_playback_dynamic_info(const file_info &)
	{
		LibCC::g_pLog->Message(L"on_playback_dynamic_info");
	}
	void on_playback_dynamic_info_track(const file_info &)
	{
		LibCC::g_pLog->Message(L"on_playback_dynamic_info_track");
	}
	void on_playback_time(double)
	{
		LibCC::g_pLog->Message(L"on_playback_time");
	}
	void on_volume_change(float)
	{
		LibCC::g_pLog->Message(L"on_volume_change");
	}
};

struct FooBarMircApp
{
	FooBarMircApp()
	{
	  CoInitialize(0);
		
    //CoCreateInstance(CLSID_Wmp2mirc, 0, CLSCTX_INPROC_SERVER, __uuidof(IWMPPluginUI), (VOID**)&plugin);
		plugin.SetCore(&adapter);
	}
	~FooBarMircApp()
	{
		CoUninitialize();
	}
	CComObject<CWmp2mirc> plugin;
	FooBarCore adapter;
};

FooBarMircApp* g_instance = 0;



class initquit_tutorial1 : public initquit
{
	virtual void on_init()
	{
		g_instance = new FooBarMircApp();
	}

	virtual void on_quit()
	{
		delete g_instance;
	}
};

static initquit_factory_t< initquit_tutorial1 > foo_initquit;







class mainmenu_commands_tutorial1 :
	public mainmenu_commands
{
	// Return the number of commands we provide.
	virtual t_uint32 get_command_count()
	{
		return 1;
	}

	// All commands are identified by a GUID.
	virtual GUID get_command(t_uint32 p_index)
	{
		// {C818DE52-3F2C-459B-811D-D104C60C9384}
		static const GUID guid_main_tutorial1 =
		{ 0xc818de52, 0x3f2c, 0x459b, { 0x81, 0x1d, 0xd1, 0x04, 0xc6, 0x0c, 0x93, 0x84 } };

		if (p_index == 0)
			return guid_main_tutorial1;
		return pfc::guid_null;
	}

	// Set p_out to the name of the n-th command.
	// This name is used to identify the command and determines
	// the default position of the command in the menu.
	virtual void get_name(t_uint32 p_index, pfc::string_base & p_out)
	{
		if (p_index == 0)
			p_out = "Show foobarMircP";
	}

	// Set p_out to the description for the n-th command.
	virtual bool get_description(t_uint32 p_index, pfc::string_base & p_out)
	{
		if (p_index == 0)
			p_out = "Show foobarMircP";
		else
			return false;
		return true;
	}

	// Every set of commands needs to declare which group it belongs to.
	virtual GUID get_parent()
	{
		return mainmenu_groups::view;
	}

	// Execute n-th command.
	// p_callback is reserved for future use.
	virtual void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback)
	{
		if (p_index == 0 && core_api::assert_main_thread())
		{
			g_instance->plugin.DisplayPropertyPage(core_api::get_main_window());
		}
	}

	// The standard version of this command does not support checked or disabled
	// commands, so we use our own version.
	virtual bool get_display(t_uint32 p_index, pfc::string_base & p_text, t_uint32 & p_flags)
	{
		p_flags = 0;
		get_name(p_index,p_text);
		return true;
	}
};

// We need to create a service factory for our menu item class,
// otherwise the menu commands won't be known to the system.
static mainmenu_commands_factory_t< mainmenu_commands_tutorial1 > foo_menu;

#endif
