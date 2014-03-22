// dllmain.h : Declaration of module class.

class CIDeviceModule : public ATL::CAtlDllModuleT< CIDeviceModule >
{
public :
    DECLARE_LIBID(LIBID_iDeviceLib)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_IDEVICE, "{4D1F1FBE-814C-4925-A8C9-FFE8015E9B86}")
};

extern class CIDeviceModule _AtlModule;
