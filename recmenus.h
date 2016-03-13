#ifndef __TVGUIDE_RECMENUS_H
#define __TVGUIDE_RECMENUS_H

#define TEXTINPUTLENGTH 256

#include "services/epgsearch.h"
#include "recmenu.h"
#include "recmanager.h"
#include "timerconflict.h"
#include "switchtimer.h"

// --- cRecMenuMain  ---------------------------------------------------------
class cRecMenuMain : public cRecMenu {
private:
    int intcallback;
    bool boolcallback;
    int stringselectcallback;
    char searchString[TEXTINPUTLENGTH];
    int timecallback;
    time_t daycallback;
public:
    cRecMenuMain(bool epgSearchAvailable, bool timerActive, bool switchTimerActive);
    virtual ~cRecMenuMain(void) {};
};

/******************************************************************************************
*   Instant Timer Menus
******************************************************************************************/

// --- cRecMenuConfirmTimer  ---------------------------------------------------------
class cRecMenuConfirmTimer: public cRecMenu {
public:
    cRecMenuConfirmTimer(const cEvent *event);
    virtual ~cRecMenuConfirmTimer(void) {};
};

// --- cRecMenuAskFolder  ---------------------------------------------------------
class cRecMenuAskFolder: public cRecMenu {
private:
    vector<string> folders;
    eRecMenuState nextAction;
public:
    cRecMenuAskFolder(const cEvent *event, eRecMenuState nextAction);
    virtual ~cRecMenuAskFolder(void) {};
    string GetFolder(void);
};

// --- cRecMenuConfirmDeleteTimer  ---------------------------------------------------------
class cRecMenuConfirmDeleteTimer: public cRecMenu {
public:
    cRecMenuConfirmDeleteTimer(const cEvent *event);
    virtual ~cRecMenuConfirmDeleteTimer(void) {};
};

// --- cRecMenuAskDeleteTimer ---------------------------------------------------------
class cRecMenuAskDeleteTimer: public cRecMenu {
public:
    cRecMenuAskDeleteTimer(const cEvent *event);
    virtual ~cRecMenuAskDeleteTimer(void) {};
};

// --- cRecMenuTimerConflicts  ---------------------------------------------------------
class cRecMenuTimerConflicts: public cRecMenu {
public:
    cRecMenuTimerConflicts(cTVGuideTimerConflicts *conflicts);    
    virtual ~cRecMenuTimerConflicts(void) {};
    int GetTimerConflict(void);
};

// --- cRecMenuTimerConflict  ---------------------------------------------------------
class cRecMenuTimerConflict: public cRecMenu {
private:
    cTVGuideTimerConflict *conflict;
public:
    cRecMenuTimerConflict(cTVGuideTimerConflict *conflict);
    virtual ~cRecMenuTimerConflict(void) {};
    int GetTimerConflictIndex(void);
};

// --- cRecMenuNoTimerConflict ---------------------------------------------------------
class cRecMenuNoTimerConflict: public cRecMenu {
public:
    cRecMenuNoTimerConflict(void);   
    virtual ~cRecMenuNoTimerConflict(void) {};
};

// --- cRecMenuRerunResults  ---------------------------------------------------------
class cRecMenuRerunResults: public cRecMenu {
private:
    const cEvent **reruns;
    int numReruns;
public:
    cRecMenuRerunResults(const cEvent *original, const cEvent **reruns, int numReruns);
    int GetTotalNumMenuItems(void);
    virtual ~cRecMenuRerunResults(void) {
        delete[] reruns;
    };
    const cEvent *GetEvent(void);
};

// --- cRecMenuNoRerunsFound  ---------------------------------------------------------
class cRecMenuNoRerunsFound: public cRecMenu {
public:
    cRecMenuNoRerunsFound(string searchString);
    virtual ~cRecMenuNoRerunsFound(void) {};
};

// --- cRecMenuConfirmRerunUsed  ---------------------------------------------------------
class cRecMenuConfirmRerunUsed: public cRecMenu {
public:
    cRecMenuConfirmRerunUsed(const cEvent *original, const cEvent *replace);
    virtual ~cRecMenuConfirmRerunUsed(void) {};
};

// --- cRecMenuEditTimer  ---------------------------------------------------------
class cRecMenuEditTimer: public cRecMenu {
private:
    const cTimer *originalTimer;
    bool timerActive;
    time_t day;
    int start;
    int stop;
    int prio;
    int lifetime;
    char folder[TEXTINPUTLENGTH];
public:
    cRecMenuEditTimer(const cTimer *timer, eRecMenuState nextState);
    virtual ~cRecMenuEditTimer(void) {};
    cTimer GetTimer(void);
    const cTimer *GetOriginalTimer(void);
};

/******************************************************************************************
*   Series Timer Menus
******************************************************************************************/

// --- cRecMenuSeriesTimer ---------------------------------------------------------
class cRecMenuSeriesTimer: public cRecMenu {
    string folder;
    bool timerActive;
    int channel;
    time_t tstart;
    int start;
    int stop;
    int dayOfWeek;
    int priority;
    int lifetime;
    void CalculateTimes(const cEvent *event);
public:
    cRecMenuSeriesTimer(const cChannel *initialChannel, const cEvent *event, string folder);
    virtual ~cRecMenuSeriesTimer(void) {};
    cTimer *GetTimer(void);
};

// --- cRecMenuConfirmSeriesTimer  ---------------------------------------------------------
class cRecMenuConfirmSeriesTimer: public cRecMenu {
public:
    cRecMenuConfirmSeriesTimer(cTimer *seriesTimer);    
    virtual ~cRecMenuConfirmSeriesTimer(void) {};
};

/******************************************************************************************
*   SearchTimer Menus
******************************************************************************************/

// --- cRecMenuSearchTimer  ---------------------------------------------------------
class cRecMenuSearchTimer: public cRecMenu {
private:
    char searchString[TEXTINPUTLENGTH];
public:
    cRecMenuSearchTimer(const cEvent *event);
    virtual ~cRecMenuSearchTimer(void) {};
    string GetSearchString(void) { return searchString; };
};

// --- cRecMenuSearchTimerTemplates  ---------------------------------------------------------
class cRecMenuSearchTimerTemplates: public cRecMenu {
private:
    int numTemplates;
    cTVGuideSearchTimer searchTimer;
    vector<TVGuideEPGSearchTemplate> templates;
public:
    cRecMenuSearchTimerTemplates(cTVGuideSearchTimer searchTimer, vector<TVGuideEPGSearchTemplate> templates);
    virtual ~cRecMenuSearchTimerTemplates(void) {};
    cTVGuideSearchTimer GetSearchTimer(void) { return searchTimer; };
    TVGuideEPGSearchTemplate GetTemplate(void);
};

// --- cRecMenuSearchTimers  ---------------------------------------------------------
class cRecMenuSearchTimers: public cRecMenu {
private:
    int numSearchTimers;
    vector<cTVGuideSearchTimer> searchTimers;
    void SetMenuItems(void);
public:
    cRecMenuSearchTimers(vector<cTVGuideSearchTimer> searchTimers);
    virtual ~cRecMenuSearchTimers(void) {};
    cTVGuideSearchTimer GetSearchTimer(void);
};

// --- cRecMenuSearchTimerEdit  ---------------------------------------------------------
class cRecMenuSearchTimerEdit: public cRecMenu {
private:
    bool advancedOptions;
    cTVGuideSearchTimer searchTimer;
    char searchString[TEXTINPUTLENGTH];
    bool timerActive;
    int mode;
    bool useTitle;
    bool useSubtitle;
    bool useDescription;
    bool useChannel;
    int startChannel;
    int stopChannel;
    bool useTime;
    int startTime;
    int stopTime;
    bool useDayOfWeek;
    int dayOfWeek;
    int priority;
    int lifetime;
    bool useEpisode;
    char directory[TEXTINPUTLENGTH];
    int marginStart;
    int marginStop;
    bool useVPS;
    bool avoidRepeats;
    int allowedRepeats;
    bool compareTitle;
    bool compareSubtitle;
    bool compareSummary;
    bool useInFavorites;
    void CreateMenuItems(void);
public:
    cRecMenuSearchTimerEdit(cTVGuideSearchTimer searchTimer, bool advancedOptions);
    virtual ~cRecMenuSearchTimerEdit(void) {};
    cTVGuideSearchTimer GetSearchTimer(void);
};

// --- cRecMenuSearchTimerDeleteConfirm ---------------------------------------------
class cRecMenuSearchTimerDeleteConfirm: public cRecMenu {
private:
    cTVGuideSearchTimer searchTimer;
public:
    cRecMenuSearchTimerDeleteConfirm(cTVGuideSearchTimer searchTimer);
    virtual ~cRecMenuSearchTimerDeleteConfirm(void) {};
    cTVGuideSearchTimer GetSearchTimer(void);
};

// --- cRecMenuSearchTimerCreateConfirm  ---------------------------------------------------------
class cRecMenuSearchTimerCreateConfirm: public cRecMenu {
private:
public:
    cRecMenuSearchTimerCreateConfirm(bool success);
    virtual ~cRecMenuSearchTimerCreateConfirm(void) {};
};

// --- cRecMenuSearchTimerTemplatesCreate  ---------------------------------------------------------
class cRecMenuSearchTimerTemplatesCreate: public cRecMenu {
private:
    TVGuideEPGSearchTemplate templ;
    cTVGuideSearchTimer searchTimer;
public:
    cRecMenuSearchTimerTemplatesCreate(TVGuideEPGSearchTemplate templ, cTVGuideSearchTimer searchTimer);
    virtual ~cRecMenuSearchTimerTemplatesCreate(void) {};
    cTVGuideSearchTimer GetSearchTimer(void) { return searchTimer; };
    TVGuideEPGSearchTemplate GetTemplate(void) { return templ; };
};

// --- cRecMenuSearchTimerResults ---------------------------------------------------------
class cRecMenuSearchTimerResults: public cRecMenu {
private:
    const cEvent **searchResults;
    int numResults;
    eRecMenuState action2;
public:
    cRecMenuSearchTimerResults(string searchString, const cEvent **searchResults, int numResults, string templateName = "", eRecMenuState action2 = rmsDisabled);
    virtual ~cRecMenuSearchTimerResults(void) {
        delete[] searchResults;
    };
    const cEvent *GetEvent(void);
};

// --- cRecMenuSearchTimerNothingFound  ---------------------------------------------------------
class cRecMenuSearchTimerNothingFound: public cRecMenu {
public:
    cRecMenuSearchTimerNothingFound(string searchString);
    virtual ~cRecMenuSearchTimerNothingFound(void) {};
};

/******************************************************************************************
*   SwitchTimer Menus
******************************************************************************************/

// --- cRecMenuSwitchTimer  ---------------------------------------------------------
class cRecMenuSwitchTimer: public cRecMenu {
private:
    int switchMinsBefore;
    int announceOnly;
public:
    cRecMenuSwitchTimer(void);
    virtual ~cRecMenuSwitchTimer(void) {};
    cSwitchTimer GetSwitchTimer(void);
};

// --- cRecMenuSwitchTimerConfirm  ---------------------------------------------------------
class cRecMenuSwitchTimerConfirm: public cRecMenu {
private:
public:
    cRecMenuSwitchTimerConfirm(bool success);
    virtual ~cRecMenuSwitchTimerConfirm(void) {};
};

// --- cRecMenuSwitchTimerDelete  ---------------------------------------------------------
class cRecMenuSwitchTimerDelete: public cRecMenu {
private:
public:
    cRecMenuSwitchTimerDelete(void);
    virtual ~cRecMenuSwitchTimerDelete(void) {};
};

/******************************************************************************************
*   Search Menus
******************************************************************************************/

// --- cRecMenuSearch  ---------------------------------------------------------
class cRecMenuSearch: public cRecMenu {
private:
    char searchString[TEXTINPUTLENGTH];
    int mode;
    int channelNr;
    bool useTitle;
    bool useSubTitle;
    bool useDescription;
public:
    cRecMenuSearch(string searchString, bool withOptions);
    virtual ~cRecMenuSearch(void) {};
    string GetSearchString(void) { return searchString; };
    Epgsearch_searchresults_v1_0 GetEPGSearchStruct(void);
};

// --- cRecMenuSearchResults  ---------------------------------------------------------
class cRecMenuSearchResults: public cRecMenu {
private:
    string searchString;
    const cEvent **searchResults;
    int numResults;
public:
    cRecMenuSearchResults(string searchString, const cEvent **searchResults, int numResults);
    virtual ~cRecMenuSearchResults(void) {
        delete[] searchResults;
    };
    string GetSearchString(void) { return searchString; };
    const cEvent *GetEvent(void);
};

// --- cRecMenuSearchNothingFound  ---------------------------------------------------------
class cRecMenuSearchNothingFound: public cRecMenu {
public:
    cRecMenuSearchNothingFound(string searchString, bool tooShort = false);
    virtual ~cRecMenuSearchNothingFound(void) {};
};

// --- cRecMenuSearchConfirmTimer  ---------------------------------------------------------
class cRecMenuSearchConfirmTimer: public cRecMenu {
public:
    cRecMenuSearchConfirmTimer(const cEvent *event);
    virtual ~cRecMenuSearchConfirmTimer(void) {};
};

/******************************************************************************************
*   Timeline
******************************************************************************************/

// --- cRecMenuTimeline  ---------------------------------------------------------
class cRecMenuTimeline: public cRecMenu {
private:
    vector<const cTimer*> timersToday;
    int numTimersToday;
    time_t today;
    time_t timeStart;
    time_t timeStop;
    cRecMenuItemTimelineHeader *timelineHeader;
    cRecMenuItem *timelineFooter;
    void SetStartStop(void);
    void GetTimersForDay(void);
    void SetTimers(void);
    void PrevDay(void);
    void NextDay(void);
    void ClearMenu(void);
public:
    cRecMenuTimeline(void);
    virtual ~cRecMenuTimeline(void) {};
    eRecMenuState ProcessKey(eKeys Key);
    void SetHeaderTimer(void);
    const cTimer *GetTimer(void);
};

/******************************************************************************************
*   Recording Search Menus
******************************************************************************************/

// --- cRecMenuRecordingSearch  ---------------------------------------------------------
class cRecMenuRecordingSearch: public cRecMenu {
private:
    char searchString[TEXTINPUTLENGTH];
public:
    cRecMenuRecordingSearch(string search);
    virtual ~cRecMenuRecordingSearch(void) {};
    string GetSearchString(void) { return searchString; };
};

// --- cRecMenuRecordingSearchResults  ---------------------------------------------------------
class cRecMenuRecordingSearchResults: public cRecMenu {
private:
    string searchString;
    const cRecording **searchResults;
    int numResults;
public:
    cRecMenuRecordingSearchResults(string searchString, const cRecording **searchResults, int numResults);
    virtual ~cRecMenuRecordingSearchResults(void) {
        delete[] searchResults;
    };
    string GetSearchString(void) { return searchString; };
};

// --- cRecMenuRecordingSearchNotFound  ---------------------------------------------------------
class cRecMenuRecordingSearchNotFound: public cRecMenu {
public:
    cRecMenuRecordingSearchNotFound(string searchString);
    virtual ~cRecMenuRecordingSearchNotFound(void) {};
};

/******************************************************************************************
*   Favorites
******************************************************************************************/

// --- cRecMenuFavorites  ---------------------------------------------------------
class cRecMenuFavorites: public cRecMenu {
private:
    vector<cTVGuideSearchTimer> favorites;
    int numFavorites;
    void CreateFavoritesMenuItems(void);
    string NiceTime(int favTime);
public:
    cRecMenuFavorites(vector<cTVGuideSearchTimer> favorites);
    virtual ~cRecMenuFavorites(void) {};
    cTVGuideSearchTimer GetFavorite(void);
};

#endif //__TVGUIDE_RECMENUS_H
