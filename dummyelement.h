#ifndef __TVGUIDE_DUMMYGRID_H
#define __TVGUIDE_DUMMYGRID_H

#include "gridelement.h"

// --- cDummyGrid  -------------------------------------------------------------

class cDummyElement : public cGridElement {
private:
    time_t start;
    time_t end;
    time_t Duration(void);
public:
    cDummyElement(time_t start, time_t end, cChannelEpg *owner);
    virtual ~cDummyElement(void);
    time_t StartTime(void) { return start; };
    time_t EndTime(void) { return end; };
    void SetStartTime(time_t start) { this->start = start; };
    void SetEndTime(time_t end) { this->end = end; };
    const char *Title(void);
    void Debug(void);
};

#endif //__TVGUIDE_DUMMYGRID_H