#ifndef __TVGUIDE_TIMEMANAGER_H
#define __TVGUIDE_TIMEMANAGER_H

#include <vdr/tools.h>

enum eTimeFormat {
    e12Hours,
    e24Hours
};

// --- cTimeManager ------------------------------------------------------------- 

class cTimeManager {
    private:
        time_t t;
        time_t tStart;
        time_t tEnd;
        int displayHours;
        int displaySeconds;
        eTimeFormat timeFormat;
    public:
        cTimeManager(void);
        virtual ~cTimeManager(void);
        static cString PrintTime(time_t displayTime);
        void Now();
        time_t GetNow() { return t; };
        void AddMinutes(int step);
        bool DelMinutes(int step);
        void SetTime(time_t newTime);
        time_t Get() {return t;};
        time_t GetStart() {return tStart;};
        time_t GetEnd() {return tEnd;};
        cString GetCurrentTime();
        cString GetDate();
        cString GetWeekday();
        time_t GetPrevPrimetime(time_t current);
        time_t GetNextPrimetime(time_t current);
        bool TooFarInPast(time_t current);
        bool IsStart(int activeStart);
        time_t GetRounded();
        bool NowVisible(void);
        int GetDisplaySeconds(void) {return displaySeconds; };
        void Debug();
};

// --- cTimeInterval ------------------------------------------------------------- 

class cTimeInterval {
    private:
        time_t start;
        time_t stop;
    public:
        cTimeInterval(time_t start, time_t stop);
        virtual ~cTimeInterval(void);
        time_t Start(void) { return start; };
        time_t Stop(void) { return stop; };
        cTimeInterval *Intersect(cTimeInterval *interval);
        cTimeInterval *Union(cTimeInterval *interval);
};

#endif //__TVGUIDE_TIMEMANAGER_H