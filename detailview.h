#ifndef __TVGUIDE_DETAILVIEW_H
#define __TVGUIDE_DETAILVIEW_H

#include "config.h"
#include "libskindesigner/osdelements.h"

class cDetailView {
private:
    bool init;
    int lastSecond;
    cOsdView *detailView;
    const cEvent *event;
    cViewElement *back;
    cViewElement *header;
    cViewElement *footer;
    cViewElement *watch;
    cViewTab *tabs;
    void DrawBackground(void);
    void DrawHeader(void);
    void DrawFooter(void);
    void SetTabTokens(void);
    bool LoadReruns(void);
    void SetScraperTokens(void);
    void SetEpgPictures(int eventId);
public:
    cDetailView(cOsdView *detailView, const cEvent *event);
    virtual ~cDetailView(void);
    void Draw(void);
    void Left(void);
    void Right(void);
    void Up(void);
    void Down(void);
    bool DrawTime(void);
    void Flush(void) { detailView->Display(); };
};

#endif //__TVGUIDE_DETAILVIEW_H
