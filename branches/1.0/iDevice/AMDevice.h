#pragma once

#include "CoreFoundation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* AMDeviceNotificationRef;
typedef void* AMDeviceRef;
typedef void* AMServiceRef;
typedef void* AFCConnectionRef;
typedef void* AFCDirectoryRef;
typedef void* AFCDictionaryRef;
typedef void* AFCOperationRef;
typedef uint64_t AFCFileRef;

typedef uint32_t mach_error_t;

#define ERR_SUCCESS     (mach_error_t)0

#define err_amd        err_system(0x3a)

#define amd_err(errno)     (err_amd|errno)

#define ADNCI_MSG_CONNECTED     1
#define ADNCI_MSG_DISCONNECTED  2

#define AFC_FILEMODE_READ           2
#define AFC_FILEMODE_WRITE          3

struct am_device_notification_callback_info
{
    AMDeviceRef device;
    uint32_t msg;
};

typedef void (*am_device_notification_callback)(am_device_notification_callback_info*, void* callback_data);

#define AFC_API __declspec(dllimport)

void AFC_API AMDeviceRetain(AMDeviceRef device);
void AFC_API AMDeviceRelease(AMDeviceRef device);
mach_error_t AFC_API AMDeviceNotificationSubscribe(am_device_notification_callback callback, unsigned, unsigned, void *callback_data, AMDeviceNotificationRef *notification);
mach_error_t AFC_API AMDeviceNotificationUnsubscribe(AMDeviceNotificationRef notification);
mach_error_t AFC_API AMDeviceConnect(AMDeviceRef device);
mach_error_t AFC_API AMDeviceDisconnect(AMDeviceRef device);
Boolean AFC_API AMDeviceIsPaired(AMDeviceRef device);
CFStringRef AMDeviceCopyValue(AMDeviceRef device, CFStringRef domain, CFStringRef key);

mach_error_t AFC_API AMDeviceValidatePairing(AMDeviceRef device);
mach_error_t AFC_API AMDeviceStartSession(AMDeviceRef device);
mach_error_t AFC_API AMDeviceStartService(AMDeviceRef device, CFStringRef serviceName, AMServiceRef* service, void*);
mach_error_t AFC_API AMDeviceStopSession(AMDeviceRef device);

mach_error_t AMDeviceLookupApplications(AMDeviceRef device, CFDictionaryRef, CFDictionaryRef* apps);
mach_error_t AMDeviceStartHouseArrestService(AMDeviceRef device, CFStringRef identifier, void*, AMServiceRef *service, unsigned int*);

mach_error_t AFC_API AFCConnectionOpen(AMServiceRef connection, unsigned timeout, AFCConnectionRef* conn);
mach_error_t AFC_API AFCConnectionClose(AFCConnectionRef conn);
mach_error_t AFC_API AFCDeviceInfoOpen(AFCConnectionRef connection, AFCDictionaryRef *info);
mach_error_t AFC_API AFCDirectoryOpen(AFCConnectionRef connection, const char *path, AFCDirectoryRef* dir);
mach_error_t AFC_API AFCDirectoryRead(AFCConnectionRef connection, AFCDirectoryRef dir, char **dirent);
mach_error_t AFC_API AFCDirectoryClose(AFCConnectionRef connection, AFCDirectoryRef dir);
mach_error_t AFC_API AFCFileInfoOpen(AFCConnectionRef connection, const char *path, AFCDictionaryRef *info);
mach_error_t AFC_API AFCGetFileInfo(AFCConnectionRef connection, const char *path, AFCDictionaryRef *info, unsigned int *size);
mach_error_t AFC_API AFCKeyValueRead(AFCDictionaryRef dict, char **key, char **val);
mach_error_t AFC_API AFCKeyValueClose(AFCDictionaryRef dict);
mach_error_t AFC_API AFCFileRefOpen(AFCConnectionRef connection, const char *path, unsigned int mode, int zero, AFCFileRef *ref);
mach_error_t AFC_API AFCFileRefClose(AFCConnectionRef connection, AFCFileRef ref);
mach_error_t AFC_API AFCFileRefRead(AFCConnectionRef connection, AFCFileRef ref, void *buf, unsigned int *len);
mach_error_t AFC_API AFCFileRefWrite(AFCConnectionRef connection, AFCFileRef ref, void *buf, unsigned int len);
mach_error_t AFC_API AFCRenamePath(AFCConnectionRef connection, const char *oldpath, const char *newpath);
AFCOperationRef AFC_API AFCOperationCreateSetModTime(CFAllocatorRef allocator, CFStringRef filename, uint64_t mtm, void *ctx);
mach_error_t AFC_API AFCConnectionProcessOperation(AFCConnectionRef connection, AFCOperationRef op, double timeout);
uint32_t AFC_API AFCConnectionGetIOTimeout(AFCConnectionRef conn);
mach_error_t AFC_API AFCRemovePath(AFCConnectionRef connection, const char *dirname);
mach_error_t AFC_API AFCDirectoryCreate(AFCConnectionRef connection, const char *dirname);
mach_error_t AFC_API AFCConnectionSetIOTimeout(AFCConnectionRef conn, uint32_t timeout);

#define kAMDSuccess 0x0
#define kAMDUndefinedError amd_err(0x1)
#define kAMDBadHeaderError amd_err(0x2)
#define kAMDNoResourcesError amd_err(0x3)
#define kAMDReadError amd_err(0x4)
#define kAMDWriteError amd_err(0x5)
#define kAMDUnknownPacketError amd_err(0x6)
#define kAMDInvalidArgumentError amd_err(0x7)
#define kAMDNotFoundError amd_err(0x8)
#define kAMDIsDirectoryError amd_err(0x9)
#define kAMDPermissionError amd_err(0xa)
#define kAMDNotConnectedError amd_err(0xb)
#define kAMDTimeOutError amd_err(0xc)
#define kAMDOverrunError amd_err(0xd)
#define kAMDEOFError amd_err(0xe)
#define kAMDUnsupportedError amd_err(0xf)
#define kAMDFileExistsError amd_err(0x10)
#define kAMDBusyError amd_err(0x11)
#define kAMDCryptoError amd_err(0x12)
#define kAMDInvalidResponseError amd_err(0x13)
#define kAMDMissingKeyError amd_err(0x14)
#define kAMDMissingValueError amd_err(0x15)
#define kAMDGetProhibitedError amd_err(0x16)
#define kAMDSetProhibitedError amd_err(0x17)
#define kAMDRemoveProhibitedError amd_err(0x18)
#define kAMDImmutableValueError amd_err(0x19)
#define kAMDPasswordProtectedError amd_err(0x1a)
#define kAMDMissingHostIDError amd_err(0x1b)
#define kAMDInvalidHostIDError amd_err(0x1c)
#define kAMDSessionActiveError amd_err(0x1d)
#define kAMDSessionInactiveError amd_err(0x1e)
#define kAMDMissingSessionIDError amd_err(0x1f)
#define kAMDInvalidSessionIDError amd_err(0x20)
#define kAMDMissingServiceError amd_err(0x21)
#define kAMDInvalidServiceError amd_err(0x22)
#define kAMDInvalidCheckinError amd_err(0x23)
#define kAMDCheckinTimeoutError amd_err(0x24)
#define kAMDMissingPairRecordError amd_err(0x25)
#define kAMDInvalidActivationRecordError    amd_err(0x26)
#define kAMDMissingActivationRecordError    amd_err(0x27)
#define kAMDWrongDroidError amd_err(0x28)
#define kAMDSUVerificationError amd_err(0x29)
#define kAMDSUPatchError    amd_err(0x2A)
#define kAMDSUFirmwareError amd_err(0x2B)
#define kAMDProvisioningProfileNotValid amd_err(0x2C)
#define kAMDSendMessageError    amd_err(0x2D)
#define kAMDReceiveMessageError amd_err(0x2E)
#define kAMDMissingOptionsError amd_err(0x2F)
#define kAMDMissingImageTypeError   amd_err(0x30)
#define kAMDDigestFailedError   amd_err(0x31)
#define kAMDStartServiceError   amd_err(0x32)
#define kAMDInvalidDiskImageError   amd_err(0x33)
#define kAMDMissingDigestError  amd_err(0x34)
#define kAMDMuxError    amd_err(0x35)
#define kAMDApplicationAlreadyInstalledError    amd_err(0x36)
#define kAMDApplicationMoveFailedError  amd_err(0x37)
#define kAMDApplicationSINFCaptureFailedError   amd_err(0x38)
#define kAMDApplicationSandboxFailedError   amd_err(0x39)
#define kAMDApplicationVerificationFailedError  amd_err(0x3A)
#define kAMDArchiveDestructionFailedError   amd_err(0x3B)
#define kAMDBundleVerificationFailedError   amd_err(0x3C)
#define kAMDCarrierBundleCopyFailedError    amd_err(0x3D)
#define kAMDCarrierBundleDirectoryCreationFailedError   amd_err(0x3E)
#define kAMDCarrierBundleMissingSupportedSIMsError  amd_err(0x3F)
#define kAMDCommCenterNotificationFailedError   amd_err(0x40)
#define kAMDContainerCreationFailedError    amd_err(0x41)
#define kAMDContainerP0wnFailedError    amd_err(0x42)
#define kAMDContainerRemovalFailedError amd_err(0x43)
#define kAMDEmbeddedProfileInstallFailedError   amd_err(0x44)
#define kAMDErrorError  amd_err(0x45)
#define kAMDExecutableTwiddleFailedError    amd_err(0x46)
#define kAMDExistenceCheckFailedError   amd_err(0x47)
#define kAMDInstallMapUpdateFailedError amd_err(0x48)
#define kAMDManifestCaptureFailedError  amd_err(0x49)
#define kAMDMapGenerationFailedError    amd_err(0x4A)
#define kAMDMissingBundleExecutableError    amd_err(0x4B)
#define kAMDMissingBundleIdentifierError    amd_err(0x4C)
#define kAMDMissingBundlePathError  amd_err(0x4D)
#define kAMDMissingContainerError   amd_err(0x4E)
#define kAMDNotificationFailedError amd_err(0x4F)
#define kAMDPackageExtractionFailedError    amd_err(0x50)
#define kAMDPackageInspectionFailedError    amd_err(0x51)
#define kAMDPackageMoveFailedError  amd_err(0x52)
#define kAMDPathConversionFailedError   amd_err(0x53)
#define kAMDRestoreContainerFailedError amd_err(0x54)
#define kAMDSeatbeltProfileRemovalFailedError   amd_err(0x55)
#define kAMDStageCreationFailedError    amd_err(0x56)
#define kAMDSymlinkFailedError  amd_err(0x57)
#define kAMDiTunesArtworkCaptureFailedError amd_err(0x58)
#define kAMDiTunesMetadataCaptureFailedError    amd_err(0x59)
#define kAMDAlreadyArchivedError    amd_err(0x5A)
#define kAMDProhibitedBySupervision amd_err(0x83)

#ifdef __cplusplus
}
#endif

