#pragma once

#include "FarPlugin.h"

struct Size
{
    int cx, cy;

    Size() : cx(0), cy(0) {}
    Size(int cx, int cy) : cx(cx), cy(cy) {}
};

struct Rect
{
    int x, y, cx, cy;
    Rect() : x(0), y(0), cx(0), cy(0) {}
    Rect(int x, int y, int cx, int cy) : x(x), y(y), cx(cx), cy(cy) {}

    void inflate(int dx, int dy) { x -= dx, y -= dy; cx += 2 * dx; cy += 2 * dy; }
};

enum HAlign
{
    eLeft,
    eHCenter,
    eRight
};

enum VAlign
{
    eTop,
    eVCenter,
    eBottom
};

enum Orientation
{
    eHorizontal,
    eVertical
};

struct Control
{
    Control():
        horzFlexible(false),
        vertFlexible(false),
        separator(false)
    {
    }
    
    virtual void layout(Rect rect, std::vector<FarDialogItem>& items) = 0;
    virtual Size size() = 0;

    bool horzFlexible;
    bool vertFlexible;
    bool separator;
};

struct Text : public Control
{
    Text(std::wstring text) : text(text) {}

    virtual Size size()
    {
        return Size((int)text.length(), 1);
    }

    virtual void layout(Rect rect, std::vector<FarDialogItem>& items)
    {
        FarDialogItem item;
        memset(&item, 0, sizeof(item));
        item.Type = DI_TEXT;
        item.X1 = rect.x;
        item.Y1 = rect.y;
        item.X2 = rect.x + rect.cx - 1;
        item.Y2 = rect.y + rect.cy - 1;
        item.Data = text.c_str();
        item.UserData = (intptr_t)this;
        items.push_back(item);
    }

    std::wstring text;
};

struct Button : public Control
{
    Button(std::wstring text, FARDIALOGITEMFLAGS flags = 0) : text(text), flags(flags) {}

    virtual Size size()
    {
        return Size((int)text.length() + 4, 1);
    }

    virtual void layout(Rect rect, std::vector<FarDialogItem>& items)
    {
        FarDialogItem item;
        memset(&item, 0, sizeof(item));
        item.Type = DI_BUTTON;
        item.X1 = rect.x;
        item.Y1 = rect.y;
        item.X2 = rect.x + rect.cx - 1;
        item.Y2 = rect.y + rect.cy - 1;
        item.Data = text.c_str();
        item.Flags = flags;
        item.UserData = (intptr_t)this;
        items.push_back(item);
    }

    std::wstring text;
    FARDIALOGITEMFLAGS flags;
};

struct Separator : public Control
{
    Separator(int border = 1) : border(border) { horzFlexible = true; separator = true; }

    virtual Size size()
    {
        return Size(0, 1);
    }

    virtual void layout(Rect rect, std::vector<FarDialogItem>& items)
    {
        FarDialogItem item;
        memset(&item, 0, sizeof(item));
        item.Type = DI_TEXT;
        item.X1 = -1;
        item.Y1 = rect.y;
        item.X2 = -1;
        item.Y2 = rect.y + rect.cy - 1;
        item.Flags = border == 2 ? DIF_SEPARATOR2 : DIF_SEPARATOR;
        item.UserData = (intptr_t)this;
        items.push_back(item);
    }

    int border;
};

struct ProgressBar : public Control
{
    ProgressBar(int width) : width(width) { setProgress(0); }

    void setProgress(int progress)
    {
        this->progress = progress;
    }

    virtual Size size()
    {
        return Size(width, 1);
    }

    virtual void layout(Rect rect, std::vector<FarDialogItem>& items)
    {
        FarDialogItem item;
        memset(&item, 0, sizeof(item));
        item.Type = DI_TEXT;
        item.X1 = rect.x;
        item.Y1 = rect.y;
        item.X2 = rect.x + rect.cx - 1;
        item.Y2 = rect.y + rect.cy - 1;
        int len = progress * width / 100;
        text = std::wstring(len, 9608) + std::wstring(width - len, 9617);
        item.Data = text.c_str();
        item.UserData = (intptr_t)this;
        items.push_back(item);
    }

    int width;
    int progress;
    std::wstring text;
};

struct Group : public Control
{
    Group(Orientation orientation = eHorizontal):
        orientation(orientation),
        halign(eHCenter),
        valign(eVCenter),
        border(0)
    {
        horzFlexible = true;
        vertFlexible = true;
    }

    ~Group()
    {
        for (std::vector<Control*>::iterator it = controls.begin();it != controls.end();it++)
            delete *it;
    }

    virtual Size size()
    {
        Size size;
        for (std::vector<Control*>::iterator it = controls.begin();it != controls.end();it++)
        {
            Size s = (*it)->size();
            switch (orientation)
            {
                case eHorizontal:
                    if (it != controls.begin())
                        size.cx++;
                    size.cx = size.cx + s.cx;
                    size.cy = std::max(size.cy, s.cy);
                    break;
                case eVertical:
                    size.cx = std::max(size.cx, s.cx);
                    size.cy = size.cy + s.cy;
                    break;
            }
        }
        if (border)
        {
            size.cx += 4;
            size.cy += 2;
        }
        return size;
    }

    virtual void layout(Rect rect, std::vector<FarDialogItem>& items)
    {
        Size size = this->size();
        if (border)
        {
            FarDialogItem item;
            memset(&item, 0, sizeof(item));
            item.Type = border == 2 ? DI_DOUBLEBOX : DI_SINGLEBOX;
            item.X1 = rect.x;
            item.Y1 = rect.y;
            item.X2 = rect.x + rect.cx - 1;
            item.Y2 = rect.y + rect.cy - 1;
            if (!title.empty())
                item.Data = title.c_str();
            item.UserData = (intptr_t)this;
            items.push_back(item);
            rect.inflate(-2, -1);
            size.cx -= 4;
            size.cy -= 2;
        }

        int numFlex = 0;
        for (std::vector<Control*>::iterator it = controls.begin();it != controls.end();it++)
            if (((*it)->horzFlexible && orientation == eHorizontal) || ((*it)->vertFlexible && orientation == eVertical))
                numFlex++;

        int flexSize = 0;
        if (orientation == eHorizontal && size.cx < rect.cx)
        {
            flexSize = rect.cx - size.cx;
            if (numFlex == 0)
                switch (halign)
                {
                    case eHCenter: rect.x += flexSize / 2; break;
                    case eRight: rect.x += flexSize; break;
                }
            else
                flexSize /= numFlex;
        }
        else if (orientation == eVertical && size.cy < rect.cy)
        {
            flexSize = rect.cy - size.cy;
            if (numFlex == 0)
                switch (valign)
                {
                    case eVCenter: rect.y += flexSize / 2; break;
                    case eBottom: rect.y += flexSize; break;
                }
            else
                flexSize /= numFlex;
        }

        for (std::vector<Control*>::iterator it = controls.begin();it != controls.end();it++)
        {
            Size s = (*it)->size();
            Rect rc = rect;
            rc.cx = std::min(s.cx, rc.cx);
            rc.cy = std::min(s.cy, rc.cy);
            if (orientation == eHorizontal)
            {
                if (it != controls.begin())
                {
                    rc.x++;
                    rect.x++;
                }
                if ((*it)->vertFlexible)
                    rc.cy = rect.cy;
                if ((*it)->horzFlexible)
                    rc.cx += flexSize;
                rect.x += rc.cx;
            }
            else if (orientation == eVertical)
            {
                if ((*it)->horzFlexible)
                    rc.cx = rect.cx;
                else
                    switch (halign)
                    {
                        case eHCenter:
                            rc.x += (rect.cx - rc.cx) / 2;
                            break;
                        case eRight:
                            rc.x += rect.cx - rc.cx;
                            break;
                    }
                if (border && (*it)->separator)
                {
                    rc.x -= 3;
                    rc.cx += 6;
                }
                if ((*it)->vertFlexible)
                    rc.cy += flexSize;
                rect.y += rc.cy;
            }
            (*it)->layout(rc, items);
        }
    }

    void add(Control* control) { controls.push_back(control); }

    Orientation orientation;
    HAlign halign;
    VAlign valign;
    std::vector<Control*> controls;
    int border;
    std::wstring title;
};

class Dialog : public Group
{
public:
    Dialog(std::wstring title, bool warning = false):
        m_warning(warning)
    {
        m_handle = NULL;
        orientation = eVertical;
        border = 2;
        this->title = title;
    }

    ~Dialog()
    {
    }

    Control* Run();

private:
    typedef std::map<HANDLE, Dialog*> DialogMap;
    static DialogMap m_map;
    static intptr_t WINAPI DlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2);

protected:
    virtual intptr_t DlgProc(intptr_t Msg, intptr_t Param1, void *Param2);

protected:
    HANDLE m_handle;
    bool m_warning;
};
