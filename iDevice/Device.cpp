// Device.cpp : Implementation of CDevice

#include "stdafx.h"
#include "AMDevice.h"
#include "Device.h"
#include "Utils.h"

#define BUF_SIZE 0x100000


// CDevice

STDMETHODIMP CDevice::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_IDevice
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


CDevice::CDevice():
    m_handle(NULL)
{
}

HRESULT CDevice::Init(AMDeviceRef handle)
{
    m_handle = handle;

    HRESULT hr = Connect();
    if (FAILED(hr))
    {
        m_handle = NULL;
        return hr;
    }
    
    CFStringPtr nameVal = AMDeviceCopyValue(handle, 0, CFSTR(L"DeviceName"));
    m_name = nameVal;

    CFStringPtr udidVal = AMDeviceCopyValue(handle, 0, CFSTR(L"UniqueDeviceID"));
    m_udid = udidVal;

    CFDictionaryPtr apps;
    mach_error_t err = AMDeviceLookupApplications(m_handle, NULL, &apps);
    if (err == ERR_SUCCESS)
    {
        std::vector<std::pair<const void*, const void*>> data;
        apps.getData(data);
        /*
        FILE* f = fopen("d:\\1.txt", "w");
        std::vector<std::wstring> props;
        for (size_t i = 0;i < data.size();i++)
        {
            CFDictionaryPtr descr = CFDictionaryPtr((CFDictionaryRef)data[i].second);
            std::vector<std::pair<const void*, const void*>> appdata;
            descr.getData(appdata);
            for (size_t i = 0;i < appdata.size();i++)
            {
                CFTypeID keyType = CFGetTypeID((CFTypeRef)appdata[i].first);
                if (keyType == CFStringGetTypeID())
                {
                    std::wstring name = CFStringPtr((CFStringRef)appdata[i].first);
                    if (std::find(props.begin(), props.end(), name) == props.end())
                        props.push_back(name);
                }
            }
        }
        for (size_t i = 0;i < props.size();i++)
            fwprintf(f, L"%s\t", props[i].c_str());
        fwprintf(f, L"\n");
        for (size_t i = 0;i < data.size();i++)
        {
            std::wstring bundle = CFStringPtr((CFStringRef)data[i].first);

            CFDictionaryPtr descr = CFDictionaryPtr((CFDictionaryRef)data[i].second);
            std::wstring name = CFStringPtr((CFStringRef)CFDictionaryGetValue(descr, (CFStringRef)CFStringPtr(L"CFBundleDisplayName")));

            for (size_t i = 0;i < props.size();i++)
            {
                const void* val = CFDictionaryGetValue(descr, (CFStringRef)CFStringPtr(props[i]));
                CFTypeID valType = val ? CFGetTypeID((CFTypeRef)val) : 0;
                std::wstring valStr;
                if (valType == CFStringGetTypeID())
                {
                    valStr = CFStringPtr((CFStringRef)val);
                    fwprintf(f, L"%s\t", valStr.c_str());
                }
                else if (valType == CFNumberGetTypeID())
                {
                    double dval = 0;
                    CFNumberGetValue((CFNumberRef)val, kCFNumberDoubleType, &dval);
                    fwprintf(f, L"%f\t", dval);
                }
                else
                    fwprintf(f, L"%s\t", val ? L"unk" : L"");
            }
            if (!name.empty())
                m_applications.insert(std::map<std::wstring, std::wstring>::value_type(name, bundle));
            fwprintf(f, L"\n");
        }
        fclose(f);
        */

        for (size_t i = 0;i < data.size();i++)
        {
            std::wstring bundle = CFStringPtr((CFStringRef)data[i].first);

            CFDictionaryPtr descr = CFDictionaryPtr((CFDictionaryRef)data[i].second);
            std::wstring name = CFStringPtr((CFStringRef)CFDictionaryGetValue(descr, (CFStringRef)CFStringPtr(L"CFBundleDisplayName")));
            std::wstring type = CFStringPtr((CFStringRef)CFDictionaryGetValue(descr, (CFStringRef)CFStringPtr(L"ApplicationType")));
            if (type == L"User" && !name.empty())
                m_applications.insert(std::map<std::wstring, std::wstring>::value_type(name, bundle));
        }
    }

    Disconnect();

    for (int i = 0;i < 16;i++)
    {
        int val;
        swscanf(m_udid.substr(i * 2, 2).c_str(), L"%x", &val);
        ((BYTE*)&m_guid)[i] = (BYTE)val;
    }

    //CoCreateGuid(&m_guid);
    AMDeviceRetain(handle);
    return S_OK;
}

void CDevice::Term()
{
    PurgeConnections();
    AMDeviceRelease(m_handle);
    m_handle = NULL;
}

void CDevice::PurgeConnections()
{
    for (auto it = m_connections.begin();it != m_connections.end();it++)
        AFCConnectionClose(it->second);
    m_connections.clear();
}


HRESULT CDevice::Connect()
{
    mach_error_t err = AMDeviceConnect(m_handle);
    if (err != ERR_SUCCESS)
        return E_FAIL;
    if (!AMDeviceIsPaired(m_handle))
        return E_FAIL;
    err = AMDeviceValidatePairing(m_handle);
    if (err != ERR_SUCCESS)
        return E_FAIL;
    err = AMDeviceStartSession(m_handle);
    if (err != ERR_SUCCESS)
        return E_FAIL;
    return S_OK;
}

void CDevice::Disconnect()
{
     AMDeviceStopSession(m_handle);
     AMDeviceDisconnect(m_handle);
}

HRESULT CDevice::GetFileSystem(std::wstring bundle, AFCConnectionRef* pConn)
{
    auto it = m_connections.find(bundle);
    if (it != m_connections.end())
    {
        *pConn = it->second;
        return S_OK;
    }

    AFCConnectionRef connection = NULL;
    mach_error_t err;
    for (;;)
    {
        HRESULT hr = Connect();
        if (FAILED(hr))
            return hr;

        AMServiceRef service;
        if (bundle.empty())
            err = AMDeviceStartService(m_handle, CFStringPtr(L"com.apple.afc"), &service, NULL);
        else
            err = AMDeviceStartHouseArrestService(m_handle, CFStringPtr(bundle), NULL, &service, NULL);
        if (err != ERR_SUCCESS)
        {
            Disconnect();
            if (!m_connections.empty())
            {
                auto it = m_connections.begin();
                AFCConnectionClose(it->second);
                m_connections.erase(it);
                continue;
            }
            else
                break;
        }
        if (err == ERR_SUCCESS)
            err = AFCConnectionOpen(service, 0, &connection);
        Disconnect();
        break;
    }
    if (err == ERR_SUCCESS)
    {
        err = AFCConnectionSetIOTimeout(connection, 2);
        m_connections.insert(std::map<std::wstring, AFCConnectionRef>::value_type(bundle, connection));
        *pConn = connection;
        return S_OK;
    }
    return E_FAIL;
}

STDMETHODIMP CDevice::get_Name(BSTR* pVal)
{
    if (!pVal)
        return E_POINTER;
    CComBSTR name(m_name.c_str());
    *pVal = name.Detach();
    return S_OK;
}

STDMETHODIMP CDevice::get_Id(GUID* pVal)
{
    if (!pVal)
        return E_POINTER;
    *pVal = m_guid;
    return S_OK;
}


STDMETHODIMP CDevice::get_IsValid(VARIANT_BOOL* pVal)
{
    if (!pVal)
        return E_POINTER;
    *pVal = m_handle != NULL ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

HRESULT CDevice::GetDeviceInfo(AFCConnectionRef connection, DeviceInfo* info)
{
    AFCDictionaryRef dict;
    mach_error_t err = AFCDeviceInfoOpen(connection, &dict);
    if (err != ERR_SUCCESS)
        return E_FAIL;
    memset(info, 0, sizeof(DeviceInfo));

    for (;;)
    {
        char *key, *val;
        AFCKeyValueRead(dict, &key, &val);
        if (!key)
            break;
        std::string sk = key;
        std::string sv = val ? val : "";
        if (sk == "FSTotalBytes")
            info->total = std::stoull(sv.c_str());
        else if (sk == "FSFreeBytes")
            info->free = std::stoull(sv.c_str());
    }
    AFCKeyValueClose(dict);
    return S_OK;
}

HRESULT CDevice::GetDeviceInfo(DeviceInfo* info)
{
    if (!info)
        return E_POINTER;
    AFCConnectionRef connection;
    HRESULT hr = GetFileSystem(L"", &connection);
    if (FAILED(hr))
        return hr;
    return GetDeviceInfo(connection, info);
}

HRESULT CDevice::GetFileInfo(AFCConnectionRef connection, std::string path, FileInfo& info)
{
    AFCDictionaryRef dict;
    mach_error_t err = AFCFileInfoOpen(connection, path.c_str(), &dict);
    if (err != ERR_SUCCESS)
        return E_FAIL;
    memset(&info, 0, sizeof(FileInfo));

    for (;;)
    {
        char *key, *val;
        AFCKeyValueRead(dict, &key, &val);
        if (!key)
            break;
        std::string sk = key;
        std::string sv = val ? val : "";
        if (sk == "st_ifmt")
        {
            if (sv == "S_IFIFO" || sv == "S_IFCHR" || sv == "S_IFBLK" || sv == "S_IFSOCK")
                info.attributes |= FILE_ATTRIBUTE_SYSTEM;
            else if (sv == "S_IFDIR")
                info.attributes |= FILE_ATTRIBUTE_DIRECTORY;
            else if (sv == "S_IFREG")
                info.attributes |= FILE_ATTRIBUTE_NORMAL;
            else if (sv == "S_IFLNK")
            {}
        }
        else if (sk == "st_size")
            info.size = std::stoull(sv.c_str());
        else if (sk == "st_mtime")
            info.modificationTime = std::stoull(sv.c_str()) / 100 + 116444736000000000ull;
        else if (sk == "st_birthtime")
            info.creationTime = std::stoull(sv.c_str()) / 100 + 116444736000000000ull;
    }
    AFCKeyValueClose(dict);
    return S_OK;
}

struct PathMap
{
    const wchar_t* prefix;
    const wchar_t* path;
};

const PathMap s_pathMap[] =
{
    { L"Books", L"Books" },
    { L"Camera", L"DCIM/100APPLE" },
    { L"General", L"general_storage" },
    { L"Raw", L"" },
    { L"Recordings", L"Recordings" },
};

HRESULT CDevice::ParsePath(std::wstring& path, AFCConnectionRef* pConn)
{
    std::wstring str = path;
    std::wstring bundle;
    std::replace(str.begin(), str.end(), '\\', '/');
    if (str.length() > 13 && str.substr(0, 13) == L"Applications/")
    {
        str = str.substr(13);
        size_t pos = str.find('/');
        if (pos != std::wstring::npos)
        {
            bundle = str.substr(0, pos);
            path = str.substr(pos);
        }
        else
        {
            bundle = str;
            path = L"";
        }
        auto it = m_applications.find(bundle);
        if (it == m_applications.end())
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        return GetFileSystem(it->second, pConn);
    }
    else
    {
        for (int i = 0;i < ARRAYSIZE(s_pathMap);i++)
        {
            size_t len = lstrlen(s_pathMap[i].prefix);
            if (str.length() >= len && str.substr(0, len) == s_pathMap[i].prefix)
            {
                path = s_pathMap[i].path + str.substr(len);
                return GetFileSystem(L"", pConn);
            }
        }
    }
    return E_INVALIDARG;
}

STDMETHODIMP CDevice::ListFiles(BSTR path, SAFEARRAY * files)
{
    std::wstring wpath = path;

    std::vector<FileInfo> infos;
    std::vector<std::wstring> names;
    if (wpath.empty())
    {
        FileInfo info;
        memset(&info, 0, sizeof(info));
        info.attributes = FILE_ATTRIBUTE_DIRECTORY;
        infos.push_back(info);
        names.push_back(L"Applications");
        for (int i = 0;i < ARRAYSIZE(s_pathMap);i++)
        {
            FileInfo info;
            memset(&info, 0, sizeof(info));
            info.attributes = FILE_ATTRIBUTE_DIRECTORY;
            infos.push_back(info);
            names.push_back(s_pathMap[i].prefix);
        }
    }
    else if (wpath == L"Applications")
    {
        for (std::map<std::wstring, std::wstring>::iterator it = m_applications.begin();it != m_applications.end();it++)
        {
            FileInfo info;
            memset(&info, 0, sizeof(info));
            info.attributes = FILE_ATTRIBUTE_DIRECTORY;
            infos.push_back(info);
            names.push_back(it->first);
        }
    }
    else
    {

        AFCConnectionRef connection;
        HRESULT hr = ParsePath(wpath, &connection);
        if (FAILED(hr))
            return hr;
        std::string bpath = unicode_to_utf8(wpath);
    
        AFCDirectoryRef dir;
        mach_error_t err = AFCDirectoryOpen(connection, bpath.c_str(), &dir);
        if (err != ERR_SUCCESS)
            return E_FAIL;

        for (;;)
        {
            char* name;
            if (AFCDirectoryRead(connection, dir, &name) != ERR_SUCCESS)
                break;
            if (name == NULL)
                break;
            if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0)))
                continue;
            std::string file = bpath;
            file += "/";
            file += name;
            FileInfo info;
            err = GetFileInfo(connection, file, info);
            if (err != ERR_SUCCESS)
            {
                AFCDirectoryClose(connection, dir);
                return E_FAIL;
            }
            names.push_back(utf8_to_unicode(name));
            infos.push_back(info);
        }
        AFCDirectoryClose(connection, dir);
    }

    SAFEARRAYBOUND bounds = { infos.size(), 0 };
    HRESULT hr = SafeArrayRedim(files, &bounds);
    if (FAILED(hr))
        return hr;

    FileInfo* pData = NULL;
    hr = SafeArrayAccessData(files, (void**)&pData);
    if (FAILED(hr))
        return hr;

    for (size_t i = 0, size = infos.size();i < size;i++)
    {
        memcpy(pData + i, &infos[i], sizeof(FileInfo));
        pData[i].name = SysAllocString(names[i].c_str());
    }
    SafeArrayUnaccessData(files);
    return S_OK;
}

STDMETHODIMP CDevice::CreateDirectory(BSTR path)
{
    std::wstring wpath = path;
    AFCConnectionRef connection;
    HRESULT hr = ParsePath(wpath, &connection);
    if (FAILED(hr))
        return hr;
    if (wpath.empty())
        return E_INVALIDARG;
    std::string bpath = unicode_to_utf8(wpath);
    mach_error_t err = AFCDirectoryCreate(connection, bpath.c_str());
    if (err != ERR_SUCCESS)
        return E_FAIL;
    return S_OK;
}

STDMETHODIMP CDevice::DeleteDirectory(BSTR path)
{
    std::wstring wpath = path;
    AFCConnectionRef connection;
    HRESULT hr = ParsePath(wpath, &connection);
    if (FAILED(hr))
        return hr;
    if (wpath.empty())
        return E_INVALIDARG;
    std::string bpath = unicode_to_utf8(wpath);
    mach_error_t err = AFCRemovePath(connection, bpath.c_str());
    if (err != ERR_SUCCESS)
        return E_FAIL;
    return S_OK;
}

STDMETHODIMP CDevice::GetFile(BSTR fromPath, BSTR toPath, IProgressCallback* cb)
{
    ULONG progress = 0;
    std::wstring wpath = fromPath;
    AFCConnectionRef connection;
    HRESULT hr = ParsePath(wpath, &connection);
    if (FAILED(hr))
        return hr;
    if (wpath.empty())
        return E_INVALIDARG;
    std::string bpath = unicode_to_utf8(wpath);

    FileInfo info;
    hr = GetFileInfo(connection, bpath.c_str(), info);
    if (FAILED(hr))
        return hr;

    AFCFileRef fin;
    mach_error_t ret = AFCFileRefOpen(connection, bpath.c_str(), AFC_FILEMODE_READ, 0, &fin);
    if (ret != ERR_SUCCESS)
        return E_FAIL;

    hr = S_OK;
    char* buffer = new char[BUF_SIZE];
    if (buffer)
    {
        FILE* fout = _wfopen(toPath, L"wb");
        if (fout)
        {
            uint64_t total = 0;
            while (total < info.size)
            {
                unsigned len = (unsigned)min(info.size - total, BUF_SIZE);
                ret = AFCFileRefRead(connection, fin, buffer, &len);
                if (ret != ERR_SUCCESS)
                {
                    hr = E_FAIL;
                    break;
                }
                if (!len)
                    break;
                if (fwrite(buffer, 1, len, fout) != len)
                {
                    hr = E_FAIL;
                    break;
                }
                total += len;
                if (cb)
                {
                    ULONG p = (ULONG)(total * 100 / info.size);
                    if (p != progress)
                    {
                        hr = cb->UpdateProgress(progress = p);
                        if (FAILED(hr))
                            break;
                    }
                }
            }
            delete[] buffer;
            fclose(fout);
            if (SUCCEEDED(hr))
            {
                __utimbuf64 times;
                uint64_t fileTime = info.modificationTime;
                times.modtime = times.actime = fileTime / 10000000 - 11644473600;
                _wutime64(toPath, &times);
            }
            else
                _wremove(toPath);
        }
        else
            hr = E_FAIL;
    }
    else
        hr = E_OUTOFMEMORY;
    AFCFileRefClose(connection, fin);
    if (FAILED(hr))
        _wremove(toPath);
    return hr;
}

STDMETHODIMP CDevice::PutFile(BSTR fromPath, BSTR toPath, IProgressCallback* cb)
{
    ULONG progress = 0;
    std::wstring wpath = toPath;
    AFCConnectionRef connection;
    HRESULT hr = ParsePath(wpath, &connection);
    if (FAILED(hr))
        return hr;
    if (wpath.empty())
        return E_INVALIDARG;
    std::string bpath = unicode_to_utf8(wpath);


    WIN32_FIND_DATA wfd;
    HANDLE hFind = ::FindFirstFile(fromPath, &wfd);
    if (hFind == INVALID_HANDLE_VALUE)
        return E_FAIL;
    FindClose(hFind);

    FILE* fin = _wfopen(fromPath, L"rb");
    if (!fin)
        return E_FAIL;

    AFCFileRef fout;
    mach_error_t ret = AFCFileRefOpen(connection, bpath.c_str(), AFC_FILEMODE_WRITE, 0, &fout);
    if (ret != ERR_SUCCESS)
    {
        fclose(fin);
        return E_FAIL;
    }

    hr = S_OK;
    char* buffer = new char[BUF_SIZE];
    if (buffer)
    {
        uint64_t total = 0;
        uint64_t fileSize = ((uint64_t)wfd.nFileSizeHigh << 32) | wfd.nFileSizeLow;
        while (total < fileSize)
        {
            unsigned len = (unsigned)min(fileSize - total, BUF_SIZE);
            len = fread(buffer, 1, len, fin);
            if (!len)
                break;
            ret = AFCFileRefWrite(connection, fout, buffer, len);
            if (ret != ERR_SUCCESS)
            {
                hr = E_FAIL;
                break;
            }
            total += len;
            if (cb)
            {
                ULONG p = (ULONG)(total * 100 / fileSize);
                if (p != progress)
                {
                    hr = cb->UpdateProgress(progress = p);
                    if (FAILED(hr))
                        break;
                }
            }
        }
        delete[] buffer;
        fclose(fin);
    }
    else
        hr = E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
    {
        uint64_t tm = ((uint64_t)wfd.ftLastWriteTime.dwHighDateTime << 32) | wfd.ftLastWriteTime.dwLowDateTime;
        tm = tm * 100 - 11644473600000000000;
        AFCOperationRef op = AFCOperationCreateSetModTime(NULL, CFStringPtr(wpath), tm, 0);
        if (op)
            AFCConnectionProcessOperation(connection, op, 2);
    }

    AFCFileRefClose(connection, fout);

    if (FAILED(hr))
        AFCRemovePath(connection, bpath.c_str());

    return hr;
}

STDMETHODIMP CDevice::DeleteFile(BSTR path)
{
    std::wstring wpath = path;
    AFCConnectionRef connection;
    HRESULT hr = ParsePath(wpath, &connection);
    if (FAILED(hr))
        return hr;
    if (wpath.empty())
        return E_INVALIDARG;
    std::string bpath = unicode_to_utf8(wpath);

    mach_error_t err = AFCRemovePath(connection, bpath.c_str());
    if (err != ERR_SUCCESS)
        return E_FAIL;
    return S_OK;
}
