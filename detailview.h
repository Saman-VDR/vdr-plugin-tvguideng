#ifndef __TVGUIDE_DETAILVIEW_H
#define __TVGUIDE_DETAILVIEW_H

#include "config.h"
#include <libskindesignerapi/skindesignerosdbase.h>

class cDetailView {
private:
    bool init;
    int lastSecond;
    skindesignerapi::cOsdView *detailView;
    const cEvent *event;
    skindesignerapi::cViewElement *back;
    skindesignerapi::cViewElement *header;
    skindesignerapi::cViewElement *footer;
    skindesignerapi::cViewElement *watch;
    skindesignerapi::cViewTab *tabs;
    void DrawBackground(void);
    void DrawHeader(void);
    void DrawFooter(void);
    void SetTabTokens(void);
    bool LoadReruns(void);
    void SetScraperTokens(void);
    void SetEpgPictures(int eventId);
public:
    cDetailView(skindesignerapi::cOsdView *detailView, const cEvent *event);
    virtual ~cDetailView(void);
    void Draw(void);
    void Left(void);
    void Right(void);
    void Up(void);
    void Down(void);
    bool DrawTime(void);
    void Flush(void) { detailView->Display(); };
    const cEvent *GetEvent(void) { return event; };
};

#endif //__TVGUIDE_DETAILVIEW_H
