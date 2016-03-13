#ifndef __TVGUIDE_DETAILVIEW_H
#define __TVGUIDE_DETAILVIEW_H

#include "services/scraper2vdr.h"
#include "services/epgsearch.h"
#include "config.h"
#include <libskindesignerapi/skindesignerosdbase.h>

class cDetailView {
private:
    bool init;
    int lastSecond;
    skindesignerapi::cOsdView *detailView;
    const cEvent *event;
    cMovie *movie;
    cSeries *series;
    skindesignerapi::cViewElement *back;
    skindesignerapi::cViewElement *header;
    skindesignerapi::cViewElement *footer;
    skindesignerapi::cViewElement *watch;
    skindesignerapi::cViewTab *tabs;
    void DrawBackground(void);
    void DrawHeader(void);
    void DrawFooter(void);
    void SetTabTokens(void);
    bool LoadScrapInfo(const cEvent *event);
    int NumActors(void);
    void SetScraperTokens(void);
    cList<Epgsearch_searchresults_v1_0::cServiceSearchResult> *LoadReruns(void);
    int NumReruns(cList<Epgsearch_searchresults_v1_0::cServiceSearchResult> *reruns);
    void SetReruns(cList<Epgsearch_searchresults_v1_0::cServiceSearchResult> *reruns);
    void SetEpgPictures(int eventId);
public:
    cDetailView(skindesignerapi::cOsdView *detailView, const cEvent *event);
    virtual ~cDetailView(void);
    void Activate(void) { detailView->Activate(); };
    void Draw(void);
    void Left(void);
    void Right(void);
    void Up(void);
    void Down(void);
    bool DrawTime(void);
    void Flush(void) { detailView->Display(); };
    const cEvent *GetEvent(void) { return event; };
    static void DefineTokens(eViewElementsDetail ve, skindesignerapi::cTokenContainer *tk);
    static void DefineTabTokens(skindesignerapi::cTokenContainer *tk);
};

#endif //__TVGUIDE_DETAILVIEW_H
