#pragma once

#include "DeviceBridge.h"

class iFS : public BasePluginInfo
{
public:
    iFS();
    ~iFS();
    bool Init();
    void SetLastError(std::wstring err) { m_lastError = err; }
    void SetLastError(HRESULT err);
    CDeviceBridge* GetBridge() { return m_pBridge; }
    HANDLE GetConsoleInput() { return m_hConIn; }

    virtual std::wstring GetLastError() { return L"XXX"; }//m_lastError; }
    virtual void GetDiskMenuItems(std::vector<MenuItem>& items);
    virtual BasePanel* CreatePanel(const GUID& guid);

    CComTypeInfoHolder tih;

private:
    CDeviceBridge* m_pBridge;
    std::wstring m_lastError;
    HANDLE m_hConIn;
};

struct CopyInfo
{
    bool move;
    std::wstring title;
    std::wstring verb;
    std::wstring src;
    std::wstring dst;
    ULONGLONG totalSize;
    size_t totalFiles;
    ULONGLONG completedSize;
    size_t completedFiles;
    ULONGLONG currentSize;
    HANDLE hConIn;
    bool interrupted;

    CopyInfo(HANDLE hConIn = NULL, bool move = false):
        move(move),
        totalSize(1),
        totalFiles(1),
        completedSize(0),
        completedFiles(0),
        currentSize(0),
        interrupted(false),
        hConIn(hConIn)
    {
        title = move ? L"Rename or move" : L"Copy";
        verb = move ? L"Moving" : L"Copying";
    }
};

class iFSPanel : public BasePanel, public IProgressCallback
{
public:
    iFSPanel(iFS* plugin, IDevice*);
    ~iFSPanel();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
    {
        if (InlineIsEqualGUID(riid, IID_IUnknown) ||
            InlineIsEqualGUID(riid, IID_IDispatch) ||
            InlineIsEqualGUID(riid, __uuidof(IProgressCallback)))
        {
            *ppvObject = (IProgressCallback*)this;
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef() { return 1; }
    virtual ULONG STDMETHODCALLTYPE Release() { return 1; }
    virtual HRESULT STDMETHODCALLTYPE UpdateProgress(ULONG progress);

// IDispatch
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
	{
		if (pctinfo == NULL) 
			return E_POINTER; 
		*pctinfo = 1;
		return S_OK;
	}
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
	{
		return m_plugin->tih.GetTypeInfo(itinfo, lcid, pptinfo);
	}
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
	{
		return m_plugin->tih.GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
	}
	STDMETHOD(Invoke)(DISPID dispidMember,  REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams,  VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
		return m_plugin->tih.Invoke((IDispatch*)this, dispidMember, riid, lcid,
		wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
	}

    void onDeviceRemoved();

    intptr_t SetDirectory(const std::wstring& dir, OPERATION_MODES mode);
    intptr_t MakeDirectory(const std::wstring& dir, OPERATION_MODES mode);
    intptr_t GetFindData(PluginPanelItem*& items, size_t& count, OPERATION_MODES mode);
    void FreeFindData(PluginPanelItem* items, size_t count);
    void GetOpenPanelInfo(OpenPanelInfo* info);
    intptr_t DeleteFiles(PluginPanelItem* items, size_t count, OPERATION_MODES mode);
    intptr_t ProcessKey(const INPUT_RECORD* pRec);
    intptr_t ProcessEvent(intptr_t evt, void* param);
    intptr_t GetFiles(PluginPanelItem* items, size_t count, bool move, const wchar_t** dstPath, OPERATION_MODES mode);
    intptr_t PutFiles(PluginPanelItem* items, size_t count, bool move, const wchar_t* srcPath, OPERATION_MODES mode);

    enum DialogResult
    {
        eCancel,
        eOK,
        eRetry,
        eSkip,
        eSkipAll
    };

    struct FileInfoEx
    {
        ULONG attributes;
        ULONGLONG creationTime;
        ULONGLONG modificationTime;
        ULONGLONG size;
        std::wstring name;
        std::vector<FileInfoEx> children;

        bool IsDirectory() { return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0; }
        int GetCount()
        {
            int count = 1;
            for (auto it = children.begin();it != children.end();it++)
                count += it->GetCount();
            return count;
        }
        ULONGLONG GetSize()
        {
            ULONGLONG size = this->size;
            for (auto it = children.begin();it != children.end();it++)
                size += it->GetSize();
            return size;
        }
    };

    HRESULT ListFiles(std::wstring path, std::vector<FileInfoEx>& files, bool recursive = false);
    HRESULT ListFilesWin(std::wstring path, std::vector<FileInfoEx>& files, bool recursive = false);

    intptr_t GetFindData(std::wstring path, PluginPanelItem*& items, size_t& count, OPERATION_MODES mode);

    intptr_t GetFiles(std::wstring path, std::vector<FileInfoEx>& files, bool move, std::wstring dstPath, OPERATION_MODES mode, bool& continueOnError);
    intptr_t PutFiles(std::wstring path, std::vector<FileInfoEx>& files, bool move, std::wstring dstPath, OPERATION_MODES mode, bool& continueOnError);
    intptr_t DeleteFiles(std::wstring path, std::vector<FileInfoEx>& files, OPERATION_MODES mode, bool& continueOnError, int& progress, int total);

    void UpdateSizes();

private:
    iFS* m_plugin;
    std::wstring m_path;
    std::wstring m_deviceName;
    std::wstring m_title;
    CComPtr<IDevice> m_pDevice;
    CopyInfo m_copyInfo;
    ULONGLONG m_totalSize;
    ULONGLONG m_freeSize;
};

