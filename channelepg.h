#ifndef __TVGUIDE_CHANNELEPG_H
#define __TVGUIDE_CHANNELEPG_H

#include <set>
#include <vdr/tools.h>
#include "libskindesigner/osdelements.h"
#include "config.h"
#include "gridelement.h"
#include "epgelement.h"
#include "dummyelement.h"
#include "timemanager.h"
#include "switchtimer.h"

// --- cChannelEpg  -------------------------------------------------------------

class cChannelEpg : public cListObject {
private:
    bool init;
    int channelsPerPage;
    cTimeManager *timeManager;
    int position;
    const cChannel *channel;
    cList<cGridElement> grids;
    set<long> deletedElements;
    cSchedulesLock *schedulesLock;
    const cSchedules *schedules;
    bool hasTimer;
    bool hasSwitchTimer;
    cGridElement *AddEpgGrid(const cEvent *event, cGridElement *after = NULL);
    cGridElement *AddDummyGrid(time_t start, time_t end, cGridElement *after = NULL);
public:
    cChannelEpg(int position, const cChannel *channel, cTimeManager *timeManager);
    virtual ~cChannelEpg(void);
    void SetPosition(int newPos) { position = newPos; };
    bool ReadGrids(void);
    const cChannel *Channel(void) { return channel; };
    const char* Name(void) { return channel->Name(); };
    int GetChannelNumber(void) {return channel->Number();}
    cGridElement * GetActive(void);
    cGridElement * GetNext(cGridElement *activeGrid);
    cGridElement * GetPrev(cGridElement *activeGrid);
    cGridElement * GetNeighbor(cGridElement *activeGrid);
    bool IsFirst(cGridElement *grid);
    void AddNewGridsAtStart(void);
    void AddNewGridsAtEnd(void);
    void ClearOutdatedStart(void);
    void ClearOutdatedEnd(void);
    void SetTimers(void);
    void SetTimer(void) { hasTimer = channel->HasTimer(); };
    bool HasTimer(void) { return hasTimer; };
    void SetSwitchTimer() {hasSwitchTimer = SwitchTimers.ChannelInSwitchList(channel);};
    bool HasSwitchTimer() { return hasSwitchTimer; };
    void ClearGrids(void);
    void DrawHeader(cViewGrid *channelsGrid);
    void DrawGrids(cViewGrid *epgGrid);
    void DeleteOutdated(cViewGrid *epgGrid);
    void DeleteGridViews(cViewGrid *epgGrid);
    void Debug(void);
};

#endif //__TVGUIDE_CHANNELEPG_H
