#pragma once

extern PluginStartupInfo s_startupInfo;
extern FarStandardFunctions s_stdFunc;
extern const GUID kPluginId;

inline bool is_silent(OPERATION_MODES mode) { return (mode & OPM_SILENT) != 0; }
__int64 GetSetting(FARSETTINGS_SUBFOLDERS root, const wchar_t* name);

class BasePanel
{
public:
    virtual ~BasePanel() {}

    virtual intptr_t SetDirectory(const std::wstring& dir, OPERATION_MODES mode) = 0;
    virtual intptr_t MakeDirectory(const std::wstring& dir, OPERATION_MODES mode) = 0;
    virtual intptr_t GetFindData(PluginPanelItem*& items, size_t& count, OPERATION_MODES mode) = 0;
    virtual void FreeFindData(PluginPanelItem* items, size_t count) = 0;
    virtual void GetOpenPanelInfo(OpenPanelInfo* info) = 0;
    virtual intptr_t DeleteFiles(PluginPanelItem* items, size_t count, OPERATION_MODES mode) = 0;
    virtual intptr_t ProcessKey(const INPUT_RECORD* pRec) = 0;
    virtual intptr_t ProcessEvent(intptr_t evt, void* param) = 0;
    virtual intptr_t GetFiles(PluginPanelItem* items, size_t count, bool move, const wchar_t** dstPath, OPERATION_MODES mode) = 0;
    virtual intptr_t PutFiles(PluginPanelItem* items, size_t count, bool move, const wchar_t* srcPath, OPERATION_MODES mode) = 0;
};

struct MenuItem
{
    GUID guid;
    std::wstring name;
};

class BasePluginInfo
{
public:
    virtual ~BasePluginInfo() {}

    virtual std::wstring GetLastError() = 0;
    virtual void GetDiskMenuItems(std::vector<MenuItem>& items) = 0;
    virtual BasePanel* CreatePanel(const GUID& guid) = 0;
};

BasePluginInfo* CreatePluginInfo();

