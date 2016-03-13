#include <time.h>
#include <vdr/tools.h>
#include "config.h"
#include "timemanager.h"

cTimeManager::cTimeManager(void) {
    displayHours = config.displayHours;
    displaySeconds = displayHours * 3600;
    timeFormat = e24Hours;
}

cTimeManager::~cTimeManager(void) {
}

cString cTimeManager::PrintTime(time_t displayTime) {
    struct tm *ts;
    ts = localtime(&displayTime);
    cString strTime = cString::sprintf("%d.%d-%d:%d.%d", ts->tm_mday, ts->tm_mon+1, ts->tm_hour, ts->tm_min, ts->tm_sec);
    return strTime;
}


void cTimeManager::Now() {   
    t = time(0);
    tStart = t;
    tStart = GetRounded();
    tEnd = tStart + displaySeconds;
}

void cTimeManager::AddMinutes(int step) {
    tStart += step*60;
    tEnd  += step*60;
}

bool cTimeManager::DelMinutes(int step) {
    if ((tStart - step*60)+30*60 < t) {
        return true;
    }
    tStart -= step*60;
    tEnd -= step*60;
    return false;
}

void cTimeManager::SetTime(time_t newTime) {
    tStart = newTime;
    tEnd = tStart + displaySeconds;
}

time_t cTimeManager::GetPrevPrimetime(time_t current) {
    tm *st = localtime(&current);
    if (st->tm_hour < 21) {
        current -= 24 * 60* 60;
        st = localtime(&current);
    }
    st->tm_hour = 20;
    st->tm_min = 0;
    time_t primeTime = mktime(st);
    return primeTime;
}

time_t cTimeManager::GetNextPrimetime(time_t current){
    tm *st = localtime(&current);
    if (st->tm_hour > 19) {
        current += 24 * 60* 60;
        st = localtime(&current);
    }
    st->tm_hour = 20;
    st->tm_min = 0;
    time_t primeTime = mktime(st);
    return primeTime;
}

bool cTimeManager::TooFarInPast(time_t current) {
    if (current < t) {
        return true;
    }
    return false;
}

cString cTimeManager::GetCurrentTime() {
    char buf[25];
    t = time(0);
    tm *st = localtime(&t);
    if (timeFormat == e12Hours) {
        strftime(buf, sizeof(buf), "%I:%M %p", st);
    } else if (timeFormat == e24Hours)
        strftime(buf, sizeof(buf), "%H:%M", st);
    return buf;
    
}

cString cTimeManager::GetDate() {
    char text[6];
    tm *st = localtime(&tStart);
    snprintf(text, sizeof(text), "%d.%d", st->tm_mday, st->tm_mon+1);
    return text;
}

cString cTimeManager::GetWeekday() {
    return WeekDayName(tStart);
}

bool cTimeManager::IsStart(int activeStart) {
    if (tStart <= t && activeStart <= t)
        return true;
    return false;
}

time_t cTimeManager::GetRounded() {
    tm *rounded = localtime ( &tStart );
    rounded->tm_sec = 0;
    if (rounded->tm_min > 29)
        rounded->tm_min = 30;
    else
        rounded->tm_min = 0;
    return mktime(rounded);
}

bool cTimeManager::NowVisible(void) {
    if (t > tStart)
        return true;
    return false;
}

void cTimeManager::Debug() {
    esyslog("tvguideng: now %s, tStart: %s, tEnd: %s", *TimeString(t), *TimeString(tStart), *TimeString(tEnd));
}

// --- cTimeInterval ------------------------------------------------------------- 

cTimeInterval::cTimeInterval(time_t start, time_t stop) {
    this->start = start;
    this->stop = stop;
}

cTimeInterval::~cTimeInterval(void) {
}

cTimeInterval *cTimeInterval::Intersect(cTimeInterval *interval) {
    time_t startIntersect, stopIntersect;
    
    if ((stop <= interval->Start()) || (interval->Stop() <= start)) {
        return NULL;
    }
    
    if (start <= interval->Start()) {
        startIntersect = interval->Start();
    } else {
        startIntersect = start;
    }
    if (stop <= interval->Stop()) {
        stopIntersect = stop;
    } else {
        stopIntersect = interval->Stop();
    }
    return new cTimeInterval(startIntersect, stopIntersect);
}

cTimeInterval *cTimeInterval::Union(cTimeInterval *interval) {
    time_t startUnion, stopUnion;
    
    if (start <= interval->Start()) {
        startUnion = start;
    } else {
        startUnion = interval->Start();
    }
    if (stop <= interval->Stop()) {
        stopUnion = interval->Stop();
    } else {
        stopUnion = stop;
    }
    return new cTimeInterval(startUnion, stopUnion);
}