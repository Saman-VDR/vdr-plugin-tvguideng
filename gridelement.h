#ifndef __TVGUIDE_GRID_H
#define __TVGUIDE_GRID_H

#include <vdr/tools.h>
#include <vdr/epg.h>

class cChannelEpg;

class cGridElement : public cListObject {
protected:
    static long idCounter;
    long id;
    bool init;
    bool active;
    bool hasTimer;
    bool hasSwitchTimer;
    cChannelEpg *owner;
    bool Intersects(cGridElement *neighbor);
    virtual time_t Duration(void) { return 0; };
    bool dummy;
public:
    cGridElement(cChannelEpg *owner);
    virtual ~cGridElement(void);
    bool IsNew(void) { return init; };
    void Dirty(void) { init = true; };
    void InitFinished(void) { init = false; };
    void SetActive(void) {active = true;};
    void SetInActive(void) {active = false;};
    bool Match(time_t t);
    virtual time_t StartTime(void) { return 0; };
    virtual time_t EndTime(void) { return 0; };
    virtual void SetStartTime(time_t start) {};
    virtual void SetEndTime(time_t end) {};
    int CalcOverlap(cGridElement *neighbor);
    virtual void SetTimer(void) {};
    virtual void SetSwitchTimer(void) {};
    long Id(void) { return id; };
    bool Active(void) { return active; };
    bool HasTimer(void) {return hasTimer;};
    bool HasSwitchTimer(void) {return hasSwitchTimer;};
    bool IsFirst(void);
    bool IsDummy(void) { return dummy; };
    const cChannel *Channel(void);
    virtual const char *Title(void) { return ""; };
    virtual const char *ShortText(void) { return ""; };
    virtual const cEvent *Event(void) { return NULL; };
    cChannelEpg *Owner(void);
    virtual void Debug(void) {};
};

#endif //__TVGUIDE_GRID_H