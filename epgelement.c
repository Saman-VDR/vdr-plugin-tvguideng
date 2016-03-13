#include "switchtimer.h"
#include "services/remotetimers.h"
#include "epgelement.h"

cEpgElement::cEpgElement(const cEvent *event, cChannelEpg *owner) : cGridElement(owner) {
    this->event = event;
    hasTimer = false;
    SetTimer();
    hasSwitchTimer = false;
    SetSwitchTimer();
    dummy = false;
}

cEpgElement::~cEpgElement(void) {
}

void cEpgElement::SetTimer() {
    if (config.useRemoteTimers && pRemoteTimers) {
        RemoteTimers_Event_v1_0 rt;
        rt.event = event;
        if (pRemoteTimers->Service("RemoteTimers::GetTimerByEvent-v1.0", &rt))
            hasTimer = true;
	    else
	        hasTimer = false;
    } else if (owner->HasTimer()) {
        hasTimer = event->HasTimer();
    } else {
        hasTimer = false;
    }
}

void cEpgElement::SetSwitchTimer() {
    if (owner->HasSwitchTimer()) {
        hasSwitchTimer = SwitchTimers.EventInSwitchList(event);
    } else {
        hasSwitchTimer = false;
    }
}

const char *cEpgElement::Title(void) {
    return event->Title();
}

const char *cEpgElement::ShortText(void) {
    if (event->ShortText())
        return event->ShortText();
    return "";
}

void cEpgElement::Debug() {
    esyslog("tvguideng: epgelement %ld: \"%s\" %s - %s, channel %s, timer: %d", id, event->Title(), *(event->GetTimeString()), *(event->GetEndTimeString()), Channel()->Name(), hasTimer);
}
