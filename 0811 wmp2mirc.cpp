
#include "stdafx.h"
#include "0811 wmp2mirc.h"
#include "CPropertyDialog.h"

LibCC::Log* LibCC::g_pLog = 0;


//// {865DCA0D-4493-4392-9B52-09D2986008EF}
//static const IID IID_FooBarMirc =
//{ 0x865dca0d, 0x4493, 0x4392, { 0x9b, 0x52, 0x09, 0xd2, 0x98, 0x60, 0x08, 0xef } };
//

CWmp2mirc::CWmp2mirc() :
  m_dwAdviseCookie(0),
  m(m_cs)
{
  o.Load();
  RebuildMeat();
}

void CWmp2mirc::RebuildMeat()
{
  m_cs.Enter();
  m.Rebuild(o);
  m_cs.Leave();
}

HRESULT CWmp2mirc::FinalConstruct()
{
  INITCOMMONCONTROLSEX icc = { 0 };
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_LISTVIEW_CLASSES;
  InitCommonControlsEx(&icc);

#ifdef _DEBUG
	bool debug = true;
#else
	bool debug = false;
#endif

  m_log.Create(L"WMircP.log", _Module.GetModuleInstance(), debug, debug, true, true, false, true);
  return S_OK;
}

void CWmp2mirc::FinalRelease()
{
  ReleaseCore();
}

HRESULT CWmp2mirc::SetCore(IWMPCore *pCore)
{
  HRESULT hr = S_OK;

  // release any existing WMP core interfaces
  ReleaseCore();

  // If we get passed a NULL core, this  means
  // that the plugin is being shutdown.
  if (pCore == NULL)
  {
    return S_OK;
  }

  m_spCore = pCore;

  // connect up the event interface. we hook it up either the simple way (if this is the foobar plugin), or the complex COM event sink way (for WMP)

  hr = m_spCore.QueryInterface(&m_foobarPlugin);
	if(SUCCEEDED(hr))
	{
		m_foobarPlugin->SetEventHandler(this);
	}
	else
	{
		CComPtr<IConnectionPointContainer>  spConnectionContainer;
		hr = m_spCore->QueryInterface( &spConnectionContainer );
		if (SUCCEEDED(hr))
		{
			hr = spConnectionContainer->FindConnectionPoint( __uuidof(IWMPEvents), &m_spConnectionPoint );
		}

		if (SUCCEEDED(hr))
		{
			hr = m_spConnectionPoint->Advise( GetUnknown(), &m_dwAdviseCookie );

			if ((FAILED(hr)) || (0 == m_dwAdviseCookie))
			{
				m_spConnectionPoint = NULL;
			}
		}
	}
  return hr;
}

void CWmp2mirc::ReleaseCore()
{
  if (m_spConnectionPoint)
  {
    if (0 != m_dwAdviseCookie)
    {
      m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
      m_dwAdviseCookie = 0;
    }
    m_spConnectionPoint = NULL;
  }

  if (m_spCore)
  {
    m_spCore = NULL;
  }
}


class FakeMedia :
  public IWMPMedia
{
  typedef std::pair<std::wstring, std::wstring> Attribute;
public:
  FakeMedia()
  {
    m_attributes.push_back(Attribute(L"Title", L"omg title!"));
    m_attributes.push_back(Attribute(L"Genre", L"jazzy pop thingy!"));
    m_attributes.push_back(Attribute(L"Duration", L"287"));
    m_attributes.push_back(Attribute(L"Filename", L"C:\\test\\shit.wma"));
  }
  // IUnknown methods
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject){ return E_FAIL; }
  ULONG STDMETHODCALLTYPE AddRef() { return 0; }
  ULONG STDMETHODCALLTYPE Release() { return 0; }

  // IDispatch methods
  HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) { return E_FAIL; }

  // IWMPMedia methods
  HRESULT STDMETHODCALLTYPE get_isIdentical(IWMPMedia *pIWMPMedia, VARIANT_BOOL *pvbool) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE get_sourceURL(BSTR *pbstrSourceURL) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE get_name(BSTR *pbstrName) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE put_name(BSTR bstrName) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE get_imageSourceWidth(long *pWidth) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE get_imageSourceHeight(long *pHeight) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE get_markerCount(long *pMarkerCount) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE getMarkerTime(long MarkerNum, double *pMarkerTime) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE getMarkerName(long MarkerNum, BSTR *pbstrMarkerName) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE get_duration(double *pDuration) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE get_durationString(BSTR *pbstrDuration) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE get_attributeCount(long *plCount)
  {
    *plCount = m_attributes.size();
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE getAttributeName(long lIndex, BSTR *pbstrItemName)
  {
    if(lIndex < 0 || lIndex >= m_attributes.size()) return E_FAIL;
    *pbstrItemName = SysAllocString(m_attributes[lIndex].second.c_str());
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE getItemInfo(BSTR bstrItemName, BSTR *pbstrVal)
  {
    for(std::vector<Attribute>::iterator it = m_attributes.begin(); it != m_attributes.end(); ++ it)
    {
      if(LibCC::StringEqualsI(it->first, bstrItemName))
      {
        *pbstrVal = SysAllocString(it->second.c_str());
        return S_OK;
      }
    }
    return E_FAIL;
  }
  HRESULT STDMETHODCALLTYPE setItemInfo(BSTR bstrItemName, BSTR bstrVal) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE getItemInfoByAtom(long lAtom, BSTR *pbstrVal) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE isMemberOf(IWMPPlaylist *pPlaylist, VARIANT_BOOL *pvarfIsMemberOf) { return E_FAIL; }
  HRESULT STDMETHODCALLTYPE isReadOnlyItem(BSTR bstrItemName, VARIANT_BOOL *pvarfIsReadOnly) { return E_FAIL; }

  std::vector<Attribute> m_attributes;
};


HRESULT CWmp2mirc::DisplayPropertyPage(HWND hwndParent)
{
  m_cs.Enter();

  FakeMedia fm;
  IWMPMedia* pMedia = &fm;
  if(!(m_spCore == 0))
  {
    m_spCore->get_currentMedia(&pMedia);
  }
  CPropertyDialog dialog(o, pMedia);
  dialog.DoModal(hwndParent);
  m_cs.Leave();

  if(pMedia)
  {
    pMedia->Release();
  }

  // do we need to reset stuff?
  if(dialog.ChangedOptions())
  {
    RebuildMeat();
    //
  }
  return S_OK;
}

bool IsPressed(int vk)
{
  return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

HRESULT CWmp2mirc::Create(HWND hwndParent, HWND *phwndWindow)
{
  return E_NOTIMPL;
}

HRESULT CWmp2mirc::Destroy()
{
  return E_NOTIMPL;
}

HRESULT CWmp2mirc::TranslateAccelerator(LPMSG msg)
{
  //if(msg->message == WM_KEYDOWN)
  //{
  //  if(msg->wParam == o.HotKeyCode())
  //  {
  //    bool bOK = false;
  //    bool alt = IsPressed(VK_MENU) || IsPressed(VK_LMENU) || IsPressed(VK_RMENU);
  //    bool ctrl = IsPressed(VK_CONTROL) || IsPressed(VK_LCONTROL) || IsPressed(VK_RCONTROL);
  //    bool shift = IsPressed(VK_SHIFT) || IsPressed(VK_LSHIFT) || IsPressed(VK_RSHIFT);

  //    // see which modifiers are satisfied.  if a key should NOT be pressed, then just flip its state to
  //    // see whether it's satisfied.
  //    // i do not support HOTKEYF_EXT.
  //    if(0 == (o.HotKeyFlags() & HOTKEYF_ALT))
  //    {
  //      alt = !alt;
  //    }
  //    if(0 == (o.HotKeyFlags() & HOTKEYF_CONTROL))
  //    {
  //      ctrl = !ctrl;
  //    }
  //    if(0 == (o.HotKeyFlags() & HOTKEYF_SHIFT))
  //    {
  //      shift = !shift;
  //    }
  //    
  //    if(alt && shift && ctrl)
  //    {
  //      // BLAM.
  //      DisplayPropertyPage(0);
  //    }
  //  }
  //}
  return S_FALSE;
}

HRESULT CWmp2mirc::GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty)
{
  //FakeMedia fm;
  //mirc::MediaInfoWrapper mi(&fm);
  //std::wstring out = mirc::ProcessCommandString(mi, pwszName);
  //pvarProperty->vt = VT_BSTR;
  //pvarProperty->bstrVal = SysAllocString(out.c_str());
  //return S_OK;

  if (NULL == pvarProperty)
  {
    return E_POINTER;
  }

  return E_NOTIMPL;
}

HRESULT CWmp2mirc::SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty)
{
  return E_NOTIMPL;
}
