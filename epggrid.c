#include "tvguidengosd.h"
#include "epggrid.h"
#include "helpers.h"
#include "services/scraper2vdr.h"

cEpgGrid::cEpgGrid(skindesignerapi::cOsdView *rootView, cTimeManager *timeManager) {
    lastSecond = -1;
    channelGroupLast = -1;
    active = NULL;
    oldActiveGridId = -1;

    this->rootView = rootView;
    this->timeManager = timeManager;
    
    channelsPerPage = (config.displayMode == eHorizontal) ? config.channelsPerPageHorizontal : config.channelsPerPageVertical;

    back = rootView->GetViewElement(config.displayMode == eHorizontal ? (int)eViewElementsRoot::backgroundHor : (int)eViewElementsRoot::backgroundVer);
    back->Display();

    header = rootView->GetViewElement(config.displayMode == eHorizontal ? (int)eViewElementsRoot::headerHor : (int)eViewElementsRoot::headerVer);
    footer = rootView->GetViewElement(config.displayMode == eHorizontal ? (int)eViewElementsRoot::footerHor : (int)eViewElementsRoot::footerVer);
    watch = rootView->GetViewElement(config.displayMode == eHorizontal ? (int)eViewElementsRoot::timeHor : (int)eViewElementsRoot::timeVer);

    channelsGrid = rootView->GetViewGrid(config.displayMode == eHorizontal ? (int)eViewGridsRoot::channelsHor : (int)eViewGridsRoot::channelsVer);
    epgGrid = rootView->GetViewGrid(config.displayMode == eHorizontal ? (int)eViewGridsRoot::schedulesHor : (int)eViewGridsRoot::schedulesVer);

    skindesignerapi::cViewElement *timelineDate = rootView->GetViewElement(config.displayMode == eHorizontal ? (int)eViewElementsRoot::dateTimelineHor : (int)eViewElementsRoot::dateTimelineVer);
    skindesignerapi::cViewElement *timeIndicator = rootView->GetViewElement(config.displayMode == eHorizontal ? (int)eViewElementsRoot::timeIndicatorHor : (int)eViewElementsRoot::timeIndicatorVer);
    skindesignerapi::cViewGrid *timelineGrid = rootView->GetViewGrid(config.displayMode == eHorizontal ? (int)eViewGridsRoot::timelineHor : (int)eViewGridsRoot::timelineVer);
    timeline = new cTimeline(timelineGrid, timelineDate, timeIndicator, timeManager);

    skindesignerapi::cViewGrid *channelgroupsGrid = rootView->GetViewGrid(config.displayMode == eHorizontal ? (int)eViewGridsRoot::channelGroupsHor : (int)eViewGridsRoot::channelGroupsVer);
    channelGroups = new cChannelgroups(channelgroupsGrid);
    channelGroups->Init();    
}

cEpgGrid::~cEpgGrid(void) {
    delete back;
    delete header;
    delete footer;
    for (cChannelEpg *channelEpg = channels.First(); channelEpg; channelEpg = channels.Next(channelEpg)) {
        channelEpg->DeleteGridViews(epgGrid);
    }
    channels.Clear();
    delete channelsGrid;
    delete channelGroups;
    delete epgGrid;
    delete timeline;
    delete watch;
    delete rootView;
}

void cEpgGrid::Init(const cChannel *startChannel) {
    if (!startChannel)
        return;
    timeline->Init();
    int numBack = channelsPerPage / 2;
    int offset = 0;
    const cChannel *newStartChannel = startChannel;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannels* channels = Channels;
#else
    const cChannels* channels = &Channels;
#endif

    for (; newStartChannel ; newStartChannel = channels->Prev(newStartChannel)) {
        if (newStartChannel && !newStartChannel->GroupSep()) {
            offset++;
        }
        if (offset == numBack)
            break;
    }
    if (!newStartChannel)
        newStartChannel = channels->First();
    offset--;
    if (offset < 0)
        offset = 0;

    CreateChannels(newStartChannel, offset);
}

void cEpgGrid::CreateChannels(const cChannel *startChannel, int activeChannel) {
    int pos = 0;
    bool foundEnough = false;
    channels.Clear();
    if (!startChannel)
        return;

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
   LOCK_CHANNELS_READ;
   const cChannels* chls = Channels;
#else
   cChannels* chls = &Channels;
#endif

    for (const cChannel *channel = startChannel; channel; channel = chls->Next(channel)) {
        if (config.hideLastChannelGroup && channelGroups->IsInLastGroup(channel)) {
            break;
        }
        if (!channel->GroupSep()) {
            cChannelEpg *channelEpg = new cChannelEpg(pos, channel, timeManager);
            if (channelEpg->ReadGrids()) {
                channels.Add(channelEpg);
                if (pos == activeChannel) {
                    active = channelEpg->GetActive();
                    if (active) {
                        active->SetActive();
                    }
                }
                pos++;
            } else {
                delete channelEpg;
            }
        }
        if (pos == channelsPerPage) {
            foundEnough = true;
            break;
        }
    }
    if (!foundEnough) {
        int numCurrent = channels.Count();
        int numBack = channelsPerPage - numCurrent;
        cChannelEpg *first = channels.First();
        int newChannelNumber = 1;
        if (first)
            newChannelNumber = first->GetChannelNumber() - numBack;
        const cChannel *newStart = chls->GetByNumber(newChannelNumber);
        CreateChannels(newStart, pos+1);
    }
}

void cEpgGrid::Clear(void) {
    timeline->Clear();
    channelsGrid->Clear();
    channels.Clear();
    header->Clear();
    footer->Clear();
    active = NULL;
    oldActiveGridId = -1;
    epgGrid->Clear();
}

void cEpgGrid::SetTimers(void) {
    for (cChannelEpg *channelEpg = channels.First(); channelEpg; channelEpg = channels.Next(channelEpg)) {
        channelEpg->SetTimers();
    }    
}


void cEpgGrid::RebuildEpgGrid(void) {
    if (!active)
        return;
    int activeChannelNumber = active->Channel()->Number();
    oldActiveGridId = -1;
    active = NULL;
    epgGrid->Clear();
    timeline->Clear();
    timeline->Init();
    for (cChannelEpg *channelEpg = channels.First(); channelEpg; channelEpg = channels.Next(channelEpg)) {
        channelEpg->ClearGrids();
        channelEpg->ReadGrids();
        if (channelEpg->Channel()->Number() == activeChannelNumber) {
            active = channelEpg->GetActive();
            active->SetActive();
        }
    }
}


void cEpgGrid::SetActiveGrid(cGridElement *newActive) {
    if (!newActive || !active) {
        return;
    }
    oldActiveGridId = active->Id();
    if (newActive->Id() == active->Id()) {
        return;
    }
    active->SetInActive();
    active = newActive;
    active->SetActive();
}

bool cEpgGrid::TimeForward(void) {
    bool scrolled = false;
    if (!active) {
        return false;
    }
    bool actionDone = false;
    int minutesLeft = (timeManager->GetEnd() - active->EndTime())/60;
    if (  minutesLeft < 30 ) {
        actionDone = true;
        ScrollForward();
        scrolled = true;
    }
    cGridElement *next = (cGridElement*)active->Next();
    if (next) {
        if ( (next->EndTime() < timeManager->GetEnd()) || ( (timeManager->GetEnd() - next->StartTime())/60 > 30 ) ) {
            SetActiveGrid(next);
            actionDone = true;
        }
    }
    if (!actionDone) {
        ScrollForward();
        scrolled = true;
    }
    return scrolled;
}

bool cEpgGrid::TimeBack(void) {
    bool scrolled = false;
    if (!active) {
        return false;
    }
    
    if (timeManager->IsStart(active->StartTime())) {
        oldActiveGridId = active->Id();
        return false;
    }

    bool actionDone = false;
    int minutesLeft = (active->StartTime() - timeManager->GetStart())/60;
    if (  minutesLeft < 30 ) {
        actionDone = true;
        ScrollBack();
        scrolled = true;
    }
    cGridElement *prev = (cGridElement*)active->Prev();
    if (prev) {
        if (   (prev->StartTime() > timeManager->GetStart())
            || ( (prev->EndTime() - timeManager->GetStart())/60 > 30 )
            || ( prev->IsFirst()) ) {
            SetActiveGrid(prev);
            actionDone = true;
        }
    }
    if (!actionDone) {
        ScrollBack();
        scrolled = true;
    }
    return scrolled;
}

bool cEpgGrid::ChannelForward(void) {
    if (!active) {
        return false;
    }
    //Scrolling in Grid
    cChannelEpg *channelNext = channels.Next(active->Owner());
    if (!channelNext && config.hideLastChannelGroup) {
        const cChannel *next = SeekChannelForward(1);
        if (next == active->Channel())
            return false;
    }

    if (channelNext) {
        cGridElement *neighbor = channelNext->GetNeighbor(active);
        if (neighbor) {
            SetActiveGrid(neighbor);
        }
        return false;
    }
    //end of grid reached, scrolling half page forward
    int numJumpForward = channelsPerPage/2;
    const cChannel *currentChannel = active->Channel();
    const cChannel *newLastChannel = SeekChannelForward(numJumpForward);
    if (!newLastChannel)
        return false;
    //insert new channels at end
    int numInserted = 0;

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannels* chls = Channels;
#else
    const cChannels* chls = &Channels;
#endif

    for (const cChannel *channel = (const cChannel*)currentChannel->Next(); channel ; channel = chls->Next(channel)) {
        if (channel->GroupSep()) {
            continue;
        }
        cChannelEpg *channelEpg = new cChannelEpg(0, channel, timeManager);
        if (channelEpg->ReadGrids()) {
            channels.Add(channelEpg);
            numInserted++;
        }
        if (numInserted == 1) {
            cGridElement *neighbor = channelEpg->GetNeighbor(active);
            if (neighbor) {
                SetActiveGrid(neighbor);
            }
        }
        if (channel->Number() == newLastChannel->Number())
            break;
    }
    if (!numInserted)
        return false;
    //delete first channels
    deletedChannels.clear();
    for (int i=0; i < numInserted; i++) {
        cChannelEpg *first = channels.First();
        first->DeleteGridViews(epgGrid);
        deletedChannels.insert(first->Channel()->Number());
        channels.Del(first);
    }
    //renumber channels
    int newPos = 0;
    for (cChannelEpg *channelEpg = channels.First(); channelEpg; channelEpg = channels.Next(channelEpg)) {
        channelEpg->SetPosition(newPos);
        newPos++;
    }
    
    return true;
}

bool cEpgGrid::ChannelBack(void) {
    if (!active) {
        return false;
    }
    //Scrolling in Grid
    cChannelEpg *channelPrev = channels.Prev(active->Owner());
    if (channelPrev) {
        cGridElement *neighbor = channelPrev->GetNeighbor(active);
        if (neighbor) {
            SetActiveGrid(neighbor);
        }
        return false;
    }
    //start of grid reached, scrolling half page back
    int numJumpBack = channelsPerPage/2;
    const cChannel *currentChannel = active->Channel();
    const cChannel *newFirstChannel = SeekChannelBack(numJumpBack);
    if (!newFirstChannel)
        return false;
    //insert new channels at start
    int numInserted = 0;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannels* chls = Channels;
#else
    const cChannels* chls = &Channels;
#endif
    for (const cChannel *channel = (const cChannel*)currentChannel->Prev(); channel ; channel = chls->Prev(channel)) {
        if (channel->GroupSep()) {
            continue;
        }
        cChannelEpg *channelEpg = new cChannelEpg(0, channel, timeManager);
        if (channelEpg->ReadGrids()) {
            channels.Ins(channelEpg, channels.First());
            numInserted++;
        }
        if (numInserted == 1) {
            cGridElement *neighbor = channelEpg->GetNeighbor(active);
            if (neighbor) {
                SetActiveGrid(neighbor);
            }
        }
        if (channel->Number() == newFirstChannel->Number())
            break;
    }
    if (!numInserted)
        return false;
    //delete last channels
    deletedChannels.clear();
    for (int i=0; i < numInserted; i++) {
        cChannelEpg *last = channels.Last();
        last->DeleteGridViews(epgGrid);
        deletedChannels.insert(last->Channel()->Number());
        channels.Del(last);
    }
    //renumber channels
    int newPos = 0;
    for (cChannelEpg *channelEpg = channels.First(); channelEpg; channelEpg = channels.Next(channelEpg)) {
        channelEpg->SetPosition(newPos);
        newPos++;
    }
    return true;
}

const cChannel *cEpgGrid::GetNextChannelNumJump(void) {
    const cChannel *newFirst = SeekChannelForward(channelsPerPage);
    if (!newFirst)
    {
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    newFirst = Channels->Last();
#else
    newFirst = Channels.Last();
#endif
    }
    
    return newFirst;
}

const cChannel *cEpgGrid::GetPrevChannelNumJump(void) {
    const cChannel *newFirst = SeekChannelBack(channelsPerPage);
    if (!newFirst)
    {
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    newFirst = Channels->First();
#else
    newFirst = Channels.First();
#endif
    }
    return newFirst;
}

const cChannel *cEpgGrid::GetNextChannelGroupJump(void) {
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannels* chls = Channels;
#else
    cChannels* chls = &Channels;
#endif

    if (!active)
        return chls->Last();

    int currentGroup = channelGroups->GetGroup(active->Channel());
    int nextChannelNumber = channelGroups->GetNextGroupFirstChannel(currentGroup);
    const cChannel *next = chls->GetByNumber(nextChannelNumber);
    if (next)
        return next;
    return chls->Last();
}

const cChannel *cEpgGrid::GetPrevChannelGroupJump(void) {
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannels* chls = Channels;
#else
    cChannels* chls = &Channels;
#endif
    if (!active)
        return chls->First();

    int currentGroup = channelGroups->GetGroup(active->Channel());
    int prevChannelNumber = channelGroups->GetPrevGroupFirstChannel(currentGroup);
    const cChannel *prev = chls->GetByNumber(prevChannelNumber);
    if (prev)
        return prev;
    return chls->First();
}

bool cEpgGrid::IsFirstGroup(void) {
    if (!active) {
        return true;
    }
    return channelGroups->IsInFirstGroup(active->Channel());
}

bool cEpgGrid::IsLastGroup(void) {
    if (!active) {
        return true;
    }
    return channelGroups->IsInLastGroup(active->Channel());
}

bool cEpgGrid::IsSecondLastGroup(void) {
    if (!active) {
        return true;
    }
    return channelGroups->IsInSecondLastGroup(active->Channel());
}

const cChannel *cEpgGrid::GetCurrentChannel(void) {
    if (!active) {
        return NULL;
    }
    return active->Channel();
}

int cEpgGrid::GetLastValidChannel(void) {
    return channelGroups->GetLastValidChannel();
}

const cEvent *cEpgGrid::GetCurrentEvent(void) {
    if (!active)
        return NULL;
    return active->Event();
}

void cEpgGrid::DrawChannelHeaders(void) {
    for (set<int>::iterator it = deletedChannels.begin(); it != deletedChannels.end(); it++) {
        channelsGrid->Delete(*it);
    }
    for (cChannelEpg *channel = channels.First(); channel; channel = channels.Next(channel)) {
        channel->DrawHeader(channelsGrid);
    }
    channelsGrid->Display();
}

void cEpgGrid::DrawChannelgroups(void) {
    channelGroups->Clear();
    channelGroups->Draw(channels.First()->Channel(), channels.Last()->Channel());
}

void cEpgGrid::DrawTimeline(void) {
    timeline->Draw();
}

void cEpgGrid::DrawGrid(void) {
    if (!active) {
        return;
    }
    epgGrid->SetCurrent(oldActiveGridId, false);
    for (cChannelEpg *channel = channels.First(); channel; channel = channels.Next(channel)) {
        channel->DeleteOutdated(epgGrid);
        channel->DrawGrids(epgGrid);
    }
    epgGrid->Display();
}

void cEpgGrid::UpdateActive(void) {
    if (!active) {
        return;
    }
    if (oldActiveGridId == active->Id())
        return;
    epgGrid->SetCurrent(oldActiveGridId, false);
    epgGrid->SetCurrent(active->Id(), true);
    epgGrid->Display();
}

void cEpgGrid::DrawHeader(void) {
    if (!active)
        return;
    if (oldActiveGridId == active->Id())
        return;

    int isDummy = 0;
    const char *title = active->Title();
    const char *shorttext = "";
    const char *description = "";
    string start = "";
    string stop = "";
    string day = "";
    string date = "";
    int daynumeric = 0;
    int month = 0;
    int year = 0;
    int running = 0;
    int elapsed = 0;
    int duration = 0;
    int durationhours = 0;
    string durationminutes = "";
    string channelname = "";
    int channelnumber = 0;
    string channelid = "";
    int channellogoexists = 0;
    int hasposter = 0;
    int posterwidth = -1;
    int posterheight = -1;
    string posterpath = "";

    header->ClearTokens();
    header->Clear();
    if (active->IsDummy()) {
        isDummy = 1;
        start = *(TimeString(active->StartTime()));
        stop = *(TimeString(active->EndTime()));
    } else {
        const cEvent *event = active->Event();
        if (event) {
            shorttext = event->ShortText() ? event->ShortText() : "";
            description = event->Description() ? event->Description() : "";
            start = *(event->GetTimeString());
            stop = *(event->GetEndTimeString());
            time_t startTime = event->StartTime();
            day = *WeekDayName(startTime);
            date = *ShortDateString(startTime);
            struct tm * sStartTime = localtime(&startTime);
            year = sStartTime->tm_year + 1900;
            daynumeric = sStartTime->tm_mday;
            month = sStartTime->tm_mon+1;
            time_t now = time(NULL);
            if ((now >= event->StartTime()) && (now <= event->EndTime()))
                running = 1;
            if (running) {
                elapsed = (now - event->StartTime())/60;
            }
            duration = event->Duration() / 60;
            durationhours = event->Duration() / 3600;
            durationminutes = *cString::sprintf("%.2d", (event->Duration() / 60)%60);

            static cPlugin *pScraper = GetScraperPlugin();
            if (pScraper) {
                ScraperGetPoster call;
                call.event = event;
                if (pScraper->Service("GetPoster", &call)) {
                    hasposter = FileExists(call.poster.path);
                    posterwidth = call.poster.width;
                    posterheight = call.poster.height;
                    posterpath = call.poster.path;
                }
            }
        }
        const cChannel *channel = active->Channel();
        if (channel) {
            channelname = channel->Name() ? channel->Name() : "";
            channelid = *(channel->GetChannelID().ToString());
            channelnumber = channel->Number();
            channellogoexists = header->ChannelLogoExists(channelid);
       }
    }
    header->AddIntToken((int)eHeaderIT::isdummy, isDummy);
    header->AddStringToken((int)eHeaderST::title, title);
    header->AddStringToken((int)eHeaderST::shorttext, shorttext);
    header->AddStringToken((int)eHeaderST::description, description);
    header->AddStringToken((int)eHeaderST::start, start.c_str());
    header->AddStringToken((int)eHeaderST::stop, stop.c_str());
    header->AddStringToken((int)eHeaderST::day, day.c_str());
    header->AddStringToken((int)eHeaderST::date, date.c_str());
    header->AddIntToken((int)eHeaderIT::daynumeric, daynumeric);
    header->AddIntToken((int)eHeaderIT::month, month);
    header->AddIntToken((int)eHeaderIT::year, year);
    header->AddIntToken((int)eHeaderIT::running, running);
    header->AddIntToken((int)eHeaderIT::elapsed, elapsed);
    header->AddIntToken((int)eHeaderIT::duration, duration);
    header->AddIntToken((int)eHeaderIT::durationhours, durationhours);
    header->AddStringToken((int)eHeaderST::durationminutes, durationminutes.c_str());
    header->AddStringToken((int)eHeaderST::channelname, channelname.c_str());
    header->AddStringToken((int)eHeaderST::channelid, channelid.c_str());
    header->AddIntToken((int)eHeaderIT::channelnumber, channelnumber);
    header->AddIntToken((int)eHeaderIT::channellogoexists, channellogoexists);
    header->AddIntToken((int)eHeaderIT::hasposter, hasposter);
    header->AddIntToken((int)eHeaderIT::posterwidth, posterwidth);
    header->AddIntToken((int)eHeaderIT::posterheight, posterheight);
    header->AddStringToken((int)eHeaderST::posterpath, posterpath.c_str());
    header->Display();
}
    
void cEpgGrid::DrawFooter(void) {
    if (!active)
        return;

    string textGreen = "";
    string textYellow = "";
    string textRed = tr("Search & Record");
    string textBlue = "";
    if (config.channelJumpMode == eNumJump) {
        textGreen = *cString::sprintf("%d %s", channelsPerPage, tr("Channels back"));
        textYellow = *cString::sprintf("%d %s", channelsPerPage, tr("Channels forward"));
    } else if (config.channelJumpMode == eGroupJump) {
        int currentGroup = channelGroups->GetGroup(active->Channel());
        if (currentGroup == channelGroupLast) {
            return;
        }
        channelGroupLast = currentGroup;
        textGreen = channelGroups->GetPrevGroupName(currentGroup);
        textYellow = channelGroups->GetNextGroupName(currentGroup);
    }
    if (config.blueKeyMode == eBlueKeySwitch) {
        textBlue = tr("Switch");
    } else if (config.blueKeyMode == eBlueKeyEPG) {
        textBlue = tr("Detailed EPG");
    } else if (config.blueKeyMode == eBlueKeyFavorites) {
        textBlue = tr("Favorites");
    }

    int colorKeys[4] = { Setup.ColorKey0, Setup.ColorKey1, Setup.ColorKey2, Setup.ColorKey3 };

    footer->Clear();
    footer->ClearTokens();

    footer->AddStringToken((int)eFooterST::red, textRed.c_str());
    footer->AddStringToken((int)eFooterST::green, textGreen.c_str());
    footer->AddStringToken((int)eFooterST::yellow, textYellow.c_str());
    footer->AddStringToken((int)eFooterST::blue, textBlue.c_str());

    for (int button = 0; button < 4; button++) {
        bool isRed = false;
        bool isGreen = false;
        bool isYellow = false;
        bool isBlue = false;
        switch (colorKeys[button]) {
            case 0:
                isRed = true;
                break;
            case 1:
                isGreen = true;
                break;
            case 2:
                isYellow = true;
                break;
            case 3:
                isBlue = true;
                break;
            default:
                break;
        }
        footer->AddIntToken(0  + button, isRed);
        footer->AddIntToken(4  + button, isGreen);
        footer->AddIntToken(8  + button, isYellow);
        footer->AddIntToken(12 + button, isBlue);
    }

    footer->Display();
}

bool cEpgGrid::DrawTime(void) {
    time_t t = time(0);   // get time now
    struct tm * now = localtime(&t);
    int sec = now->tm_sec;
    if (sec == lastSecond)
        return false;

    int min = now->tm_min;
    int hour = now->tm_hour;
    int hourMinutes = hour%12 * 5 + min / 12;

    char monthname[20];
    char monthshort[10];
    strftime(monthshort, sizeof(monthshort), "%b", now);
    strftime(monthname, sizeof(monthname), "%B", now);

    watch->Clear();
    watch->ClearTokens();
    watch->AddIntToken((int)eTimeIT::sec, sec);
    watch->AddIntToken((int)eTimeIT::min, min);
    watch->AddIntToken((int)eTimeIT::hour, hour);
    watch->AddIntToken((int)eTimeIT::hmins, hourMinutes);
    watch->AddIntToken((int)eTimeIT::year, now->tm_year + 1900);
    watch->AddIntToken((int)eTimeIT::day, now->tm_mday);
    watch->AddStringToken((int)eTimeST::time, *TimeString(t));
    watch->AddStringToken((int)eTimeST::monthname, monthname);
    watch->AddStringToken((int)eTimeST::monthnameshort, monthshort);
    watch->AddStringToken((int)eTimeST::month, *cString::sprintf("%02d", now->tm_mon + 1));
    watch->AddStringToken((int)eTimeST::dayleadingzero, *cString::sprintf("%02d", now->tm_mday));
    watch->AddStringToken((int)eTimeST::dayname, *WeekDayNameFull(now->tm_wday));
    watch->AddStringToken((int)eTimeST::daynameshort, *WeekDayName(now->tm_wday));
    watch->Display();

    lastSecond = sec;
    return true;
}

cChannelJump *cEpgGrid::GetChannelJumper(void) {
    skindesignerapi::cViewElement *channelJump = rootView->GetViewElement((int)eViewElementsRoot::channelJump);
    int lastValidChannel = GetLastValidChannel();
    return new cChannelJump(channelJump, lastValidChannel);
}

/*******************************************************************************************************
* Private  Functions
*******************************************************************************************************/

void cEpgGrid::ScrollForward(void) {
    timeManager->AddMinutes(config.timeStep);
    timeline->ScrollForward(config.timeStep);
    timeline->Draw();
    for (cChannelEpg *channel = channels.First(); channel; channel = channels.Next(channel)) {
        channel->AddNewGridsAtEnd();
        channel->ClearOutdatedStart();
    }
}

void cEpgGrid::ScrollBack(void) {
    timeManager->DelMinutes(config.timeStep);
    timeline->ScrollBack(config.timeStep);
    timeline->Draw();
    for (cChannelEpg *channel = channels.First(); channel; channel = channels.Next(channel)) {
        channel->AddNewGridsAtStart();
        channel->ClearOutdatedEnd();
    }
}

const cChannel *cEpgGrid::SeekChannelForward(int num) {
    if (!active)
        return NULL;
    const cChannel *currentChannel = active->Channel();
    const cChannel *destChannel;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannels* chls = Channels;
#else
    const cChannels* chls = &Channels;
#endif

    int found = 0;
    for (destChannel = currentChannel; destChannel ; destChannel = chls->Next(destChannel)) {
        if (destChannel->GroupSep()) {
            continue;
        }
        if (config.hideLastChannelGroup && channelGroups->IsInLastGroup(destChannel)) {
            destChannel = chls->Prev(destChannel);
            while (destChannel && destChannel->GroupSep()) {
                destChannel = chls->Prev(destChannel);
            }
            break;
        }
        if (found == num)
            break;
        found++;
    }
    if (!destChannel)
        destChannel = chls->Last();
    while (destChannel && destChannel->GroupSep()) {
        destChannel = chls->Prev(destChannel);
    }
    return destChannel;
}

const cChannel *cEpgGrid::SeekChannelBack(int num) {
    if (!active)
        return NULL;
    const cChannel *currentChannel = active->Channel();
    const cChannel *destChannel;
    int found = 0;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannels* chls = Channels;
#else
    const cChannels* chls = &Channels;
#endif
    for (destChannel = currentChannel; destChannel ; destChannel = chls->Prev(destChannel)) {
        if (destChannel->GroupSep()) {
            continue;
        }
        if (found == num)
            break;
        found++;
    }
    if (!destChannel)
        destChannel = chls->First();
    while (destChannel && destChannel->GroupSep()) {
        destChannel = chls->Next(destChannel);
    }
    return destChannel;
}

void cEpgGrid::Debug(void) {
    esyslog("tvguideng: ------------ debugging EpgGrid -------------");
    timeManager->Debug();
    for (cChannelEpg *channel = channels.First(); channel; channel = channels.Next(channel)) {
        channel->Debug();
    }
    if (active) {
        esyslog("tvguideng: Active Grid Element:");
        active->Debug();
    } else {
        esyslog("tvguideng: No Active Grid Element");        
    }
    esyslog("tvguideng: -----------------------------------------------");
}
