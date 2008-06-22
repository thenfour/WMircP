/////////////////////////////////////////////////////////////////////////////
//
// 0811 wmp2mircEvents.cpp : Implementation of CWmp2mirc events
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "0811 wmp2mirc.h"
#include "meat.h"
#include "DebugLog.h"


void CWmp2mirc::OnPlay()
{
  // if we are locked (property page could be doing stuff, or the timer proc), then just ignore the request.
  if(!m_cs.TryEnter())
  {
    g_pLog->Message(_T("WMircP is busy; ignoring the play event."));
    return;
  }

  IWMPMedia* pMedia = 0;
  m_spCore->get_currentMedia(&pMedia);

  if(!pMedia)
  {
    g_pLog->Message(_T("WMircP could not get a pointer to the current media."));
  }
  else
  {
    CComBSTR url;
    pMedia->get_sourceURL(&url);

    g_pLog->Message(LibCC::Format("OnPlay %").qs((BSTR)url));
    g_pLog->Indent();
    m.OnPlay(pMedia, (PCWSTR)url);
    pMedia->Release();
    g_pLog->Outdent();
  }

  m_cs.Leave();
}

void CWmp2mirc::PlayStateChange( long NewState )
{
  switch (NewState)
  {
	case wmppsPlaying:
    OnPlay();
    break;
  default:
    break;
  }
}

void CWmp2mirc::OpenStateChange( long NewState ) { }
void CWmp2mirc::AudioLanguageChange( long LangID ){ }
void CWmp2mirc::StatusChange(){ }
void CWmp2mirc::ScriptCommand( BSTR scType, BSTR Param ){ }
void CWmp2mirc::NewStream(){ }
void CWmp2mirc::Disconnect( long Result ){ }
void CWmp2mirc::Buffering( VARIANT_BOOL Start ){ }
void CWmp2mirc::Error(){ }
void CWmp2mirc::Warning( long WarningType, long Param, BSTR Description ){ }
void CWmp2mirc::EndOfStream( long Result ){ }
void CWmp2mirc::PositionChange( double oldPosition, double newPosition){ }
void CWmp2mirc::MarkerHit( long MarkerNum ){ }
void CWmp2mirc::DurationUnitChange( long NewDurationUnit ){ }
void CWmp2mirc::CdromMediaChange( long CdromNum ){ }
void CWmp2mirc::PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change ){ }
void CWmp2mirc::CurrentPlaylistChange( WMPPlaylistChangeEventType change ){ }
void CWmp2mirc::CurrentPlaylistItemAvailable( BSTR bstrItemName ){ }
void CWmp2mirc::MediaChange( IDispatch * Item ){ }
void CWmp2mirc::CurrentMediaItemAvailable( BSTR bstrItemName ){ }
void CWmp2mirc::CurrentItemChange( IDispatch *pdispMedia){ }
void CWmp2mirc::MediaCollectionChange(){ }
void CWmp2mirc::MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal ){ }
void CWmp2mirc::MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal ){ }
void CWmp2mirc::MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal){ }
void CWmp2mirc::PlaylistCollectionChange(){ }
void CWmp2mirc::PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName){ }
void CWmp2mirc::PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName){ }
void CWmp2mirc::PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted){ }
void CWmp2mirc::ModeChange( BSTR ModeName, VARIANT_BOOL NewValue){ }
void CWmp2mirc::MediaError( IDispatch * pMediaObject){ }
void CWmp2mirc::OpenPlaylistSwitch( IDispatch *pItem ){ }
void CWmp2mirc::DomainChange( BSTR strDomain){ }
void CWmp2mirc::SwitchedToPlayerApplication(){ }
void CWmp2mirc::SwitchedToControl(){ }
void CWmp2mirc::PlayerDockedStateChange(){ }
void CWmp2mirc::PlayerReconnect(){ }
void CWmp2mirc::Click( short nButton, short nShiftState, long fX, long fY ){ }
void CWmp2mirc::DoubleClick( short nButton, short nShiftState, long fX, long fY ){ }
void CWmp2mirc::KeyDown( short nKeyCode, short nShiftState ){ }
void CWmp2mirc::KeyUp( short nKeyCode, short nShiftState ) { }
void CWmp2mirc::KeyPress( short nKeyAscii ){ }
void CWmp2mirc::MouseDown( short nButton, short nShiftState, long fX, long fY ){ }
void CWmp2mirc::MouseMove( short nButton, short nShiftState, long fX, long fY ){ }
void CWmp2mirc::MouseUp( short nButton, short nShiftState, long fX, long fY ){ }