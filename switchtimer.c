#include "switchtimer.h"

cSwitchTimers SwitchTimers;

// -- cSwitchTimer -----------------------------------------------------------------
cSwitchTimer::cSwitchTimer(void) {
    eventID = 0;
    startTime = 0;
    switchMinsBefore = config.switchMinsBefore;
    announceOnly = 0;
}

cSwitchTimer::cSwitchTimer(const cEvent* Event) {
    eventID = 0;
    startTime = 0;
    if (Event) {
      eventID = Event->EventID();
      channelID = Event->ChannelID();
      startTime = Event->StartTime();
    }
}

bool cSwitchTimer::Parse(const char *s) {
    char *line;
    char *pos;
    char *pos_next;
    int parameter = 1;
    int valuelen;
#define MAXVALUELEN (10 * MaxFileName)

    char value[MAXVALUELEN];
    startTime=0;

    pos = line = strdup(s);
    pos_next = pos + strlen(pos);
    if (*pos_next == '\n') *pos_next = 0;
        while (*pos) {
            while (*pos == ' ') 
                pos++;
            if (*pos) {
                if (*pos != ':') {
                    pos_next = strchr(pos, ':');
                    if (!pos_next)
                        pos_next = pos + strlen(pos);
                    valuelen = pos_next - pos + 1;
                    if (valuelen > MAXVALUELEN)
                        valuelen = MAXVALUELEN;
                    strn0cpy(value, pos, valuelen);
                    pos = pos_next;
                    switch (parameter) {
                        case 1:
                            channelID = tChannelID::FromString(value);
                            break;
                        case 2:
                            eventID = atoi(value);
                            break;
                        case 3:
                            startTime = atol(value);
                            break;
                        default:
                            break;
                    }
                }
                parameter++;
            }
            if (*pos) 
                pos++;
        }
    free(line);
    return (parameter >= 3) ? true : false;
}

bool cSwitchTimers::EventInSwitchList(const cEvent* event) {
    if (!event) return false;
    cMutexLock SwitchTimersLock(this);
    cSwitchTimer* switchTimer = SwitchTimers.First();
    while (switchTimer) {
        if (switchTimer->eventID == event->EventID())
            return true;
        switchTimer = SwitchTimers.Next(switchTimer);
    }
    return false;
}

bool cSwitchTimers::ChannelInSwitchList(const cChannel* channel) {
    if (!channel) return false;
    cMutexLock SwitchTimersLock(this);
    cSwitchTimer* switchTimer = SwitchTimers.First();
    while (switchTimer) {
        if (switchTimer->channelID == channel->GetChannelID())
            return true;
        switchTimer = SwitchTimers.Next(switchTimer);
    }
    return false;
}

void cSwitchTimers::DeleteSwitchTimer(const cEvent *event) {
    if (!event) return;
    cMutexLock SwitchTimersLock(this);
    cSwitchTimer* switchTimer = SwitchTimers.First();
    cSwitchTimer* delTimer = NULL;
    
    while (switchTimer) {
        if (switchTimer->eventID == event->EventID()) {
            delTimer = switchTimer;
            break;
        }
        switchTimer = SwitchTimers.Next(switchTimer);
    }
    if (delTimer) {
        SwitchTimers.Del(delTimer, true);
    }
}
