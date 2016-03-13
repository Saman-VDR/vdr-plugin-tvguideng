#ifndef __TVGUIDE_EPGGRID_H
#define __TVGUIDE_EPGGRID_H

#include <vdr/epg.h>
#include "gridelement.h"
#include "channelepg.h"

// --- cEpgGrid  -------------------------------------------------------------

class cEpgElement : public cGridElement {
private:
    const cEvent *event;
    time_t Duration(void) { return event->Duration(); };
public:
    cEpgElement(const cEvent *event, cChannelEpg *owner);
    virtual ~cEpgElement(void);
    const cEvent *GetEvent(void) {return event;};
    time_t StartTime(void) { return event->StartTime(); };
    time_t EndTime(void) { return event->EndTime(); };
    void SetTimer(void);
    void SetSwitchTimer(void);
    const char *Title(void);
    const char *ShortText(void);
    const cEvent *Event(void) { return event; };
    void Debug(void);
};

#endif //__TVGUIDE_EPGGRID_H