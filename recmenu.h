#ifndef __TVGUIDE_RECMENU_H
#define __TVGUIDE_RECMENU_H

#include <list>
#include "libskindesigner/osdelements.h"
#include "recmenuitem.h"

// --- cRecMenu  -------------------------------------------------------------

class cRecMenu {
private:
    bool hidden;
    int menuWidth;
    int menuHeight;
    int maxMenuHeight;
    cOsdView *osdView;
    cViewElement *back;
    cViewElement *scrollBar;
    cViewGrid *recMenuGrid;
    cList<cRecMenuItem> menuItems;
    cRecMenuItem *start;
    cRecMenuItem *stop;
    int itemCount;
    cRecMenuItem *active;
    cRecMenuItem *header;
    cRecMenuItem *footer;
    bool scrolling;
    bool SeekForward(bool page);
    bool SeekBack(bool page);
    void AdjustActive(bool down);
    void DrawScrollbar(void);
    void DrawHeader(void);
    void DrawFooter(void);
protected:
    void SetMenuWidth(int width) { menuWidth = width; };
    void AddMenuItem(cRecMenuItem *item, bool inFront = false);
    void AddHeader(cRecMenuItem *header);
    void AddFooter(cRecMenuItem *footer);
    int GetNumActive(void);
    cRecMenuItem *GetActive(void) { return active; };
    bool ScrollUp(bool retry);
    bool ScrollDown(bool retry);
    bool PageUp(void);
    bool PageDown(void);
    void ClearMenuItems(void);
    void InitMenuItems(void);
    int GetWidth(void) { return menuWidth; };
    int GetHeight(void);
public:
    cRecMenu(void);
    virtual ~cRecMenu(void);
    void Init(cOsdView *osdView);
    void DrawBackground(void);
    void Draw(void);
    void Hide(void) { hidden = true; osdView->Deactivate(true); };
    void Show(void) { hidden = false; osdView->Activate(); };
    void Flush(void) { osdView->Display(); };
    virtual eRecMenuState ProcessKey(eKeys Key);
};
#endif //__TVGUIDE_RECMENU_H