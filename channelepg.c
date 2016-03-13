#include "channelepg.h"

cChannelEpg::cChannelEpg(int position, const cChannel *channel, cTimeManager *timeManager) {
    init = true;
    this->channel = channel;
    this->position = position;
    this->timeManager = timeManager;
    channelsPerPage = (config.displayMode == eHorizontal) ? config.channelsPerPageHorizontal : config.channelsPerPageVertical;
    SetTimer();
    SetSwitchTimer();
}

cChannelEpg::~cChannelEpg(void) {
    grids.Clear();
}

void cChannelEpg::ClearGrids(void) {
    grids.Clear();
}

bool cChannelEpg::ReadGrids(void) {
    
    const cSchedule *Schedule = NULL;

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_SCHEDULES_READ;
    const cSchedules* schedules = Schedules;
#else
    cSchedulesLock schedulesLock;
    const cSchedules* schedules = (cSchedules*)cSchedules::Schedules(schedulesLock);
#endif
    if (!schedules)
       return false;

    Schedule = schedules->GetSchedule(channel);
    if (!Schedule) {
        AddDummyGrid(timeManager->GetStart(), timeManager->GetEnd());
        return true;
    }
    bool eventFound = false;
    bool dummyAtStart = false;
    const cEvent *startEvent = Schedule->GetEventAround(timeManager->GetStart());
    if (startEvent != NULL) {
        eventFound = true;
    } else {
        for (int i=1; i<6; i++) {
            startEvent = Schedule->GetEventAround(timeManager->GetStart()+i*5*60);
            if (startEvent) {
                eventFound = true;
                dummyAtStart = true;
                break;
            }
        }
    } 
    if (eventFound) {
        if (dummyAtStart) {
            AddDummyGrid(timeManager->GetStart(), startEvent->StartTime());
        }
        bool dummyNeeded = true;
        bool toFarInFuture = false;
        time_t endLast = timeManager->GetStart();
        const cEvent *event = startEvent;
        const cEvent *eventLast = NULL;
        for (; event; event = Schedule->Events()->Next(event)) {
            if (endLast < event->StartTime()) {
                if (dummyAtStart) {
                    dummyAtStart = false;
                } else {
                    //gap, dummy needed
                    time_t endTime = event->StartTime();
                    if (endTime > timeManager->GetEnd()) {
                        endTime = timeManager->GetEnd();
                        toFarInFuture = true;
                    }
                    AddDummyGrid(endLast, endTime);
                }
            }
            if (toFarInFuture) {
                break;
            }
            if (event->StartTime() >= timeManager->GetEnd()) {
                dummyNeeded = false;
                break;
            }
            AddEpgGrid(event);
            endLast = event->EndTime();
            if (event->EndTime() > timeManager->GetEnd()) {
                dummyNeeded = false;
                break;
            }
            eventLast = event;
        }
        if (dummyNeeded) {
            AddDummyGrid(eventLast->EndTime(), timeManager->GetEnd());
        }
        return true;
    } else {
        AddDummyGrid(timeManager->GetStart(), timeManager->GetEnd());
        return true;
    }
    return false;
}

cGridElement *cChannelEpg::GetActive(void) {
    cTimeManager t;
    t.Now();
    for (cGridElement *grid = grids.First(); grid; grid = grids.Next(grid)) {
        if (grid->Match(t.Get()))
            return grid;
    }
    return grids.First();
}

cGridElement *cChannelEpg::GetNext(cGridElement *activeGrid) {
    if (activeGrid == NULL)
        return NULL;
    cGridElement *next = grids.Next(activeGrid);
    if (next)
        return next;
    return NULL;
}

cGridElement *cChannelEpg::GetPrev(cGridElement *activeGrid) {
    if (activeGrid == NULL)
        return NULL;
    cGridElement *prev = grids.Prev(activeGrid);
    if (prev)
        return prev;
    return NULL;
}

cGridElement *cChannelEpg::GetNeighbor(cGridElement *activeGrid) {
    if (!activeGrid)
        return NULL;
    cGridElement *neighbor = NULL;
    int overlap = 0;
    int overlapNew = 0;
    cGridElement *grid = NULL;
    grid = grids.First();
    if (grid) {
        for (; grid; grid = grids.Next(grid)) {
            if ( (grid->StartTime() == activeGrid->StartTime()) ) {
                neighbor = grid;
                break;
            }
            overlapNew = activeGrid->CalcOverlap(grid);
            if (overlapNew > overlap) {
                neighbor = grid;
                overlap = overlapNew;
            }
        }
    }
    if (!neighbor)
        neighbor = grids.First();
    return neighbor;
}

bool cChannelEpg::IsFirst(cGridElement *grid) {
    if (grid == grids.First())
        return true;
    return false;
}

void cChannelEpg::AddNewGridsAtStart(void) {
    cGridElement *firstGrid = NULL;
    firstGrid = grids.First();
    if (firstGrid == NULL)
        return;
    //if first event is long enough, nothing to do.
    if (firstGrid->StartTime() <= timeManager->GetStart()) {
        return;
    }
    //if not, i have to add new ones to the list
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_SCHEDULES_READ;
    const cSchedules* schedules = Schedules;
#else
    cSchedulesLock schedulesLock;
    const cSchedules* schedules = (cSchedules*)cSchedules::Schedules(schedulesLock);
#endif
    if (!schedules)
       return ;
    const cSchedule *Schedule = NULL;
    Schedule = schedules->GetSchedule(channel);
    if (!Schedule) {
        if (firstGrid->IsDummy()) {
            firstGrid->SetStartTime(timeManager->GetStart());
            firstGrid->SetEndTime(timeManager->GetEnd());
        }
        return;
    }
    bool dummyNeeded = true;
    for (const cEvent *event = Schedule->GetEventAround(firstGrid->StartTime()-60); event; event = Schedule->Events()->Prev(event)) {
        if (!event)
            break;
        if (event->EndTime() < timeManager->GetStart()) {
            break;
        }
        cGridElement *grid = AddEpgGrid(event, firstGrid);
        firstGrid = grid;
        if (event->StartTime() <= timeManager->GetStart()) {
            dummyNeeded = false;
            break;
        }
    }
    if (dummyNeeded) {
        firstGrid = grids.First();
        if (firstGrid->IsDummy()) {
            firstGrid->SetStartTime(timeManager->GetStart());
            if (firstGrid->EndTime() >= timeManager->GetEnd())
                firstGrid->SetEndTime(timeManager->GetEnd());
        } else {
            AddDummyGrid(timeManager->GetStart(), firstGrid->StartTime(), firstGrid);
        }
    }
}

void cChannelEpg::AddNewGridsAtEnd(void) {
    cGridElement *lastGrid = NULL;
    lastGrid = grids.Last();
    if (lastGrid == NULL)
        return;
    //if last event is long enough, nothing to do.
    if (lastGrid->EndTime() >= timeManager->GetEnd()) {
        return;
    }
    //if not, i have to add new ones to the list
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_SCHEDULES_READ;
    const cSchedules* schedules = Schedules;
#else
    cSchedulesLock schedulesLock;
    const cSchedules* schedules = (cSchedules*)cSchedules::Schedules(schedulesLock);
#endif
    if (!schedules)
       return ;
    const cSchedule *Schedule = NULL;
    Schedule = schedules->GetSchedule(channel);
    if (!Schedule) {
        if (lastGrid->IsDummy()) {
            lastGrid->SetStartTime(timeManager->GetStart());
            lastGrid->SetEndTime(timeManager->GetEnd());
        }
        return;
    }
    bool dummyNeeded = true;
    for (const cEvent *event = Schedule->GetEventAround(lastGrid->EndTime()+60); event; event = Schedule->Events()->Next(event)) {
        if (!event)
            break;
        if (event->StartTime() > timeManager->GetEnd()) {
            break;
        }
        AddEpgGrid(event);
        if (event->EndTime() > timeManager->GetEnd()) {
            dummyNeeded = false;
            break;
        }
    }
    if (dummyNeeded) {
        lastGrid = grids.Last();
        if (lastGrid->IsDummy()) {
            lastGrid->SetEndTime(timeManager->GetEnd());
            if (lastGrid->StartTime() <= timeManager->GetStart())
                lastGrid->SetStartTime(timeManager->GetStart());
        } else {
            AddDummyGrid(lastGrid->EndTime(), timeManager->GetEnd());
        }
    }
}

void cChannelEpg::ClearOutdatedStart(void) {
    deletedElements.clear();
    cGridElement *firstGrid = NULL;
    while (true) {
        firstGrid = grids.First();
        if (!firstGrid)
            break;
        if (firstGrid->EndTime() <= timeManager->GetStart()) {
            deletedElements.insert(firstGrid->Id());
            grids.Del(firstGrid);
            firstGrid = NULL;
        } else {
            if (firstGrid->IsDummy()) {
                firstGrid->SetStartTime(timeManager->GetStart());
                cGridElement *next = GetNext(firstGrid);
                if (next) {
                    firstGrid->SetEndTime(next->StartTime());
                } else {
                    firstGrid->SetEndTime(timeManager->GetEnd());
                }
            }
            break;
        }
    }
}

void cChannelEpg::ClearOutdatedEnd(void) {
    deletedElements.clear();
    cGridElement *lastGrid = NULL;
    while (true) {
        lastGrid = grids.Last();    
        if (!lastGrid)
            break;
        if (lastGrid->StartTime() >= timeManager->GetEnd()) {
            deletedElements.insert(lastGrid->Id());
            grids.Del(lastGrid);
            lastGrid = NULL;
        } else {
            if (lastGrid->IsDummy()) {
                lastGrid->SetEndTime(timeManager->GetEnd());
                cGridElement *prev = GetPrev(lastGrid);
                if (prev) {
                    lastGrid->SetStartTime(prev->EndTime());
                } else {
                    lastGrid->SetStartTime(timeManager->GetStart());
                }
            }
            break;
        }
    }
}

void cChannelEpg::SetTimers(void) {
    SetTimer();
    SetSwitchTimer();
    for (cGridElement *grid = grids.First(); grid; grid = grids.Next(grid)) {
        bool gridHadTimer = grid->HasTimer();
        grid->SetTimer();
        if (gridHadTimer != grid->HasTimer()) {
            grid->Dirty();
        }
        bool gridHadSwitchTimer = grid->HasSwitchTimer();
        grid->SetSwitchTimer();
        if (gridHadSwitchTimer != grid->HasSwitchTimer())
            grid->Dirty();
    }
}

void cChannelEpg::DrawHeader(skindesignerapi::cViewGrid *channelsGrid) {

    double x, y, width, height;
    
    if (config.displayMode == eHorizontal) {
        x = 0.0;
        width = 1.0;
        height = (double)1 / (double)channelsPerPage;
        y = height * (double)position;
    } else {
        y = 0.0;
        height = 1.0;
        width = (double)1 / (double)channelsPerPage;
        x = width * (double)position;
    }

    int id = channel->Number();
    if (init) {
        channelsGrid->ClearTokens();
        channelsGrid->AddIntToken((int)eChannelGridIT::number, id);
        channelsGrid->AddStringToken((int)eChannelGridST::name, channel->Name());
        cString channelId = channel->GetChannelID().ToString();
        bool channelLogoExisis = channelsGrid->ChannelLogoExists(*channelId);
        channelsGrid->AddStringToken((int)eChannelGridST::channelid, *channelId);
        channelsGrid->AddIntToken((int)eChannelGridIT::channellogoexists, channelLogoExisis);
        channelsGrid->SetGrid(id, x, y, width, height);
        init = false;
    } else {
        channelsGrid->MoveGrid(id, x, y, width, height);
    }
}

void cChannelEpg::DrawGrids(skindesignerapi::cViewGrid *epgGrid) {
    int displaySeconds = timeManager->GetDisplaySeconds();
    double x, y, width, height;
    if (config.displayMode == eHorizontal) {
        x = 0.0;
        width = 0.0;
        height = (double)1 / (double)channelsPerPage;
        y = height * (double)position;
    } else {
        y = 0.0;
        height = 0.0;
        width = (double)1 / (double)channelsPerPage;
        x = width * (double)position;
    }
    time_t startTime = timeManager->GetStart();
    time_t stopTime = timeManager->GetEnd();

    for (cGridElement *grid = grids.First(); grid; grid = grids.Next(grid)) {
        time_t gridStart = grid->StartTime();
        time_t gridStop = grid->EndTime();
        if (grid->IsNew()) {
            epgGrid->ClearTokens();
            epgGrid->AddIntToken((int)eSchedulesGridIT::color, grid->Id() % 2);
            epgGrid->AddIntToken((int)eSchedulesGridIT::dummy, grid->IsDummy());
            epgGrid->AddIntToken((int)eSchedulesGridIT::timer, grid->HasTimer());
            epgGrid->AddIntToken((int)eSchedulesGridIT::switchtimer, grid->HasSwitchTimer());
            epgGrid->AddStringToken((int)eSchedulesGridST::title, grid->Title());
            epgGrid->AddStringToken((int)eSchedulesGridST::shorttext, grid->ShortText());
            epgGrid->AddStringToken((int)eSchedulesGridST::start, *TimeString(gridStart));
            epgGrid->AddStringToken((int)eSchedulesGridST::stop, *TimeString(gridStop));
        }

        if (gridStart < startTime) {
            gridStart = startTime;
        }
        if (gridStop > stopTime) {
            gridStop = stopTime;
        }
        if (config.displayMode == eHorizontal) {
            int xGrid = gridStart - startTime;
            x = (double)xGrid / (double)displaySeconds;
            int duration = gridStop - gridStart;
            width = (double)duration / (double)displaySeconds;
        } else {
            int yGrid = gridStart - startTime;
            y = (double)yGrid / (double)displaySeconds;
            int duration = gridStop - gridStart;
            height = (double)duration / (double)displaySeconds;
        }

        if (grid->IsNew()) {
            epgGrid->SetGrid(grid->Id(), x, y, width, height);
            grid->InitFinished();
        } else {
            epgGrid->MoveGrid(grid->Id(), x, y, width, height);
        }
        if (grid->Active())
            epgGrid->SetCurrent(grid->Id(), true);

    }
}

void cChannelEpg::DeleteOutdated(skindesignerapi::cViewGrid *epgGrid) {
    for (set<long>::iterator it = deletedElements.begin(); it != deletedElements.end(); it++) {
        epgGrid->Delete(*it);
    }
}

void cChannelEpg::DeleteGridViews(skindesignerapi::cViewGrid *epgGrid) {
    for (cGridElement *ge = grids.First(); ge; ge = grids.Next(ge)) {
        epgGrid->Delete(ge->Id());
    }
}

cGridElement *cChannelEpg::AddEpgGrid(const cEvent *event, cGridElement *after) {
    cGridElement *grid = new cEpgElement(event, this);
    if (!after)
        grids.Add(grid);
    else
        grids.Ins(grid, after);
    return grid;
}

void cChannelEpg::AddDummyGrid(time_t start, time_t end, cGridElement *after) {
    cGridElement *dummy = new cDummyElement(start, end, this);
    if (!after)
        grids.Add(dummy);
    else
        grids.Ins(dummy, after);
}

void cChannelEpg::Debug(void) {
    esyslog("tvguideng: channel %s", channel->Name());
    for (cGridElement *grid = grids.First(); grid; grid = grids.Next(grid)) {
        grid->Debug();
    }
}

void cChannelEpg::SetTimer() 
{ 
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
#else
   hasTimer = channel->HasTimer(); 
#endif
   
};
