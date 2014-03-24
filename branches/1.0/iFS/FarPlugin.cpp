#include "stdafx.h"
#include "FarPlugin.h"

#define VER_MAJOR 1
#define VER_MINOR 1
#define VER_REVISION 2
#define VER_BUILD 2

//---------------------------------------------------------------------------//
// {34FCA795-CDDD-47C3-9507-45D4D40BD794}
DECLARE_GUID(kPluginId, 0x34fca795, 0xcddd, 0x47c3, 0x95, 0x7, 0x45, 0xd4, 0xd4, 0xb, 0xd7, 0x94);

//---------------------------------------------------------------------------//
PluginStartupInfo s_startupInfo;
static BasePluginInfo* s_pluginInfo = NULL;
FarStandardFunctions s_stdFunc;

//---------------------------------------------------------------------------//
void ShowError(std::wstring message)
{
    GUID msgid;
    CoCreateGuid(&msgid);
    std::wstring str = L"Error\n";
    str += message;
    s_startupInfo.Message(&kPluginId, &msgid, FMSG_WARNING|FMSG_MB_OK|FMSG_ALLINONE, NULL, (const wchar_t**)str.c_str(), NULL, 0);
}

//---------------------------------------------------------------------------//
__int64 GetSetting(FARSETTINGS_SUBFOLDERS root, const wchar_t* name)
{
    __int64 result = 0;
    FarSettingsCreate settings = { sizeof(FarSettingsCreate), FarGuid, INVALID_HANDLE_VALUE };
    HANDLE hSettings = s_startupInfo.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, PSL_ROAMING, &settings) ? settings.Handle : 0;
    if (hSettings)
    {
        FarSettingsItem item = { sizeof(FarSettingsItem), root, name, FST_UNKNOWN, {0} };
        if (s_startupInfo.SettingsControl(hSettings, SCTL_GET, 0, &item) && FST_QWORD == item.Type)
            result = item.Number;
        s_startupInfo.SettingsControl(hSettings, SCTL_FREE, 0, 0);
    }
    return result;
}

//---------------------------------------------------------------------------//
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    return TRUE; 
}

//---------------------------------------------------------------------------//
void WINAPI GetGlobalInfoW(GlobalInfo *pInfo)
{
    pInfo->StructSize = sizeof(GlobalInfo);
    pInfo->MinFarVersion = MAKEFARVERSION(3, 0, 0, 0, VS_RELEASE);
    pInfo->Version = MAKEFARVERSION(VER_MAJOR, VER_MINOR, VER_REVISION, VER_BUILD, VS_RELEASE);
    pInfo->Guid = kPluginId;
    pInfo->Title = L"iFS";
    pInfo->Description = L"FAR iOS Device Viewer";
    pInfo->Author = L"Evgeny S";
}

//---------------------------------------------------------------------------//
void WINAPI GetPluginInfoW(struct PluginInfo *pInfo)
{
    if (pInfo->StructSize < sizeof(PluginInfo))
        return;

    pInfo->StructSize = sizeof(*pInfo);
    pInfo->Flags = PF_PRELOAD;

    if (!s_pluginInfo)
        s_pluginInfo = CreatePluginInfo();

    if (s_pluginInfo)
    {
        static std::vector<MenuItem> items;
        s_pluginInfo->GetDiskMenuItems(items);

        if (!items.empty())
        {
            static std::vector<GUID> guids;
            static std::vector<const wchar_t*> strings;
            guids.clear();
            strings.clear();
            for (std::vector<MenuItem>::iterator it = items.begin();it != items.end();it++)
            {
                guids.push_back(it->guid);
                strings.push_back(it->name.c_str());
            }
            pInfo->DiskMenu.Guids = &guids[0];
            pInfo->DiskMenu.Strings = &strings[0];
            pInfo->DiskMenu.Count = guids.size();
        }
    }
}

//---------------------------------------------------------------------------//
void WINAPI ExitFARW(const ExitInfo *pInfo)
{
    if (s_pluginInfo)
    {
        delete s_pluginInfo;
        s_pluginInfo = NULL;
    }
}

//---------------------------------------------------------------------------//
void WINAPI SetStartupInfoW(const struct PluginStartupInfo *pInfo)
{
    if (pInfo->StructSize < sizeof(PluginStartupInfo))
        return;

    s_startupInfo = *pInfo;
    s_stdFunc = *pInfo->FSF;
}

//---------------------------------------------------------------------------//
HANDLE WINAPI OpenW(const OpenInfo *pInfo)
{
    if (pInfo->StructSize < sizeof(OpenInfo))
        return NULL;

    if (pInfo->OpenFrom == OPEN_LEFTDISKMENU || pInfo->OpenFrom == OPEN_RIGHTDISKMENU)
    {
        assert(s_pluginInfo);
        BasePanel* panel = s_pluginInfo->CreatePanel(*pInfo->Guid);
        if (panel)
            return (HANDLE)panel;
        ShowError(s_pluginInfo->GetLastError());
    }
    return NULL;
}

//---------------------------------------------------------------------------//
void WINAPI ClosePanelW(const ClosePanelInfo* pInfo)
{
    delete (BasePanel*)pInfo->hPanel;
}

//-----------------------------------------------------------------------------
intptr_t WINAPI GetFindDataW(GetFindDataInfo *pInfo)
{
    BasePanel* panel = (BasePanel*)pInfo->hPanel;
    return panel->GetFindData(pInfo->PanelItem, pInfo->ItemsNumber, pInfo->OpMode);
}

//-----------------------------------------------------------------------------
void WINAPI FreeFindDataW(const FreeFindDataInfo *pInfo)
{
    BasePanel* panel = (BasePanel*)pInfo->hPanel;
    panel->FreeFindData(pInfo->PanelItem, pInfo->ItemsNumber);
}

//-----------------------------------------------------------------------------
void WINAPI GetOpenPanelInfoW(OpenPanelInfo *pInfo)
{
    BasePanel* panel = (BasePanel*)pInfo->hPanel;
    panel->GetOpenPanelInfo(pInfo);
}

//-----------------------------------------------------------------------------
intptr_t WINAPI SetDirectoryW(const struct SetDirectoryInfo *pInfo)
{
    BasePanel* panel = (BasePanel*)pInfo->hPanel;
    return panel->SetDirectory(pInfo->Dir, pInfo->OpMode);
}

//-----------------------------------------------------------------------------
intptr_t WINAPI DeleteFilesW(const struct DeleteFilesInfo *pInfo)
{
    BasePanel* panel = (BasePanel*)pInfo->hPanel;
    return panel->DeleteFiles(pInfo->PanelItem, pInfo->ItemsNumber, pInfo->OpMode);
}

//-----------------------------------------------------------------------------
intptr_t WINAPI ProcessPanelInputW(const ProcessPanelInputInfo *pInfo)
{
    BasePanel* panel = (BasePanel*)pInfo->hPanel;
    return panel->ProcessKey(&pInfo->Rec);
}

//-----------------------------------------------------------------------------
intptr_t WINAPI ProcessPanelEventW(const ProcessPanelEventInfo *pInfo)
{
    BasePanel* panel = (BasePanel*)pInfo->hPanel;
    return panel->ProcessEvent(pInfo->Event, pInfo->Param);
}

//-----------------------------------------------------------------------------
intptr_t WINAPI MakeDirectoryW(struct MakeDirectoryInfo *pInfo)
{
    BasePanel* panel = (BasePanel*)pInfo->hPanel;
    return panel->MakeDirectory(pInfo->Name, pInfo->OpMode);
}

//-----------------------------------------------------------------------------
intptr_t WINAPI GetFilesW(struct GetFilesInfo *pInfo)
{
  BasePanel* panel = (BasePanel*)pInfo->hPanel;
  return panel->GetFiles(pInfo->PanelItem, pInfo->ItemsNumber, pInfo->Move != 0, &pInfo->DestPath, pInfo->OpMode);
}

//-----------------------------------------------------------------------------
intptr_t WINAPI PutFilesW(const struct PutFilesInfo *pInfo)
{
  BasePanel* panel= (BasePanel*)pInfo->hPanel;
  return panel->PutFiles(pInfo->PanelItem, pInfo->ItemsNumber, pInfo->Move != 0, pInfo->SrcPath, pInfo->OpMode);
}

