#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define CF_ENUM(_type, _name) _type _name; enum

typedef int32_t OSStatus;

enum { noErr = 0 };

typedef signed long CFIndex;
typedef unsigned char Boolean;
typedef wchar_t UniChar;
typedef unsigned char           Boolean;
typedef unsigned char           UInt8;
typedef signed char             SInt8;
typedef unsigned short          UInt16;
typedef signed short            SInt16;
typedef unsigned int            UInt32;
typedef signed int              SInt32;
typedef uint64_t            UInt64;
typedef int64_t         SInt64;
typedef SInt32                  OSStatus;
typedef unsigned long CFTypeID;
typedef unsigned long CFHashCode;

typedef const void* CFTypeRef;
typedef const struct __CFAllocator* CFAllocatorRef;
typedef const struct __CFString* CFStringRef;
typedef const struct __CFURL* CFURLRef;
typedef const struct __CFReadStream* CFReadStreamRef;
typedef const struct __CFWriteStream* CFWriteStreamRef;
typedef const struct __CFData* CFDataRef;
typedef const struct __CFPropertyList* CFPropertyListRef;
typedef const struct __CFDictionary* CFDictionaryRef;
typedef const struct __CFArray* CFArrayRef;
typedef const struct __CFNumber* CFNumberRef;

typedef unsigned long CFOptionFlags;

typedef CF_ENUM(CFIndex, CFURLPathStyle)
{
    kCFURLPOSIXPathStyle = 0,
    kCFURLHFSPathStyle,
    kCFURLWindowsPathStyle
};

typedef struct {
    CFIndex location;
    CFIndex length;
} CFRange;

inline CFRange CFRangeMake(CFIndex loc, CFIndex len)
{
    CFRange range;
    range.location = loc;
    range.length = len;
    return range;
}

typedef UInt32 CFStringEncoding;

#define CF_API __declspec(dllimport)

/* Platform-independent built-in encodings; always available on all platforms.
   Call CFStringGetSystemEncoding() to get the default system encoding.
*/
#define kCFStringEncodingInvalidId (0xffffffffU)
typedef CF_ENUM(CFStringEncoding, CFStringBuiltInEncodings) {
    kCFStringEncodingMacRoman = 0,
    kCFStringEncodingWindowsLatin1 = 0x0500, /* ANSI codepage 1252 */
    kCFStringEncodingISOLatin1 = 0x0201, /* ISO 8859-1 */
    kCFStringEncodingNextStepLatin = 0x0B01, /* NextStep encoding*/
    kCFStringEncodingASCII = 0x0600, /* 0..127 (in creating CFString, values greater than 0x7F are treated as corresponding Unicode value) */
    kCFStringEncodingUnicode = 0x0100, /* kTextEncodingUnicodeDefault  + kTextEncodingDefaultFormat (aka kUnicode16BitFormat) */
    kCFStringEncodingUTF8 = 0x08000100, /* kTextEncodingUnicodeDefault + kUnicodeUTF8Format */
    kCFStringEncodingNonLossyASCII = 0x0BFF, /* 7bit Unicode variants used by Cocoa & Java */

    kCFStringEncodingUTF16 = 0x0100, /* kTextEncodingUnicodeDefault + kUnicodeUTF16Format (alias of kCFStringEncodingUnicode) */
    kCFStringEncodingUTF16BE = 0x10000100, /* kTextEncodingUnicodeDefault + kUnicodeUTF16BEFormat */
    kCFStringEncodingUTF16LE = 0x14000100, /* kTextEncodingUnicodeDefault + kUnicodeUTF16LEFormat */

    kCFStringEncodingUTF32 = 0x0c000100, /* kTextEncodingUnicodeDefault + kUnicodeUTF32Format */
    kCFStringEncodingUTF32BE = 0x18000100, /* kTextEncodingUnicodeDefault + kUnicodeUTF32BEFormat */
    kCFStringEncodingUTF32LE = 0x1c000100 /* kTextEncodingUnicodeDefault + kUnicodeUTF32LEFormat */
};

typedef enum {
    kCFPropertyListOpenStepFormat = 1,
    kCFPropertyListXMLFormat_v1_0 = 100,
    kCFPropertyListBinaryFormat_v1_0 = 200
} CFPropertyListFormat; 

typedef enum {
    kCFPropertyListImmutable = 0,
    kCFPropertyListMutableContainers,
    kCFPropertyListMutableContainersAndLeaves
} CFPropertyListMutabilityOptions; 

enum CFNumberType {
   kCFNumberSInt8Type = 1,
   kCFNumberSInt16Type = 2,
   kCFNumberSInt32Type = 3,
   kCFNumberSInt64Type = 4,
   kCFNumberFloat32Type = 5,
   kCFNumberFloat64Type = 6,
   kCFNumberCharType = 7,
   kCFNumberShortType = 8,
   kCFNumberIntType = 9,
   kCFNumberLongType = 10,
   kCFNumberLongLongType = 11,
   kCFNumberFloatType = 12,
   kCFNumberDoubleType = 13,
   kCFNumberCFIndexType = 14,
   kCFNumberNSIntegerType = 15,
   kCFNumberCGFloatType = 16,
   kCFNumberMaxType = 16
};
typedef enum CFNumberType CFNumberType;

extern CF_API const CFAllocatorRef kCFAllocatorDefault;

typedef const void *    (*CFDictionaryRetainCallBack)(CFAllocatorRef allocator, const void *value);
typedef void        (*CFDictionaryReleaseCallBack)(CFAllocatorRef allocator, const void *value);
typedef CFStringRef (*CFDictionaryCopyDescriptionCallBack)(const void *value);
typedef Boolean     (*CFDictionaryEqualCallBack)(const void *value1, const void *value2);
typedef CFHashCode  (*CFDictionaryHashCallBack)(const void *value);
typedef struct {
    CFIndex             version;
    CFDictionaryRetainCallBack      retain;
    CFDictionaryReleaseCallBack     release;
    CFDictionaryCopyDescriptionCallBack copyDescription;
    CFDictionaryEqualCallBack       equal;
    CFDictionaryHashCallBack        hash;
} CFDictionaryKeyCallBacks;

extern CF_API const CFDictionaryKeyCallBacks kCFTypeDictionaryKeyCallBacks;
extern CF_API const CFDictionaryKeyCallBacks kCFCopyStringDictionaryKeyCallBacks;

typedef struct {
    CFIndex             version;
    CFDictionaryRetainCallBack      retain;
    CFDictionaryReleaseCallBack     release;
    CFDictionaryCopyDescriptionCallBack copyDescription;
    CFDictionaryEqualCallBack       equal;
} CFDictionaryValueCallBacks;

extern CF_API const CFDictionaryValueCallBacks kCFTypeDictionaryValueCallBacks;


void CF_API CFRetain(CFTypeRef cf);
void CF_API CFRelease(CFTypeRef cf);
void CF_API CFStringGetCharacters(CFStringRef theString, CFRange range, UniChar *buffer);
CFIndex CF_API CFStringGetLength(CFStringRef theString);
CFStringRef CF_API CFStringCreateWithCharacters(CFAllocatorRef alloc, const UniChar *chars, CFIndex numChars);
CFTypeID CFGetTypeID(CFTypeRef cf);

CFWriteStreamRef CF_API CFWriteStreamCreateWithFile(CFAllocatorRef, CFURLRef);
void CF_API CFWriteStreamClose(CFWriteStreamRef);
CFReadStreamRef CF_API CFReadStreamCreateWithFile(CFAllocatorRef, CFURLRef);
CFURLRef CF_API CFURLCreateWithFileSystemPath(CFAllocatorRef, CFStringRef, CFURLPathStyle, Boolean);
Boolean CF_API CFReadStreamOpen(CFReadStreamRef);
Boolean CF_API CFWriteStreamOpen(CFWriteStreamRef);
CFPropertyListRef CF_API CFPropertyListCreateFromStream(CFAllocatorRef, CFReadStreamRef, CFIndex, CFOptionFlags, CFPropertyListFormat*, CFStringRef*);
void CF_API CFReadStreamClose(CFReadStreamRef);
Boolean CF_API CFPropertyListIsValid(CFPropertyListRef, CFPropertyListFormat);
CFIndex CF_API CFPropertyListWriteToStream(CFPropertyListRef, CFWriteStreamRef, CFPropertyListFormat, CFStringRef*);
Boolean CF_API CFURLCreateDataAndPropertiesFromResource(CFAllocatorRef alloc, CFURLRef url, CFDataRef *resourceData, CFDictionaryRef *properties, CFArrayRef desiredProperties, SInt32 *errorCode);
CFPropertyListRef CF_API CFPropertyListCreateFromXMLData(CFAllocatorRef allocator, CFDataRef xmlData, CFOptionFlags mutabilityOption, CFStringRef *errorString);
CFDataRef CF_API CFPropertyListCreateXMLData(CFAllocatorRef allocator, CFPropertyListRef propertyList);
Boolean CF_API CFURLWriteDataAndPropertiesToResource(CFURLRef url, CFDataRef dataToWrite, CFDictionaryRef propertiesToWrite, SInt32 *errorCode);
CFStringRef CF_API CFStringCreateWithCString(CFAllocatorRef, const char *, CFStringEncoding);

CFIndex CF_API CFDictionaryGetCount(CFDictionaryRef theDict);
void CF_API CFDictionaryGetKeysAndValues(CFDictionaryRef theDict, const void **keys, const void **values);
CFDictionaryRef CF_API CFDictionaryCreate(CFAllocatorRef allocator, const void **keys, const void **values, CFIndex numValues, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks);
CF_API const void* CFDictionaryGetValue(CFDictionaryRef theDict, const void *key);

CFTypeID CF_API CFDateGetTypeID();
CFTypeID CF_API CFNumberGetTypeID();
CFTypeID CF_API CFStringGetTypeID();
CFTypeID CF_API CFDictionaryGetTypeID();
CFTypeID CF_API CFDataGetTypeID();
CFTypeID CF_API CFBundleGetTypeID();
Boolean CFNumberGetValue (CFNumberRef number, CFNumberType theType, void *valuePtr);

#ifdef __cplusplus
}
#endif

