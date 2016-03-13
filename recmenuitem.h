#ifndef __TVGUIDE_RECMENUITEM_H
#define __TVGUIDE_RECMENUITEM_H

#define AUTO_ADVANCE_TIMEOUT 1500

using namespace std;

#include <string>
#include <sstream>
#include <vdr/plugin.h>
#include <vdr/tools.h>
#include <libskindesignerapi/skindesignerosdbase.h>
#include "definitions.h"
#include "searchtimer.h"

enum eRecMenuState {
    rmsConsumed,
    rmsNotConsumed,
    rmsRefresh,
    rmsContinue,
    rmsClose,
    rmsDisabled,
    //INSTANT TIMER
    rmsInstantRecord,
    rmsInstantRecordFolder,
    rmsIgnoreTimerConflict,
    rmsDeleteTimerConflictMenu,
    rmsEditTimerConflictMenu,
    rmsSearchRerunsTimerConflictMenu,
    rmsSaveTimerConflictMenu,
    rmsTimerConflictShowInfo,
    rmsDeleteTimer,
    rmsDeleteTimerConfirmation,
    rmsEditTimer,
    rmsSaveTimer,
    //SEARCH
    rmsSearch,
    rmsSearchWithOptions,
    rmsSearchPerform,
    rmsSearchShowInfo,
    rmsSearchRecord,
    rmsSearchRecordConfirm,
    rmsSearchNothingFoundConfirm,
    //SERIES TIMER
    rmsSeriesTimer,
    rmsSeriesTimerFolder,
    rmsSeriesTimerCreate,
    //SEARCHTIMER
    rmsSearchTimer,
    rmsSearchTimerOptions,
    rmsSearchTimers,
    rmsSearchTimerEdit,
    rmsSearchTimerEditAdvanced,
    rmsSearchTimerTest,
    rmsSearchTimerSave,
    rmsSearchTimerCreateWithTemplate,
    rmsSearchTimerDeleteConfirm,
    rmsSearchTimerDelete,
    rmsSearchTimerDeleteWithTimers,
    rmsSearchTimerRecord,
    //SWITCHTIMER
    rmsSwitchTimer,
    rmsSwitchTimerCreate,
    rmsSwitchTimerDelete,
    //RECORDINGS SEARCH
    rmsRecordingSearch,
    rmsRecordingSearchResult,
    //TIMER CONFLICTS
    rmsTimerConflict,
    rmsTimerConflicts,
    rmsTimerConflictIgnoreReruns,
    rmsTimerConflictRecordRerun,
    //TIMELINE
    rmsTimeline,
    rmsTimelineTimerEdit,
    rmsTimelineTimerSave,
    rmsTimelineTimerDelete,
    //FAVORITES
    rmsFavoritesRecord,
    rmsFavoritesNow,
    rmsFavoritesNext,    
    rmsFavoritesUser1,
    rmsFavoritesUser2,
    rmsFavoritesUser3,
    rmsFavoritesUser4,
};

// --- cRecMenuItem  -------------------------------------------------------------
class cRecMenuItem  : public cListObject {
protected:
    static long idCounter;
    long id;
    bool init;
    bool active;
    bool selectable;
    eRecMenuState action;
    //height of item in percent of screen height
    int height;
    string text;
public:
    cRecMenuItem(void);
    virtual ~cRecMenuItem(void);
    virtual void SetActive(void) { active = true; }
    virtual void SetInactive(void) { active = false; }
    long Id(void) { return id; };
    bool Selectable(void) { return selectable; }
    bool IsNew(void);
    void SetNew(void) { init = true; };
    bool Active(void) { return active; }
    int GetHeight(void) { return height; };
    string GetText(void) { return text; };
    virtual void SetTokens(skindesignerapi::cViewGrid *menu) {};
    virtual eRecMenuState ProcessKey(eKeys Key) { return rmsNotConsumed; };
};

// --- cRecMenuItemInfo  -------------------------------------------------------
class cRecMenuItemInfo : public cRecMenuItem {
private:
    int numLines;
    string line1;
    string line2;
    string line3;
    string line4;
public:
    cRecMenuItemInfo(string line1, int numLines = 1, string line2 = "", string line3 = "", string line4 = "");
    virtual ~cRecMenuItemInfo(void);
    string GetText(void) { return text; };
    void SetTokens(skindesignerapi::cViewGrid *menu);
};

// --- cRecMenuItemButton  -------------------------------------------------------
class cRecMenuItemButton : public cRecMenuItem {
private:
    string text;
public:
    cRecMenuItemButton(string text, eRecMenuState action, bool active);
    virtual ~cRecMenuItemButton(void);
    string GetText(void) { return text; };
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemButtonYesNo  -------------------------------------------------------
class cRecMenuItemButtonYesNo : public cRecMenuItem {
private:
    string textYes;
    string textNo;
    eRecMenuState actionNo;
    bool yesActive;
public:
    cRecMenuItemButtonYesNo(string textYes,
                            string textNo,
                            eRecMenuState actionYes, 
                            eRecMenuState actionNo,
                            bool active);
    virtual ~cRecMenuItemButtonYesNo(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemInt  -------------------------------------------------------
class cRecMenuItemInt : public cRecMenuItem {
private:
    int currentVal;
    int *callback;
    int minVal;
    int maxVal;
    bool fresh;
public:
    cRecMenuItemInt(string text,
                    int initialVal,
                    int minVal,
                    int maxVal,
                    bool active = false,
                    int *callback = NULL,
                    eRecMenuState action = rmsNotConsumed);
    virtual ~cRecMenuItemInt(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemBool  -------------------------------------------------------
class cRecMenuItemBool : public cRecMenuItem {
private:
    bool yes;
    bool *callback;
public:
    cRecMenuItemBool(string text,
                     bool initialVal,
                     bool active = false,
                     bool *callback = NULL,
                     eRecMenuState action = rmsNotConsumed);
    virtual ~cRecMenuItemBool(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemSelect  -------------------------------------------------------
class cRecMenuItemSelect : public cRecMenuItem {
private:
    int currentVal;
    int *callback;
    vector<string> strings;
    int numValues;
public:
    cRecMenuItemSelect(string text,
                       vector<string> strings,
                       int initialVal,
                       bool active = false,
                       int *callback = NULL,
                       eRecMenuState action = rmsNotConsumed);
    virtual ~cRecMenuItemSelect(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemText  -------------------------------------------------------
class cRecMenuItemText : public cRecMenuItem {
private:
    char *value;
    string buffer;
    char *callback;
    int length;
    const char *allowed;
    int pos, offset;
    bool insert, newchar, uppercase;
    int lengthUtf8;
    uint *valueUtf8;
    uint *allowedUtf8;
    uint *charMapUtf8;
    uint *currentCharUtf8;
    eKeys lastKey;
    cTimeMs autoAdvanceTimeout;
    uint *IsAllowed(uint c);
    void AdvancePos(void);
    uint Inc(uint c, bool Up);
    void Type(uint c);
    void Insert(void);
    void Delete(void);
    void EnterEditMode(void);
    void LeaveEditMode(bool SaveValue = false);
    bool InEditMode(void) { return valueUtf8 != NULL; };    
    void SetText(void);
public:
    cRecMenuItemText(string title,
                     char *initialVal,
                     int length,
                     bool active = false,
                     char *callback = NULL);
    virtual ~cRecMenuItemText(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemTime  -------------------------------------------------------
class cRecMenuItemTime : public cRecMenuItem {
private:
    int value;
    int *callback;
    int mm;
    int hh;
    int pos;
    bool fresh;
public:
    cRecMenuItemTime(string text,
                    int initialVal,
                    bool active = false,
                    int *callback = NULL,
                    eRecMenuState action = rmsNotConsumed);
    virtual ~cRecMenuItemTime(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemDay  -------------------------------------------------------
class cRecMenuItemDay : public cRecMenuItem {
private:
    time_t currentVal;
    time_t *callback;
public:
    cRecMenuItemDay(string text,
                    time_t initialVal,
                    bool active = false,
                    time_t *callback = NULL,
                    eRecMenuState action = rmsNotConsumed);
    virtual ~cRecMenuItemDay(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemChannelChooser -------------------------------------------------------
class cRecMenuItemChannelChooser : public cRecMenuItem {
private:
    const cChannel *channel;
    int channelNumber;
    int *callback;
    bool initialChannelSet;
    bool fresh;
public:
    cRecMenuItemChannelChooser (string text,
                                const cChannel *initialChannel,
                                bool active = false,
                                int *callback = NULL,
                                eRecMenuState action = rmsNotConsumed);
    virtual ~cRecMenuItemChannelChooser(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemDayChooser -------------------------------------------------------
class cRecMenuItemDayChooser : public cRecMenuItem {
private:
    int weekdays;
    int *callback;
    int selectedDay;
    void ToggleDay(void);
    bool WeekDaySet(unsigned day);
public:
    cRecMenuItemDayChooser (string text,
                            int weekdays,
                            bool active = false,
                            int *callback = NULL);
    virtual ~cRecMenuItemDayChooser(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemSelectDirectory  -------------------------------------------------------
class cRecMenuItemSelectDirectory : public cRecMenuItem {
private:
    string text;
    string originalFolder;
    vector<string> folders;
    int currentVal;
    char *callback;
    int numValues;
    void SetCallback(void);
    int GetInitial(void);
public:
    cRecMenuItemSelectDirectory(string text,
                                string originalFolder,
                                bool active = false,
                                char *callback = NULL,
                                eRecMenuState action = rmsNotConsumed,
                                bool isSearchTimer = false);
    virtual ~cRecMenuItemSelectDirectory(void) {};
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemTimerConflictHeader -------------------------------------------------------
class cRecMenuItemTimerConflictHeader: public cRecMenuItem {
private:
    time_t conflictStart;
    time_t conflictStop;
    time_t overlapStart;
    time_t overlapStop;
public:
    cRecMenuItemTimerConflictHeader(time_t conflictStart,
                                    time_t conflictStop,
                                    time_t overlapStart,
                                    time_t overlapStop);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    virtual ~cRecMenuItemTimerConflictHeader(void);
};

// --- cRecMenuItemTimer  -------------------------------------------------------
class cRecMenuItemTimer : public cRecMenuItem {
private:
    const cTimer *timer;
    eRecMenuState action2;
    eRecMenuState action3;
    eRecMenuState action4;
    int iconActive;
    time_t conflictStart;
    time_t conflictStop;
    time_t overlapStart;
    time_t overlapStop;
public:
    cRecMenuItemTimer(const cTimer *timer, 
                      eRecMenuState action1, 
                      eRecMenuState action2,
                      eRecMenuState action3,
                      eRecMenuState action4,
                      time_t conflictStart,
                      time_t conflictStop,
                      time_t overlapStart,
                      time_t overlapStop,
                      bool active);
    virtual ~cRecMenuItemTimer(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemEvent  -------------------------------------------------------
class cRecMenuItemEvent : public cRecMenuItem {
private:
    const cEvent *event;
    eRecMenuState action2;
    int iconActive;
public:
    cRecMenuItemEvent(const cEvent *event, 
                      eRecMenuState action1, 
                      eRecMenuState action2, 
                      bool active);
    virtual ~cRecMenuItemEvent(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
    const cEvent *GetEvent(void) { return event; };
};

// --- cRecMenuItemRecording  -------------------------------------------------------
class cRecMenuItemRecording : public cRecMenuItem {
private:
    const cRecording *recording;
public:
    cRecMenuItemRecording(const cRecording *recording, bool active);
    virtual ~cRecMenuItemRecording(void) {};
    void SetTokens(skindesignerapi::cViewGrid *menu);
};

// --- cRecMenuItemSearchTimer  -------------------------------------------------------
class cRecMenuItemSearchTimer : public cRecMenuItem {
private:
    cTVGuideSearchTimer timer;
    eRecMenuState action1;
    eRecMenuState action2;
    eRecMenuState action3;
    int iconActive;
public:
    cRecMenuItemSearchTimer(cTVGuideSearchTimer timer, 
                            eRecMenuState action1,
                            eRecMenuState action2,
                            eRecMenuState action3,
                            bool active);
    virtual ~cRecMenuItemSearchTimer(void);
    cTVGuideSearchTimer GetTimer(void) { return timer; };
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemTimelineHeader  -------------------------------------------------------
class cRecMenuItemTimelineHeader : public cRecMenuItem {
private:
    time_t day;
    const cTimer *timer;
public:
    cRecMenuItemTimelineHeader(time_t day);
    virtual ~cRecMenuItemTimelineHeader(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    void SetDay(time_t day) { this->day = day; };
    void SetCurrentTimer(const cTimer *timer) { this->timer = timer; };
    void UnsetCurrentTimer(void) { timer = NULL; };
};

// --- cRecMenuItemTimelineTimer  -------------------------------------------------------
class cRecMenuItemTimelineTimer : public cRecMenuItem {
private:
    const cTimer *timer;
    time_t start;
    time_t stop;
public:
    cRecMenuItemTimelineTimer(const cTimer *timer, time_t start, time_t stop, bool active);
    virtual ~cRecMenuItemTimelineTimer(void);
    void SetTokens(skindesignerapi::cViewGrid *menu);
    const cTimer *GetTimer(void);
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemFavorite  -------------------------------------------------------
class cRecMenuItemFavorite : public cRecMenuItem {
private:
    cTVGuideSearchTimer favorite;
    eRecMenuState action1;
public:
    cRecMenuItemFavorite(cTVGuideSearchTimer favorite, 
                         eRecMenuState action1,
                         bool active);
    virtual ~cRecMenuItemFavorite(void) {};
    void SetTokens(skindesignerapi::cViewGrid *menu);
    cTVGuideSearchTimer GetFavorite(void) { return favorite; };
    eRecMenuState ProcessKey(eKeys Key);
};

// --- cRecMenuItemFavoriteStatic  -------------------------------------------------------
class cRecMenuItemFavoriteStatic : public cRecMenuItem {
private:
    string text;
public:
    cRecMenuItemFavoriteStatic(string text, eRecMenuState action, bool active);
    virtual ~cRecMenuItemFavoriteStatic(void) {};
    void SetTokens(skindesignerapi::cViewGrid *menu);
    eRecMenuState ProcessKey(eKeys Key);
};

#endif //__TVGUIDE_RECMENUITEM_H