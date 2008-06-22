// tester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include "..\wmpplug.h"

// {EB7DACFF-8854-427D-AC59-F66AF79179BC}
static const IID CLSID_Wmp2mirc =
{ 0xeb7dacff, 0x8854, 0x427d, { 0xac, 0x59, 0xf6, 0x6a, 0xf7, 0x91, 0x79, 0xbc } };


int _tmain(int argc, _TCHAR* argv[])
{
  CoInitialize(0);
  {
    IWMPPluginUI* p;
    CoCreateInstance(CLSID_Wmp2mirc, 0, CLSCTX_INPROC_SERVER, __uuidof(IWMPPluginUI), (VOID**)&p);

    std::cout << "'p' for property page" << std::endl;
    std::cout << "'q' to quit" << std::endl;
//    std::cout << "'...' to format a command string" << std::endl;
    
    for(;;)
    {
      std::wstring s;
      std::getline(std::wcin, s);
      if(s == L"q") break;
      else if(s == L"p")
      {
        p->DisplayPropertyPage(0);
      }
      else
      {
        //VARIANT var;
        //p->GetProperty(s.c_str(), &var);
        //std::wcout << var.bstrVal << std::endl;
        //SysFreeString(var.bstrVal);
      }
    }

    p->Release();
  }
  CoUninitialize();
	return 0;
}

