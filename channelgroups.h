#ifndef __TVGUIDE_CHANNELGROUPS_H
#define __TVGUIDE_CHANNELGROUPS_H

#include <set>
#include <vdr/tools.h>
#include "libskindesigner/osdelements.h"
#include "config.h"

// --- cChannelGroup  -------------------------------------------------------------

class cChannelGroup : public cListObject {
private:
    int id;
    int channelStart;
    int channelStop;
    string name;
public:
    cChannelGroup(string name, int id);
    virtual ~cChannelGroup(void);
    int GetId(void) { return id; };
    void SetChannelStart(int start) { channelStart = start; };
    int StartChannel(void) { return channelStart; };
    void SetChannelStop(int stop) { channelStop = stop; };
    int StopChannel(void) { return channelStop; };
    string GetName(void) { return name; };
    void Debug(void);
};


// --- cChannelgroups  -------------------------------------------------------------

class cChannelgroups {
private:
    cViewGrid *channelgroupGrid;
    vector<cChannelGroup> channelGroups;
    double SetGroup(int groupId, int fields, double offset);
public:
    cChannelgroups(cViewGrid *channelgroupGrid);
    virtual ~cChannelgroups(void);
    void Init(void);
    void Clear(void);
    void Draw(const cChannel *start, const cChannel *stop);
    int GetGroup(const cChannel *channel);
    string GetPrevGroupName(int group);
    string GetNextGroupName(int group);
    int GetPrevGroupFirstChannel(int group);
    int GetNextGroupFirstChannel(int group);
    bool IsInFirstGroup(const cChannel *channel);
    bool IsInLastGroup(const cChannel *channel);
    bool IsInSecondLastGroup(const cChannel *channel);
    int GetLastValidChannel(void);
};

#endif //__TVGUIDE_TIMELINE_H
