// 0811 wmp2mircdll.cpp : Implementation of DLL Exports.
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>

#include "0811 wmp2mirc.h"
#include "options.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Wmp2mirc, CWmp2mirc)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point
#ifdef _USRDLL
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}
#else
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR /*lpCmdLine*/, int nShowCmd)
{
  _Module.Init(ObjectMap, hInstance);
  {
  }
  _Module.Term();
  return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib

    HRESULT hr = _Module.RegisterServer();

    // Notify WMP that plugin has been added

    WMPNotifyPluginAddRemove();

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = _Module.UnregisterServer();

    // Notify WMP that plugin has been removed

    WMPNotifyPluginAddRemove();

    return hr;
}


