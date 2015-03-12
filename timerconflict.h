#ifndef __TVGUIDE_TIMERCONFLICT_H
#define __TVGUIDE_TIMERCONFLICT_H

class cTVGuideTimerConflict {
public:
    cTVGuideTimerConflict(void);
    virtual ~cTVGuideTimerConflict(void);
    time_t time;
    time_t timeStart;
    time_t timeStop;
    time_t overlapStart;
    time_t overlapStop;
    int percentPossible;
    int timerID;
    std::vector<int> timerIDs;
    bool timerInvolved(int involvedID);
};

class cTVGuideTimerConflicts {
private:
    std::vector<cTVGuideTimerConflict*> conflicts;
    int numConflicts;
    int currentConflict;
public:
    cTVGuideTimerConflicts(void);
    virtual ~cTVGuideTimerConflicts(void);
    void AddConflict(std::string epgSearchConflictLine);
    void CalculateConflicts(void);
    int NumConflicts(void) {return numConflicts; };
    void SetCurrentConflict(int current) { currentConflict = current; };
    cTVGuideTimerConflict *GetCurrentConflict(void);
    int GetCurrentConflictTimerID(int timerIndex);
    int GetCorrespondingConflict(int timerID);
    cTVGuideTimerConflict *GetConflict(int conflictIndex);
    std::vector<cTVGuideTimerConflict*> GetConflictsBetween(time_t start, time_t stop);
};

#endif //__TVGUIDE_RECMMANAGER_H