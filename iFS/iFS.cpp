#include "StdAfx.h"
#include "FarPlugin.h"
#include "iFS.h"
#include "DeviceBridge.h"

// {1F085C43-61FD-4F7F-A11A-59528E4DB80A}
DECLARE_GUID(kConfirmDeleteId, 0x1f085c43, 0x61fd, 0x4f7f, 0xa1, 0x1a, 0x59, 0x52, 0x8e, 0x4d, 0xb8, 0xa);

// {140FE1FE-B750-489E-BCE9-B2F862237D7E}
DECLARE_GUID(kErrorId, 0x140fe1fe, 0xb750, 0x489e, 0xbc, 0xe9, 0xb2, 0xf8, 0x62, 0x23, 0x7d, 0x7e);


//---------------------------------------------------------------------------//
BasePluginInfo* CreatePluginInfo()
{
    iFS* ifs = new iFS();
    if (!ifs->Init())
    {
        delete ifs;
        return NULL;
    }
    return ifs;
}

//---------------------------------------------------------------------------//
std::wstring FitString(std::wstring str, size_t size)
{
    if (str.length() > size)
        str = str.substr(0, size / 2 - 1) + L"..." + str.substr(str.length() - size / 2 + 2);
    return str;
}

//---------------------------------------------------------------------------//
bool InputBox(std::wstring title, std::wstring message, std::wstring& value)
{
    GUID msgid;
    CoCreateGuid(&msgid);
    wchar_t buffer[1024];
    if (s_startupInfo.InputBox(&kPluginId, &msgid, title.c_str(), message.c_str(), NULL, value.c_str(), buffer, 1024, NULL, FIB_BUTTONS))
    {
        value = buffer;
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------//
std::wstring GetProgressString(size_t percent, size_t width)
{
    size_t len = percent * (width - 5) / 100;
    wchar_t str[12];
    wsprintf(str, L"%4d%%", percent);
    return std::wstring(len, 9608) + std::wstring(width - len - 5, 9617) + str;
}

//---------------------------------------------------------------------------//
std::wstring FormatSize(ULONGLONG size)
{
    if (size == 0)
        return L"0";
    std::wstring str;
    wchar_t buf[10];
    while (size)
    {
        size_t s = size % 1000;
        size /= 1000;
        wsprintf(buf, size ? L" %03d" : L"%d", s);
        str.insert(0, buf);
    }
    return str;
}

//---------------------------------------------------------------------------//
void ShowProgressDialog(std::wstring title, std::wstring file, int progress)
{
    GUID guid;
    CoCreateGuid(&guid);
    size_t width = 52;
    size_t len = progress * width / 100;
    std::wstring prefix = std::wstring(L"File: ");
    std::wstring str = title + L"\n" + prefix + FitString(file, width - prefix.length()) + L"\n" + GetProgressString(progress, width);
    s_startupInfo.Message(&kPluginId, &guid, FMSG_ALLINONE|FMSG_LEFTALIGN, NULL, (const wchar_t* const*)str.c_str(), 0, 0);
    s_startupInfo.AdvControl(&kPluginId, ACTL_SETPROGRESSSTATE, TBPS_NORMAL, 0);
    ProgressValue progressVal = { sizeof(ProgressValue), progress, 100 };
    s_startupInfo.AdvControl(&kPluginId, ACTL_SETPROGRESSVALUE, 0, &progressVal);
}

//---------------------------------------------------------------------------//
void ShowCopyProgressDialog(CopyInfo& info, size_t progress = 0)
{
    GUID guid;
    CoCreateGuid(&guid);
    size_t width = 52;
    wchar_t procStr[40];
    wsprintf(procStr, L"%d of %d", info.completedFiles, info.totalFiles);
    ULONGLONG completedSize = info.completedSize + info.currentSize * progress / 100;
    std::wstring str = info.title + L"\n" + info.verb + L" the file\n" + FitString(info.src, width) + L"\nto\n" + FitString(info.dst, width) + L"\n" +
        GetProgressString(progress, width) + L"\n\1Total: " + FormatSize(info.totalSize) + L"\n" + GetProgressString((size_t)(completedSize * 100 / info.totalSize), width) + L"\n" +
        L"Files processed: " + procStr;
    s_startupInfo.Message(&kPluginId, &guid, FMSG_ALLINONE|FMSG_LEFTALIGN, NULL, (const wchar_t* const*)str.c_str(), 0, 0);
    s_startupInfo.AdvControl(&kPluginId, ACTL_SETPROGRESSSTATE, TBPS_NORMAL, 0);
    ProgressValue progressVal = { sizeof(ProgressValue), completedSize, info.totalSize };
    s_startupInfo.AdvControl(&kPluginId, ACTL_SETPROGRESSVALUE, 0, &progressVal);

    INPUT_RECORD rec;
    DWORD count;
    while ((PeekConsoleInput(info.hConIn, &rec, 1, &count), count) == 1)
    {
        ReadConsoleInput(info.hConIn, &rec, 1, &count);
        if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE && rec.Event.KeyEvent.bKeyDown)
            info.interrupted = true;
    }
}

//---------------------------------------------------------------------------//
bool ConfirmDialog(std::wstring title, std::wstring message)
{
    GUID guid;
    CoCreateGuid(&guid);
    std::wstring str = title + L"\n" + message + L"\nYes\nCancel";
    return s_startupInfo.Message(&kPluginId, &guid, FMSG_ALLINONE, NULL, (const wchar_t* const*)str.c_str(), 0, 2) == 0;
}

//---------------------------------------------------------------------------//
int ErrorDialog(HRESULT hr, std::wstring description, std::wstring path, bool multiple = false)
{
    GUID guid;
    CoCreateGuid(&guid);
    WCHAR message[300];
    wsprintf(message, L"0x%08x", hr);
    std::wstring str = std::wstring(L"Error\n") + message + L"\n" + description + L"\n" + FitString(path, 50) + (multiple ? L"\nRetry\nSkip\nSkip All\nCancel" : L"\nRetry\nCancel");
    return (int)s_startupInfo.Message(&kPluginId, &guid, FMSG_ALLINONE|FMSG_WARNING, NULL, (const wchar_t* const*)str.c_str(), 0, multiple ? 4 : 2);
}

///////////////////////////////////////////////////////////////////////////////
iFS::iFS():
    m_pBridge(NULL),
    m_hConIn(NULL)
{
}

//---------------------------------------------------------------------------//
iFS::~iFS()
{
    delete m_pBridge;
    if (m_hConIn)
        CloseHandle(m_hConIn);
}

//---------------------------------------------------------------------------//
bool iFS::Init()
{
    m_hConIn = ::CreateFile(L"CONIN$", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    CDeviceBridge* pBridge = new CDeviceBridge();
    HRESULT hr = pBridge->Init();
    if (SUCCEEDED(hr))
    {
        m_pBridge = pBridge;
        return true;
    }
    else
    {
        delete pBridge;
        return false;
    }
}

//---------------------------------------------------------------------------//
void iFS::SetLastError(HRESULT err)
{
    wchar_t str[256];
    wsprintf(str, L"Operation failed (HRESULT: 0x%x)", err);
    m_lastError = str;
}

//---------------------------------------------------------------------------//
void iFS::GetDiskMenuItems(std::vector<MenuItem>& items)
{
    std::vector<DeviceData> devices;
    items.clear();
    HRESULT hr = m_pBridge->GetDeviceList(devices);
    if (SUCCEEDED(hr))
    {
        for (std::vector<DeviceData>::iterator it = devices.begin();it != devices.end();it++)
        {
            MenuItem item;
            item.guid = it->id;
            item.name = it->name;
            items.push_back(item);
        }
    }
}

//---------------------------------------------------------------------------//
BasePanel* iFS::CreatePanel(const GUID& guid)
{
    std::vector<DeviceData> devices;
    HRESULT hr = m_pBridge->GetDeviceList(devices);
    if (SUCCEEDED(hr))
    {
        for (std::vector<DeviceData>::iterator it = devices.begin();it != devices.end();it++)
            if (InlineIsEqualGUID(it->id, guid))
            {
                iFSPanel* panel = new iFSPanel(this, it->device);
                return panel;
            }
    }
    return NULL;
}


//---------------------------------------------------------------------------//
iFSPanel::iFSPanel(iFS* plugin, IDevice* pDevice):
    m_plugin(plugin),
    m_pDevice(pDevice)
{
    CComBSTR name;
    pDevice->get_Name(&name);
    m_deviceName = name;
    m_title = m_deviceName + L":/";
    m_totalSize = 0;
    m_freeSize = 0;
    UpdateSizes();
}

//---------------------------------------------------------------------------//
iFSPanel::~iFSPanel()
{
}

//---------------------------------------------------------------------------//
intptr_t iFSPanel::SetDirectory(const std::wstring& dir, OPERATION_MODES mode)
{
    if (dir.empty())
        return 0;
    if (dir == L"\\")
        m_path.clear();
    else if (dir == L"..")
    {
        if (m_path.empty())
            return 0;
        size_t pos = m_path.find_last_of('/');
        if (pos != std::wstring::npos)
            m_path.erase(pos);
        else
            m_path.clear();
    }
    else
    {
        if (!m_path.empty())
            m_path += L"/";
        m_path += dir;
    }
    m_title = m_deviceName + L":/" + m_path;
    return 1;
}

//---------------------------------------------------------------------------//
intptr_t iFSPanel::MakeDirectory(const std::wstring& dir, OPERATION_MODES mode)
{
    std::wstring folder = dir;
    if (!folder.empty() || InputBox(L"Make Folder", L"Create the folder:", folder))
    {
        std::wstring path = m_path + L"/" + folder;
        HRESULT hr = m_pDevice->CreateDirectory(CComBSTR(path.c_str()));
        if (FAILED(hr))
            ErrorDialog(hr, L"Cannot create folder", folder);
        return 1;
    }
    return -1;
}

//---------------------------------------------------------------------------//
HRESULT iFSPanel::ListFiles(std::wstring path, std::vector<FileInfoEx>& files, bool recursive)
{
    LPSAFEARRAY psa = m_plugin->GetBridge()->CreateFileInfoArray();
    HRESULT hr = m_pDevice->ListFiles(CComBSTR(path.c_str()), psa);
    if (SUCCEEDED(hr))
    {
        LONG cnt = 0;
        SafeArrayGetUBound(psa, 1, &cnt);
        cnt++;

        FileInfo* pData = NULL;
        hr = SafeArrayAccessData(psa, (void**)&pData);
        if (SUCCEEDED(hr))
        {
            for (LONG i = 0;i < cnt;i++)
            {
                files.push_back(FileInfoEx());
                FileInfoEx& file = files.back();
                FileInfo& info = pData[i];
        
                file.creationTime = info.creationTime;
                file.modificationTime = info.modificationTime;
                file.size = info.size;
                file.name = info.name;
                file.attributes = info.attributes;
                if (file.IsDirectory() && recursive)
                    ListFiles(path + L"/" + file.name, file.children, true);
            }
            SafeArrayUnaccessData(psa);
        }
        SafeArrayDestroy(psa);
    }
    return hr;
}


//---------------------------------------------------------------------------//
HRESULT iFSPanel::ListFilesWin(std::wstring path, std::vector<FileInfoEx>& files, bool recursive)
{
    WIN32_FIND_DATA wfd;
    HANDLE hFind = FindFirstFile((path + L"\\*.*").c_str(), &wfd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (wfd.cFileName[0] == '.' && (wfd.cFileName[1] == 0 || (wfd.cFileName[1] == '.' && wfd.cFileName[2] == 0)))
                continue;

            files.push_back(FileInfoEx());
            FileInfoEx& file = files.back();
        
            file.creationTime = ((uint64_t)wfd.ftCreationTime.dwHighDateTime << 32) | wfd.ftCreationTime.dwLowDateTime;
            file.modificationTime = ((uint64_t)wfd.ftLastWriteTime.dwHighDateTime << 32) | wfd.ftLastWriteTime.dwLowDateTime;
            file.size = ((uint64_t)wfd.nFileSizeHigh << 32) | wfd.nFileSizeLow;
            file.name = wfd.cFileName;
            file.attributes = wfd.dwFileAttributes;
            if (file.IsDirectory() && recursive)
                ListFilesWin(path + L"\\" + file.name, file.children, true);
        }
        while (FindNextFile(hFind, &wfd));
        return S_OK;
    }
    else
        return HRESULT_FROM_WIN32(GetLastError());
}


//---------------------------------------------------------------------------//
intptr_t iFSPanel::GetFindData(PluginPanelItem*& items, size_t& count, OPERATION_MODES mode)
{
    std::vector<FileInfoEx> files;
    ListFiles(m_path, files);
    items = new PluginPanelItem[count = files.size()];
    memset(items, 0, sizeof(PluginPanelItem) * count);
    for (size_t i = 0;i < count;i++)
    {
        PluginPanelItem& item = items[i];
        FileInfoEx& file = files[i];
        
        item.CreationTime.dwLowDateTime = (DWORD)file.creationTime;
        item.CreationTime.dwHighDateTime = (DWORD)(file.creationTime >> 32);
        item.LastWriteTime.dwLowDateTime = (DWORD)file.modificationTime;
        item.LastWriteTime.dwHighDateTime = (DWORD)(file.modificationTime >> 32);
        item.FileSize = file.size;
        item.FileName = _wcsdup(file.name.c_str());
        item.Flags = PPIF_NONE;
        item.FileAttributes = file.attributes;
    }
    return TRUE;
}

//---------------------------------------------------------------------------//
void iFSPanel::FreeFindData(PluginPanelItem* items, size_t count)
{
    for (size_t i = 0;i < count;i++)
    {
        PluginPanelItem& item = items[i];
        free((void*)item.FileName);
    }
    delete[] items;
}

//---------------------------------------------------------------------------//
void iFSPanel::GetOpenPanelInfo(OpenPanelInfo* info)
{
    info->StructSize = sizeof(OpenPanelInfo);
    info->Flags = OPIF_SHOWPRESERVECASE|OPIF_ADDDOTS|OPIF_USEFREESIZE;
    info->HostFile = NULL;
    info->CurDir = m_path.c_str();
    info->Format = m_deviceName.c_str();
    info->PanelTitle = m_title.c_str();
    info->InfoLines = NULL;
    info->InfoLinesNumber = 0;
    info->DescrFiles = NULL;
    info->DescrFilesNumber = 0;
    info->PanelModesArray = NULL;
    info->PanelModesNumber = 0;
    info->StartPanelMode = 0;
    info->StartSortMode = SM_DEFAULT;
    info->StartSortOrder = 0;
    info->KeyBar = NULL;
    info->ShortcutData = NULL;
    info->FreeSize = m_freeSize;
}

//---------------------------------------------------------------------------//
intptr_t iFSPanel::DeleteFiles(std::wstring path, std::vector<FileInfoEx>& files, OPERATION_MODES mode, bool& continueOnError, int& progress, int total)
{
    for (size_t i = 0;i < files.size();i++)
    {
        std::wstring subPath = path + L"/" + files[i].name;
        ShowProgressDialog(L"Deleting", subPath, progress * 100 / total);
        if (files[i].IsDirectory())
        {
            if (!DeleteFiles(subPath, files[i].children, mode, continueOnError, progress, total))
                return FALSE;
        }

        HRESULT hr = m_pDevice->DeleteFile(CComBSTR(subPath.c_str()));
        if (FAILED(hr) && !continueOnError)
        {
            int result = ErrorDialog(hr, files[i].IsDirectory() ? L"Cannot delete folder" : L"Cannot delete file", subPath, true);
            if (result == 0)
                i--;
            else if (result == 2)
                continueOnError = true;
            else if (result != 1)
                return FALSE;
        }
        ShowProgressDialog(L"Deleting", subPath, ++progress * 100 / total);
    }
    return TRUE;
}

//---------------------------------------------------------------------------//
intptr_t iFSPanel::DeleteFiles(PluginPanelItem* items, size_t count, OPERATION_MODES mode)
{
    std::vector<FileInfoEx> files;
    int numFiles = 0, numFolders = 0;
    for (size_t i = 0;i < count;i++)
    {
        files.push_back(FileInfoEx());
        FileInfoEx& file = files.back();
        file.name = items[i].FileName;
        file.attributes = (ULONG)items[i].FileAttributes;
        if (file.IsDirectory())
            numFolders++;
        else
            numFiles++;
    }

    if ((GetSetting(FSSF_CONFIRMATIONS, L"Delete") && numFiles) ||
        (GetSetting(FSSF_CONFIRMATIONS, L"DeleteFolder") && numFolders))
    {
        WCHAR message[300];
        if (numFiles == 1 && numFolders == 0 || numFiles == 0 && numFolders == 1)
            wsprintf(message, L"Do you wish to delete \"%s\"?", items[0].FileName);
        else
            wsprintf(message, L"Do you wish to delete %d items?", count);

        if (!ConfirmDialog(L"Delete", message))
            return FALSE;
    }

    ShowProgressDialog(L"Deleting", L"", 0);
    s_startupInfo.AdvControl(&kPluginId, ACTL_SETPROGRESSSTATE, TBPS_INDETERMINATE, 0);

    int total = 0;
    for (size_t i = 0;i < count;i++)
    {
        FileInfoEx& file = files[i];
        if (file.IsDirectory())
            ListFiles(m_path + L"/" + file.name, file.children, true);
        total += file.GetCount();
    }

    bool continueOnError = false;
    int progress = 0;
    intptr_t result = DeleteFiles(m_path, files, mode, continueOnError, progress, total);
    s_startupInfo.AdvControl(&kPluginId, ACTL_REDRAWALL, 0, NULL);
    s_startupInfo.AdvControl(&kPluginId, ACTL_SETPROGRESSSTATE, TBPS_NOPROGRESS, 0);
    UpdateSizes();
    return result;
}

//---------------------------------------------------------------------------//
intptr_t iFSPanel::GetFiles(std::wstring path, std::vector<FileInfoEx>& files, bool move, std::wstring dstPath, OPERATION_MODES mode, bool& continueOnError)
{
    for (size_t i = 0;i < files.size();i++)
    {
        std::wstring sPath = path + L"/" + files[i].name;
        std::wstring dPath = dstPath + L"\\" + files[i].name;
        m_copyInfo.src = sPath;
        m_copyInfo.dst = dPath;

        ShowCopyProgressDialog(m_copyInfo);
        if (m_copyInfo.interrupted)
            return FALSE;
        if (files[i].IsDirectory())
        {
            if (!CreateDirectoryW(dPath.c_str(), NULL))
            {
                if (!continueOnError)
                {
                    int result = ErrorDialog(HRESULT_FROM_WIN32(GetLastError()), L"Cannot create folder", dPath, true);
                    if (result == 0)
                        i--;
                    else if (result == 2)
                        continueOnError = true;
                    else if (result != 1)
                        return FALSE;
                    continue;
                }
            }
            else
            {
                if (!GetFiles(sPath, files[i].children, move, dPath, mode, continueOnError))
                    return FALSE;
                if (move)
                    m_pDevice->DeleteFile(CComBSTR(sPath.c_str()));
            }
        }
        else
        {
            m_copyInfo.currentSize = files[i].size;
            HRESULT hr = m_pDevice->GetFile(CComBSTR(sPath.c_str()), CComBSTR(dPath.c_str()), this);
            m_copyInfo.currentSize = 0;
            if (hr == E_ABORT)
                return FALSE;
            if (FAILED(hr) && !continueOnError)
            {
                int result = ErrorDialog(hr, L"Cannot copy file", sPath, true);
                if (result == 0)
                    i--;
                else if (result == 2)
                    continueOnError = true;
                else if (result != 1)
                    return FALSE;
                continue;
            }
            if (SUCCEEDED(hr) && move)
            {
                hr = m_pDevice->DeleteFile(CComBSTR(sPath.c_str()));
                if (FAILED(hr) && !continueOnError)
                {
                    int result = ErrorDialog(hr, L"Cannot delete file", sPath, true);
                    if (result == 0)
                        i--;
                    else if (result == 2)
                        continueOnError = true;
                    else if (result != 1)
                        return FALSE;
                    continue;
                }
            }
        }
        m_copyInfo.completedSize += files[i].size;
        m_copyInfo.completedFiles++;
        ShowCopyProgressDialog(m_copyInfo, 100);
    }
    return TRUE;
}

//---------------------------------------------------------------------------//
intptr_t iFSPanel::GetFiles(PluginPanelItem* items, size_t count, bool move, const wchar_t** dstPath, OPERATION_MODES mode)
{
    std::vector<FileInfoEx> files;
    int numFiles = 0, numFolders = 0;
    for (size_t i = 0;i < count;i++)
    {
        files.push_back(FileInfoEx());
        FileInfoEx& file = files.back();
        file.name = items[i].FileName;
        file.size = items[i].FileSize;
        file.attributes = (ULONG)items[i].FileAttributes;
        if (file.IsDirectory())
            numFolders++;
        else
            numFiles++;
    }

    std::wstring folder = *dstPath;
    if (!is_silent(mode))
    {
        WCHAR message[300];
        if (numFiles == 1 && numFolders == 0 || numFiles == 0 && numFolders == 1)
            wsprintf(message, L"%s \"%s\" to:", move ? L"Rename or move" : L"Copy", items[0].FileName);
        else
            wsprintf(message, L"%s %d items to:", move ? L"Rename or move" : L"Copy", count);
        if (InputBox(move ? L"Rename/Move" : L"Copy", message, folder))
        {
            if (folder != *dstPath)
            {
                static std::wstring buffer;
                buffer = folder;
                *dstPath = buffer.c_str();
            }
        }
        else
            return -1;
    }

    m_copyInfo = CopyInfo(m_plugin->GetConsoleInput(), move);
    ShowCopyProgressDialog(m_copyInfo);
    s_startupInfo.AdvControl(&kPluginId, ACTL_SETPROGRESSSTATE, TBPS_INDETERMINATE, 0);

    for (size_t i = 0;i < count;i++)
    {
        FileInfoEx& file = files[i];
        if (file.IsDirectory())
            ListFiles(m_path + L"/" + file.name, file.children, true);
        m_copyInfo.totalFiles += file.GetCount();
        m_copyInfo.totalSize += file.GetSize();
    }

    bool continueOnError = false;
    intptr_t result = GetFiles(m_path, files, move, folder, mode, continueOnError);
    s_startupInfo.AdvControl(&kPluginId, ACTL_REDRAWALL, 0, NULL);
    s_startupInfo.AdvControl(&kPluginId, ACTL_SETPROGRESSSTATE, TBPS_NOPROGRESS, 0);
    return result ? 1 : -1;
}

//---------------------------------------------------------------------------//
intptr_t iFSPanel::PutFiles(std::wstring path, std::vector<FileInfoEx>& files, bool move, std::wstring dstPath, OPERATION_MODES mode, bool& continueOnError)
{
    for (size_t i = 0;i < files.size();i++)
    {
        std::wstring sPath = path + L"\\" + files[i].name;
        std::wstring dPath = dstPath + L"/" + files[i].name;
        m_copyInfo.src = sPath;
        m_copyInfo.dst = dPath;

        ShowCopyProgressDialog(m_copyInfo);
        if (m_copyInfo.interrupted)
            return FALSE;
        if (files[i].IsDirectory())
        {
            HRESULT hr = m_pDevice->CreateDirectory(CComBSTR(dPath.c_str()));
            if (FAILED(hr))
            {
                if (!continueOnError)
                {
                    int result = ErrorDialog(hr, L"Cannot create folder", dPath, true);
                    if (result == 0)
                        i--;
                    else if (result == 2)
                        continueOnError = true;
                    else if (result != 1)
                        return FALSE;
                    continue;
                }
            }
            else
            {
                if (!PutFiles(sPath, files[i].children, move, dPath, mode, continueOnError))
                    return FALSE;
                if (move)
                    RemoveDirectoryW(sPath.c_str());
            }
        }
        else
        {
            m_copyInfo.currentSize = files[i].size;
            HRESULT hr = m_pDevice->PutFile(CComBSTR(sPath.c_str()), CComBSTR(dPath.c_str()), this);
            m_copyInfo.currentSize = 0;
            if (hr == E_ABORT)
                return FALSE;
            if (FAILED(hr) && !continueOnError)
            {
                int result = ErrorDialog(hr, L"Cannot copy file", sPath, true);
                if (result == 0)
                    i--;
                else if (result == 2)
                    continueOnError = true;
                else if (result != 1)
                    return FALSE;
                continue;
            }
            if (SUCCEEDED(hr) && move)
            {
                if (!DeleteFileW(sPath.c_str()) && !continueOnError)
                {
                    int result = ErrorDialog(HRESULT_FROM_WIN32(GetLastError()), L"Cannot delete file", sPath, true);
                    if (result == 0)
                        i--;
                    else if (result == 2)
                        continueOnError = true;
                    else if (result != 1)
                        return FALSE;
                    continue;
                }
            }
        }
        m_copyInfo.completedSize += files[i].size;
        m_copyInfo.completedFiles++;
        ShowCopyProgressDialog(m_copyInfo, 100);
    }
    return TRUE;
}

//---------------------------------------------------------------------------//
intptr_t iFSPanel::PutFiles(PluginPanelItem* items, size_t count, bool move, const wchar_t* srcPath, OPERATION_MODES mode)
{
    std::vector<FileInfoEx> files;
    int numFiles = 0, numFolders = 0;
    for (size_t i = 0;i < count;i++)
    {
        files.push_back(FileInfoEx());
        FileInfoEx& file = files.back();
        file.name = items[i].FileName;
        file.size = items[i].FileSize;
        file.attributes = (ULONG)items[i].FileAttributes;
        if (file.IsDirectory())
            numFolders++;
        else
            numFiles++;
    }

    std::wstring folder = m_path;
    if (!is_silent(mode))
    {
        WCHAR message[300];
        if (numFiles == 1 && numFolders == 0 || numFiles == 0 && numFolders == 1)
            wsprintf(message, L"%s \"%s\" to:", move ? L"Rename or move" : L"Copy", items[0].FileName);
        else
            wsprintf(message, L"%s %d items to:", move ? L"Rename or move" : L"Copy", count);
        if (!InputBox(move ? L"Rename/Move" : L"Copy", message, folder))
            return -1;
    }

    m_copyInfo = CopyInfo(m_plugin->GetConsoleInput(), move);
    ShowCopyProgressDialog(m_copyInfo);
    s_startupInfo.AdvControl(&kPluginId, ACTL_SETPROGRESSSTATE, TBPS_INDETERMINATE, 0);

    for (size_t i = 0;i < count;i++)
    {
        FileInfoEx& file = files[i];
        if (file.IsDirectory())
            ListFilesWin(std::wstring(srcPath) + L"\\" + file.name, file.children, true);
        m_copyInfo.totalFiles += file.GetCount();
        m_copyInfo.totalSize += file.GetSize();
    }

    bool continueOnError = false;
    intptr_t result = PutFiles(std::wstring(srcPath), files, move, folder, mode, continueOnError);
    s_startupInfo.AdvControl(&kPluginId, ACTL_REDRAWALL, 0, NULL);
    s_startupInfo.AdvControl(&kPluginId, ACTL_SETPROGRESSSTATE, TBPS_NOPROGRESS, 0);
    UpdateSizes();
    return result ? 1 : -1;
}

//---------------------------------------------------------------------------//
intptr_t iFSPanel::ProcessKey(const INPUT_RECORD* pRec)
{
    return 0;
}

//---------------------------------------------------------------------------//
intptr_t iFSPanel::ProcessEvent(intptr_t evt, void* param)
{
    if (evt == FE_IDLE)
    {
        VARIANT_BOOL valid;
        HRESULT hr = m_pDevice->get_IsValid(&valid);
        if (FAILED(hr) || !valid)
            s_startupInfo.PanelControl((HANDLE)this, FCTL_CLOSEPANEL, 0, NULL);
    }
    return 0;
}

//---------------------------------------------------------------------------//
HRESULT iFSPanel::UpdateProgress(ULONG progress)
{
    ShowCopyProgressDialog(m_copyInfo, progress);
    return m_copyInfo.interrupted ? E_ABORT : S_OK;
}

//---------------------------------------------------------------------------//
void iFSPanel::UpdateSizes()
{
    DeviceInfo info;
    if (SUCCEEDED(m_pDevice->GetDeviceInfo(&info)))
    {
        m_totalSize = info.total;
        m_freeSize = info.free;
    }
}
