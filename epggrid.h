#ifndef __TVGUIDE_EPGGRID_H
#define __TVGUIDE_EPGGRID_H

#include "config.h"
#include "channelepg.h"
#include "timeline.h"
#include "channelgroups.h"
#include "channeljump.h"
#include <libskindesignerapi/skindesignerosdbase.h>

class cEpgGrid {
private:
    int lastSecond;
    int channelsPerPage;
    int channelGroupLast;
    long oldActiveGridId;
    skindesignerapi::cOsdView *rootView;
    cTimeManager *timeManager;
    skindesignerapi::cViewElement *back;
    skindesignerapi::cViewElement *header;
    skindesignerapi::cViewElement *watch;
    skindesignerapi::cViewElement *footer;
    skindesignerapi::cViewGrid *channelsGrid;
    skindesignerapi::cViewGrid *epgGrid;
    cTimeline *timeline;
    cChannelgroups *channelGroups;
    cList<cChannelEpg> channels;
    cGridElement *active;
    set<int> deletedChannels;
    void SetActiveGrid(cGridElement *newActive);
    void ScrollForward(void);
    void ScrollBack(void);
    const cChannel *SeekChannelForward(int num);
    const cChannel *SeekChannelBack(int num);
    void ClearOutdatedChannelHeaders(void);
public:
    cEpgGrid(skindesignerapi::cOsdView *rootView, cTimeManager *timeManager);
    virtual ~cEpgGrid(void);
    void Deactivate(bool hide) { lastSecond = -1; rootView->Deactivate(hide); };
    void Activate(void) { DrawTime(); rootView->Activate(); };
    void Init(const cChannel *startChannel);
    void CreateChannels(const cChannel *startChannel, int activeChannel);
    void Clear(void);
    void SetTimers(void);
    void RebuildEpgGrid(void);
    bool TimeForward(void);
    bool TimeBack(void);
    bool ChannelForward(void);
    bool ChannelBack(void);
    const cChannel *GetNextChannelNumJump(void);
    const cChannel *GetPrevChannelNumJump(void);
    const cChannel *GetNextChannelGroupJump(void);
    const cChannel *GetPrevChannelGroupJump(void);
    bool IsFirstGroup(void);
    bool IsLastGroup(void);
    bool IsSecondLastGroup(void);
    const cChannel *GetCurrentChannel(void);
    int GetLastValidChannel(void);
    const cEvent *GetCurrentEvent(void);
    void DrawChannelHeaders(void);
    void DrawTimeline(void);
    void DrawGrid(void);
    void UpdateActive(void);
    void DrawHeader(void);
    void DrawFooter(void);
    void DrawChannelgroups(void);
    bool DrawTime(void);
    void Flush(void) { rootView->Display(); };
    cChannelJump *GetChannelJumper(void);
    void Debug(void);
};

#endif //__TVGUIDE_EPGGRID_H
