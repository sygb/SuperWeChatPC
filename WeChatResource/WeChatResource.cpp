// WeChatResource.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "util.h"

#define WECHATRESOURCE TEXT("WeChatResource.dll.1")
#define WECHATWINDLL TEXT("WeChatWin.dll")

typedef struct _FAKE_WX_CODE
{
	DWORD orig_code_count;
	BYTE orig_code[100];
	DWORD fake_code_count;
	BYTE fake_code[100];
}FakeWxCode, *PFakeWxCode;

typedef struct _SUP_WX_CFG
{
	const TCHAR* version;
	DWORD revoke_offset;
	FakeWxCode code;
}SuppWxCfg, *PSuppWxCfg;

const SuppWxCfg g_Supported_wx_Version[] = {
	{ TEXT("2.6.5.38"), 0x247EF1 ,{3, {0x8A, 0x45, 0xF3}, 3, {0x33, 0xc0, 0x90}}},
	{ TEXT("2.6.6.25"), 0x24BA81 ,{3, {0x8A, 0x45, 0xF3}, 3, {0x33, 0xc0, 0x90}}},
	{ TEXT("2.6.6.28"), 0x24B451 ,{3, {0x8A, 0x45, 0xF3}, 3, {0x33, 0xc0, 0x90}}},
};

bool IsSupportedWxVersion(
	DWORD* offset,
	BYTE orig_code[] = NULL,
	DWORD* orig_code_count = NULL,
	BYTE fake_code[] = NULL,
	DWORD* fake_code_count = NULL)
{
	TCHAR tszDllPath[MAX_PATH] = { 0 };

	GetModuleFileName(NULL, tszDllPath, MAX_PATH);
	PathRemoveFileSpec(tszDllPath);
	PathAppend(tszDllPath, WECHATWINDLL);

	TCHAR version[100] = { 0 };
	if (!GetFileVersion(tszDllPath, version))
	{
		return false;
	}

	for (int i = 0; i < ARRAYSIZE(g_Supported_wx_Version); i++) {
		if (!_tcsicmp(g_Supported_wx_Version[i].version, version)) {
			*offset = g_Supported_wx_Version[i].revoke_offset;
			if (orig_code) {
				memcpy(orig_code, g_Supported_wx_Version[i].code.orig_code, g_Supported_wx_Version[i].code.orig_code_count);
			}
			if (fake_code) {
				memcpy(fake_code, g_Supported_wx_Version[i].code.fake_code, g_Supported_wx_Version[i].code.fake_code_count);
			}
			if (orig_code_count) {
				*orig_code_count = g_Supported_wx_Version[i].code.orig_code_count;
			}
			if (fake_code_count) {
				*fake_code_count = g_Supported_wx_Version[i].code.fake_code_count;
			}
			return true;
		}
	}

	return false;
}

/* //2.6.5.38
text:10247EF1 8A 45 F3                                      mov     al, [ebp+var_D]
*/

bool FakeRevokeMsg()
{
	DWORD offset = 0x247EF1;
	//33 C0                xor eax,eax 
	BYTE code[] = { 0x33, 0xc0, 0x90 };
	DWORD code_count = 3;

	if (!IsSupportedWxVersion(&offset, NULL, NULL, code, &code_count)) {
		return false;
	}
	
	HMODULE hMod = GetModuleHandle(WECHATWINDLL);
	if (!hMod) {
		return false;
	}

	PVOID addr = (BYTE*)hMod + offset;
	Patch(addr, code_count, code);

	return true;
}

void RestoreRevokeMsg()
{
	DWORD offset = 0x247EF1;
	BYTE code[] = { 0x8A, 0x45, 0xF3 };
	DWORD code_count = 3;

	if (!IsSupportedWxVersion(&offset, code, &code_count)) {
		return;
	}
	
	HMODULE hMod = GetModuleHandle(WECHATWINDLL);
	if (!hMod) {
		return;
	}

	PVOID addr = (BYTE*)hMod + offset;
	Patch(addr, code_count, code);
}