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
    recMenuGrid = osdView->GetViewGrid(vgRecordingMenu);
    scrollBar = osdView->GetViewElement(vemScrollbar);
    back = osdView->GetViewElement(vemBackground);
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
    for (cRecMenuItem *current = start; current; current = menuItems.Next(current)) {
        if (current == active)
            return num;
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
    back->AddIntToken("menuwidth", menuWidth + 2);
    back->AddIntToken("menuheight", GetHeight() + 2);
    back->AddIntToken("hasscrollbar", scrolling);
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
    scrollBar->AddIntToken("menuwidth", menuWidth + 2);
    int y = (100 - GetHeight())/2;
    if (header)
        y += header->GetHeight();

    scrollBar->AddIntToken("posy", y);
    scrollBar->AddIntToken("totalheight", menuHeight);
    scrollBar->AddIntToken("height", scrollBarHeight);
    scrollBar->AddIntToken("offset", offset);
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