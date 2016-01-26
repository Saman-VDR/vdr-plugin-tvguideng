#ifndef __TVGUIDE_TIMELINE_H
#define __TVGUIDE_TIMELINE_H

#include <set>
#include <vdr/tools.h>
#include <libskindesignerapi/skindesignerosdbase.h>
#include "definitions.h"
#include "config.h"
#include "timemanager.h"

// --- cTimelineElement  -------------------------------------------------------------

class cTimelineElement : public cListObject {
private:
    static long idCounter;
    long id;
    bool init;
    time_t elementTime;
public:
    cTimelineElement(time_t elementTime);
    virtual ~cTimelineElement(void);
    long Id(void) { return id; };
    bool IsNew(void);
    bool IsFullHour(void);
    string ToString(void) { return *TimeString(elementTime); };
};

// --- cTimeline  -------------------------------------------------------------

class cTimeline {
private:
    skindesignerapi::cViewGrid *timelineGrid;
    skindesignerapi::cViewElement *timelineDate;
    skindesignerapi::cViewElement *timeIndicator;
    cTimeManager *timeManager;
    cList<cTimelineElement> grids;
    int steps;
    int stepDuration;
    string weekday;
    bool timeIndicatorShown;
    void DrawDate(void);
    void DrawTimeIndicator(void);
public:
    cTimeline(skindesignerapi::cViewGrid *timelineGrid, skindesignerapi::cViewElement *timelineDate, skindesignerapi::cViewElement *timeIndicator, cTimeManager *timeManager);
    virtual ~cTimeline(void);
    void Init(void);
    void Clear(void);
    void ScrollForward(int stepMinutes);
    void ScrollBack(int stepMinutes);
    void Draw(void);
};

#endif //__TVGUIDE_TIMELINE_H
