#ifndef __TVGUIDE_RECMENUVIEW_H
#define __TVGUIDE_RECMENUVIEW_H

#include "config.h"
#include <libskindesignerapi/skindesignerosdbase.h>
#include "recmanager.h"
#include "recmenus.h"

class cRecMenuView {
private:
    bool active;
    skindesignerapi::cOsdView *recMenuView;
    skindesignerapi::cOsdView *recMenuViewBuffer;
    skindesignerapi::cOsdView *recMenuViewBuffer2;
    const cEvent *event;
    const cEvent *displayEvent;
    cRecManager *recManager;
    cRecMenu *activeMenu;
    cRecMenu *activeMenuBuffer;
    cRecMenu *activeMenuBuffer2;
    cTVGuideTimerConflicts *timerConflicts;
    void DrawBackground(int menuWidth, int menuHeight, bool scrolling);
    void DisplayMenu(bool buffer = false, bool buffer2 = false);
    void DisplaySearchTimerList(void);
    bool DisplayTimerConflict(const cTimer *timer);
    bool DisplayTimerConflict(int timerID);
    void DisplayFavoriteResults(string header, const cEvent **result, int numResults);
    eOSState StateMachine(eRecMenuState nextState);
public:
    cRecMenuView(void);
    virtual ~cRecMenuView(void);
    void Init(skindesignerapi::cOsdView *recMenuView, skindesignerapi::cOsdView *recMenuViewBuffer, skindesignerapi::cOsdView *recMenuViewBuffer2);
    void DisplayRecMenu(const cEvent *event);
    void DisplayFavorites(void);
    void DisplayRecSearch(const cEvent *e);
    void DisplaySearchEPG(const cEvent *e);
    void Close(void);
    void Hide(bool full = false);
    void Activate(bool full = false);
    eOSState ProcessKey(eKeys Key);
    void Flush(void) { activeMenu->Flush(); };
    const cEvent *GetEvent(void) { return displayEvent; };
    bool Active(void) { return active; };
};

#endif //__TVGUIDE_RECMENUVIEW_H
