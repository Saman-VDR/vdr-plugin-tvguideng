#ifndef __TVGUIDE_SWITCHTIMER_H
#define __TVGUIDE_SWITCHTIMER_H

#include <vdr/plugin.h>
#include "config.h"

class cSwitchTimer : public cListObject {
public:
    tEventID eventID;
    time_t startTime;
    tChannelID channelID;
    int switchMinsBefore;
    int announceOnly;
    cSwitchTimer(void);
    cSwitchTimer(const cEvent* Event);
    bool Parse(const char *s);
    void SetEventID(tEventID eventID) { this->eventID = eventID; };
    void SetStartTime(time_t startTime) { this->startTime = startTime; };
};

class cSwitchTimers : public cConfig<cSwitchTimer>, public cMutex {
public:
    cSwitchTimers(void) {}
    ~cSwitchTimers(void) {}
    bool EventInSwitchList(const cEvent* event);
    bool ChannelInSwitchList(const cChannel* channel);
    void DeleteSwitchTimer(const cEvent *event);
};

extern cSwitchTimers SwitchTimers;

#endif //__TVGUIDE_SWITCHTIMER_H
