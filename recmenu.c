#include "recmenu.h"
#include "tvguidengosd.h"

// --- cRecMenu  -------------------------------------------------------------

cRecMenu::cRecMenu() {
    hidden = false;
    menuWidth = 50;
    menuHeight = 0;
    maxMenuHeight = 98;
    recMenuGrid = NULL;
    osdView = NULL;
    scrollBar = NULL;
    back = NULL;
    start = NULL;
    stop = NULL;
    itemCount = 0;
    active = NULL;
    header = NULL;
    footer = NULL;
    scrolling = false;
}

cRecMenu::~cRecMenu(void) {
    menuItems.Clear();
    if (scrollBar) {
        scrollBar->Clear();
        delete scrollBar;
        scrollBar = NULL;
    }
    if (back) {
        back->Clear();
        delete back;
        back = NULL;
    }
    if (recMenuGrid) {
        recMenuGrid->Clear();
        delete recMenuGrid;
    }
    if (hidden)
        osdView->Activate();
}

/********************************************************************
* Public Functions
********************************************************************/

void cRecMenu::Init(skindesignerapi::cOsdView *osdView) {
    this->osdView = osdView;
    recMenuGrid = osdView->GetViewGrid((int)eViewGridsRecMenu::menu);
    scrollBar = osdView->GetViewElement((int)eViewElementsRecMenu::scrollbar);
    back = osdView->GetViewElement((int)eViewElementsRecMenu::background);
    InitMenuItems();
}

void cRecMenu::Draw(void) {
    DrawHeader();
    double width = (double)menuWidth / (double)100;
    double x = (double)(100 - menuWidth)/(double)200;
    int totalHeight = GetHeight();
    int yPerc = (100 - totalHeight) / 2;
    if (header)
        yPerc += header->GetHeight();
    double y = (double)yPerc/(double)100;

    for (cRecMenuItem *current = start; current; current = menuItems.Next(current)) {

        double itemHeight = (double)(current->GetHeight())/(double)100;
        if (current->IsNew()) {
            current->SetTokens(recMenuGrid);
            recMenuGrid->SetGrid(current->Id(), x, y, width, itemHeight);
        } else {
            recMenuGrid->MoveGrid(current->Id(), x, y, width, itemHeight);            
        }
        if (current->Active()) {
            recMenuGrid->SetCurrent(current->Id(), true);
        }
        y += itemHeight;
        if (current == stop)
            break;
    }
    DrawFooter();
    recMenuGrid->Display();
}

eRecMenuState cRecMenu::ProcessKey(eKeys Key) {
    eRecMenuState state = rmsContinue;
    if (!active)
        return state;

    state = active->ProcessKey(Key);
    if (state == rmsRefresh) {
        //Refresh current
        active->SetTokens(recMenuGrid);
        active->SetNew();
        Draw();
    } else if (state == rmsNotConsumed) {
        switch (Key & ~k_Repeat) {
            case kUp:
                if (!ScrollUp(false))
                    SetLast();
                Draw();
                state = rmsConsumed;
                break;
            case kDown:
                if (!ScrollDown(false))
                    SetFirst();
                Draw();
                state = rmsConsumed;
                break;
            case kLeft:
                if (PageUp())
                    Draw();
                state = rmsConsumed;
                break;
            case kRight:
                if (PageDown())
                    Draw();
                state = rmsConsumed;
                break;
            case kBack:
                state = rmsClose;
                break;
            default:
                break;
        }
    }
    return state;
}

void cRecMenu::DefineTokens(eViewElementsRecMenu ve, skindesignerapi::cTokenContainer *tk) {
    switch (ve) {
        case eViewElementsRecMenu::background:
            tk->DefineIntToken("{menuwidth}", (int)eBackgroundRecMenuIT::menuwidth);
            tk->DefineIntToken("{menuheight}", (int)eBackgroundRecMenuIT::menuheight);
            tk->DefineIntToken("{hasscrollbar}", (int)eBackgroundRecMenuIT::hasscrollbar);
            break;
        case eViewElementsRecMenu::scrollbar:
            tk->DefineIntToken("{menuwidth}", (int)eScrollbarRecMenuIT::menuwidth);
            tk->DefineIntToken("{posy}", (int)eScrollbarRecMenuIT::posy);
            tk->DefineIntToken("{totalheight}", (int)eScrollbarRecMenuIT::totalheight);
            tk->DefineIntToken("{height}", (int)eScrollbarRecMenuIT::height);
            tk->DefineIntToken("{offset}", (int)eScrollbarRecMenuIT::offset);
            break;
        default:
            break;
    }
}

void cRecMenu::DefineTokens(eViewGridsRecMenu vg, skindesignerapi::cTokenContainer *tk) {
    tk->DefineIntToken("{info}", (int)eRecMenuIT::info);
    tk->DefineIntToken("{lines}", (int)eRecMenuIT::lines);
    tk->DefineIntToken("{button}", (int)eRecMenuIT::button);
    tk->DefineIntToken("{buttonyesno}", (int)eRecMenuIT::buttonyesno);
    tk->DefineIntToken("{yes}", (int)eRecMenuIT::yes);
    tk->DefineIntToken("{intselector}", (int)eRecMenuIT::intselector);
    tk->DefineIntToken("{intvalue}", (int)eRecMenuIT::value);
    tk->DefineIntToken("{boolselector}", (int)eRecMenuIT::boolselector);
    tk->DefineIntToken("{stringselector}", (int)eRecMenuIT::stringselector);
    tk->DefineIntToken("{textinput}", (int)eRecMenuIT::textinput);
    tk->DefineIntToken("{editmode}", (int)eRecMenuIT::editmode);
    tk->DefineIntToken("{timeselector}", (int)eRecMenuIT::timeselector);
    tk->DefineIntToken("{dayselector}", (int)eRecMenuIT::dayselector);
    tk->DefineIntToken("{channelselector}", (int)eRecMenuIT::channelselector);
    tk->DefineIntToken("{channelnumber}", (int)eRecMenuIT::channelnumber);
    tk->DefineIntToken("{channellogoexisis}", (int)eRecMenuIT::channellogoexisis);
    tk->DefineIntToken("{weekdayselector}", (int)eRecMenuIT::weekdayselector);
    tk->DefineIntToken("{dayselected}", (int)eRecMenuIT::dayselected);
    tk->DefineIntToken("{day0set}", (int)eRecMenuIT::day0set);
    tk->DefineIntToken("{day1set}", (int)eRecMenuIT::day1set);
    tk->DefineIntToken("{day2set}", (int)eRecMenuIT::day2set);
    tk->DefineIntToken("{day3set}", (int)eRecMenuIT::day3set);
    tk->DefineIntToken("{day4set}", (int)eRecMenuIT::day4set);
    tk->DefineIntToken("{day5set}", (int)eRecMenuIT::day5set);
    tk->DefineIntToken("{day6set}", (int)eRecMenuIT::day6set);
    tk->DefineIntToken("{directoryselector}", (int)eRecMenuIT::directoryselector);
    tk->DefineIntToken("{timerconflictheader}", (int)eRecMenuIT::timerconflictheader);
    tk->DefineIntToken("{overlapstartpercent}", (int)eRecMenuIT::overlapstartpercent);
    tk->DefineIntToken("{overlapwidthpercent}", (int)eRecMenuIT::overlapwidthpercent);
    tk->DefineIntToken("{timerconflict}", (int)eRecMenuIT::timerconflict);
    tk->DefineIntToken("{infoactive}", (int)eRecMenuIT::infoactive);
    tk->DefineIntToken("{deleteactive}", (int)eRecMenuIT::deleteactive);
    tk->DefineIntToken("{editactive}", (int)eRecMenuIT::editactive);
    tk->DefineIntToken("{searchactive}", (int)eRecMenuIT::searchactive);
    tk->DefineIntToken("{timerstartpercent}", (int)eRecMenuIT::timerstartpercent);
    tk->DefineIntToken("{timerwidthpercent}", (int)eRecMenuIT::timerwidthpercent);
    tk->DefineIntToken("{event}", (int)eRecMenuIT::event);
    tk->DefineIntToken("{hastimer}", (int)eRecMenuIT::hastimer);
    tk->DefineIntToken("{recording}", (int)eRecMenuIT::recording);
    tk->DefineIntToken("{recduration}", (int)eRecMenuIT::recduration);
    tk->DefineIntToken("{searchtimer}", (int)eRecMenuIT::searchtimer);
    tk->DefineIntToken("{timeractive}", (int)eRecMenuIT::timeractive);
    tk->DefineIntToken("{activetimers}", (int)eRecMenuIT::activetimers);
    tk->DefineIntToken("{recordingsdone}", (int)eRecMenuIT::recordingsdone);
    tk->DefineIntToken("{timelineheader}", (int)eRecMenuIT::timelineheader);
    tk->DefineIntToken("{timerset}", (int)eRecMenuIT::timerset);
    tk->DefineIntToken("{channeltransponder}", (int)eRecMenuIT::channeltransponder);
    tk->DefineIntToken("{timelinetimer}", (int)eRecMenuIT::timelinetimer);
    tk->DefineIntToken("{timerstart}", (int)eRecMenuIT::timerstart);
    tk->DefineIntToken("{timerwidth}", (int)eRecMenuIT::timerwidth);
    tk->DefineIntToken("{favorite}", (int)eRecMenuIT::favorite);
    tk->DefineStringToken("{line1}", (int)eRecMenuST::line1);
    tk->DefineStringToken("{line2}", (int)eRecMenuST::line2);
    tk->DefineStringToken("{line3}", (int)eRecMenuST::line3);
    tk->DefineStringToken("{line4}", (int)eRecMenuST::line4);
    tk->DefineStringToken("{stringvalue}", (int)eRecMenuST::value);
    tk->DefineStringToken("{buttontext}", (int)eRecMenuST::buttontext);
    tk->DefineStringToken("{textyes}", (int)eRecMenuST::textyes);
    tk->DefineStringToken("{textno}", (int)eRecMenuST::textno);
    tk->DefineStringToken("{text}", (int)eRecMenuST::text);
    tk->DefineStringToken("{channelname}", (int)eRecMenuST::channelname);
    tk->DefineStringToken("{channelid}", (int)eRecMenuST::channelid);
    tk->DefineStringToken("{transponder}", (int)eRecMenuST::transponder);
    tk->DefineStringToken("{day0abbr}", (int)eRecMenuST::day0abbr);
    tk->DefineStringToken("{day1abbr}", (int)eRecMenuST::day1abbr);
    tk->DefineStringToken("{day2abbr}", (int)eRecMenuST::day2abbr);
    tk->DefineStringToken("{day3abbr}", (int)eRecMenuST::day3abbr);
    tk->DefineStringToken("{day4abbr}", (int)eRecMenuST::day4abbr);
    tk->DefineStringToken("{day5abbr}", (int)eRecMenuST::day5abbr);
    tk->DefineStringToken("{day6abbr}", (int)eRecMenuST::day6abbr);
    tk->DefineStringToken("{folder}", (int)eRecMenuST::folder);
    tk->DefineStringToken("{conflictstart}", (int)eRecMenuST::conflictstart);
    tk->DefineStringToken("{conflictstop}", (int)eRecMenuST::conflictstop);
    tk->DefineStringToken("{overlapstart}", (int)eRecMenuST::overlapstart);
    tk->DefineStringToken("{overlapstop}", (int)eRecMenuST::overlapstop);
    tk->DefineStringToken("{timertitle}", (int)eRecMenuST::timertitle);
    tk->DefineStringToken("{starttime}", (int)eRecMenuST::starttime);
    tk->DefineStringToken("{stoptime}", (int)eRecMenuST::stoptime);
    tk->DefineStringToken("{date}", (int)eRecMenuST::date);
    tk->DefineStringToken("{weekday}", (int)eRecMenuST::weekday);
    tk->DefineStringToken("{title}", (int)eRecMenuST::title);
    tk->DefineStringToken("{shorttext}", (int)eRecMenuST::shorttext);
    tk->DefineStringToken("{recname}", (int)eRecMenuST::recname);
    tk->DefineStringToken("{recstarttime}", (int)eRecMenuST::recstarttime);
    tk->DefineStringToken("{recdate}", (int)eRecMenuST::recdate);
    tk->DefineStringToken("{searchstring}", (int)eRecMenuST::searchstring);
    tk->DefineStringToken("{timerstart}", (int)eRecMenuST::timerstart);
    tk->DefineStringToken("{timerstop}", (int)eRecMenuST::timerstop);
    tk->DefineStringToken("{eventtitle}", (int)eRecMenuST::eventtitle);
    tk->DefineStringToken("{eventshorttext}", (int)eRecMenuST::eventshorttext);
    tk->DefineStringToken("{eventstart}", (int)eRecMenuST::eventstart);
    tk->DefineStringToken("{eventstop}", (int)eRecMenuST::eventstop);
    tk->DefineStringToken("{favdesc}", (int)eRecMenuST::favdesc);
}

/********************************************************************
* Protected Functions
********************************************************************/
void cRecMenu::AddMenuItem(cRecMenuItem *item, bool inFront) {
    if (!inFront)
        menuItems.Add(item);
    else
        menuItems.Ins(item);
}

void cRecMenu::AddHeader(cRecMenuItem *header) {
    this->header = header;
    maxMenuHeight -= header->GetHeight();
}

void cRecMenu::AddFooter(cRecMenuItem *footer) {
    this->footer = footer;
    maxMenuHeight -= footer->GetHeight();
}

int cRecMenu::GetNumActive(void) {
    int num = 0;
    for (cRecMenuItem *current = menuItems.First(); current; current = menuItems.Next(current)) {
        if (current == active) {
            return num;
        }
        num++;
    }
    return 0;
}

bool cRecMenu::ScrollUp(bool retry) {
    if (active == start) {
        bool scrolled = SeekBack(false);
        if (scrolled && scrolling) DrawScrollbar();
    }
    if (footer && active == footer) {
        recMenuGrid->SetCurrent(footer->Id(), false);
        footer->SetInactive();
        active = stop;
        active->SetActive();
        return true;
    }
    cRecMenuItem *prev = (cRecMenuItem*)active->Prev();
    while (prev && !prev->Selectable()) {
        prev = (cRecMenuItem*)prev->Prev();
    }
    if (prev) {
        recMenuGrid->SetCurrent(active->Id(), false);
        active->SetInactive();
        active = prev;
        active->SetActive();
        return true;
    } else {
        SeekBack(false);
        if (!retry)
            ScrollUp(true);
    }
    return false;
}

bool cRecMenu::ScrollDown(bool retry) {
    if (active == stop || retry) {
        bool scrolled = SeekForward(false);
        if (scrolled && scrolling) DrawScrollbar();
    }
    cRecMenuItem *next = (cRecMenuItem*)active->Next();
    while (next && !next->Selectable()) {
        if (next == stop) {
            return ScrollDown(true);
        }
        next = (cRecMenuItem*)next->Next();
    }
    if (next) {
        recMenuGrid->SetCurrent(active->Id(), false);
        active->SetInactive();
        active = next;
        active->SetActive();
        return true;
    } else {
        SeekForward(false);
        if (!retry)
            return ScrollDown(true);
    }
    if (footer && active != footer) {
        recMenuGrid->SetCurrent(active->Id(), false);
        active->SetInactive();
        active = footer;
        active->SetActive();
        return true;
    }
    return false;
}

bool cRecMenu::PageUp(void) {
    bool scrolled = SeekBack(true);
    if (scrolled && scrolling) DrawScrollbar();
    if (!scrolled) {
        recMenuGrid->SetCurrent(active->Id(), false);
        active->SetInactive();
        active = start;
        active->SetActive();
        return true;
    }
    if (!active) {
        active = stop;
        active->SetActive();
    }
    return scrolled;
}

bool cRecMenu::PageDown(void) {
    bool scrolled = SeekForward(true);
    if (scrolled && scrolling) DrawScrollbar();
    if (!scrolled) {
        recMenuGrid->SetCurrent(active->Id(), false);
        active->SetInactive();
        active = stop;
        active->SetActive();
        return true;
    }
    if (!active) {
        active = start;
        active->SetActive();
    }
    return scrolled;
}

void cRecMenu::ClearMenuItems(bool deleteItems) {
    if (deleteItems) {
        menuItems.Clear();
        active = NULL;
    } else {
        for (cRecMenuItem *current = menuItems.First(); current; current = menuItems.Next(current)) {
            current->SetNew();
        }
    }
    itemCount = 0;
    back->Clear();
    scrollBar->Clear();
    recMenuGrid->Clear();
    if (header)
        header->SetNew();
    if (footer)
        footer->SetNew();
    if (active)
        active->SetInactive();
    active = NULL;
}

void cRecMenu::InitMenuItems(void) {
    if (menuItems.Count() == 0)
        return;
    scrolling = false;
    menuHeight = 0;
    start = menuItems.First();
    cRecMenuItem *current = start; 
    while (current) {
        int itemHeight = current->GetHeight();
        if (menuHeight + itemHeight > maxMenuHeight) {
            scrolling = true;
            break;
        }
        if (current->Active())
            active = current;
        itemCount++;
        stop = current;
        menuHeight += itemHeight;
        current = menuItems.Next(current);
    }
    DrawBackground();
    Flush();
    if (scrolling) {
        DrawScrollbar();
        Flush();
    }
}

void cRecMenu::InitMenuItemsLast(void) {
    if (menuItems.Count() == 0)
        return;
    scrolling = false;
    menuHeight = 0;
    stop = menuItems.Last();
    active = stop;
    active->SetActive();
    cRecMenuItem *current = stop; 
    while (current) {
        int itemHeight = current->GetHeight();
        if (menuHeight + itemHeight > maxMenuHeight) {
            scrolling = true;
            break;
        }
        itemCount++;
        start = current;
        menuHeight += itemHeight;
        current = menuItems.Prev(current);
    }
    DrawBackground();
    Flush();
    if (scrolling) {
        DrawScrollbar();
        Flush();
    }
}

int cRecMenu::GetHeight(void) {
    int totalHeight = menuHeight;
    if (header)
        totalHeight += header->GetHeight();
    if (footer)
        totalHeight += footer->GetHeight();
    return totalHeight;
}

/********************************************************************
* Private Functions
********************************************************************/

bool cRecMenu::SeekForward(bool page) {
    int jumpStep = 0;
    if (page)
        jumpStep = itemCount;
    else
        jumpStep = itemCount/2;
    int jump = 0;
    cRecMenuItem *next = (cRecMenuItem*)stop->Next();
    while (next && jump < jumpStep) {
        stop = next;
        menuHeight += next->GetHeight();
        next = (cRecMenuItem*)next->Next();
        jump++;
    }
    while (start && menuHeight > maxMenuHeight) {
        if (active == start) {
            active = NULL;
            start->SetInactive();
        }
        menuHeight -= start->GetHeight();
        recMenuGrid->Delete(start->Id());
        start->SetNew();
        start = (cRecMenuItem*)start->Next();
    }
    if (jump > 0)
        return true;
    return false;
}

bool cRecMenu::SeekBack(bool page) {
    int jumpStep = 0;
    if (page)
        jumpStep = itemCount;
    else
        jumpStep = itemCount/2;
    int jump = 0;
    cRecMenuItem *prev = (cRecMenuItem*)start->Prev();
    while (prev && jump < jumpStep) {
        start = prev;
        menuHeight += prev->GetHeight();
        prev = (cRecMenuItem*)prev->Prev();
        jump++;
    }
    while (stop && menuHeight > maxMenuHeight) {
        if (active == stop) {
            active = NULL;
            stop->SetInactive();
        }
        menuHeight -= stop->GetHeight();
        recMenuGrid->Delete(stop->Id());
        stop->SetNew();
        stop = (cRecMenuItem*)stop->Prev();
    }
    if (jump > 0)
        return true;
    return false;
}

void cRecMenu::SetFirst(void) {
    if (!scrolling) {
        recMenuGrid->SetCurrent(active->Id(), false);
        active->SetInactive();
        active = start;
        active->SetActive();
        return;
    }
    ClearMenuItems(false);
    menuItems.First()->SetActive();
    InitMenuItems();
}

void cRecMenu::SetLast(void) {
    if (!scrolling) {
        recMenuGrid->SetCurrent(active->Id(), false);
        active->SetInactive();
        if (footer) {
            active = footer;
        } else {
            active = stop;
        }
        active->SetActive();
        return;
    }
    ClearMenuItems(false);
    InitMenuItemsLast();
}

void cRecMenu::DrawBackground(void) {
    back->Clear();
    back->ClearTokens();
    back->AddIntToken((int)eBackgroundRecMenuIT::menuwidth, menuWidth + 2);
    back->AddIntToken((int)eBackgroundRecMenuIT::menuheight, GetHeight() + 2);
    back->AddIntToken((int)eBackgroundRecMenuIT::hasscrollbar, scrolling);
    back->Display();
}

void cRecMenu::DrawScrollbar(void) {
    if (menuItems.Count() == 0)
        return;
    int scrollBarHeight = (double)itemCount / (double)menuItems.Count() * 1000;
    int startPos = 0;
    for (cRecMenuItem *current = menuItems.First(); current; current = menuItems.Next(current)) {
        if (start == current)
            break;
        startPos++;
    }
    int offset = (double)startPos / (double)menuItems.Count() * 1000;
    scrollBar->Clear();
    scrollBar->ClearTokens();
    scrollBar->AddIntToken((int)eScrollbarRecMenuIT::menuwidth, menuWidth + 2);
    int y = (100 - GetHeight())/2;
    if (header)
        y += header->GetHeight();

    scrollBar->AddIntToken((int)eScrollbarRecMenuIT::posy, y);
    scrollBar->AddIntToken((int)eScrollbarRecMenuIT::totalheight, menuHeight);
    scrollBar->AddIntToken((int)eScrollbarRecMenuIT::height, scrollBarHeight);
    scrollBar->AddIntToken((int)eScrollbarRecMenuIT::offset, offset);
    scrollBar->Display();
}

void cRecMenu::DrawHeader(void) {
    if (!header)
        return;
    double width = (double)menuWidth / (double)100;
    double x = (double)(100 - menuWidth)/(double)200;
    int totalHeight = GetHeight();
    double y = (double)((100 - totalHeight) / 2) / (double)100;

    if (header->IsNew()) {
        recMenuGrid->ClearTokens();
        header->SetTokens(recMenuGrid);
        recMenuGrid->SetGrid(header->Id(), x, y, width, (double)header->GetHeight()/(double)100);
    }
}

void cRecMenu::DrawFooter(void) {
    if (!footer)
        return;
    double width = (double)menuWidth / (double)100;
    double x = (double)(100 - menuWidth)/(double)200;
    int totalHeight = GetHeight();
    int totalMenuHeight = menuHeight;
    if (header)
        totalMenuHeight += header->GetHeight();
    double y = (double)((100 - totalHeight) / 2 + totalMenuHeight) / (double)100;
    
    if (footer->IsNew()) {
        recMenuGrid->ClearTokens();
        footer->SetTokens(recMenuGrid);
        recMenuGrid->SetGrid(footer->Id(), x, y, width, (double)footer->GetHeight()/(double)100);
    } else {
        recMenuGrid->MoveGrid(footer->Id(), x, y, width, (double)footer->GetHeight()/(double)100);
    }
    if (footer->Active()) {
        active = footer;
        recMenuGrid->SetCurrent(footer->Id(), true);
    }
}