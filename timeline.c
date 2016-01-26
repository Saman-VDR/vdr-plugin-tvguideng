#include "timeline.h"

long cTimelineElement::idCounter;

// --- cTimelineElement  -------------------------------------------------------------

cTimelineElement::cTimelineElement(time_t elementTime) {
    id = idCounter;
    idCounter++;
    init = true;
    this->elementTime = elementTime;
}

cTimelineElement::~cTimelineElement() {
    
}

bool cTimelineElement::IsNew(void) { 
    if (init) {
        init = false;
        return true;
    }
    return init;
}

bool cTimelineElement::IsFullHour(void) {
    if (elementTime%3600 == 0)
        return true;
    return false;
}

// --- cTimeline  -------------------------------------------------------------

cTimeline::cTimeline(skindesignerapi::cViewGrid *timelineGrid, skindesignerapi::cViewElement *timelineDate, skindesignerapi::cViewElement *timeIndicator, cTimeManager *timeManager) {
    this->timelineGrid = timelineGrid;
    this->timelineDate = timelineDate;
    this->timeIndicator = timeIndicator;
    this->timeManager = timeManager;
    steps = config.displayHours * 2;
    stepDuration = 30 * 60;
    weekday = "";
    timeIndicatorShown = true;
}

cTimeline::~cTimeline(void) {
    Clear();
    delete timelineDate;
    delete timeIndicator;
    delete timelineGrid;
}

void cTimeline::Init(void) {
    time_t startTime = timeManager->GetStart();
    time_t elementTime = startTime;
    for (int i=0; i < steps; i++) {
        cTimelineElement *e = new cTimelineElement(elementTime);
        grids.Add(e);
        elementTime += stepDuration;
    }
}

void cTimeline::Clear(void) {
    timelineGrid->Clear();
    grids.Clear();
}

void cTimeline::ScrollForward(int stepMinutes) {
    int numSteps = stepMinutes / 30;
    for (int i=0; i<numSteps; i++) {
        cTimelineElement *e = grids.First();
        timelineGrid->Delete(e->Id());
        grids.Del(e);
    }
    time_t newStartTime = timeManager->GetEnd() - stepMinutes * 60;
    for (int i=0; i<numSteps; i++) {
        cTimelineElement *e = new cTimelineElement(newStartTime);
        grids.Add(e);
        newStartTime += stepDuration;
    }
}

void cTimeline::ScrollBack(int stepMinutes) {
    int numSteps = stepMinutes / 30;
    for (int i=0; i<numSteps; i++) {
        cTimelineElement *e = grids.Last();
        timelineGrid->Delete(e->Id());
        grids.Del(e);
    }
    time_t newStartTime = timeManager->GetStart() + (numSteps-1) * 30 * 60;
    for (int i=0; i<numSteps; i++) {
        cTimelineElement *e = new cTimelineElement(newStartTime);
        grids.Ins(e);
        newStartTime -= stepDuration;
    }
}

void cTimeline::Draw(void) {
    DrawDate();
    DrawTimeIndicator();
    double x, y, width, height;
    if (config.displayMode == eHorizontal) {
        x = 0.0;
        y = 0.0;
        width = (double)1 / (double)steps;
        height = 1.0;
    } else {
        x = 0.0;
        y = 0.0;
        width = 1.0;
        height = (double)1 / (double)steps;
    }
    for (cTimelineElement *e = grids.First(); e; e = grids.Next(e)) {
        if (e->IsNew()) {
            timelineGrid->ClearTokens();
            timelineGrid->AddIntToken((int)eTimelineGridIT::fullhour, e->IsFullHour());
            timelineGrid->AddStringToken((int)eTimelineGridST::timestring, e->ToString().c_str());
            timelineGrid->SetGrid(e->Id(), x, y, width, height);
        } else {
            timelineGrid->MoveGrid(e->Id(), x, y, width, height);
        }
        if (config.displayMode == eHorizontal)
            x += width;
        else
            y += height;
    }
    timelineGrid->Display();
}

void cTimeline::DrawDate(void) {
    string weekdayNew = *(timeManager->GetWeekday());
    if (!weekdayNew.compare(weekday))
        return;
    weekday = weekdayNew;
    timelineDate->Clear();
    timelineDate->ClearTokens();
    timelineDate->AddStringToken((int)eDateTimeST::weekday, weekday.c_str());
    timelineDate->AddStringToken((int)eDateTimeST::date, *(timeManager->GetDate()));
    timelineDate->Display();
}

void cTimeline::DrawTimeIndicator(void) {
    if (timeManager->NowVisible()) {
        timeIndicatorShown = true;
        int distance = (timeManager->GetNow() - timeManager->GetStart()) / 60;
        int percentTotal = distance*1000/(config.displayHours*60);
        timeIndicator->Clear();
        timeIndicator->ClearTokens();
        timeIndicator->AddIntToken((int)eTimeIndicatorIT::percenttotal, percentTotal);
        timeIndicator->Display();
    } else if (timeIndicatorShown) {
        timeIndicatorShown = false;
        timeIndicator->Clear();
    }
}
