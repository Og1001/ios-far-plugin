// EnumDevices.h : Declaration of the CEnumDevices

#pragma once
#include "resource.h"       // main symbols
#include "iDevice_i.h"
#include "DeviceManager.h"
#include "Utils.h"

using namespace ATL;


// CEnumDevices

class ATL_NO_VTABLE CEnumDevices :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CEnumDevices>,
    public IDispatchImpl<IEnumDevices, &IID_IEnumDevices, &LIBID_iDeviceLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	typedef std::vector<CDevicePtr> Devices;
	
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
        CAutoLock lock(pDeviceManager->m_cs);
		for (auto it = pDeviceManager->m_devices.begin(); it != pDeviceManager->m_devices.end(); it++)
			m_devices.push_back(it->second);
        m_it = m_devices.begin();
	}

    void Init(Devices& devices, Devices::iterator it)
    {
        m_devices = devices;
		m_it = find(m_devices.begin(), m_devices.end(), *it);
	}

    // IEnumDevices Methods
public:
    STDMETHOD(Clone)(IEnumDevices ** ppenum)
    {
        HRESULT hr;
        if (FAILED(hr = CEnumDevices::_CreatorClass::CreateInstance(NULL, IID_IEnumDevices, (void**)ppenum)))
            return hr;
        CEnumDevices* p = (CEnumDevices*)*ppenum;
		p->Init(m_devices, m_it);
        return S_OK;
    }

    STDMETHOD(Next)(ULONG celt, IDevice ** rgelt, ULONG * pceltFetched)
    {
	    if (rgelt == NULL || (celt > 1 && pceltFetched == NULL))
		    return E_POINTER;
	    if (pceltFetched != NULL)
		    *pceltFetched = 0;

		ULONG nActual = 0;
	    HRESULT hr = S_OK;
	    IDevice** pelt = rgelt;
	    while (SUCCEEDED(hr) && m_it != m_devices.end() && nActual < celt)
	    {
            *pelt = *m_it;
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
        m_it = m_devices.begin();
		return S_OK;
    }
    
    STDMETHOD(Skip)(ULONG celt)
    {
	    HRESULT hr = S_OK;
		while (celt--)
	    {
		    if (m_it != m_devices.end())
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
    Devices::iterator m_it;
	Devices m_devices;
};
