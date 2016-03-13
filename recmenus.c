#include <string>
#include <sstream>
#include <algorithm>
#include "config.h"
#include "services/remotetimers.h"
#include "helpers.h"
#include "recmenus.h"

// --- cRecMenuMain  ---------------------------------------------------------
cRecMenuMain::cRecMenuMain(bool epgSearchAvailable, bool timerActive, bool switchTimerActive) {
    SetMenuWidth(50);
    eRecMenuState action = rmsInstantRecord;
    if (!timerActive) {
        if (config.instRecFolderMode == eFolderSelect)
            action = rmsInstantRecordFolder;
        AddMenuItem(new cRecMenuItemButton(tr("Instant Record"), action, true));
    } else {
        AddMenuItem(new cRecMenuItemButton(tr("Delete Timer"), rmsDeleteTimer, true));
        AddMenuItem(new cRecMenuItemButton(tr("Edit Timer"), rmsEditTimer, false));
    }

    AddMenuItem(new cRecMenuItemButton(tr("Timer Timeline"), rmsTimeline, false));

    if (epgSearchAvailable) {
        AddMenuItem(new cRecMenuItemButton(tr("Create Search Timer"), rmsSearchTimer, false));
        AddMenuItem(new cRecMenuItemButton(tr("Search Timers"), rmsSearchTimers, false));
    }

    if (config.instRecFolderMode == eFolderSelect)
        action = rmsSeriesTimerFolder;
    else
        action = rmsSeriesTimer;
    AddMenuItem(new cRecMenuItemButton(tr("Create Series Timer"), action, false));

    if (epgSearchAvailable) {
        if (!switchTimerActive) {
            AddMenuItem(new cRecMenuItemButton(tr("Create Switch Timer"), rmsSwitchTimer, false));
        } else {
            AddMenuItem(new cRecMenuItemButton(tr("Delete Switch Timer"), rmsSwitchTimerDelete, false));
        }
        AddMenuItem(new cRecMenuItemButton(tr("Search"), rmsSearch, false));
    }

    if (epgSearchAvailable) {
        AddMenuItem(new cRecMenuItemButton(tr("Check for Timer Conflicts"), rmsTimerConflicts, false));
    }

    AddMenuItem(new cRecMenuItemButton(tr("Search in Recordings"), rmsRecordingSearch, false));
};

/******************************************************************************************
*   Instant Timer Menus
******************************************************************************************/

// --- cRecMenuConfirmTimer  ---------------------------------------------------------
cRecMenuConfirmTimer::cRecMenuConfirmTimer(const cEvent *event) {
    SetMenuWidth(50);
    
    bool eventHasTimer = false;
    if (config.useRemoteTimers && pRemoteTimers) {
        RemoteTimers_GetMatch_v1_0 rtMatch;
        rtMatch.event = event;
        pRemoteTimers->Service("RemoteTimers::GetMatch-v1.0", &rtMatch);
        if (rtMatch.timerMatch == tmFull) {
            eventHasTimer = true;                
        }
    } else {
        eventHasTimer = event->HasTimer();
    }

    string message;
    if (eventHasTimer) {
        message = tr("Timer created");
    } else {
        message = tr("Timer NOT created");
    }
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    cString channelName = Channels->GetByChannelID(event->ChannelID())->Name();
#else
    cString channelName = Channels.GetByChannelID(event->ChannelID())->Name();
#endif
    string datetime = *cString::sprintf("%s %s - %s", *event->GetDateString(), *event->GetTimeString(), *event->GetEndTimeString());
    string eventTitle = event->Title() ? event->Title() : "";
    AddHeader(new cRecMenuItemInfo(message, 4, *channelName, datetime, eventTitle));
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsClose, true));
}

// --- cRecMenuAskFolder  ---------------------------------------------------------
cRecMenuAskFolder::cRecMenuAskFolder(const cEvent *event, eRecMenuState nextAction) {
    SetMenuWidth(80);
    this->nextAction = nextAction;
    string message = tr("Set Folder for");
    string eventTitle = event->Title() ? event->Title() : "";
    AddHeader(new cRecMenuItemInfo(message, 2, eventTitle));
    AddMenuItem(new cRecMenuItemButton(tr("root video folder"), nextAction, true));
    
    ReadRecordingDirectories(&folders, NULL, "");
    int numFolders = folders.size();
    for (int i=0; i < numFolders; i++) {
        AddMenuItem(new cRecMenuItemButton(folders[i].c_str(), nextAction, false));
    }
}

string cRecMenuAskFolder::GetFolder(void) {
    std::string folder = "";
    int folderActive = GetNumActive();
    if (folderActive > 0 && folderActive < (int)folders.size() + 1)
        folder = folders[folderActive - 1];
    return folder;
}

// --- cRecMenuConfirmDeleteTimer  ---------------------------------------------------------
cRecMenuConfirmDeleteTimer::cRecMenuConfirmDeleteTimer(const cEvent *event) {
    SetMenuWidth(50);
    string message = tr("Timer deleted");
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    cString channelName = Channels->GetByChannelID(event->ChannelID())->Name();
#else
    cString channelName = Channels.GetByChannelID(event->ChannelID())->Name();
#endif
    string datetime = *cString::sprintf("%s %s - %s", *event->GetDateString(), *event->GetTimeString(), *event->GetEndTimeString());
    string eventTitle = event->Title() ? event->Title() : "";

    AddHeader(new cRecMenuItemInfo(message, 4, *channelName, datetime, eventTitle));
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsClose, true));
}

// --- cRecMenuAskDeleteTimer ---------------------------------------------------------
cRecMenuAskDeleteTimer::cRecMenuAskDeleteTimer(const cEvent *event) {
    SetMenuWidth(50);
    string message = tr("Timer");
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    cString channelName = Channels->GetByChannelID(event->ChannelID())->Name();
#else
    cString channelName = Channels.GetByChannelID(event->ChannelID())->Name();
#endif
    string eventTitle = event->Title() ? event->Title() : "";
    string message2 = tr("still recording - really delete?");

    AddHeader(new cRecMenuItemInfo(message, 4, *channelName, eventTitle, message2));
    AddMenuItem(new cRecMenuItemButtonYesNo(tr("Yes"), tr("No"), rmsDeleteTimerConfirmation, rmsClose, true));
}

// --- cRecMenuTimerConflicts  ---------------------------------------------------------
cRecMenuTimerConflicts::cRecMenuTimerConflicts(cTVGuideTimerConflicts *conflicts) {
    SetMenuWidth(80);
    int numConflicts = conflicts->NumConflicts();
    
    string text;
    if (numConflicts == 1) {
        text = *cString::sprintf("%s %s %s", tr("One"), tr("Timer Conflict"), tr("detected"));
    } else {
        text = *cString::sprintf("%d %s %s", conflicts->NumConflicts(), tr("Timer Conflicts"), tr("detected"));
    }
    AddHeader(new cRecMenuItemInfo(text));
    
    for (int i=0; i<numConflicts; i++) {
        cTVGuideTimerConflict *conflict = conflicts->GetConflict(i);
        if (!conflict)
            continue;
        cString dateTime = DayDateTime(conflict->time);
        int numTimers = conflict->timerIDs.size();
        cString textConflict = cString::sprintf("%s: %s (%d %s)", tr("Show conflict"), *dateTime, numTimers, tr("timers involved"));
        bool isActive = (i==0)?true:false;
        AddMenuItem(new cRecMenuItemButton(*textConflict, rmsTimerConflict, isActive));
    }

    AddFooter(new cRecMenuItemButton(tr("Ignore Conflicts"), rmsClose, false));
}

int cRecMenuTimerConflicts::GetTimerConflict(void) {
    return GetNumActive();
}

// --- cRecMenuTimerConflict  ---------------------------------------------------------
cRecMenuTimerConflict::cRecMenuTimerConflict(cTVGuideTimerConflict *conflict) {
    SetMenuWidth(95);
    this->conflict = conflict;
    AddHeader(new cRecMenuItemTimerConflictHeader(conflict->timeStart, 
                                                  conflict->timeStop, 
                                                  conflict->overlapStart, 
                                                  conflict->overlapStop));
    
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_TIMERS_READ;
    const cTimers* timers = Timers;
#else
    const cTimers* timers = &Timers;
#endif
    int i=0;
    for(vector<int>::iterator it = conflict->timerIDs.begin(); it != conflict->timerIDs.end(); it++) {
        const cTimer *timer = timers->Get(*it);
        if (timer) {
            AddMenuItem(new cRecMenuItemTimer(timer,
                                              rmsTimerConflictShowInfo,
                                              rmsDeleteTimerConflictMenu, 
                                              rmsEditTimerConflictMenu,
                                              rmsSearchRerunsTimerConflictMenu,
                                              conflict->timeStart, 
                                              conflict->timeStop, 
                                              conflict->overlapStart, 
                                              conflict->overlapStop, 
                                              (!i)?true:false));
            i++;
        }
    }
    AddMenuItem(new cRecMenuItemButton(tr("Ignore Conflict"), rmsIgnoreTimerConflict, false));
}

int cRecMenuTimerConflict::GetTimerConflictIndex(void) {
    return GetNumActive();
}

// --- cRecMenuNoTimerConflict ---------------------------------------------------------
cRecMenuNoTimerConflict::cRecMenuNoTimerConflict(void) {
    SetMenuWidth(50);
    string text = tr("No Timer Conflicts found");
    AddHeader(new cRecMenuItemInfo(text));
    AddMenuItem(new cRecMenuItemButton(tr("Close"), rmsClose, true));
}

// --- cRecMenuRerunResults ---------------------------------------------------------
cRecMenuRerunResults::cRecMenuRerunResults(const cEvent *original, const cEvent **reruns, int numReruns) {
    this->reruns = reruns;
    this->numReruns = numReruns;
    SetMenuWidth(70);

    string message1 = tr("reruns for");
    string message2 = tr("rerun for");
    string message3 = tr("found");
    string line1 = *cString::sprintf("%d %s:", numReruns, (numReruns>1)?(message1.c_str()):(message2.c_str()));
    string line2 = *cString::sprintf("\"%s\" %s", original->Title(), message3.c_str());
    
    AddHeader(new cRecMenuItemInfo(line1, 2, line2));
    
    cRecMenuItem *button = new cRecMenuItemButton(tr("Ignore reruns"), rmsTimerConflictIgnoreReruns, false);
    AddFooter(button);
    
    if (reruns && (numReruns > 0)) {
        for (int i=0; i<numReruns; i++) {
            AddMenuItem(new cRecMenuItemEvent(reruns[i], rmsSearchShowInfo, rmsTimerConflictRecordRerun, (i==0)?true:false));
        }
    }
}

int cRecMenuRerunResults::GetTotalNumMenuItems(void) {
    return numReruns;
}

const cEvent *cRecMenuRerunResults::GetEvent(void) {
    cRecMenuItemEvent *activeItem = dynamic_cast<cRecMenuItemEvent*>(GetActive());
    if (activeItem)
        return activeItem->GetEvent();
    return NULL;
}

// --- cRecMenuNoRerunsFound  ---------------------------------------------------------
cRecMenuNoRerunsFound::cRecMenuNoRerunsFound(string searchString) {
    SetMenuWidth(50);
    string message = tr("No reruns found for Event");
    string eventQuoted = *cString::sprintf("\"%s\"", searchString.c_str());

    AddHeader(new cRecMenuItemInfo(message, 2, eventQuoted));
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsClose, true));
}

// --- cRecMenuConfirmRerunUsed  ---------------------------------------------------------
cRecMenuConfirmRerunUsed::cRecMenuConfirmRerunUsed(const cEvent *original, const cEvent *replace) {
    SetMenuWidth(70);
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    cString channelOrig = Channels->GetByChannelID(original->ChannelID())->Name();
    cString channelReplace = Channels->GetByChannelID(replace->ChannelID())->Name();
#else
    cString channelOrig = Channels.GetByChannelID(original->ChannelID())->Name();
    cString channelReplace = Channels.GetByChannelID(replace->ChannelID())->Name();
#endif
    string line1 = *cString::sprintf("%s \"%s\"", tr("Timer for"), original->Title());
    string line2 = *cString::sprintf("%s %s %s", *original->GetDateString(), *original->GetTimeString(), *channelOrig);
    string line3 = *cString::sprintf("%s \"%s\"", tr("replaced by rerun"), replace->Title());
    string line4 = *cString::sprintf("%s %s %s", *replace->GetDateString(), *replace->GetTimeString(), *channelReplace);

    AddHeader(new cRecMenuItemInfo(line1, 4, line2, line3, line4));
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsTimerConflicts, true));
}

// --- cRecMenuEditTimer  ---------------------------------------------------------
cRecMenuEditTimer::cRecMenuEditTimer(const cTimer *timer, eRecMenuState nextState) {
    SetMenuWidth(70);
    if (!timer)
        return;
    originalTimer = timer;
    string title = "";
    string channelName = "";
    if (timer->Event() && timer->Event()->Title())
        title = timer->Event()->Title();
    if (timer->Channel() && timer->Channel()->Name())
        channelName = timer->Channel()->Name();
    cRecMenuItemInfo *infoItem = new cRecMenuItemInfo(tr("Edit Timer"), 3, title, channelName);
    AddHeader(infoItem);
                
    timerActive = false;
    if (config.useRemoteTimers && pRemoteTimers) {
        RemoteTimers_GetMatch_v1_0 rtMatch;
        rtMatch.event = timer->Event();
        pRemoteTimers->Service("RemoteTimers::GetMatch-v1.0", &rtMatch);
        if (rtMatch.timer) {
            if (rtMatch.timerMatch == tmFull)
                timerActive = true;
        }
    } else
        timerActive = timer->HasFlags(tfActive);

    day = timer->Day();
    start = timer->Start();
    stop = timer->Stop();
    prio = timer->Priority();
    lifetime = timer->Lifetime();
    strncpy(folder, GetDirectoryFromTimer(timer->File()).c_str(), TEXTINPUTLENGTH);
    
    AddMenuItem(new cRecMenuItemBool(tr("Timer Active"), timerActive, true, &timerActive));
    AddMenuItem(new cRecMenuItemInt(tr("Priority"), prio, 0, MAXPRIORITY, false, &prio));
    AddMenuItem(new cRecMenuItemInt(tr("Lifetime"), lifetime, 0, MAXLIFETIME, false, &lifetime));
    AddMenuItem(new cRecMenuItemDay(tr("Day"), day, false, &day));
    AddMenuItem(new cRecMenuItemTime(tr("Timer start time"), start, false, &start));
    AddMenuItem(new cRecMenuItemTime(tr("Timer stop time"), stop, false, &stop));
    AddMenuItem(new cRecMenuItemInfo(tr("Timer File"), 2, timer->File() ? timer->File() : ""));
    AddMenuItem(new cRecMenuItemSelectDirectory(tr("New Folder"), string(folder), false, folder));
    if (nextState == rmsTimelineTimerSave) {
        AddMenuItem(new cRecMenuItemButton(tr("Delete Timer"), rmsTimelineTimerDelete, false));
        AddFooter(new cRecMenuItemButtonYesNo(tr("Save"), tr("Cancel"), nextState, rmsTimeline, false));
    } else {
        AddFooter(new cRecMenuItemButtonYesNo(tr("Save"), tr("Cancel"), nextState, rmsClose, false));
    }
}

const cTimer *cRecMenuEditTimer::GetOriginalTimer(void) {
    return originalTimer;
}

cTimer cRecMenuEditTimer::GetTimer(void) {
    cTimer t;
    if (timerActive)
        t.SetFlags(tfActive);
    else 
        t.SetFlags(tfNone);
    t.SetDay(day);
    t.SetStart(start);
    t.SetStop(stop);
    t.SetPriority(prio);
    t.SetLifetime(lifetime);
    string newFolder(folder);
    string newFile = originalTimer->File();
    size_t found = newFile.find_last_of('~');
    if (found != string::npos) {
        string fileName = newFile.substr(found+1);
        if (newFolder.size() > 0)
            newFile = *cString::sprintf("%s~%s", newFolder.c_str(), fileName.c_str());
        else
            newFile = fileName;
    } else {
        if (newFolder.size() > 0)
            newFile = *cString::sprintf("%s~%s", newFolder.c_str(), newFile.c_str());
    }
    std::replace(newFile.begin(), newFile.end(), '/', '~');
    t.SetFile(newFile.c_str());
    return t;
}

/******************************************************************************************
*   Series Timer Menus
******************************************************************************************/

// --- cRecMenuSeriesTimer ---------------------------------------------------------
cRecMenuSeriesTimer::cRecMenuSeriesTimer(const cChannel *initialChannel, const cEvent *event, string folder) {
    if (!initialChannel)
        return;
    timerActive = true;
    channel = initialChannel->Number();
    dayOfWeek = 127; 
    priority =  MAXPRIORITY;
    lifetime = MAXLIFETIME;
    CalculateTimes(event);
    this->folder = folder;

    SetMenuWidth(70);
    string line1 = tr("Create Series Timer based on");
    string line2 = *cString::sprintf("\"%s\"", event->Title());
    AddHeader(new cRecMenuItemInfo(line1, 2, line2));

    AddMenuItem(new cRecMenuItemBool(tr("Timer Active"), timerActive, false, &timerActive));
    AddMenuItem(new cRecMenuItemChannelChooser(tr("Channel"), (cChannel*)initialChannel, false, &channel));
    AddMenuItem(new cRecMenuItemTime(tr("Series Timer start time"), start, false, &start));
    AddMenuItem(new cRecMenuItemTime(tr("Series Timer stop time"), stop, false, &stop));
    AddMenuItem(new cRecMenuItemDayChooser(tr("Days to record"), dayOfWeek, false, &dayOfWeek));
    AddMenuItem(new cRecMenuItemDay(tr("Day to start"), tstart, false, &tstart));
    AddMenuItem(new cRecMenuItemInt(tr("Priority"), priority, 0, MAXPRIORITY, false, &priority));
    AddMenuItem(new cRecMenuItemInt(tr("Lifetime"), lifetime, 0, MAXLIFETIME, false, &lifetime));

    AddFooter(new cRecMenuItemButtonYesNo(tr("Create Timer"), tr("Cancel"), rmsSeriesTimerCreate, rmsClose, true));
}

cTimer *cRecMenuSeriesTimer::GetTimer(void) {
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    cTimer *seriesTimer = new cTimer(NULL, NULL, Channels->GetByNumber(channel));
#else
    cTimer *seriesTimer = new cTimer(NULL, NULL, Channels.GetByNumber(channel));
#endif
    cString fileName = "TITLE EPISODE";
    if (folder.size() > 0) {
        std::replace(folder.begin(), folder.end(), '/', '~');
        fileName = cString::sprintf("%s~%s", folder.c_str(), *fileName);
    }
    seriesTimer->SetDay(tstart);
    seriesTimer->SetStart(start);
    seriesTimer->SetStop(stop);
    seriesTimer->SetPriority(priority);
    seriesTimer->SetLifetime(lifetime);
    seriesTimer->SetWeekDays(dayOfWeek);
    seriesTimer->SetFile(*fileName);
    if (timerActive)
        seriesTimer->SetFlags(tfActive);
    else 
        seriesTimer->SetFlags(tfNone);
    return seriesTimer;
}

void cRecMenuSeriesTimer::CalculateTimes(const cEvent *event) {
    tstart = event->StartTime();
    tstart -= Setup.MarginStart * 60;
    time_t tstop = tstart + event->Duration();
    tstop  += Setup.MarginStop * 60;
    
    struct tm tm_r;
    struct tm *time = localtime_r(&tstart, &tm_r);
    start = time->tm_hour * 100 + time->tm_min;
    time = localtime_r(&tstop, &tm_r);
    stop = time->tm_hour * 100 + time->tm_min;
    if (stop >= 2400)
        stop -= 2400;
}

// --- cRecMenuConfirmSeriesTimer  ---------------------------------------------------------
cRecMenuConfirmSeriesTimer::cRecMenuConfirmSeriesTimer(cTimer *seriesTimer) {
    SetMenuWidth(50);
    string line1 = tr("Series Timer created");
    string line2 = "";
    if (seriesTimer) {
        cString days = cTimer::PrintDay(seriesTimer->Day(), seriesTimer->WeekDays(), true);
        line2 = cString::sprintf("%s, %s: %s, %s: %s", *days, tr("Start"), *TimeString(seriesTimer->StartTime()), tr("Stop"), *TimeString(seriesTimer->StopTime()));
    }
    AddHeader(new cRecMenuItemInfo(line1, 2, line2));
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsClose, true));
}

/******************************************************************************************
*   SearchTimer Menus
******************************************************************************************/

// --- cRecMenuSearchTimer  ---------------------------------------------------------
cRecMenuSearchTimer::cRecMenuSearchTimer(const cEvent *event) {
    SetMenuWidth(70);
    string message = tr("Configure Search Timer based on");
    string infoText = *cString::sprintf("\"%s\"", event->Title());
    AddHeader(new cRecMenuItemInfo(message, 2, infoText));
    strncpy(searchString, event->Title(), TEXTINPUTLENGTH);
    AddMenuItem(new cRecMenuItemText(tr("Search Expression:"), searchString, TEXTINPUTLENGTH, false));
    AddFooter(new cRecMenuItemButtonYesNo(tr("Continue"), tr("Cancel"), rmsSearchTimerOptions, rmsClose, true));
}

// --- cRecMenuSearchTimerTemplates  ---------------------------------------------------------
cRecMenuSearchTimerTemplates::cRecMenuSearchTimerTemplates(cTVGuideSearchTimer searchTimer, vector<TVGuideEPGSearchTemplate> templates) {
    SetMenuWidth(70);
    this->searchTimer = searchTimer;
    this->templates = templates;
    string message = tr("Configure Search Timer for Search String");
    AddHeader(new cRecMenuItemInfo(message, 2, searchTimer.SearchString()));
    AddMenuItem(new cRecMenuItemButton(tr("Manually configure Options"), rmsSearchTimerEdit, true));
    
    numTemplates = templates.size();
    for (int i=0; i<numTemplates; i++) {
        string buttonText = *cString::sprintf("%s \"%s\"", tr("Use Template"), templates[i].name.c_str());
        AddMenuItem(new cRecMenuItemButton(buttonText, rmsSearchTimerCreateWithTemplate, false));
    }
}
    
TVGuideEPGSearchTemplate cRecMenuSearchTimerTemplates::GetTemplate(void) {
    TVGuideEPGSearchTemplate templ;
    int tmplActive = GetNumActive() - 1;
    if (tmplActive >= 0 && tmplActive < (int)templates.size())
        templ = templates[tmplActive];
    return templ;
}

// --- cRecMenuSearchTimers  ---------------------------------------------------------
cRecMenuSearchTimers::cRecMenuSearchTimers(std::vector<cTVGuideSearchTimer> searchTimers) {
    SetMenuWidth(70);
    this->searchTimers = searchTimers;
    numSearchTimers = searchTimers.size();
    string headline;
    if (numSearchTimers > 0) {
        headline = tr("EPGSearch Search Timers");
    } else {
        headline = tr("No Search Timers Configured");
    } 
    AddHeader(new cRecMenuItemInfo(headline));
    cRecMenuItem *button = new cRecMenuItemButton(tr("Close"), rmsClose, (!numSearchTimers)?true:false);
    if (numSearchTimers > 0) {
        SetMenuItems();    
        AddFooter(button);
    } else {
        AddMenuItem(button);
    }
}

void cRecMenuSearchTimers::SetMenuItems(void) {
    for (int i = 0; i < numSearchTimers; i++) {
        AddMenuItem(new cRecMenuItemSearchTimer(searchTimers[i], rmsSearchTimerTest, rmsSearchTimerEdit, rmsSearchTimerDeleteConfirm, (i==0)?true:false));
    }
}

cTVGuideSearchTimer cRecMenuSearchTimers::GetSearchTimer(void) {
    cRecMenuItemSearchTimer *activeItem = dynamic_cast<cRecMenuItemSearchTimer*>(GetActive());
    return activeItem->GetTimer();
}

// --- cRecMenuSearchTimerEdit  ---------------------------------------------------------
cRecMenuSearchTimerEdit::cRecMenuSearchTimerEdit(cTVGuideSearchTimer searchTimer, bool advancedOptions) {
    this->advancedOptions = advancedOptions;
    this->searchTimer = searchTimer;
    strncpy(searchString, searchTimer.SearchString().c_str(), TEXTINPUTLENGTH);
    timerActive = searchTimer.Active();
    mode = searchTimer.SearchMode();
    useTitle = searchTimer.UseTitle();
    useSubtitle = searchTimer.UseSubtitle();
    useDescription = searchTimer.UseDescription();
    useChannel = searchTimer.UseChannel();
    startChannel = searchTimer.StartChannel();
    stopChannel = searchTimer.StopChannel();
    useTime = searchTimer.UseTime();
    startTime = searchTimer.StartTime();
    stopTime = searchTimer.StopTime();
    useDayOfWeek = searchTimer.UseDayOfWeek();
    dayOfWeek = searchTimer.DayOfWeek();
    priority = searchTimer.Priority();
    lifetime = searchTimer.Lifetime();
    useEpisode = searchTimer.UseEpisode();
    std::string dir = searchTimer.Directory();
    strncpy(directory, dir.c_str(), TEXTINPUTLENGTH);
    marginStart = searchTimer.MarginStart();
    marginStop = searchTimer.MarginStop();
    useVPS = searchTimer.UseVPS();
    avoidRepeats = searchTimer.AvoidRepeats();
    allowedRepeats = searchTimer.AllowedRepeats();
    compareTitle = searchTimer.CompareTitle();
    compareSubtitle = searchTimer.CompareSubtitle();
    compareSummary = searchTimer.CompareSummary();
    useInFavorites = searchTimer.UseInFavorites();

    SetMenuWidth(70);
    string infoText;
    if (searchTimer.GetID() > -1) {
        infoText = tr("Configure Search Timer Options");
    } else {
        infoText = tr("Create Search Timer");
    }
    AddHeader(new cRecMenuItemInfo(infoText));
    cRecMenuItemButtonYesNo *footerButton = new cRecMenuItemButtonYesNo(tr("Save Search Timer"), tr("Cancel"), rmsSearchTimerSave, rmsSearchTimers, false);
    AddFooter(footerButton);
    CreateMenuItems();
}

void cRecMenuSearchTimerEdit::CreateMenuItems(void) {
    AddMenuItem(new cRecMenuItemText(tr("Search String"), searchString, TEXTINPUTLENGTH, true, searchString));
    AddMenuItem(new cRecMenuItemBool(tr("Active"), timerActive, false, &timerActive, rmsSearchTimerSave));
    vector<string> searchModes;
    searchTimer.GetSearchModes(&searchModes);
    esyslog("tvguideng: num search modes %ld", searchModes.size());
    AddMenuItem(new cRecMenuItemSelect(tr("Search Mode"), searchModes, mode, false, &mode, rmsSearchTimerSave));
    AddMenuItem(new cRecMenuItemBool(tr("Use Title"), useTitle, false, &useTitle, rmsSearchTimerSave));
    AddMenuItem(new cRecMenuItemBool(tr("Use Subtitle"), useSubtitle, false, &useSubtitle, rmsSearchTimerSave));
    AddMenuItem(new cRecMenuItemBool(tr("Use Description"), useDescription, false, &useDescription, rmsSearchTimerSave));
    AddMenuItem(new cRecMenuItemBool(tr("Limit Channels"), useChannel, false, &useChannel, rmsSearchTimerSave));
    if (startChannel == 0)
        startChannel = 1;
    if (stopChannel == 0)
        stopChannel = 1;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    AddMenuItem(new cRecMenuItemChannelChooser(tr("Start Channel"), Channels->GetByNumber(startChannel), false, &startChannel, rmsSearchTimerSave));
    AddMenuItem(new cRecMenuItemChannelChooser(tr("Stop Channel"), Channels->GetByNumber(stopChannel), false, &stopChannel, rmsSearchTimerSave));
#else
    AddMenuItem(new cRecMenuItemChannelChooser(tr("Start Channel"), Channels.GetByNumber(startChannel), false, &startChannel, rmsSearchTimerSave));
    AddMenuItem(new cRecMenuItemChannelChooser(tr("Stop Channel"), Channels.GetByNumber(stopChannel), false, &stopChannel, rmsSearchTimerSave));
#endif
    AddMenuItem(new cRecMenuItemBool(tr("Use Time"), useTime, false, &useTime, rmsSearchTimerSave));
    AddMenuItem(new cRecMenuItemTime(tr("Start after"), startTime, false, &startTime, rmsSearchTimerSave));
    AddMenuItem(new cRecMenuItemTime(tr("Start before"), stopTime, false, &stopTime, rmsSearchTimerSave));

    if (!advancedOptions) {
        AddMenuItem(new cRecMenuItemButton(tr("Display advanced Options"), rmsSearchTimerEditAdvanced, false));
    } else {
        AddMenuItem(new cRecMenuItemBool(tr("Limit Days of the Week"), useDayOfWeek, false, &useDayOfWeek, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemDayChooser(tr("Select Days"), dayOfWeek, false, &dayOfWeek));
        AddMenuItem(new cRecMenuItemInt(tr("Priority"), priority, 0, 99, false, &priority, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemInt(tr("Lifetime"), lifetime, 0, 99, false, &lifetime, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemInt(tr("Time margin for start in minutes"), marginStart, 0, 30, false, &marginStart, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemInt(tr("Time margin for stop in minutes"), marginStop, 0, 30, false, &marginStop, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemBool(tr("Series Recording"), useEpisode, false, &useEpisode, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemSelectDirectory(tr("Folder"), string(directory), false, directory, rmsSearchTimerSave, true));
        AddMenuItem(new cRecMenuItemBool(tr("Use VPS"), useVPS, false, &useVPS, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemBool(tr("Avoid Repeats"), avoidRepeats, false, &avoidRepeats, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemInt(tr("Number of allowed repeats"), allowedRepeats, 0, 30, false, &allowedRepeats, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemBool(tr("Compare Title"), compareTitle, false, &compareTitle, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemBool(tr("Compare Subtitle"), compareSubtitle, false, &compareSubtitle, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemBool(tr("Compare Description"), compareSummary, false, &compareSummary, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemBool(tr("Use in Favorites"), useInFavorites, false, &useInFavorites, rmsSearchTimerSave));
        AddMenuItem(new cRecMenuItemButton(tr("Hide advanced Options"), rmsSearchTimerEdit, false));
    }

    AddMenuItem(new cRecMenuItemButton(tr("Display Results for Search Timer"), rmsSearchTimerTest, false));
}

cTVGuideSearchTimer cRecMenuSearchTimerEdit::GetSearchTimer(void) {
    searchTimer.SetSearchString(searchString);
    searchTimer.SetActive(timerActive);
    searchTimer.SetSearchMode(mode);
    searchTimer.SetUseTitle(useTitle);
    searchTimer.SetUseSubtitle(useSubtitle);
    searchTimer.SetUseDesription(useDescription);
    searchTimer.SetUseChannel(useChannel);
    if (useChannel) {
        searchTimer.SetStartChannel(startChannel);
        searchTimer.SetStopChannel(stopChannel);
    }
    searchTimer.SetUseTime(useTime);
    if (useTime) {
        searchTimer.SetStartTime(startTime);
        searchTimer.SetStopTime(stopTime);
    }
    searchTimer.SetUseDayOfWeek(useDayOfWeek);
    if (useDayOfWeek) {
        searchTimer.SetDayOfWeek(dayOfWeek);
    }
    searchTimer.SetPriority(priority);
    searchTimer.SetLifetime(lifetime);
    searchTimer.SetUseEpisode(useEpisode);
    string dir(directory);
    std::replace(dir.begin(), dir.end(), '/', '~');
    searchTimer.SetDirectory(dir);
    searchTimer.SetMarginStart(marginStart);
    searchTimer.SetMarginStop(marginStop);
    searchTimer.SetUseVPS(useVPS);
    searchTimer.SetAvoidRepeats(avoidRepeats);
    if (avoidRepeats) {
        searchTimer.SetAllowedRepeats(allowedRepeats);
        searchTimer.SetCompareTitle(compareTitle);
        searchTimer.SetCompareSubtitle(compareSubtitle);
        searchTimer.SetCompareSummary(compareSummary);
    }
    searchTimer.SetUseInFavorites(useInFavorites);
    return searchTimer;
}

// --- cRecMenuSearchTimerDeleteConfirm ---------------------------------------------
cRecMenuSearchTimerDeleteConfirm::cRecMenuSearchTimerDeleteConfirm(cTVGuideSearchTimer searchTimer) {
    SetMenuWidth(70);
    this->searchTimer = searchTimer;
    string line1 = tr("Really delete Search Timer");
    string line2 = *cString::sprintf("\"%s\"?", searchTimer.SearchString().c_str());
    AddHeader(new cRecMenuItemInfo(line1, 2, line2));
    AddMenuItem(new cRecMenuItemButton(tr("Delete only Search Timer"), rmsSearchTimerDelete, true));
    AddMenuItem(new cRecMenuItemButton(tr("Delete Search Timer and created Timers"), rmsSearchTimerDeleteWithTimers, false));
    AddFooter(new cRecMenuItemButton(tr("Cancel"), rmsClose, false));
}

cTVGuideSearchTimer cRecMenuSearchTimerDeleteConfirm::GetSearchTimer(void) {
    return searchTimer;
}

// --- cRecMenuSearchTimerCreateConfirm  ---------------------------------------------------------
cRecMenuSearchTimerCreateConfirm::cRecMenuSearchTimerCreateConfirm(bool success) {
    SetMenuWidth(50);
    cRecMenuItemInfo *infoItem = NULL;
    if (success) {
        string line1 = tr("Search Timer sucessfully created");
        string line2 = tr("Search Timer update initialised");
        infoItem = new cRecMenuItemInfo(line1, 2, line2);
    } else {
        string line1 = tr("Search Timer NOT sucessfully created");
        infoItem = new cRecMenuItemInfo(line1);
    }
    AddHeader(infoItem);
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsClose, true));
}

// --- cRecMenuSearchTimerTemplatesCreate  ---------------------------------------------------------
cRecMenuSearchTimerTemplatesCreate::cRecMenuSearchTimerTemplatesCreate(TVGuideEPGSearchTemplate templ, cTVGuideSearchTimer searchTimer) {
    SetMenuWidth(70);
    this->templ = templ;
    this->searchTimer = searchTimer;

    string line1 = tr("Creating Search Timer");
    string line2 = *cString::sprintf("%s: \"%s\"", tr("Search Term"), searchTimer.SearchString().c_str());
    string line3 = *cString::sprintf("%s \"%s\"", tr("Using Template"), templ.name.c_str());
    
    AddHeader(new cRecMenuItemInfo(line1, 3, line2, line3));
    AddMenuItem(new cRecMenuItemButton(tr("Display Results for Search Timer"), rmsSearchTimerTest, true));
    AddMenuItem(new cRecMenuItemButtonYesNo(tr("Create Search Timer"), tr("Use other Template"), rmsSearchTimerSave, rmsSearchTimerOptions, false));
}


// --- cRecMenuSearchTimerResults ---------------------------------------------------------
cRecMenuSearchTimerResults::cRecMenuSearchTimerResults(string searchString, const cEvent **searchResults, int numResults, string templateName, eRecMenuState action2) {
    SetMenuWidth(70);
    this->searchResults = searchResults;
    this->action2 = action2;
    this->numResults = numResults;
    string message1 = "", message2 = "", message3 = "";
    if (action2 == rmsFavoritesRecord) {
        message1 = tr("search results for Favorite");
        message2 = tr("search result for Favorite");
    } else {
        message1 = tr("search results for Search Timer");
        message2 = tr("search result for Search Timer");
        message3 = tr("Using Template");
    }
    cRecMenuItem *infoItem = NULL;
    if (templateName.size() > 0) {
        string line1 = *cString::sprintf("%d %s:", numResults, (numResults>1)?(message1.c_str()):(message2.c_str()));
        string line2 = *cString::sprintf("\"%s\"", searchString.c_str());
        string line3 = *cString::sprintf("%s \"%s\"", message3.c_str(), templateName.c_str());
        infoItem = new cRecMenuItemInfo(line1, 3, line2, line3);
    } else {
        string line1 = *cString::sprintf("%d %s:", numResults, (numResults>1)?(message1.c_str()):(message2.c_str()));
        string line2 = *cString::sprintf("\"%s\"", searchString.c_str());
        infoItem = new cRecMenuItemInfo(line1, 2, line2);
    }
    AddHeader(infoItem);
    
    cRecMenuItem *button = new cRecMenuItemButton(tr("Close"), rmsClose, false);
    if (searchResults && (numResults > 0)) {
        for (int i=0; i<numResults; i++) {
            AddMenuItem(new cRecMenuItemEvent(searchResults[i], rmsSearchShowInfo, action2, (i==0)?true:false));
        }
        AddFooter(button);
    } else {
        AddMenuItem(button);        
    }
}
    
const cEvent *cRecMenuSearchTimerResults::GetEvent(void) {
    const cEvent *ev = NULL;
    if (cRecMenuItemEvent *activeItem = dynamic_cast<cRecMenuItemEvent*>(GetActive()))
        ev = activeItem->GetEvent();
    return ev;
}

// --- cRecMenuSearchTimerNothingFound  ---------------------------------------------------------
cRecMenuSearchTimerNothingFound::cRecMenuSearchTimerNothingFound(string searchString) {
    SetMenuWidth(50);
    string line1 = tr("Nothing found for Search String");
    string line2 = *cString::sprintf("\"%s\"", searchString.c_str());
    AddHeader(new cRecMenuItemInfo(line1, 2, line2));
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsClose, true));
}

/******************************************************************************************
*   SwitchTimer Menus
******************************************************************************************/

// --- cRecMenuSwitchTimer  ---------------------------------------------------------
cRecMenuSwitchTimer::cRecMenuSwitchTimer(void) {
    SetMenuWidth(60);
    switchMinsBefore = config.switchMinsBefore;
    announceOnly = 0;
    
    AddHeader(new cRecMenuItemInfo(tr("Configure Options for Switchtimer")));
    AddMenuItem(new cRecMenuItemInt(tr("Minutes before switching"), switchMinsBefore, 0, 10, false, &switchMinsBefore));
    vector<string> switchModes;
    switchModes.push_back(tr("switch"));
    switchModes.push_back(tr("announce only"));
    switchModes.push_back(tr("ask for switch"));
    AddMenuItem(new cRecMenuItemSelect(tr("Switch Mode"), switchModes, announceOnly, false, &announceOnly));
    
    AddFooter(new cRecMenuItemButtonYesNo(tr("Create"), tr("Cancel"), rmsSwitchTimerCreate, rmsClose, true));
}

cSwitchTimer cRecMenuSwitchTimer::GetSwitchTimer(void) {
    cSwitchTimer st;
    st.switchMinsBefore = switchMinsBefore;
    st.announceOnly = announceOnly;
    return st;
}

// --- cRecMenuSwitchTimerConfirm  ---------------------------------------------------------
cRecMenuSwitchTimerConfirm::cRecMenuSwitchTimerConfirm(bool success) {
    SetMenuWidth(50);
    string message1 = tr("Switch Timer sucessfully created");
    string message2 = tr("Switch Timer NOT sucessfully created");
    string infoText = success?message1:message2;
    AddHeader(new cRecMenuItemInfo(infoText));
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsClose, true));
}

// --- cRecMenuSwitchTimerDelete  ---------------------------------------------------------
cRecMenuSwitchTimerDelete::cRecMenuSwitchTimerDelete(void) {
    SetMenuWidth(50);
    AddHeader(new cRecMenuItemInfo(tr("Switch Timer deleted")));
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsClose, true));
}

/******************************************************************************************
*   Search Menus
******************************************************************************************/

// --- cRecMenuSearch  ---------------------------------------------------------
cRecMenuSearch::cRecMenuSearch(string searchString, bool withOptions) {
    SetMenuWidth(60);
    strncpy(this->searchString, searchString.c_str(), TEXTINPUTLENGTH);
    mode = 0;
    channelNr = 0;
    useTitle = true;
    useSubTitle = true;
    useDescription = false;
    
    AddHeader(new cRecMenuItemInfo(tr("Search")));
    AddMenuItem(new cRecMenuItemText(tr("Search Expression:"), this->searchString, TEXTINPUTLENGTH, false, this->searchString));
    
    if (withOptions) {
        vector<string> searchModes;
        cTVGuideSearchTimer searchTimer;
        searchTimer.GetSearchModes(&searchModes);
        AddMenuItem(new cRecMenuItemSelect(tr("Search Mode"), searchModes, 0, false, &mode));
        AddMenuItem(new cRecMenuItemChannelChooser(tr("Channel to Search"), NULL, false, &channelNr));
        AddMenuItem(new cRecMenuItemBool(tr("Search in title"), true, false, &useTitle));
        AddMenuItem(new cRecMenuItemBool(tr("Search in Subtitle"), true, false, &useSubTitle));
        AddMenuItem(new cRecMenuItemBool(tr("Search in Description"), false, false, &useDescription));
    } else {
        AddMenuItem(new cRecMenuItemButton(tr("Show Search Options"), rmsSearchWithOptions, false));
    }

    cRecMenuItemButtonYesNo *button = new cRecMenuItemButtonYesNo(tr("Perform Search"), tr("Cancel"), rmsSearchPerform, rmsClose, true);
    AddFooter(button);
}

Epgsearch_searchresults_v1_0 cRecMenuSearch::GetEPGSearchStruct(void) {
    Epgsearch_searchresults_v1_0 data;
    data.query = searchString;
    data.mode = mode;
    data.channelNr = channelNr;
    data.useTitle = useTitle;
    data.useSubTitle = useSubTitle;
    data.useDescription = useDescription;
    return data;
}

// --- cRecMenuSearchResults  ---------------------------------------------------------
cRecMenuSearchResults::cRecMenuSearchResults(string searchString, const cEvent **searchResults, int numResults) {
    this->searchResults = searchResults;
    SetMenuWidth(70);
    this->searchString = searchString;
    this->numResults = numResults;
    
    string message = "";
    if (numResults > 1) {
        message = *cString::sprintf("%d %s", numResults, tr("search results for"));
    } else {
        message = *cString::sprintf("%d %s", numResults, tr("search result for"));        
    }
    string searchStringQuoted = *cString::sprintf("\"%s\"", searchString.c_str()); 
    AddHeader(new cRecMenuItemInfo(message, 2, searchStringQuoted));
    
    cRecMenuItem *buttons = new cRecMenuItemButtonYesNo(tr("Adapt Search"), tr("Close"), rmsSearch, rmsClose, false);
    AddFooter(buttons);
    if (searchResults && (numResults > 0)) {
        for (int i=0; i<numResults; i++) {
            AddMenuItem(new cRecMenuItemEvent(searchResults[i], rmsSearchShowInfo, rmsSearchRecord, (i==0)?true:false));
        }
    }
}

const cEvent *cRecMenuSearchResults::GetEvent(void) {
    cRecMenuItemEvent *activeItem = dynamic_cast<cRecMenuItemEvent*>(GetActive());
    if (activeItem)
        return activeItem->GetEvent();
    return NULL;
}

// --- cRecMenuSearchNothingFound  ---------------------------------------------------------
cRecMenuSearchNothingFound::cRecMenuSearchNothingFound(string searchString, bool tooShort) {
    SetMenuWidth(50);
    string text;
    if (!tooShort) {
        cString message = tr("Nothing found for Search String");
        text = *cString::sprintf("%s\n\"%s\"", 
                                *message, 
                                searchString.c_str());
    } else {
        cString message = tr("Search String has to have at least three letters");
        text = *cString::sprintf("%s\n\"%s\"", 
                                *message, 
                                searchString.c_str());

    }
    AddHeader(new cRecMenuItemInfo(text));
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsClose, true));
}

// --- cRecMenuSearchConfirmTimer  ---------------------------------------------------------
cRecMenuSearchConfirmTimer::cRecMenuSearchConfirmTimer(const cEvent *event) {
    SetMenuWidth(50);

    string message = tr("Timer created");
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    cString channelName = Channels->GetByChannelID(event->ChannelID())->Name();
#else
    cString channelName = Channels.GetByChannelID(event->ChannelID())->Name();
#endif
    string line3 = *cString::sprintf("%s %s - %s", *event->GetDateString(), *event->GetTimeString(), *event->GetEndTimeString());
    string line4 = event->Title() ? event->Title() : ""; 

    AddHeader(new cRecMenuItemInfo(message, 4, *channelName, line3, line4));
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsClose, true));
}

/******************************************************************************************
*   Timeline
******************************************************************************************/

// --- cRecMenuTimeline  ---------------------------------------------------------
cRecMenuTimeline::cRecMenuTimeline(void) {
    SetMenuWidth(95);
    SetStartStop();
    GetTimersForDay();

    timelineHeader = new cRecMenuItemTimelineHeader(timeStart);
    AddHeader(timelineHeader);

    timelineFooter = new cRecMenuItemButton(tr("Close"), rmsClose, false);
    AddFooter(timelineFooter);
    SetTimers();
}

void cRecMenuTimeline::SetHeaderTimer(void) {
    const cTimer *currentTimer = NULL;
    cRecMenuItem *currentItem = GetActive();
    if (!currentItem) {
        timelineHeader->UnsetCurrentTimer();
        return;
    }
    if (cRecMenuItemTimelineTimer *myActiveItem = dynamic_cast<cRecMenuItemTimelineTimer*>(currentItem)) {
        currentTimer = myActiveItem->GetTimer();
    }
    timelineHeader->SetCurrentTimer(currentTimer);
    timelineHeader->SetNew();
}

void cRecMenuTimeline::SetStartStop(void) {
    time_t now = time(0);
    tm *timeStruct = localtime(&now);
    timeStart = now - timeStruct->tm_hour * 3600 - timeStruct->tm_min * 60 - timeStruct->tm_sec;
    today = timeStart;
    timeStop = timeStart + 24*3600 - 1;
}

void cRecMenuTimeline::GetTimersForDay(void) {
    timersToday.clear();
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_TIMERS_READ;
    const cTimers* timers = Timers;
#else
    const cTimers* timers = &Timers;
#endif
    for (const cTimer *t = timers->First(); t; t = timers->Next(t)) {
        if (((t->StartTime() > timeStart) && (t->StartTime() <= timeStop)) || ((t->StopTime() > timeStart) && (t->StopTime() <= timeStop))) {
            timersToday.push_back(t);
        }
    }
    numTimersToday = timersToday.size();
}

void cRecMenuTimeline::SetTimers(void) {
    if (numTimersToday == 0) {
        AddMenuItem(new cRecMenuItemInfo(tr("No active Timers")));
        timelineFooter->SetActive();
    } else {
        for (int i=0; i<numTimersToday; i++) {
            AddMenuItem(new cRecMenuItemTimelineTimer(timersToday[i], timeStart, timeStop, (i==0)?true:false));
        }
        timelineFooter->SetInactive();
    }
}

void cRecMenuTimeline::PrevDay(void) {
    if ((timeStart - 3600*24) < today)
        return;
    timeStart -= 3600*24;
    timeStop -= 3600*24;
    ClearMenuItems(true);
    GetTimersForDay();
    SetTimers();
    InitMenuItems();
    SetHeaderTimer();
    timelineHeader->SetDay(timeStart);
    timelineHeader->SetNew();
}

void cRecMenuTimeline::NextDay(void) {
    timeStart += 3600*24;
    timeStop += 3600*24;
    ClearMenuItems(true);
    GetTimersForDay();
    SetTimers();
    InitMenuItems();
    SetHeaderTimer();
    timelineHeader->SetDay(timeStart);
    timelineHeader->SetNew();
}

const cTimer *cRecMenuTimeline::GetTimer(void) {
    if (cRecMenuItemTimelineTimer *activeItem = dynamic_cast<cRecMenuItemTimelineTimer*>(GetActive()))
        return activeItem->GetTimer();
    return NULL;
}


eRecMenuState cRecMenuTimeline::ProcessKey(eKeys Key) {
    eRecMenuState state = rmsContinue;
    switch (Key & ~k_Repeat) {
        case kLeft:
            PrevDay();
            Draw();
            break;
            state = rmsConsumed;
        case kRight:
            NextDay();
            Draw();
            state = rmsConsumed;
            break;
        case kUp: {
            if (ScrollUp(false)) {
                SetHeaderTimer();
                Draw();
            }
            state = rmsConsumed;
            break; }
        case kDown: {
            if (ScrollDown(false)) {
                SetHeaderTimer();
                Draw();
            }
            state = rmsConsumed;
            break; }
        default:
            break;
    }
    if (state != rmsConsumed) {
        state = cRecMenu::ProcessKey(Key);
    }
    return state;
}

/******************************************************************************************
*   Recording Search Menus
******************************************************************************************/

// --- cRecMenuRecordingSearch  ---------------------------------------------------------
cRecMenuRecordingSearch::cRecMenuRecordingSearch(string search) {
    SetMenuWidth(60);
    strncpy(searchString, search.c_str(), TEXTINPUTLENGTH);
    AddHeader(new cRecMenuItemInfo(tr("Search in Recordings")));
    AddMenuItem(new cRecMenuItemText(tr("Search Expression:"), searchString, TEXTINPUTLENGTH, false, searchString));
    AddMenuItem(new cRecMenuItemButtonYesNo(tr("Perform Search"), tr("Cancel"), rmsRecordingSearchResult, rmsClose, true));
}

// --- cRecMenuRecordingSearchResults  ---------------------------------------------------------
cRecMenuRecordingSearchResults::cRecMenuRecordingSearchResults(string searchString, const cRecording **searchResults, int numResults) {
    SetMenuWidth(80);
    this->searchString = searchString;
    this->searchResults = searchResults;
    this->numResults = numResults;
    string line1 = *cString::sprintf("%s %d %s %s:", tr("Found"), numResults, (numResults>1)?tr("recordings"):tr("recording"), tr("for"));
    string line2 = *cString::sprintf("\"%s\"", searchString.c_str());
    AddHeader(new cRecMenuItemInfo(line1, 2, line2));
    
    if (searchResults && (numResults > 0)) {
        for (int i=0; i<numResults; i++) {
            AddMenuItem(new cRecMenuItemRecording(searchResults[i], (i==0)?true:false));
        }
        cRecMenuItem *buttons = new cRecMenuItemButtonYesNo(tr("Adapt Search"), tr("Close"), rmsRecordingSearch, rmsClose, false);
        AddFooter(buttons);
    } else {
        cRecMenuItem *buttons = new cRecMenuItemButtonYesNo(tr("Adapt Search"), tr("Close"), rmsRecordingSearch, rmsClose, true);
        AddMenuItem(buttons);
    }
}

// --- cRecMenuRecordingSearchNotFound  ---------------------------------------------------------
cRecMenuRecordingSearchNotFound::cRecMenuRecordingSearchNotFound(string searchString) {
    SetMenuWidth(50);
    string line2 = *cString::sprintf("\"%s\"", searchString.c_str());
    AddHeader(new cRecMenuItemInfo(tr("No recordings found for"), 2, line2));
    AddMenuItem(new cRecMenuItemButton(tr("OK"), rmsClose, true));
}

/******************************************************************************************
*   Favorites
******************************************************************************************/

// --- cRecMenuFavorites  ---------------------------------------------------------

cRecMenuFavorites::cRecMenuFavorites(vector<cTVGuideSearchTimer> favorites) {
    SetMenuWidth(70);
    this->favorites = favorites;
  
    CreateFavoritesMenuItems();

    string header;
    if (numFavorites > 0) {
        header = tr("Favorites");
    } else {
        header = tr("No Favorites available");
    }
    AddHeader(new cRecMenuItemInfo(header));
    cRecMenuItem *button = new cRecMenuItemButton(tr("Close"), rmsClose, (numFavorites==0)?true:false);
    if (numFavorites != 0)
        AddFooter(button);
    else
        AddMenuItem(button);
}

void cRecMenuFavorites::CreateFavoritesMenuItems(void) {
    bool active = true;
    if (config.favWhatsOnNow) {
        AddMenuItem(new cRecMenuItemFavoriteStatic(tr("What's on now"), rmsFavoritesNow, active));
        active = false;
        numFavorites++;
    }
    if (config.favWhatsOnNext) {
        AddMenuItem(new cRecMenuItemFavoriteStatic(tr("What's on next"), rmsFavoritesNext, active));
        active = false;
        numFavorites++;
    }
    if (config.favUseTime1) {
        string desc = *cString::sprintf("%s (%s)", config.descUser1.c_str(), NiceTime(config.favTime1).c_str());
        AddMenuItem(new cRecMenuItemFavoriteStatic(desc, rmsFavoritesUser1, active));
        active = false;
        numFavorites++;
    }
    if (config.favUseTime2) {
        string desc = *cString::sprintf("%s (%s)", config.descUser2.c_str(), NiceTime(config.favTime2).c_str());
        AddMenuItem(new cRecMenuItemFavoriteStatic(desc, rmsFavoritesUser2, active));
        active = false;
        numFavorites++;
    }
    if (config.favUseTime3) {
        string desc = *cString::sprintf("%s (%s)", config.descUser3.c_str(), NiceTime(config.favTime3).c_str());
        AddMenuItem(new cRecMenuItemFavoriteStatic(desc, rmsFavoritesUser3, active));
        active = false;
        numFavorites++;
    }
    if (config.favUseTime4) {
        string desc = *cString::sprintf("%s (%s)", config.descUser4.c_str(), NiceTime(config.favTime4).c_str());
        AddMenuItem(new cRecMenuItemFavoriteStatic(desc, rmsFavoritesUser4, active));
        active = false;
        numFavorites++;
    }

    int numAdditionalFavs = favorites.size();
    for (int i = 0; i < numAdditionalFavs; i++) {
        AddMenuItem(new cRecMenuItemFavorite(favorites[i], rmsSearchTimerTest, active));
        active = false;
        numFavorites++;
    }
}

string cRecMenuFavorites::NiceTime(int favTime) {
    int hours = favTime/100;
    int mins = favTime - hours * 100;
    return *cString::sprintf("%02d:%02d", hours, mins);
}

cTVGuideSearchTimer cRecMenuFavorites::GetFavorite(void) {
    cRecMenuItemFavorite *activeItem = dynamic_cast<cRecMenuItemFavorite*>(GetActive());
    return activeItem->GetFavorite();
}
