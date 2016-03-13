#include "gridelement.h"
#include "channelepg.h"

long cGridElement::idCounter;

cGridElement::cGridElement(cChannelEpg *owner) {
    id = idCounter;
    idCounter++;
    init = true;
    active = false;
    hasTimer = false;
    hasSwitchTimer = false;
    this->owner = owner;
}

cGridElement::~cGridElement(void) {
}

bool cGridElement::Match(time_t t) {
    if ((StartTime() < t) && (EndTime() > t))
        return true;
    else
        return false;
}

bool cGridElement::IsFirst(void) {
    return owner->IsFirst(this);
}

int cGridElement::CalcOverlap(cGridElement *neighbor) {
    int overlap = 0;
    if (Intersects(neighbor)) {
        if ((StartTime() <= neighbor->StartTime()) && (EndTime() <= neighbor->EndTime())) {
            overlap = EndTime() - neighbor->StartTime();
        } else if ((StartTime() >= neighbor->StartTime()) && (EndTime() >= neighbor->EndTime())) {
            overlap = neighbor->EndTime() - StartTime();
        } else if ((StartTime() >= neighbor->StartTime()) && (EndTime() <= neighbor->EndTime())) {
            overlap = Duration();
        } else if ((StartTime() <= neighbor->StartTime()) && (EndTime() >= neighbor->EndTime())) {
            overlap = neighbor->EndTime() - neighbor->StartTime();
        }
    }
    return overlap;
}

bool cGridElement::Intersects(cGridElement *neighbor) {
    return ! ( (neighbor->EndTime() <= StartTime()) || (neighbor->StartTime() >= EndTime()) ); 
}

const cChannel *cGridElement::Channel(void) { 
    return owner->Channel(); 
}

cChannelEpg *cGridElement::Owner(void) {
    return owner;
}
