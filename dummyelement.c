#include "dummyelement.h"

cDummyElement::cDummyElement(time_t start, time_t end, cChannelEpg *owner) : cGridElement(owner) {
    this->start = start;
    this->end = end;
    dummy = true;
}

cDummyElement::~cDummyElement(void) {
}

time_t cDummyElement::Duration(void) { 
    //max Duration 5h
    int dur = end - start; 
    if (dur > 18000)
        return 18000;
    return dur; 
};

const char *cDummyElement::Title(void) {
    return tr("No EPG available");
}
    
void cDummyElement::Debug(void) {
    esyslog("tvguideng: dummyelement %ld, %s - %s, Channel %s", id, *TimeString(start), *TimeString(end), Channel()->Name());
}
