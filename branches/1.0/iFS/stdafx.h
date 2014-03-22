#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <SDKDDKVer.h>
#include <windows.h>
#include <plugin.hpp>
#include <DlgBuilder.hpp>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <algorithm>
#include <sys/utime.h>
#include <atlbase.h>
#include <atlcom.h>
#include <assert.h>

#undef CreateDirectory
#undef DeleteFile

#include <iDevice_i.h>

#define DECLARE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#define ANNOUNCE_GUID(name) \
        extern const GUID name;

#undef min
#undef max