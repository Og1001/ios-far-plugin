#include "StdAfx.h"
#include "Dialog.h"

Dialog::DialogMap Dialog::m_map;

intptr_t Dialog::DlgProc(intptr_t Msg, intptr_t Param1, void *Param2)
{
    return s_startupInfo.DefDlgProc(m_handle, Msg, Param1, Param2);
}

intptr_t Dialog::DlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2)
{
    DialogMap::iterator it = m_map.find(hDlg);
    return it->second->DlgProc(Msg, Param1, Param2);
}

Control* Dialog::Run()
{
    GUID dlgid;
    CoCreateGuid(&dlgid);

    Size size = this->size();
    Rect rect(3, 1, size.cx, size.cy);
    std::vector<FarDialogItem> items;
    layout(rect, items);
    if (items.empty())
        return 0;

    m_handle = s_startupInfo.DialogInit(&kPluginId, &dlgid, -1, -1, rect.cx + 6, rect.cy + 2, NULL, &items[0], items.size(), 0, m_warning ? FDLG_WARNING : FDLG_NONE, DlgProc, NULL);
    std::pair<DialogMap::iterator, bool> it = m_map.insert(DialogMap::value_type(m_handle, this));
    intptr_t result = s_startupInfo.DialogRun(m_handle);
    s_startupInfo.DialogFree(m_handle);
    m_map.erase(it.first);
    m_handle = NULL;
    if (result == -1)
        return NULL;
    return (Control*)items[result].UserData;
}
