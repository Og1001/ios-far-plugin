#include "StdAfx.h"
#include "Utils.h"

std::string unicode_to_utf8(std::wstring str)
{
    if (str.empty())
        return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0, NULL, NULL);
    std::string result(size, ' ');
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), &*result.begin(), size + 1, NULL, NULL);
    return result;
}

std::wstring utf8_to_unicode(std::string str)
{
    if (str.empty())
        return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring result(size, ' ');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &*result.begin(), size + 1);
    return result;
}

std::wstring get_registry_value(std::wstring key, std::wstring value)
{
    HKEY hKey;
    std::wstring res;
    if (RegCreateKey(HKEY_LOCAL_MACHINE, key.c_str(), &hKey) == ERROR_SUCCESS)
    {
        DWORD len = 0;
        if (RegQueryValueEx(hKey, value.c_str(), NULL, NULL, NULL, &len) == ERROR_SUCCESS)
        {
            res.resize(len, ' ');
            RegQueryValueEx(hKey, value.c_str(), NULL, NULL, (LPBYTE)&*res.begin(), &len);
        }
        RegCloseKey(hKey);
    }
    return res;
}
