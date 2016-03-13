#include "tvguidengosd.h"

cTVGuideOSD::cTVGuideOSD(skindesignerapi::cPluginStructure *plugStruct) : cSkindesignerOsdObject(plugStruct) {
    timeManager = NULL;
    epgGrid = NULL;
    channelJumper = NULL;
    detailView = NULL;
    recMenuView = NULL;
}

cTVGuideOSD::~cTVGuideOSD(void) {
    esyslog("tvguideng: closing");
    if (recMenuView && recMenuView->Active()) {
        recMenuView->Close();
    }
    if (timeManager)
        delete timeManager;
    if (epgGrid)
        delete epgGrid;
    if (channelJumper)
        delete channelJumper;
    if (detailView)
        delete detailView;
    if (recMenuView)
        delete recMenuView;
}

void cTVGuideOSD::Show(void) {
    if (!SkindesignerAvailable()) {
        esyslog("tvguideng: skindesigner not available but mandatorily needed - aborting");
        return;        
    }
    skindesignerapi::cOsdView *rootView = GetOsdView();
    if (!rootView) {
        esyslog("tvguideng: used skindesigner skin does not support tvguideng");
        return;
    }

    SwitchTimers.Load(AddDirectory(cPlugin::ConfigDirectory("epgsearch"), "epgsearchswitchtimers.conf"));
    recMenuView = new cRecMenuView();
    pRemoteTimers = cPluginManager::GetPlugin("remotetimers");
    if (pRemoteTimers) {
        isyslog("tvguideng: remotetimers-plugin is available");
    }
    if (config.useRemoteTimers && pRemoteTimers) {
        cString errorMsg;
        if (!pRemoteTimers->Service("RemoteTimers::RefreshTimers-v1.0", &errorMsg)) {
            esyslog("tvguideng: Remotetimers Error: %s", *errorMsg);
        }
    }

    timeManager = new cTimeManager();
    timeManager->Now();

    epgGrid = new cEpgGrid(rootView, timeManager);
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannel *startChannel = Channels->GetByNumber(cDevice::CurrentChannel());
#else
    const cChannel *startChannel = Channels.GetByNumber(cDevice::CurrentChannel());
#endif
    epgGrid->Init(startChannel);
    epgGrid->Activate();
    epgGrid->DrawHeader();
    epgGrid->DrawTime();
    epgGrid->DrawFooter();
    epgGrid->DrawChannelHeaders();
    epgGrid->DrawTimeline();
    epgGrid->DrawChannelgroups();
    epgGrid->DrawGrid();
    epgGrid->Flush();
}

eOSState cTVGuideOSD::ProcessKey(eKeys Key) {
    if (!epgGrid) {
        return osEnd;
    }

    eOSState state = osContinue;

    if (recMenuView->Active() && !detailView) {
        state = recMenuView->ProcessKey(Key);
        if (state == osEnd) {
            epgGrid->SetTimers();
            epgGrid->Activate();
            epgGrid->DrawGrid();
            epgGrid->Flush();
        } else if (state == osUser1) {
            DetailView(recMenuView->GetEvent());
        }
        state = osContinue;
        if (epgGrid->DrawTime())
            epgGrid->Flush();
    } else if (detailView) {
        //Detail View Key Handling
        switch (Key & ~k_Repeat) {
            case kBack:     
                CloseDetailedView();
                break;
            case kUp:
                detailView->Up();
                detailView->Flush();
                break;
            case kDown:
                detailView->Down();
                detailView->Flush();
                break;
            case kLeft:
                detailView->Left();
                detailView->Flush();
                break;
            case kRight:
                detailView->Right();
                detailView->Flush();
                break;
            case kOk:
                CloseDetailedView();
                break;
            case kBlue:
                state = ChannelSwitch(detailView->GetEvent());
                break;
            case kRed:
                CloseDetailedView();
                state = osContinue; 
                KeyRed();
                break;
            default:
                break;
        }
        if (detailView && state != osEnd && detailView->DrawTime()) {
            detailView->Flush();
        }
    } else {
        //EPG Grid is active
        switch (Key & ~k_Repeat) {
            case kBack:     
                state=osEnd; 
                break;
            case kUp:
                KeyUp();
                break;
            case kDown:
                KeyDown();
                break;
            case kLeft:
                KeyLeft();
                break;
            case kRight:
                KeyRight();
                break;
            case kOk:
                state = KeyOk(epgGrid->GetCurrentEvent());
                break;
            case k0 ... k9:
                NumericKey(Key - k0);
                break;
            case kRed:
                KeyRed();
                break;
            case kGreen:
                KeyGreen();
                break;
            case kYellow:
                KeyYellow();
                break;
            case kBlue:
                state = KeyBlue(epgGrid->GetCurrentEvent());
                break;
            case kNone:
                if (channelJumper) CheckTimeout();
                break;          
            default:        
                break;
        }
        if (state != osEnd && epgGrid->DrawTime())
            epgGrid->Flush();
    }
    return state;
}

void cTVGuideOSD::KeyLeft(void) {
    if (config.displayMode == eHorizontal) {
        TimeBack();
    } else if (config.displayMode == eVertical) {
        ChannelsBack();
    }
}

void cTVGuideOSD::KeyRight(void) {
    if (config.displayMode == eHorizontal) {
        TimeForward();
    } else if (config.displayMode == eVertical) {
        ChannelsForward();
    }
}

void cTVGuideOSD::KeyUp(void) {
    if (config.displayMode == eHorizontal) {
        ChannelsBack();
    } else if (config.displayMode == eVertical) {
        TimeBack();
    }
}

void cTVGuideOSD::KeyDown(void) {
    if (config.displayMode == eHorizontal) {
        ChannelsForward();
    } else if (config.displayMode == eVertical) {
        TimeForward();
    }
}

void cTVGuideOSD::TimeForward(void) {
    bool scrolled = epgGrid->TimeForward();
    if (!scrolled) {
        epgGrid->UpdateActive();
    } else {
        epgGrid->DrawGrid();
    }
    epgGrid->DrawHeader();
    epgGrid->Flush();
}

void cTVGuideOSD::TimeBack(void) {
    bool scrolled = epgGrid->TimeBack();
    if (!scrolled) {
        epgGrid->UpdateActive();
    } else {
        epgGrid->DrawGrid();
    }
    epgGrid->DrawHeader();
    epgGrid->Flush();
}

void cTVGuideOSD::ChannelsForward(void) {
    bool scrolled = epgGrid->ChannelForward();
    if (!scrolled) {
        epgGrid->UpdateActive();
    } else {
        epgGrid->DrawChannelHeaders();
        epgGrid->DrawChannelgroups();
        epgGrid->DrawGrid();
    }
    if (config.channelJumpMode == eGroupJump)
        epgGrid->DrawFooter();
    epgGrid->DrawHeader();
    epgGrid->Flush();
}

void cTVGuideOSD::ChannelsBack(void) {
    bool scrolled = epgGrid->ChannelBack();
    if (!scrolled) {
        epgGrid->UpdateActive();
    } else {
        epgGrid->DrawChannelHeaders();
        epgGrid->DrawChannelgroups();
        epgGrid->DrawGrid();
    }
    if (config.channelJumpMode == eGroupJump)
        epgGrid->DrawFooter();
    epgGrid->DrawHeader();
    epgGrid->Flush();
}

void cTVGuideOSD::NumericKey(int key) {
    if (config.numKeyMode == eTimeJump) {
        if (key == 2) {
            CreateInstantTimer();
        } else if (key == 5) {
            DisplaySearchRecordings();
        } else if (key == 8) {
            DisplaySearchEPG();
        } else {
            NumericKeyTimeJump(key);
        }
    } else if (config.numKeyMode == eChannelJump) {
        NumericKeyChannelJump(key);
    }
}

void cTVGuideOSD::NumericKeyTimeJump(int key) {
    switch (key) {
        case 1: {
            bool tooFarInPast = timeManager->DelMinutes(config.bigStepHours*60);
            if (tooFarInPast)
                return;
            }
            break;
        case 3: {
            timeManager->AddMinutes(config.bigStepHours*60);
        }
            break;
        case 4: {
            bool tooFarInPast = timeManager->DelMinutes(config.hugeStepHours*60);
            if (tooFarInPast)
                return;
            }
            break;
        case 6: {
            timeManager->AddMinutes(config.hugeStepHours*60);
            }
            break;
        case 7: {
            cTimeManager primeChecker;
            primeChecker.Now();
            time_t prevPrime = primeChecker.GetPrevPrimetime(timeManager->GetStart());
            if (primeChecker.TooFarInPast(prevPrime))
                return;
            timeManager->SetTime(prevPrime);
        }
            break;
        case 9: {
            cTimeManager primeChecker;
            time_t nextPrime = primeChecker.GetNextPrimetime(timeManager->GetStart());
            timeManager->SetTime(nextPrime);
        }
            break;
        default:
            return;
    }
    epgGrid->RebuildEpgGrid();
    epgGrid->DrawGrid();
    epgGrid->DrawHeader();
    epgGrid->DrawTimeline();
    epgGrid->Flush();
}

void cTVGuideOSD::NumericKeyChannelJump(int key) {
    if (!channelJumper) {
        channelJumper = epgGrid->GetChannelJumper();
    }
    channelJumper->Set(key);
    channelJumper->Draw();
    epgGrid->Flush();
}

void cTVGuideOSD::CheckTimeout(void) {
    if (!channelJumper)
        return;
    if (!channelJumper->TimeOut())
        return;
    int newChannelNum = channelJumper->GetChannel(); 
    delete channelJumper;
    channelJumper = NULL;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
   LOCK_CHANNELS_READ;
   const cChannels* channels = Channels;
#else
   cChannels* channels = &Channels;
#endif
    const cChannel *newChannel = channels->GetByNumber(newChannelNum);
    if (!newChannel) {
        return;
    }
    epgGrid->Clear();
    epgGrid->Init(newChannel);
    epgGrid->DrawHeader();
    epgGrid->DrawTime();
    epgGrid->DrawFooter();
    epgGrid->DrawChannelHeaders();
    epgGrid->DrawTimeline();
    epgGrid->DrawChannelgroups();
    epgGrid->DrawGrid();
    epgGrid->Flush();
}

void cTVGuideOSD::KeyGreen(void) {
    const cChannel *prevStart = NULL;
    if (config.channelJumpMode == eNumJump) {
        prevStart = epgGrid->GetPrevChannelNumJump();
    } else if (config.channelJumpMode == eGroupJump) {
        if (epgGrid->IsFirstGroup()) {
            return;
        }
        prevStart = epgGrid->GetPrevChannelGroupJump();
    }
    epgGrid->Clear();
    epgGrid->Init(prevStart);
    epgGrid->DrawHeader();
    epgGrid->DrawTime();
    epgGrid->DrawFooter();
    epgGrid->DrawChannelHeaders();
    epgGrid->DrawChannelgroups();
    if (config.channelJumpMode == eGroupJump)
        epgGrid->DrawFooter();
    epgGrid->DrawTimeline();
    epgGrid->DrawGrid();
    epgGrid->Flush();
}

void cTVGuideOSD::KeyYellow(void) {
    const cChannel *nextStart = NULL;
    if (config.channelJumpMode == eNumJump) {
        nextStart = epgGrid->GetNextChannelNumJump();
    } else if (config.channelJumpMode == eGroupJump) {
        if (epgGrid->IsLastGroup()) {
            return;
        }
        if (config.hideLastChannelGroup && epgGrid->IsSecondLastGroup()) {
            return;
        }
        nextStart = epgGrid->GetNextChannelGroupJump();        
    }
    epgGrid->Clear();
    epgGrid->Init(nextStart);
    epgGrid->DrawHeader();
    epgGrid->DrawTime();
    epgGrid->DrawFooter();
    epgGrid->DrawChannelHeaders();
    epgGrid->DrawChannelgroups();
        if (config.channelJumpMode == eGroupJump)
            epgGrid->DrawFooter();
    epgGrid->DrawTimeline();
    epgGrid->DrawGrid();
    epgGrid->Flush();
}

eOSState cTVGuideOSD::KeyBlue(const cEvent *e) {
    if (config.blueKeyMode == eBlueKeySwitch) {
        return ChannelSwitch(e);
    } else if (config.blueKeyMode == eBlueKeyEPG) {
        DetailView(e);
    } else if (config.blueKeyMode == eBlueKeyFavorites) {
        Favorites();
    }
    return osContinue;
}

eOSState cTVGuideOSD::KeyOk(const cEvent *e) {
    if (config.blueKeyMode == eBlueKeySwitch) {
        DetailView(e);
    } else if (config.blueKeyMode == eBlueKeyEPG) {
        return ChannelSwitch(e);
    } else if (config.blueKeyMode == eBlueKeyFavorites) {
        DetailView(e);
    }
    return osContinue;
}

eOSState cTVGuideOSD::ChannelSwitch(const cEvent *e) {
    bool running = false;
    if (e) {
        time_t now = time(0);
        if (((e->StartTime() - 5*60) <= now) && (e->EndTime() >= now))
            running = true;
    }
    if (running || !config.intelligentSwitch) {
        const cChannel *currentChannel = epgGrid->GetCurrentChannel();
        if (!currentChannel) {
            return osContinue;
        }
        cDevice::PrimaryDevice()->SwitchChannel(currentChannel, true);
        if (config.closeOnSwitch) {
            return osEnd;
        }
    } else {
        CreateSwitchTimer(e);
        epgGrid->SetTimers();
        epgGrid->DrawGrid();
        epgGrid->Flush();
    }
    return osContinue;    
}

void cTVGuideOSD::CreateSwitchTimer(const cEvent *e) {
    if (!e)
        return;
    if (SwitchTimers.EventInSwitchList(e))
        return;
    cSwitchTimer st;
    cRecManager recManager;
    recManager.SetEPGSearchPlugin();
    recManager.CreateSwitchTimer(e, st);
}

void cTVGuideOSD::DetailView(const cEvent *e) {
    if (!e)
        return;
    epgGrid->Deactivate(true);
    if (recMenuView->Active())
        recMenuView->Hide(true);
    skindesignerapi::cOsdView *dV = GetOsdView((int)eViews::detailView);
    detailView = new cDetailView(dV, e);
    detailView->Activate();
    detailView->Draw();
    detailView->DrawTime();
    detailView->Flush();
}

void cTVGuideOSD::CloseDetailedView(void) {
    delete detailView;
    detailView = NULL;
    epgGrid->Activate();
    epgGrid->Flush();
    if (recMenuView->Active()) {
        recMenuView->Activate(true);
        recMenuView->Flush();        
    }
}

void cTVGuideOSD::KeyRed(void) {
    const cEvent *e = epgGrid->GetCurrentEvent();
    if (!e)
        return;
    epgGrid->Deactivate(false);
    skindesignerapi::cOsdView *recView = GetOsdView((int)eViews::recMenu);
    skindesignerapi::cOsdView *recViewBuffer = GetOsdView((int)eViews::recMenu2);
    skindesignerapi::cOsdView *recViewBuffer2 = GetOsdView((int)eViews::recMenu3);
    recMenuView->Init(recView, recViewBuffer, recViewBuffer2);
    recMenuView->Activate();
    recMenuView->DisplayRecMenu(e);
    recMenuView->Flush();
}

void cTVGuideOSD::Favorites(void) {
    epgGrid->Deactivate(false);
    skindesignerapi::cOsdView *recView = GetOsdView((int)eViews::recMenu);
    skindesignerapi::cOsdView *recViewBuffer = GetOsdView((int)eViews::recMenu2);
    skindesignerapi::cOsdView *recViewBuffer2 = GetOsdView((int)eViews::recMenu3);
    recMenuView->Init(recView, recViewBuffer, recViewBuffer2);
    recMenuView->Activate();
    recMenuView->DisplayFavorites();
    recMenuView->Flush();
}

void cTVGuideOSD::CreateInstantTimer(void) {
    const cEvent *e = epgGrid->GetCurrentEvent();
    if (!e)
        return;
    cRecManager recManager;
    recManager.SetEPGSearchPlugin();
    if (!e->HasTimer())
        recManager.createTimer(e, "");
    else
        recManager.DeleteTimer(e);
    epgGrid->SetTimers();
    epgGrid->DrawGrid();
    epgGrid->Flush();
}

void cTVGuideOSD::DisplaySearchRecordings(void) {
    const cEvent *e = epgGrid->GetCurrentEvent();
    if (!e)
        return;
    epgGrid->Deactivate(false);
    skindesignerapi::cOsdView *recView = GetOsdView((int)eViews::recMenu);
    skindesignerapi::cOsdView *recViewBuffer = GetOsdView((int)eViews::recMenu2);
    skindesignerapi::cOsdView *recViewBuffer2 = GetOsdView((int)eViews::recMenu3);
    recMenuView->Init(recView, recViewBuffer, recViewBuffer2);
    recMenuView->Activate();
    recMenuView->DisplayRecSearch(e);
    recMenuView->Flush();
}

void cTVGuideOSD::DisplaySearchEPG(void) {
    const cEvent *e = epgGrid->GetCurrentEvent();
    if (!e)
        return;
    epgGrid->Deactivate(false);
    skindesignerapi::cOsdView *recView = GetOsdView((int)eViews::recMenu);
    skindesignerapi::cOsdView *recViewBuffer = GetOsdView((int)eViews::recMenu2);
    skindesignerapi::cOsdView *recViewBuffer2 = GetOsdView((int)eViews::recMenu3);
    recMenuView->Init(recView, recViewBuffer, recViewBuffer2);
    recMenuView->Activate();
    recMenuView->DisplaySearchEPG(e);
    recMenuView->Flush();
}

void cTVGuideOSD::DefineTokens(eViewElementsRoot ve, skindesignerapi::cTokenContainer *tk) {
    switch (ve) {
        case eViewElementsRoot::backgroundHor:
        case eViewElementsRoot::backgroundVer:
            break;
        case eViewElementsRoot::headerHor:
        case eViewElementsRoot::headerVer:
            tk->DefineIntToken("{isdummy}", (int)eHeaderIT::isdummy);
            tk->DefineIntToken("{daynumeric}", (int)eHeaderIT::daynumeric);
            tk->DefineIntToken("{month}", (int)eHeaderIT::month);
            tk->DefineIntToken("{year}", (int)eHeaderIT::year);
            tk->DefineIntToken("{running}", (int)eHeaderIT::running);
            tk->DefineIntToken("{elapsed}", (int)eHeaderIT::elapsed);
            tk->DefineIntToken("{duration}", (int)eHeaderIT::duration);
            tk->DefineIntToken("{durationhours}", (int)eHeaderIT::durationhours);
            tk->DefineIntToken("{channelnumber}", (int)eHeaderIT::channelnumber);
            tk->DefineIntToken("{channellogoexists}", (int)eHeaderIT::isdummy);
            tk->DefineIntToken("{hasposter}", (int)eHeaderIT::hasposter);
            tk->DefineIntToken("{posterwidth}", (int)eHeaderIT::posterwidth);
            tk->DefineIntToken("{posterheight}", (int)eHeaderIT::posterheight);
            tk->DefineStringToken("{title}", (int)eHeaderST::title);
            tk->DefineStringToken("{shorttext}", (int)eHeaderST::shorttext);
            tk->DefineStringToken("{description}", (int)eHeaderST::description);
            tk->DefineStringToken("{start}", (int)eHeaderST::start);
            tk->DefineStringToken("{stop}", (int)eHeaderST::stop);
            tk->DefineStringToken("{day}", (int)eHeaderST::day);
            tk->DefineStringToken("{date}", (int)eHeaderST::date);
            tk->DefineStringToken("{durationminutes}", (int)eHeaderST::durationminutes);
            tk->DefineStringToken("{channelname}", (int)eHeaderST::channelname);
            tk->DefineStringToken("{channelid}", (int)eHeaderST::channelid);
            tk->DefineStringToken("{posterpath}", (int)eHeaderST::posterpath);
            break;
        case eViewElementsRoot::footerHor:
        case eViewElementsRoot::footerVer:
            tk->DefineIntToken("{red1}", (int)eFooterIT::red1);
            tk->DefineIntToken("{red2}", (int)eFooterIT::red2);
            tk->DefineIntToken("{red3}", (int)eFooterIT::red3);
            tk->DefineIntToken("{red4}", (int)eFooterIT::red4);
            tk->DefineIntToken("{green1}", (int)eFooterIT::green1);
            tk->DefineIntToken("{green2}", (int)eFooterIT::green2);
            tk->DefineIntToken("{green3}", (int)eFooterIT::green3);
            tk->DefineIntToken("{green4}", (int)eFooterIT::green4);
            tk->DefineIntToken("{yellow1}", (int)eFooterIT::yellow1);
            tk->DefineIntToken("{yellow2}", (int)eFooterIT::yellow2);
            tk->DefineIntToken("{yellow3}", (int)eFooterIT::yellow3);
            tk->DefineIntToken("{yellow4}", (int)eFooterIT::yellow4);
            tk->DefineIntToken("{blue1}", (int)eFooterIT::blue1);
            tk->DefineIntToken("{blue2}", (int)eFooterIT::blue2);
            tk->DefineIntToken("{blue3}", (int)eFooterIT::blue3);
            tk->DefineIntToken("{blue4}", (int)eFooterIT::blue4);
            tk->DefineStringToken("{red}", (int)eFooterST::red);
            tk->DefineStringToken("{green}", (int)eFooterST::green);
            tk->DefineStringToken("{yellow}", (int)eFooterST::yellow);
            tk->DefineStringToken("{blue}", (int)eFooterST::blue);
            break;
        case eViewElementsRoot::timeHor:
        case eViewElementsRoot::timeVer:
            tk->DefineIntToken("{sec}", (int)eTimeIT::sec);
            tk->DefineIntToken("{min}", (int)eTimeIT::min);
            tk->DefineIntToken("{hour}", (int)eTimeIT::hour);
            tk->DefineIntToken("{hmins}", (int)eTimeIT::hmins);
            tk->DefineIntToken("{year}", (int)eTimeIT::year);
            tk->DefineIntToken("{day}", (int)eTimeIT::day);
            tk->DefineStringToken("{time}", (int)eTimeST::time);
            tk->DefineStringToken("{monthname}", (int)eTimeST::monthname);
            tk->DefineStringToken("{monthnameshort}", (int)eTimeST::monthnameshort);
            tk->DefineStringToken("{month}", (int)eTimeST::month);
            tk->DefineStringToken("{dayleadingzero}", (int)eTimeST::dayleadingzero);
            tk->DefineStringToken("{dayname}", (int)eTimeST::dayname);
            tk->DefineStringToken("{daynameshort}", (int)eTimeST::daynameshort);
            break;
        case eViewElementsRoot::dateTimelineHor:
        case eViewElementsRoot::dateTimelineVer:
            tk->DefineStringToken("{weekday}", (int)eDateTimeST::weekday);
            tk->DefineStringToken("{date}", (int)eDateTimeST::date);
            break;
        case eViewElementsRoot::timeIndicatorHor:
        case eViewElementsRoot::timeIndicatorVer:
            tk->DefineIntToken("{percenttotal}", (int)eTimeIndicatorIT::percenttotal);
            break;
        case eViewElementsRoot::channelJump:
            tk->DefineStringToken("{channel}", (int)eChannelJumpST::channel);
            break;
        default:
            break;
    }
}

void cTVGuideOSD::DefineTokens(eViewGridsRoot vg, skindesignerapi::cTokenContainer *tk) {
    switch (vg) {
        case eViewGridsRoot::channelsHor:
        case eViewGridsRoot::channelsVer:
            tk->DefineIntToken("{number}", (int)eChannelGridIT::number);
            tk->DefineIntToken("{channellogoexists}", (int)eChannelGridIT::channellogoexists);
            tk->DefineStringToken("{name}", (int)eChannelGridST::name);
            tk->DefineStringToken("{channelid}", (int)eChannelGridST::channelid);
            break;
        case eViewGridsRoot::schedulesHor:
        case eViewGridsRoot::schedulesVer:
            tk->DefineIntToken("{color}", (int)eSchedulesGridIT::color);
            tk->DefineIntToken("{dummy}", (int)eSchedulesGridIT::dummy);
            tk->DefineIntToken("{timer}", (int)eSchedulesGridIT::timer);
            tk->DefineIntToken("{switchtimer}", (int)eSchedulesGridIT::switchtimer);
            tk->DefineStringToken("{title}", (int)eSchedulesGridST::title);
            tk->DefineStringToken("{shorttext}", (int)eSchedulesGridST::shorttext);
            tk->DefineStringToken("{start}", (int)eSchedulesGridST::start);
            tk->DefineStringToken("{stop}", (int)eSchedulesGridST::stop);
            break;
        case eViewGridsRoot::channelGroupsHor:
        case eViewGridsRoot::channelGroupsVer:
            tk->DefineIntToken("{color}", (int)eChannelgroupsGridIT::color);
            tk->DefineStringToken("{group}", (int)eChannelgroupsGridST::group);
            break;
        case eViewGridsRoot::timelineHor:
        case eViewGridsRoot::timelineVer:
            tk->DefineStringToken("{timestring}", (int)eTimelineGridST::timestring);
            tk->DefineIntToken("{fullhour}", (int)eTimelineGridIT::fullhour);
            break;
        default:
            break;
    }
}
