#include "tvguidengosd.h"
#include "recmenuview.h"

cRecMenuView::cRecMenuView(void) {
    active = false;
    recMenuView = NULL;
    recMenuViewBuffer = NULL;
    recMenuViewBuffer2 = NULL;
    event = NULL;
    displayEvent = NULL;
    activeMenu = NULL;
    activeMenuBuffer = NULL;
    activeMenuBuffer2 = NULL;
    recManager = new cRecManager();
    recManager->SetEPGSearchPlugin();
    timerConflicts = NULL;
}

cRecMenuView::~cRecMenuView() {
    if (activeMenu)
        delete activeMenu;
    if (activeMenuBuffer)
        delete activeMenuBuffer;
    if (activeMenuBuffer2)
        delete activeMenuBuffer2;
    if (recMenuView)
        delete recMenuView;
    if (recMenuViewBuffer)
        delete recMenuViewBuffer;
    if (recMenuViewBuffer2)
        delete recMenuViewBuffer2;
}

void cRecMenuView::Init(skindesignerapi::cOsdView *recMenuView, skindesignerapi::cOsdView *recMenuViewBuffer, skindesignerapi::cOsdView *recMenuViewBuffer2) {
    active = true;
    this->recMenuView = recMenuView;
    this->recMenuViewBuffer = recMenuViewBuffer;
    this->recMenuViewBuffer2 = recMenuViewBuffer2;
}

void cRecMenuView::DisplayRecMenu(const cEvent *event) {
    if (!event) {
        return;
    }
    this->event = event;
    activeMenu = new cRecMenuMain(recManager->EpgSearchAvailable(), recManager->CheckEventForTimer(event), SwitchTimers.EventInSwitchList(event));
    DisplayMenu();
}

void cRecMenuView::DisplayFavorites(void) {
    vector<cTVGuideSearchTimer> favorites;
    recManager->GetFavorites(&favorites);
    activeMenu = new cRecMenuFavorites(favorites);
    DisplayMenu();
}

void cRecMenuView::DisplayRecSearch(const cEvent *e) {
    activeMenu = new cRecMenuRecordingSearch(e->Title() ? e->Title() : "");
    DisplayMenu();
}

void cRecMenuView::DisplaySearchEPG(const cEvent *e) {
    activeMenu = new cRecMenuSearch(e->Title() ? e->Title() : "", false);
    DisplayMenu();
}

void cRecMenuView::Close(void) {
    if (activeMenu) {
        delete activeMenu;
        activeMenu = NULL;
    }
    if (activeMenuBuffer) {
        delete activeMenuBuffer;
        activeMenuBuffer = NULL;
    }
    if (activeMenuBuffer2) {
        delete activeMenuBuffer2;
        activeMenuBuffer2 = NULL;
    }
    delete recMenuView;
    recMenuView = NULL;
    delete recMenuViewBuffer;
    recMenuViewBuffer = NULL;
    delete recMenuViewBuffer2;
    recMenuViewBuffer2 = NULL;
    active = false;
}

void cRecMenuView::Hide(bool full) { 
    if (recMenuViewBuffer) {
        recMenuViewBuffer->Deactivate(true);
        if (full) 
            recMenuView->Deactivate(true);
    } else
        recMenuView->Deactivate(true);
}

void cRecMenuView::Activate(bool full) { 
    if (recMenuViewBuffer) {
        recMenuViewBuffer->Activate();
        if (full)
            recMenuView->Activate();
    } else
        recMenuView->Activate();
}

eOSState cRecMenuView::ProcessKey(eKeys Key) {
    eOSState state = osContinue;
    eRecMenuState nextState = rmsContinue;
    if (!activeMenu)
        return state;
    nextState = activeMenu->ProcessKey(Key);
    if ((nextState == rmsClose) || ((nextState == rmsNotConsumed)&&(Key == kBack))) {
        if (activeMenuBuffer2) {
            delete activeMenu;
            activeMenu = activeMenuBuffer2;
            activeMenuBuffer2 = NULL;
            activeMenu->Show();
            return osContinue;
        } else if (activeMenuBuffer) {
            delete activeMenu;
            activeMenu = activeMenuBuffer;
            activeMenuBuffer = NULL;
            activeMenu->Show();
            return osContinue;
        } else {
            Close();
            return osEnd;
        }
    }
    state = StateMachine(nextState);
    if (activeMenu)
        activeMenu->Flush();
    return state;
}

void cRecMenuView::DisplayMenu(bool buffer, bool buffer2) {
    if (!activeMenu)
        return;
    if (buffer) {
        activeMenu->Init(recMenuViewBuffer);
    } else if (buffer2) {
        activeMenu->Init(recMenuViewBuffer2);
    } else {
        activeMenu->Init(recMenuView);
    }
    if (cRecMenuTimeline *timeline = dynamic_cast<cRecMenuTimeline*>(activeMenu)) {
        timeline->SetHeaderTimer();
    }
    activeMenu->Draw();
}

void cRecMenuView::DisplaySearchTimerList(void) {
    delete activeMenu;
    vector<cTVGuideSearchTimer> searchTimers;
    recManager->GetSearchTimers(&searchTimers);
    activeMenu = new cRecMenuSearchTimers(searchTimers);
    DisplayMenu();
}

bool cRecMenuView::DisplayTimerConflict(const cTimer *timer) {
    int timerID = 0;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
   LOCK_TIMERS_READ;
   const cTimers* timers = Timers;
#else
   const cTimers* timers = &Timers;
#endif
    for (const cTimer *t = timers->First(); t; t = timers->Next(t)) {
        if (t == timer)
            return DisplayTimerConflict(timerID);
        timerID++;
    }
    return false;
}

bool cRecMenuView::DisplayTimerConflict(int timerID) {
    if (timerConflicts)
        delete timerConflicts;
    timerConflicts = recManager->CheckTimerConflict();
    if (!timerConflicts)
        return false;
    int showTimerConflict = timerConflicts->GetCorrespondingConflict(timerID);
    if (showTimerConflict > -1) {
        timerConflicts->SetCurrentConflict(showTimerConflict);
        cTVGuideTimerConflict *conflict = timerConflicts->GetCurrentConflict();
        if (!conflict)
            return false;
        activeMenu = new cRecMenuTimerConflict(conflict);
        DisplayMenu();
        return true;
    }
    return false;
}

void cRecMenuView::DisplayFavoriteResults(string header, const cEvent **result, int numResults) {
    if (numResults) {
        activeMenuBuffer = activeMenu;
        activeMenuBuffer->Hide();
        activeMenu = new cRecMenuSearchTimerResults(header, result, numResults, "", rmsFavoritesRecord);
    } else {
        activeMenuBuffer = activeMenu;
        activeMenuBuffer->Hide();
        activeMenu = new cRecMenuSearchTimerNothingFound(header);
    }
    DisplayMenu(true);
}

eOSState cRecMenuView::StateMachine(eRecMenuState nextState) {
    eOSState state = osContinue;
    switch (nextState) {
/*************************************************************************************** 
*    INSTANT RECORDING 
****************************************************************************************/
        case rmsInstantRecord: {
        //caller: main menu or folder chooser
        //Creating timer for active Event, if no conflict, confirm and exit
            string recFolder = "";
            if (cRecMenuAskFolder *menu = dynamic_cast<cRecMenuAskFolder*>(activeMenu)) {
                recFolder = menu->GetFolder();
            }
            delete activeMenu;
            cTimer *timer = recManager->createTimer(event, recFolder);
            if (!DisplayTimerConflict(timer)) {
                activeMenu = new cRecMenuConfirmTimer(event);
                DisplayMenu();
            }
            break; }
        case rmsInstantRecordFolder: {
        //caller: main menu
        //Asking for Folder
            delete activeMenu;
            activeMenu = new cRecMenuAskFolder(event, rmsInstantRecord);
            DisplayMenu();
            break; }
        case rmsDeleteTimer: {
            //caller: main menu
            //delete timer for active event
            delete activeMenu;
            if (recManager->IsRecorded(event)) {
                activeMenu = new cRecMenuAskDeleteTimer(event);
            } else {
                recManager->DeleteTimer(event);
                activeMenu = new cRecMenuConfirmDeleteTimer(event);
            }
            DisplayMenu();
            break; }
        case rmsDeleteTimerConfirmation: {
            //delete running timer for active event
            recManager->DeleteTimer(event);
            delete activeMenu;
            activeMenu = new cRecMenuConfirmDeleteTimer(event);
            DisplayMenu();
            break; }
        case rmsEditTimer: {
            //edit timer for active event
           const cTimer *timer = recManager->GetTimerForEvent(event);
            if (timer) {
                delete activeMenu;
                activeMenu = new cRecMenuEditTimer(timer, rmsSaveTimer);
                DisplayMenu();
            }
            break; }
        case rmsSaveTimer: {
            //caller: cRecMenuEditTimer
            //save timer for active event
           cTimer timerModified;
            const cTimer *originalTimer;
            if (cRecMenuEditTimer *menu = dynamic_cast<cRecMenuEditTimer*>(activeMenu)) {
                timerModified = menu->GetTimer();
                originalTimer = menu->GetOriginalTimer();
            } else break;
            recManager->SaveTimer(originalTimer, timerModified);
            Close();
            state = osEnd;
            break; }
        case rmsIgnoreTimerConflict:
        //caller: cRecMenuTimerConflict
        //Confirming created Timer
            delete activeMenu;
            activeMenu = new cRecMenuConfirmTimer(event);
            DisplayMenu();
            break;
        case rmsTimerConflictShowInfo: {
            //caller: cRecMenuTimerConflict
            int timerIndex;
            if (cRecMenuTimerConflict *menu = dynamic_cast<cRecMenuTimerConflict*>(activeMenu)) {
                timerIndex = menu->GetTimerConflictIndex();
            } else break;
            int timerID = timerConflicts->GetCurrentConflictTimerID(timerIndex);
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
            LOCK_TIMERS_READ;
            const cTimers* timers = Timers;
#else
            const cTimers* timers = &Timers;
#endif
            const cTimer *t = timers->Get(timerID);
            if (t) {
                displayEvent = t->Event();
                if (displayEvent) {
                    state = osUser1;
                }
            }
            break; }
        case rmsDeleteTimerConflictMenu: {
            //caller: cRecMenuTimerConflict
            //delete timer out of current timer conflict
            int timerIndex;
            if (cRecMenuTimerConflict *menu = dynamic_cast<cRecMenuTimerConflict*>(activeMenu)) {
                timerIndex = menu->GetTimerConflictIndex();
            } else break;
            int timerID = timerConflicts->GetCurrentConflictTimerID(timerIndex);
            recManager->DeleteTimer(timerID);
            delete activeMenu;
            if (!DisplayTimerConflict(timerID)) {
                activeMenu = new cRecMenuConfirmTimer(event);
                DisplayMenu();
            }
            break; }
        case rmsEditTimerConflictMenu: {
            //caller: cRecMenuTimerConflict
            //edit timer out of current timer conflict
            int timerIndex;
            if (cRecMenuTimerConflict *menu = dynamic_cast<cRecMenuTimerConflict*>(activeMenu)) {
                timerIndex = menu->GetTimerConflictIndex();
            } else break;
            int timerID = timerConflicts->GetCurrentConflictTimerID(timerIndex);
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
            LOCK_TIMERS_READ;
            const cTimers* timers = Timers;
#else
            const cTimers* timers = &Timers;
#endif
            const cTimer *timer = timers->Get(timerID);
            if (timer) {
                delete activeMenu;
                activeMenu = new cRecMenuEditTimer(timer, rmsSaveTimerConflictMenu);
                DisplayMenu();
            }
            break; }
        case rmsSaveTimerConflictMenu: {
            //caller: cRecMenuEditTimer
            //save timer from current timer conflict
            cTimer timerModified;
            const cTimer *originalTimer;
            if (cRecMenuEditTimer *menu = dynamic_cast<cRecMenuEditTimer*>(activeMenu)) {
                timerModified = menu->GetTimer();
                originalTimer = menu->GetOriginalTimer();
            } else break;
            recManager->SaveTimer(originalTimer, timerModified);
            delete activeMenu;
            if (!DisplayTimerConflict(originalTimer)) {
                activeMenu = new cRecMenuConfirmTimer(event);
                DisplayMenu();
            }
            break; }
/*************************************************************************************** 
*    SERIES TIMER
****************************************************************************************/
        case rmsSeriesTimer: {
            //caller: main menu oder folder chooser
            string recFolder = "";
            if (cRecMenuAskFolder *menu = dynamic_cast<cRecMenuAskFolder*>(activeMenu)) {
                recFolder = menu->GetFolder();
            }
            delete activeMenu;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
            LOCK_CHANNELS_READ;
            const cChannels* channels = Channels;
#else
            cChannels* channels = &Channels;
#endif
            
            const cChannel *channel = channels->GetByChannelID(event->ChannelID());
            activeMenu = new cRecMenuSeriesTimer(channel, event, recFolder);
            DisplayMenu();
            break; }
        case rmsSeriesTimerFolder:
            //caller: main menu
            //Asking for Folder
            delete activeMenu;
            activeMenu = new cRecMenuAskFolder(event, rmsSeriesTimer);
            DisplayMenu();
            break;
        case rmsSeriesTimerCreate: {
            //caller: cRecMenuSeriesTimer
            cTimer *seriesTimer;
            if (cRecMenuSeriesTimer *menu = dynamic_cast<cRecMenuSeriesTimer*>(activeMenu)) {
                seriesTimer = menu->GetTimer();
            } else break;
            recManager->CreateSeriesTimer(seriesTimer);
            delete activeMenu;
            activeMenu = new cRecMenuConfirmSeriesTimer(seriesTimer);
            DisplayMenu();
            break; }
/********************************************************************************************** 
 *    SWITCH TIMER 
 ***********************************************************************************************/
        case rmsSwitchTimer:
            delete activeMenu;
            activeMenu = new cRecMenuSwitchTimer();
            DisplayMenu();
            break;
        case rmsSwitchTimerCreate: {
            cSwitchTimer switchTimer;
            if (cRecMenuSwitchTimer *menu = dynamic_cast<cRecMenuSwitchTimer*>(activeMenu)) {
                switchTimer = menu->GetSwitchTimer();
            } else break;
            bool success = recManager->CreateSwitchTimer(event, switchTimer);
            delete activeMenu;
            activeMenu = new cRecMenuSwitchTimerConfirm(success);
            DisplayMenu();
            break; }
        case rmsSwitchTimerDelete: 
            recManager->DeleteSwitchTimer(event);
            delete activeMenu;
            activeMenu = new cRecMenuSwitchTimerDelete();
            DisplayMenu();
            break;
/********************************************************************************************** 
 *    SEARCH TIMER 
 ***********************************************************************************************/
        case rmsSearchTimer:
            //Caller: main menu
            //set search String for search timer
            delete activeMenu;
            activeMenu = new cRecMenuSearchTimer(event);
            DisplayMenu();
            break;
        case rmsSearchTimerOptions: {
            //Caller: cRecMenuSearchTimer, cRecMenuSearchTimerTemplates
            //Choose to set options manually or by template
            string searchString;
            cTVGuideSearchTimer searchTimer;
            bool reload = false;
            if (cRecMenuSearchTimer *menu = dynamic_cast<cRecMenuSearchTimer*>(activeMenu)) {
                searchString = menu->GetSearchString();
            } else if (cRecMenuSearchTimerTemplatesCreate *menu = dynamic_cast<cRecMenuSearchTimerTemplatesCreate*>(activeMenu)) {
                searchTimer = menu->GetSearchTimer();
                reload = true;
            } else break;
            delete activeMenu;
            if (searchString.size() < 4) {
                activeMenu = new cRecMenuSearchTimer(event);
            } else { 
                if (!reload) {
                    searchTimer.SetSearchString(searchString);
                }
                vector<TVGuideEPGSearchTemplate> epgSearchTemplates;
                recManager->ReadEPGSearchTemplates(&epgSearchTemplates);
                if (epgSearchTemplates.size() > 0) {
                    activeMenu = new cRecMenuSearchTimerTemplates(searchTimer, epgSearchTemplates);
                } else {
                    activeMenu = new cRecMenuSearchTimerEdit(searchTimer, false);
                }
            }
            DisplayMenu();
            break; }
        case rmsSearchTimers: {
            //caller: main menu
            DisplaySearchTimerList();
            break; }
        case rmsSearchTimerEdit:
        case rmsSearchTimerEditAdvanced: {
            //caller: cRecMenuSearchTimers, cRecMenuSearchTimerEdit, cRecMenuSearchTimerTemplates
            cTVGuideSearchTimer searchTimer;
            bool advancedOptions = false;
            if (cRecMenuSearchTimers *menu = dynamic_cast<cRecMenuSearchTimers*>(activeMenu)) {
                searchTimer = menu->GetSearchTimer();
            } else if (cRecMenuSearchTimerEdit *menu = dynamic_cast<cRecMenuSearchTimerEdit*>(activeMenu)) {
                searchTimer = menu->GetSearchTimer();
                advancedOptions = (nextState == rmsSearchTimerEditAdvanced)?true:false;
            } else if (cRecMenuSearchTimerTemplates *menu = dynamic_cast<cRecMenuSearchTimerTemplates*>(activeMenu)) {
                searchTimer = menu->GetSearchTimer();
            } else break;
            delete activeMenu;
            activeMenu = new cRecMenuSearchTimerEdit(searchTimer, advancedOptions);
            DisplayMenu();
            break; }
        case rmsSearchTimerTest: {
            //caller: cRecMenuSearchTimerEdit, cRecMenuSearchTimerTemplatesCreate, cRecMenuSearchTimers, cRecMenuFavorites
            //show results of currently choosen search timer
            cTVGuideSearchTimer searchTimer;
            eRecMenuState recState = rmsDisabled;
            if (cRecMenuSearchTimerEdit *menu = dynamic_cast<cRecMenuSearchTimerEdit*>(activeMenu)) {
                searchTimer = menu->GetSearchTimer();
                recState = rmsSearchTimerRecord;
            } else if  (cRecMenuSearchTimers *menu = dynamic_cast<cRecMenuSearchTimers*>(activeMenu)) {
                searchTimer = menu->GetSearchTimer();
                recState = rmsSearchTimerRecord;
            } else if (cRecMenuSearchTimerTemplatesCreate *menu = dynamic_cast<cRecMenuSearchTimerTemplatesCreate*>(activeMenu)) {
                searchTimer = menu->GetSearchTimer();
                TVGuideEPGSearchTemplate tmpl = menu->GetTemplate();
                searchTimer.SetTemplate(tmpl.templValue);
                searchTimer.Parse(true);
            } else if (cRecMenuFavorites *menu = dynamic_cast<cRecMenuFavorites*>(activeMenu)) {
                searchTimer = menu->GetFavorite();
                recState = rmsFavoritesRecord;
            }             
            else break;
            int numSearchResults = 0;
            string searchString = searchTimer.BuildSearchString();
            const cEvent **searchResult = recManager->PerformSearchTimerSearch(searchString, numSearchResults);
            if (numSearchResults) {
                activeMenuBuffer = activeMenu;
                activeMenuBuffer->Hide();
                activeMenu = new cRecMenuSearchTimerResults(searchTimer.SearchString(), searchResult, numSearchResults, "", recState);
            } else {
               activeMenuBuffer = activeMenu;
               activeMenuBuffer->Hide();
               activeMenu = new cRecMenuSearchTimerNothingFound(searchTimer.SearchString());
            }
            DisplayMenu(true);
            break; }
        case rmsSearchTimerSave: {
            //caller: cRecMenuSearchTimerEdit, cRecMenuSearchTimerTemplatesCreate
            //create new or modify existing search timer
            cTVGuideSearchTimer searchTimer;
            if (cRecMenuSearchTimerEdit *menu = dynamic_cast<cRecMenuSearchTimerEdit*>(activeMenu)) {
                searchTimer = menu->GetSearchTimer();
            } else if (cRecMenuSearchTimerTemplatesCreate *menu = dynamic_cast<cRecMenuSearchTimerTemplatesCreate*>(activeMenu)) {
                searchTimer = menu->GetSearchTimer();
                TVGuideEPGSearchTemplate tmpl = menu->GetTemplate();
                searchTimer.SetTemplate(tmpl.templValue);
                searchTimer.Parse(true);
            } else break;
            bool success = recManager->SaveSearchTimer(&searchTimer);
            recManager->UpdateSearchTimers();
            if (searchTimer.GetID() >= 0) {
                //Timer modified, show list
                DisplaySearchTimerList();
            } else {
                //new timer, confirm
                delete activeMenu;
                activeMenu = new cRecMenuSearchTimerCreateConfirm(success);
                DisplayMenu();
            }
            break; }
        case rmsSearchTimerCreateWithTemplate: {
            //caller: cRecMenuSearchTimerTemplates
            //create new search timer from template
            TVGuideEPGSearchTemplate templ;
            cTVGuideSearchTimer searchTimer;
            if (cRecMenuSearchTimerTemplates *menu = dynamic_cast<cRecMenuSearchTimerTemplates*>(activeMenu)) {
                templ = menu->GetTemplate();
                searchTimer = menu->GetSearchTimer();
            } else break;
            delete activeMenu;
            activeMenu = new cRecMenuSearchTimerTemplatesCreate(templ, searchTimer);
            DisplayMenu();
            break; }
        case rmsSearchTimerDeleteConfirm: {
            //caller: cRecMenuSearchTimers
            //Ask for confirmation and if timers created by this search timer should alo be deleted
            cTVGuideSearchTimer searchTimer;
            if (cRecMenuSearchTimers *menu = dynamic_cast<cRecMenuSearchTimers*>(activeMenu)) {
                searchTimer = menu->GetSearchTimer();
            } else break;
            activeMenuBuffer = activeMenu;
            activeMenuBuffer->Hide();
            activeMenu = new cRecMenuSearchTimerDeleteConfirm(searchTimer);
            DisplayMenu(true);
            break; }
        case rmsSearchTimerDelete: 
        case rmsSearchTimerDeleteWithTimers: {
            //caller: cRecMenuSearchTimerDeleteConfirm
            //actually delete searchtimer
            cTVGuideSearchTimer searchTimer;
            if (cRecMenuSearchTimerDeleteConfirm *menu = dynamic_cast<cRecMenuSearchTimerDeleteConfirm*>(activeMenu)) {
                searchTimer = menu->GetSearchTimer();
            } else break;
            bool delTimers = (nextState==rmsSearchTimerDeleteWithTimers)?true:false;
            recManager->DeleteSearchTimer(&searchTimer, delTimers);
            delete activeMenuBuffer;
            activeMenuBuffer = NULL;
            DisplaySearchTimerList();
            break; }
        case rmsSearchTimerRecord: {
            //caller: cRecMenuSearchTimerResults
            const cEvent *ev = NULL;
            if (cRecMenuSearchTimerResults *menu = dynamic_cast<cRecMenuSearchTimerResults*>(activeMenu)) {
                ev = menu->GetEvent();
            } else break;
            if (!ev)
                break;
            recManager->createTimer(ev, "");
            activeMenuBuffer2 = activeMenu;
            activeMenuBuffer2->Hide();
            activeMenu = new cRecMenuSearchConfirmTimer(ev);
            DisplayMenu(false, true);
            break; }
/********************************************************************************************** 
*    SEARCH 
***********************************************************************************************/
        case rmsSearch:
        case rmsSearchWithOptions: {
            //caller: main menu, cRecMenuSearch, cRecMenuSearchResults
            bool withOptions = false;
            string searchString = event->Title();
            if (cRecMenuSearch *menu = dynamic_cast<cRecMenuSearch*>(activeMenu)) {
                withOptions = true;
                searchString = menu->GetSearchString();
            } else if (cRecMenuSearchResults *menu = dynamic_cast<cRecMenuSearchResults*>(activeMenu)) {
                searchString = menu->GetSearchString();
            }
            delete activeMenu;
            activeMenu = new cRecMenuSearch(searchString, withOptions);
            DisplayMenu();
            break; }
        case rmsSearchPerform: {
            //caller: cRecMenuSearch
            bool useBuffer = false;
            Epgsearch_searchresults_v1_0 epgSearchData;
            if (cRecMenuSearch *menu = dynamic_cast<cRecMenuSearch*>(activeMenu)) {
                epgSearchData = menu->GetEPGSearchStruct();
            } else break;
            string searchString = epgSearchData.query;
            if (searchString.size() < 3) {
                activeMenu->Hide();
                activeMenuBuffer = activeMenu;
                useBuffer = true;
                activeMenu = new cRecMenuSearchNothingFound(searchString, true);
            } else {
                int numSearchResults = 0;
                const cEvent **searchResult = recManager->PerformSearch(epgSearchData, numSearchResults);
                if (searchResult) {
                    delete activeMenu;
                    activeMenu = new cRecMenuSearchResults(searchString, searchResult, numSearchResults);
                } else {
                    activeMenu->Hide();
                    activeMenuBuffer = activeMenu;
                    useBuffer = true;
                    activeMenu = new cRecMenuSearchNothingFound(searchString);
                }
            }
            DisplayMenu(useBuffer);
            break; }
        case rmsSearchShowInfo: {
            //caller: cRecMenuSearchResults, cRecMenuSearchTimerResults, cRecMenuRerunResults
            if (cRecMenuSearchResults *menu = dynamic_cast<cRecMenuSearchResults*>(activeMenu)) {
                displayEvent = menu->GetEvent();
            } else if (cRecMenuSearchTimerResults *menu = dynamic_cast<cRecMenuSearchTimerResults*>(activeMenu)) {
                displayEvent = menu->GetEvent();
            } else if (cRecMenuRerunResults *menu = dynamic_cast<cRecMenuRerunResults*>(activeMenu)) {
                displayEvent = menu->GetEvent();
            } else break;
            if (displayEvent) {
                state = osUser1;
            }
            break;}
        case rmsSearchRecord: {
            //caller: cRecMenuSearchResults
            const cEvent *ev = NULL;
            if (cRecMenuSearchResults *menu = dynamic_cast<cRecMenuSearchResults*>(activeMenu)) {
                ev = menu->GetEvent();
            } else break;
            if (!ev)
                break;
            recManager->createTimer(ev, "");
            activeMenuBuffer = activeMenu;
            activeMenuBuffer->Hide();
            activeMenu = new cRecMenuSearchConfirmTimer(ev);
            DisplayMenu(true);
            break;}
/********************************************************************************************** 
 *    CHECK FOR TIMER CONFLICTS 
 ***********************************************************************************************/
        case rmsTimerConflicts: {
            //caller: main menu
            //Show timer conflict
            if (timerConflicts) {
                delete timerConflicts;
            }
            timerConflicts = recManager->CheckTimerConflict();
            delete activeMenu;
            int numConflicts = timerConflicts->NumConflicts();
            if (numConflicts > 0) {
                activeMenu = new cRecMenuTimerConflicts(timerConflicts);
            } else {
                activeMenu = new cRecMenuNoTimerConflict();
            }
            DisplayMenu();
            break; }
        case rmsTimerConflict: {
            //caller: cRecMenuTimerConflicts
            //Show timer conflict
            if (!timerConflicts)
                break;
            int timerConflict;
            if (cRecMenuTimerConflicts *menu = dynamic_cast<cRecMenuTimerConflicts*>(activeMenu)) {
                timerConflict = menu->GetTimerConflict();
            } else break;
            timerConflicts->SetCurrentConflict(timerConflict);
            delete activeMenu;
            activeMenu = new cRecMenuTimerConflict(timerConflicts->GetCurrentConflict());
            DisplayMenu();
            break; }
        case rmsSearchRerunsTimerConflictMenu: {
            //caller: cRecMenuTimerConflict
            //Show reruns for timer from timer conflict
            if (!timerConflicts)
                break;
            int timerConflict;
            if (cRecMenuTimerConflict *menu = dynamic_cast<cRecMenuTimerConflict*>(activeMenu)) {
                timerConflict = menu->GetTimerConflictIndex();
            } else break;
            int timerID = timerConflicts->GetCurrentConflictTimerID(timerConflict);
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
            LOCK_TIMERS_READ;
            const cTimers* timers = Timers;
#else
            const cTimers* timers = &Timers;
#endif
            const cTimer *timer = timers->Get(timerID);
            if (timer) {
                const cEvent *event = timer->Event();
                if (event) {
                    int numReruns = 0;
                    const cEvent **reruns = recManager->LoadReruns(event, numReruns);
                    if (reruns && (numReruns > 0)) {
                        activeMenuBuffer = activeMenu;
                        activeMenuBuffer->Hide();
                        activeMenu = new cRecMenuRerunResults(event, reruns, numReruns);
                    } else {
                        activeMenuBuffer = activeMenu;
                        activeMenuBuffer->Hide();
                        activeMenu = new cRecMenuNoRerunsFound((event->Title())?event->Title():"");
                    }
                    DisplayMenu(true);
                }
            }
            break; }
        case rmsTimerConflictRecordRerun: {
            //caller: cRecMenuRerunResults
            //buffer: cRecMenuTimerConflict
            if (!activeMenuBuffer)
                break;
            if (!timerConflicts)
                break;
            const cEvent *replace;
            int originalConflictIndex;
            if (cRecMenuRerunResults *menu = dynamic_cast<cRecMenuRerunResults*>(activeMenu)) {
                replace = menu->GetEvent();
            } else break;
            if (cRecMenuTimerConflict *menu = dynamic_cast<cRecMenuTimerConflict*>(activeMenuBuffer)) {
                originalConflictIndex = menu->GetTimerConflictIndex();
            } else break;
            int originalTimerID = timerConflicts->GetCurrentConflictTimerID(originalConflictIndex);
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
            LOCK_TIMERS_READ;
            const cTimers* timers = Timers;
#else
            const cTimers* timers = &Timers;
#endif
            const cTimer *timerOriginal = timers->Get(originalTimerID);
            if (replace && timerOriginal) {
                recManager->DeleteTimer(timerOriginal->Event());
                recManager->createTimer(replace);
                activeMenuBuffer->Show();
                delete activeMenuBuffer;
                activeMenuBuffer = NULL;
                delete activeMenu;
                activeMenu = new cRecMenuConfirmRerunUsed(timerOriginal->Event(), replace);
                DisplayMenu();
            }
            break; }
        /********************************************************************************************** 
         *    TIMELINE
         ***********************************************************************************************/
       case rmsTimeline: {
            delete activeMenu;
            activeMenu = new cRecMenuTimeline();
            DisplayMenu();
            break; } 
        case rmsTimelineTimerEdit: {
            const cTimer *timer;
            if (cRecMenuTimeline *menu = dynamic_cast<cRecMenuTimeline*>(activeMenu)) {
                timer = menu->GetTimer();
            } else break;
            if (timer) {
                delete activeMenu;
                activeMenu = new cRecMenuEditTimer(timer, rmsTimelineTimerSave);
                DisplayMenu();
            }
            break;}
        case rmsTimelineTimerSave: {
            cTimer timerModified;
            const cTimer *originalTimer;
            if (cRecMenuEditTimer *menu = dynamic_cast<cRecMenuEditTimer*>(activeMenu)) {
                timerModified = menu->GetTimer();
                originalTimer = menu->GetOriginalTimer();
            } else break;
            recManager->SaveTimer(originalTimer, timerModified);
            delete activeMenu;
            activeMenu = new cRecMenuTimeline();
            DisplayMenu();
            break; }
        case rmsTimelineTimerDelete: {
            const cTimer *timer;
            if (cRecMenuEditTimer *menu = dynamic_cast<cRecMenuEditTimer*>(activeMenu)) {
                timer = menu->GetOriginalTimer();
            } else break;
            recManager->DeleteTimer(timer);
            delete activeMenu;
            activeMenu = new cRecMenuTimeline();
            DisplayMenu();
            break; }
        default:
            break;
/********************************************************************************************** 
 *    RECORDINGS SEARCH 
 ***********************************************************************************************/
        case rmsRecordingSearch: {
            //caller: main menu or rmsRecordingSearchResult
            string searchString = event->Title();
            if (cRecMenuRecordingSearchResults *menu = dynamic_cast<cRecMenuRecordingSearchResults*>(activeMenu)) {
                searchString = menu->GetSearchString();
            };
            delete activeMenu;
            activeMenu = new cRecMenuRecordingSearch(searchString);
            DisplayMenu();
            break; }
        case rmsRecordingSearchResult:  {
            //caller: cRecMenuRecordingSearch
            string searchString;
            if (cRecMenuRecordingSearch *menu = dynamic_cast<cRecMenuRecordingSearch*>(activeMenu)) {
                searchString = menu->GetSearchString();
            } else break;
            delete activeMenu;
            if (searchString.size() < 4) {
                activeMenu = new cRecMenuRecordingSearch(searchString);
            } else {
                int numSearchResults = 0;
                const cRecording **searchResult = recManager->SearchForRecordings(searchString, numSearchResults);
                if (numSearchResults == 0) {
                    activeMenu = new cRecMenuRecordingSearchNotFound(searchString);
                } else {
                    activeMenu = new cRecMenuRecordingSearchResults(searchString, searchResult, numSearchResults);
                }
            }
            DisplayMenu();
            break; }
        /********************************************************************************************** 
         *    FAVORITES
         *********************************************************************************************/
        case rmsFavoritesRecord: {
            //caller: cRecMenuSearchTimerResults
            const cEvent *ev = NULL;
            if (cRecMenuSearchTimerResults *menu = dynamic_cast<cRecMenuSearchTimerResults*>(activeMenu)) {
                ev = menu->GetEvent();
            } else break;
            if (!ev)
                break;
            recManager->createTimer(ev, "");
            activeMenuBuffer2 = activeMenu;
            activeMenuBuffer2->Hide();
            activeMenu = new cRecMenuSearchConfirmTimer(ev);
            DisplayMenu(false, true);
            break;}
        case rmsFavoritesNow:
        case rmsFavoritesNext: {
            int numResults = 0;
            bool nowOrNext;
            string header;
            if (nextState == rmsFavoritesNow) {
                nowOrNext = true;
                header = tr("What's on now");
            } else {
                nowOrNext = false;
                header = tr("What's on next");
            }
            const cEvent **result = recManager->WhatsOnNow(nowOrNext, numResults);
            DisplayFavoriteResults(header, result, numResults);
            break; }
        case rmsFavoritesUser1: {
            int numResults = 0;
            const cEvent **result = recManager->UserDefinedTime(1, numResults);
            DisplayFavoriteResults(config.descUser1, result, numResults);
            break; }
        case rmsFavoritesUser2: {
            int numResults = 0;
            const cEvent **result = recManager->UserDefinedTime(2, numResults);
            DisplayFavoriteResults(config.descUser2, result, numResults);
            break; }
        case rmsFavoritesUser3: {
            int numResults = 0;
            const cEvent **result = recManager->UserDefinedTime(3, numResults);
            DisplayFavoriteResults(config.descUser3, result, numResults);
            break; }
        case rmsFavoritesUser4: {
            int numResults = 0;
            const cEvent **result = recManager->UserDefinedTime(4, numResults);
            DisplayFavoriteResults(config.descUser4, result, numResults);
            break; }
        }
    return state;
}
