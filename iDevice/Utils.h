#pragma once

#include "CoreFoundation.h"

template<class T>
class CFPtr
{
public:
    CFPtr():
        m_p(NULL)
    {
    }
    
    CFPtr(T t, bool retain = true):
        m_p(t)
    {
        if (retain && t)
            CFRetain(t);
    }

    CFPtr(const CFPtr<T>& t)
    {
        if (m_p = t.m_p)
            CFRetain(m_p);
    }
    
    ~CFPtr()
    {
        clear();
    }

    CFPtr& operator =(const CFPtr& t)
    {
        clear();
        if (m_p = t.m_p)
            CFRetain(m_p);
        return *this;
    }

    CFPtr& operator =(T t)
    {
        clear();
        if (m_p = p)
            CFRetain(m_p);
        return *this;
    }

    operator T()
    {
        return m_p;
    }

    T* operator &()
    {
        return &m_p;
    }

protected:
    void clear()
    {
        if (m_p)
        {
            CFRelease(m_p);
            m_p = NULL;
        }
    }

    T m_p;
};

class CFStringPtr : public CFPtr<CFStringRef>
{
public:
    CFStringPtr()
    {
    }
    
    CFStringPtr(const CFStringPtr& ptr):
        CFPtr<CFStringRef>(ptr)
    {
    }
    
    CFStringPtr(CFStringRef ref, bool retain = true):
        CFPtr<CFStringRef>(ref, retain)
    {
    }
    
    CFStringPtr(std::wstring str)
    {
        *this = str;
    }
    
    CFStringPtr(const wchar_t* str)
    {
        *this = str;
    }
    
    CFStringPtr& operator =(std::wstring str)
    {
        clear();
        m_p = CFStringCreateWithCharacters(NULL, str.c_str(), (int)str.length());
        return *this;
    }

    CFStringPtr& operator =(const wchar_t* str)
    {
        clear();
        m_p = CFStringCreateWithCharacters(NULL, str, (int)wcslen(str));
        return *this;
    }

    operator std::wstring()
    {
        std::wstring str;
        if (m_p)
        {
            int len = CFStringGetLength(m_p);
            if (len > 0)
            {
                str.resize(len, ' ');
                CFStringGetCharacters(m_p, CFRangeMake(0, len), &str[0]);
            }
        }
        return str;
    }

    size_t length()
    {
        return m_p ? CFStringGetLength(m_p) : 0;
    }

};

#define CFSTR(str) CFStringPtr(str)

class CFDictionaryPtr : public CFPtr<CFDictionaryRef>
{
public:
    CFDictionaryPtr()
    {
    }
    
    CFDictionaryPtr(const CFDictionaryPtr& ptr):
        CFPtr<CFDictionaryRef>(ptr)
    {
    }
    
    CFDictionaryPtr(CFDictionaryRef ref, bool retain = true):
        CFPtr<CFDictionaryRef>(ref, retain)
    {
    }
    
    CFDictionaryPtr(size_t count, CFTypeRef* keys, CFTypeRef* values)
    {
        m_p = CFDictionaryCreate(kCFAllocatorDefault, (const void**)keys, (const void**)values,
            count, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }

    size_t size()
    {
        return m_p ? CFDictionaryGetCount(m_p) : 0;
    }

    void getData(std::vector<std::pair<const void*, const void*>>& data)
    {
        int len = size();
        data.resize(len);
        const void** keys = (const void**)malloc(len * sizeof(void*));
        const void** values = (const void**)malloc(len * sizeof(void*));
        CFDictionaryGetKeysAndValues(m_p, keys, values);
        for (int i = 0;i < len;i++)
            data[i] = std::pair<const void*, const void*>(keys[i], values[i]);
        free(keys);
        free(values);
    }
};

std::string unicode_to_utf8(std::wstring str);
std::wstring utf8_to_unicode(std::string str);
std::wstring get_registry_value(std::wstring key, std::wstring value);

#define HRESULT_FROM_WIN32(x) \
  ((HRESULT)(x) <= 0 ? ((HRESULT)(x)) \
: ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)))

void trace(const wchar_t* str, ...);
