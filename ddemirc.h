

#pragma once

#include <windows.h>

inline HDDEDATA CALLBACK DdeCallback(UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD)
{
  return 0;
}

inline bool CallDDE(PCSTR srvName, PCSTR msg)
{
	DWORD idInst;
	HSZ hszService;
	HSZ hszTopic;
	HSZ hszItem;
	HCONV conversation;

	idInst = 0;
	UINT r = DdeInitialize(&idInst, (PFNCALLBACK) DdeCallback, APPCMD_CLIENTONLY, 0);

	hszService = DdeCreateStringHandleA(idInst, srvName, CP_WINANSI);
  if(0 == hszService)
  {
    UINT x = DdeGetLastError(idInst);
    return false;
  }

	hszTopic = DdeCreateStringHandleA(idInst, "COMMAND", CP_WINANSI);
  if(0 == hszTopic)
  {
    UINT x = DdeGetLastError(idInst);
    return false;
  }
	hszItem = DdeCreateStringHandleA(idInst, " ", CP_WINANSI);
  if(0 == hszItem)
  {
    UINT x = DdeGetLastError(idInst);
    return false;
  }

	conversation = DdeConnect(idInst, hszService, hszTopic, NULL);
  if(0 == conversation)
  {
    UINT x = DdeGetLastError(idInst);
    return false;
  }

  if(0 == DdeClientTransaction((BYTE*)msg, (DWORD)(strlen(msg) + 1) * sizeof(TCHAR), 
    conversation, hszItem, CF_TEXT, XTYP_POKE, 1000, NULL))
  {
    UINT x = DdeGetLastError(idInst);
    return false;
  }

	DdeDisconnect(conversation);

	DdeFreeStringHandle(idInst, hszService);
	DdeFreeStringHandle(idInst, hszTopic);
	DdeFreeStringHandle(idInst, hszItem);

	DdeUninitialize(idInst);

	return true;
}


inline std::string UnicodeToAscii(PCWSTR in)
{
  const char def = '?';
  int bytesRequired = WideCharToMultiByte(CP_ACP, 0, in, -1, 0, 0, NULL, NULL);
  char* out = new char[bytesRequired];
  WideCharToMultiByte(CP_ACP, 0, in, -1, out, bytesRequired, NULL, NULL);
  std::string ret;
  ret = out;
  delete [] out;
  return ret;
}


inline bool CallDDE(PCWSTR srvName, PCWSTR msg)
{
  std::string srvA = UnicodeToAscii(srvName);
  std::string msgA = UnicodeToAscii(msg);
  bool ret = CallDDE(srvA.c_str(), msgA.c_str());
  return 0;
}

