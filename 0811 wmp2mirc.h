/////////////////////////////////////////////////////////////////////////////
//
// 0811 wmp2mirc.h : Declaration of the CWmp2mirc
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __WMP2MIRC_H_
#define __WMP2MIRC_H_

#pragma once

#include "resource.h"
#include "wmpplug.h"
#include <string>
#include "..\libcc\libcc\timer.hpp"
#include "..\libcc\libcc\log.hpp"
#include <vector>
#include "meat.h"

// {EB7DACFF-8854-427D-AC59-F66AF79179BC}
DEFINE_GUID(CLSID_Wmp2mirc, 0xeb7dacff, 0x8854, 0x427d, 0xac, 0x59, 0xf6, 0x6a, 0xf7, 0x91, 0x79, 0xbc);


/////////////////////////////////////////////////////////////////////////////
// CWmp2mirc
class ATL_NO_VTABLE CWmp2mirc : 
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CWmp2mirc, &CLSID_Wmp2mirc>,
  public IWMPEvents,
  public IWMPPluginUI
{
public:
  CWmp2mirc();

DECLARE_REGISTRY_RESOURCEID(IDR_WMP2MIRC)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CWmp2mirc)
  COM_INTERFACE_ENTRY(IWMPEvents)
  COM_INTERFACE_ENTRY(IWMPPluginUI)
END_COM_MAP()

  // CComCoClass methods
  HRESULT FinalConstruct();
  void FinalRelease();

  // IWMPPluginUI methods
  STDMETHODIMP SetCore(IWMPCore *pCore);
  STDMETHODIMP Create(HWND hwndParent, HWND *phwndWindow);
  STDMETHODIMP Destroy();
  STDMETHODIMP TranslateAccelerator(LPMSG lpmsg);
  STDMETHODIMP DisplayPropertyPage(HWND hwndParent);
  STDMETHODIMP GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty);
  STDMETHODIMP SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty);

  void OnPlay();

  // IWMPEvents methods
  void STDMETHODCALLTYPE OpenStateChange( long NewState );
  void STDMETHODCALLTYPE PlayStateChange( long NewState );
  void STDMETHODCALLTYPE AudioLanguageChange( long LangID );
  void STDMETHODCALLTYPE StatusChange();
  void STDMETHODCALLTYPE ScriptCommand( BSTR scType, BSTR Param );
  void STDMETHODCALLTYPE NewStream();
  void STDMETHODCALLTYPE Disconnect( long Result );
  void STDMETHODCALLTYPE Buffering( VARIANT_BOOL Start );
  void STDMETHODCALLTYPE Error();
  void STDMETHODCALLTYPE Warning( long WarningType, long Param, BSTR Description );
  void STDMETHODCALLTYPE EndOfStream( long Result );
  void STDMETHODCALLTYPE PositionChange( double oldPosition, double newPosition);
  void STDMETHODCALLTYPE MarkerHit( long MarkerNum );
  void STDMETHODCALLTYPE DurationUnitChange( long NewDurationUnit );
  void STDMETHODCALLTYPE CdromMediaChange( long CdromNum );
  void STDMETHODCALLTYPE PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change );
  void STDMETHODCALLTYPE CurrentPlaylistChange( WMPPlaylistChangeEventType change );
  void STDMETHODCALLTYPE CurrentPlaylistItemAvailable( BSTR bstrItemName );
  void STDMETHODCALLTYPE MediaChange( IDispatch * Item );
  void STDMETHODCALLTYPE CurrentMediaItemAvailable( BSTR bstrItemName );
  void STDMETHODCALLTYPE CurrentItemChange( IDispatch *pdispMedia);
  void STDMETHODCALLTYPE MediaCollectionChange();
  void STDMETHODCALLTYPE MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal );
  void STDMETHODCALLTYPE MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal );
  void STDMETHODCALLTYPE MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal);
  void STDMETHODCALLTYPE PlaylistCollectionChange();
  void STDMETHODCALLTYPE PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName);
  void STDMETHODCALLTYPE PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName);
  void STDMETHODCALLTYPE PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted);
  void STDMETHODCALLTYPE ModeChange( BSTR ModeName, VARIANT_BOOL NewValue);
  void STDMETHODCALLTYPE MediaError( IDispatch * pMediaObject);
  void STDMETHODCALLTYPE OpenPlaylistSwitch( IDispatch *pItem );
  void STDMETHODCALLTYPE DomainChange( BSTR strDomain);
  void STDMETHODCALLTYPE SwitchedToPlayerApplication();
  void STDMETHODCALLTYPE SwitchedToControl();
  void STDMETHODCALLTYPE PlayerDockedStateChange();
  void STDMETHODCALLTYPE PlayerReconnect();
  void STDMETHODCALLTYPE Click( short nButton, short nShiftState, long fX, long fY );
  void STDMETHODCALLTYPE DoubleClick( short nButton, short nShiftState, long fX, long fY );
  void STDMETHODCALLTYPE KeyDown( short nKeyCode, short nShiftState );
  void STDMETHODCALLTYPE KeyPress( short nKeyAscii );
  void STDMETHODCALLTYPE KeyUp( short nKeyCode, short nShiftState );
  void STDMETHODCALLTYPE MouseDown( short nButton, short nShiftState, long fX, long fY );
  void STDMETHODCALLTYPE MouseMove( short nButton, short nShiftState, long fX, long fY );
  void STDMETHODCALLTYPE MouseUp( short nButton, short nShiftState, long fX, long fY );

  void ReleaseCore();

private:

  void RebuildMeat();// builds the meat from the options.
  Meat m;// where all the backgroudn crap is going on like timers and commands.
  Options o;

  CriticalSection m_cs;// protects the meat.

  CComPtr<IWMPCore> m_spCore;
  CComPtr<IConnectionPoint> m_spConnectionPoint;
  DWORD m_dwAdviseCookie;

  HWND m_hWnd;
	LibCC::Log m_log;
};

#endif //__WMP2MIRC_H_
