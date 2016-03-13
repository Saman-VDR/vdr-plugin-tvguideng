#include "config.h"
#include "channelgroups.h"

// --- cChannelGroup  -------------------------------------------------------------

cChannelGroup::cChannelGroup(string name, int id) {
    this->id = id;
    channelStart = -1;
    channelStop = -1;
    this->name = name;
}

cChannelGroup::~cChannelGroup(void) {
}

void cChannelGroup::Debug(void) {
    dsyslog("tvguideng: id %d channel group %s, start %d, stop %d", id, name.c_str(), channelStart, channelStop);
}

// --- cChannelgroups  -------------------------------------------------------------

cChannelgroups::cChannelgroups(skindesignerapi::cViewGrid *channelgroupGrid) {
    this->channelgroupGrid = channelgroupGrid;
}

cChannelgroups::~cChannelgroups(void) {
    Clear();
}

void cChannelgroups::Init(void) {
    bool setStart = false;
    int lastChannelNumber = 0;
    int id = 0;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannels* channels = Channels;
#else
    const cChannels* channels = &Channels;
#endif
    const cChannel *first = channels->First();
    if (!first->GroupSep()) {
        channelGroups.push_back(cChannelGroup(tr("Main Program"), id++));
        setStart = true;
    }    
    for (const cChannel *channel = channels->First(); channel; channel = channels->Next(channel)) {
        if (setStart && (channelGroups.size() > 0)) {
            channelGroups[channelGroups.size()-1].SetChannelStart(channel->Number());
            setStart = false;
        }
        if (channel->GroupSep()) {
            if (channelGroups.size() > 0) {
                channelGroups[channelGroups.size()-1].SetChannelStop(lastChannelNumber);
            }
            channelGroups.push_back(cChannelGroup(channel->Name(), id++));
            setStart = true;
        } else {
            lastChannelNumber = channel->Number();
        }
    }
    if (channelGroups.size() > 0) {
        channelGroups[channelGroups.size()-1].SetChannelStop(lastChannelNumber);
    }
    //cleanup empty groups
    bool foundDelete = false;
    do {
        for (vector<cChannelGroup>::iterator it = channelGroups.begin(); it != channelGroups.end(); it++) {
            cChannelGroup group = *it;
            if (group.StartChannel() == 0 || group.StartChannel() == -1) {
                channelGroups.erase(it);
                foundDelete = true;
                break;
            }
            foundDelete = false;
        }
    } while (foundDelete);
}

void cChannelgroups::Clear(void) {
    channelgroupGrid->Clear();
}

void cChannelgroups::Draw(const cChannel *start, const cChannel *stop) {
    if (!start || !stop)
        return;
    
    int group = GetGroup(start);
    int groupLast = group;
    int fields = 1;
    double offset = 0.0;

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannels* channels = Channels;
#else
    const cChannels* channels = &Channels;
#endif

    for (const cChannel *channel = channels->Next(start); channel; channel = channels->Next(channel)) {
        if (channel->GroupSep())
            continue;
        if (config.hideLastChannelGroup && IsInLastGroup(channel)) {
            SetGroup(group, fields, offset);
            break;            
        }
        group = GetGroup(channel);
        if (group != groupLast) {
            offset = SetGroup(groupLast, fields, offset);
            fields = 0;
        }
        fields++;
        groupLast = group;
        if (channel == stop) {
            SetGroup(group, fields, offset);
            break;
        }
    }
    channelgroupGrid->Display();
}

double cChannelgroups::SetGroup(int groupId, int fields, double offset) {
    int channelsPerPage;
    double x, y, width, height;
    if (config.displayMode == eHorizontal) {
        channelsPerPage = config.channelsPerPageHorizontal;
        x = 0.0;
        y = offset;
        width = 1.0;
        height = (double)fields / (double)channelsPerPage;
        offset += height;
    } else {
        channelsPerPage = config.channelsPerPageVertical;
        x = offset;
        y = 0.0;
        width = (double)fields / (double)channelsPerPage;
        height = 1.0;
        offset += width;
    }
    channelgroupGrid->ClearTokens();
    channelgroupGrid->AddIntToken((int)eChannelgroupsGridIT::color, groupId % 2);
    channelgroupGrid->AddStringToken((int)eChannelgroupsGridST::group, channelGroups[groupId].GetName());
    channelgroupGrid->SetGrid(groupId, x, y, width, height);

    return offset;
}

int cChannelgroups::GetGroup(const cChannel *channel) {
    if (!channel)
        return -1;
    int channelNumber = channel->Number();
    for (vector<cChannelGroup>::iterator group = channelGroups.begin(); group != channelGroups.end(); group++) {
        if ( (*group).StartChannel() <= channelNumber && (*group).StopChannel() >= channelNumber ) {
            return (*group).GetId();
        }
    }
    return -1;
}

string cChannelgroups::GetPrevGroupName(int group) {
    if (group <= 0 || group > (int)channelGroups.size())
        return "";
    return channelGroups[group-1].GetName();
}

string cChannelgroups::GetNextGroupName(int group) {
    if (group < 0 || group >= (int)channelGroups.size() - 1)
        return "";
    return channelGroups[group+1].GetName();
}

int cChannelgroups::GetPrevGroupFirstChannel(int group) {
    if (group <= 0 || group > (int)channelGroups.size())
        return -1;
    return channelGroups[group-1].StartChannel();
}

int cChannelgroups::GetNextGroupFirstChannel(int group) {
    if (group < 0 || group >= (int)channelGroups.size())
        return -1;
    return channelGroups[group+1].StartChannel();
}

bool cChannelgroups::IsInFirstGroup(const cChannel *channel) {
    if (channelGroups.size() == 0)
        return false;
    int channelNumber = channel->Number();
    if (channelNumber >= channelGroups[0].StartChannel() && channelNumber <= channelGroups[0].StopChannel())
        return true;
    return false;
}

bool cChannelgroups::IsInLastGroup(const cChannel *channel) {
    size_t numGroups = channelGroups.size();
    if (numGroups == 0)
        return false;
    int channelNumber = channel->Number();
    if (channelNumber >= channelGroups[numGroups-1].StartChannel() && channelNumber <= channelGroups[numGroups-1].StopChannel()) {
        return true;
    }
    return false;
}

bool cChannelgroups::IsInSecondLastGroup(const cChannel *channel) {
    size_t numGroups = channelGroups.size();
    if (numGroups < 2)
        return false;
    int channelNumber = channel->Number();
    if (channelNumber >= channelGroups[numGroups-2].StartChannel() && channelNumber <= channelGroups[numGroups-2].StopChannel())
        return true;
    return false;
}

int cChannelgroups::GetLastValidChannel(void) {
    if (config.hideLastChannelGroup && channelGroups.size() > 1) {
        return channelGroups[channelGroups.size()-2].StopChannel();
    }
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
   LOCK_CHANNELS_READ;
   return Channels->MaxNumber();
#else
   return Channels.MaxNumber();
#endif

   
}
