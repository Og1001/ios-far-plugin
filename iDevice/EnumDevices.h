// EnumDevices.h : Declaration of the CEnumDevices

#pragma once
#include "resource.h"       // main symbols
#include "iDevice_i.h"
#include "DeviceManager.h"

using namespace ATL;


// CEnumDevices

class ATL_NO_VTABLE CEnumDevices :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CEnumDevices>,
    public IDispatchImpl<IEnumDevices, &IID_IEnumDevices, &LIBID_iDeviceLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
    CEnumDevices()
    {
    }


    BEGIN_COM_MAP(CEnumDevices)
        COM_INTERFACE_ENTRY(IEnumDevices)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

    void Init(CDeviceManager* pDeviceManager)
    {
        m_pDeviceManager = pDeviceManager;
        m_lock = pDeviceManager;
        CAutoLock lock(pDeviceManager->m_cs);
        m_it = pDeviceManager->m_devices.begin();
    }

    void Init(CDeviceManager* pDeviceManager, CDeviceManager::Devices::iterator it)
    {
        m_pDeviceManager = pDeviceManager;
        m_lock = pDeviceManager;
        m_it = it;
    }

    // IEnumDevices Methods
public:
    STDMETHOD(Clone)(IEnumDevices ** ppenum)
    {
        HRESULT hr;
        if (FAILED(hr = CEnumDevices::_CreatorClass::CreateInstance(NULL, IID_IEnumDevices, (void**)ppenum)))
            return hr;
        CEnumDevices* p = (CEnumDevices*)*ppenum;
        p->Init(m_pDeviceManager, m_it);
        return S_OK;
    }

    STDMETHOD(Next)(ULONG celt, IDevice ** rgelt, ULONG * pceltFetched)
    {
        CAutoLock lock(m_pDeviceManager->m_cs);
	    if (rgelt == NULL || (celt > 1 && pceltFetched == NULL))
		    return E_POINTER;
	    if (pceltFetched != NULL)
		    *pceltFetched = 0;

	    ULONG nActual = 0;
	    HRESULT hr = S_OK;
	    IDevice** pelt = rgelt;
	    while (SUCCEEDED(hr) && m_it != m_pDeviceManager->m_devices.end() && nActual < celt)
	    {
            *pelt = m_it->second;
            (*pelt)->AddRef();
			pelt++;
			m_it++;
			nActual++;
	    }
	    if (SUCCEEDED(hr))
	    {
		    if (pceltFetched)
			    *pceltFetched = nActual;
		    if (nActual < celt)
			    hr = S_FALSE;
	    }
	    return hr;
    }
    
    STDMETHOD(Reset)()
    {
        CAutoLock lock(m_pDeviceManager->m_cs);
        m_it = m_pDeviceManager->m_devices.begin();
        return S_OK;
    }
    
    STDMETHOD(Skip)(ULONG celt)
    {
        CAutoLock lock(m_pDeviceManager->m_cs);
	    HRESULT hr = S_OK;
	    while (celt--)
	    {
		    if (m_it != m_pDeviceManager->m_devices.end())
			    m_it++;
		    else
		    {
			    hr = S_FALSE;
			    break;
		    }
	    }
	    return hr;
    }

private:
    CDeviceManager* m_pDeviceManager;
    CComPtr<IDeviceManager> m_lock;
    CDeviceManager::Devices::iterator m_it;
};
