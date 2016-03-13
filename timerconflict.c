#include <string>
#include <vector>
#include <vdr/timers.h>
#include "helpers.h"
#include "timemanager.h"
#include "timerconflict.h"

cTVGuideTimerConflict::cTVGuideTimerConflict(void) {
    time = 0;
    timeStart = 0;
    timeStop = 0;
    overlapStart = 0;
    overlapStop = 0;
    percentPossible = 0;
    timerID = 0;
}

cTVGuideTimerConflict::~cTVGuideTimerConflict(void) {

}

bool cTVGuideTimerConflict::timerInvolved(int involvedID) {
    int numConflicts = timerIDs.size();
    for (int i=0; i<numConflicts; i++) {
        if (timerIDs[i] == involvedID)
            return true;
    }
    return false;
}

// --- cTVGuideTimerConflicts------------------------------------

cTVGuideTimerConflicts::cTVGuideTimerConflicts(void) {
	numConflicts = 0;
	currentConflict = -1;
}

cTVGuideTimerConflicts::~cTVGuideTimerConflicts(void) {
	for(std::vector<cTVGuideTimerConflict*>::const_iterator it = conflicts.begin(); it != conflicts.end(); it++) {
        cTVGuideTimerConflict *conf = *it;
        delete conf;
    }
    conflicts.clear();
}

void cTVGuideTimerConflicts::AddConflict(std::string epgSearchConflictLine) {
	/* TIMERCONFLICT FORMAT:
    The result list looks like this for example when we have 2 timer conflicts at one time:
    1190232780:152|30|50#152#45:45|10|50#152#45
    '1190232780' is the time of the conflict in seconds since 1970-01-01. 
    It's followed by list of timers that have a conflict at this time:
    '152|30|50#1    int editTimer(cTimer *timer, bool active, int prio, int start, int stop);
    52#45' is the description of the first conflicting timer. Here:
    '152' is VDR's timer id of this timer as returned from VDR's LSTT command
    '30' is the percentage of recording that would be done (0...100)
    '50#152#45' is the list of concurrent timers at this conflict
    '45|10|50#152#45' describes the next conflict
    */
    cTVGuideTimerConflict *conflict = new cTVGuideTimerConflict();
    splitstring s(epgSearchConflictLine.c_str());
    std::vector<std::string> flds = s.split(':');
    if (flds.size() < 2)
        return;
    conflict->time = atoi(flds[0].c_str());
    splitstring s2(flds[1].c_str());
    std::vector<std::string> flds2 = s2.split('|');
    if (flds2.size() < 3)
        return;
    conflict->timerID = atoi(flds2[0].c_str());
    conflict->percentPossible = atoi(flds2[1].c_str());
    splitstring s3(flds2[2].c_str());
    std::vector<std::string> flds3 = s3.split('#');
    std::vector<int> timerIDs;
    for (int k = 0; k < (int)flds3.size(); k++) {
        timerIDs.push_back(atoi(flds3[k].c_str()) - 1);
    }
    conflict->timerIDs = timerIDs;
    conflicts.push_back(conflict);
}

void cTVGuideTimerConflicts::CalculateConflicts(void) {
	numConflicts = conflicts.size();
    for (int i=0; i < numConflicts; i++) {
        cTimeInterval *unionSet = NULL;
        int numTimers = conflicts[i]->timerIDs.size();
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
        LOCK_TIMERS_READ;
        const cTimers* timers = Timers;
#else
        const cTimers* timers = &Timers;
#endif

        for (int j=0; j < numTimers; j++) {
            const cTimer *timer = timers->Get(conflicts[i]->timerIDs[j]);
            if (timer) {
                if (!unionSet) {
                    unionSet = new cTimeInterval(timer->StartTime(), timer->StopTime());
                } else {
                    cTimeInterval *timerInterval = new cTimeInterval(timer->StartTime(), timer->StopTime());
                    cTimeInterval *newUnion = unionSet->Union(timerInterval);
                    delete unionSet;
                    delete timerInterval;
                    unionSet = newUnion;
                }
            }
        }
        conflicts[i]->timeStart = unionSet->Start();
        conflicts[i]->timeStop = unionSet->Stop();
        delete unionSet;
        
        cTimeInterval *intersect = NULL;
        for (int j=0; j < numTimers; j++) {
            const cTimer *timer = timers->Get(conflicts[i]->timerIDs[j]);
            if (timer) {
                if (!intersect) {
                    intersect = new cTimeInterval(timer->StartTime(), timer->StopTime());
                } else {
                    cTimeInterval *timerInterval = new cTimeInterval(timer->StartTime(), timer->StopTime());
                    cTimeInterval *newIntersect = intersect->Intersect(timerInterval);
                    if (newIntersect) {
                        delete intersect;
                        intersect = newIntersect;
                    }
                    delete timerInterval;
                }
            }
        }
        conflicts[i]->overlapStart = intersect->Start();
        conflicts[i]->overlapStop = intersect->Stop();
        delete intersect;
    }
}

cTVGuideTimerConflict *cTVGuideTimerConflicts::GetCurrentConflict(void) {
	if (currentConflict < 0)
		return NULL;
	if (currentConflict > (numConflicts-1))
		return NULL;
	return conflicts[currentConflict];
}

int cTVGuideTimerConflicts::GetCurrentConflictTimerID(int timerIndex) {
	if (currentConflict < 0)
		return -1;
	if (currentConflict > (numConflicts-1))
		return -1;
	int numTimersInConflict = conflicts[currentConflict]->timerIDs.size();
	if (timerIndex > (numTimersInConflict - 1))
		return -1;
	return conflicts[currentConflict]->timerIDs[timerIndex];
}

int cTVGuideTimerConflicts::GetCorrespondingConflict(int timerID) {
    int conflictIndex = -1;
    if (numConflicts > 0) {
        for (int i=0; i<numConflicts; i++) {
            if (conflicts[i]->timerInvolved(timerID)) {
                conflictIndex = i;
                break;
            }
        }
    }
    return conflictIndex;
}

cTVGuideTimerConflict *cTVGuideTimerConflicts::GetConflict(int conflictIndex) {
    if (conflictIndex < 0)
        return NULL;
    if (conflictIndex > (numConflicts-1))
        return NULL;
    return conflicts[conflictIndex];
}

std::vector<cTVGuideTimerConflict*> cTVGuideTimerConflicts::GetConflictsBetween(time_t start, time_t stop) {
    std::vector<cTVGuideTimerConflict*> conflictsFound;
    for (int i=0; i < numConflicts; i++) {
        if ((conflicts[i]->timeStart > start) && (conflicts[i]->timeStart < stop)||
            (conflicts[i]->timeStop  > start) && (conflicts[i]->timeStop < stop))
            conflictsFound.push_back(conflicts[i]);
    }
    return conflictsFound;
}
