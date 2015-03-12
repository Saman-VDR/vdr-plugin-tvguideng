#include "tvguidengosd.h"

cTVGuideOSD::cTVGuideOSD(void) {
    timeManager = NULL;
    epgGrid = NULL;
    channelJumper = NULL;
    detailView = NULL;
    recMenuView = NULL;
}

cTVGuideOSD::~cTVGuideOSD(void) {
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
    bool skinDesignerAvailable = InitSkindesignerInterface("tvguideng");
    if (!skinDesignerAvailable) {
        return;
        esyslog("tvguideng: skindesigner not available");
    }

    cOsdView *rootView = GetOsdView(viRootView);
    if (!rootView) {
        esyslog("tvguideng: used skindesigner skin does not support tvguideng");
        return;
    }
    SwitchTimers.Load(AddDirectory(cPlugin::ConfigDirectory("epgsearch"), "epgsearchswitchtimers.conf"));
    recMenuView = new cRecMenuView();
    pRemoteTimers = cPluginManager::CallFirstService("RemoteTimers::RefreshTimers-v1.0", NULL);
    if (pRemoteTimers) {
        isyslog("tvguideng: remotetimers-plugin is available");
    }
    if (config.useRemoteTimers && pRemoteTimers) {
        cString errorMsg;
        if (!pRemoteTimers->Service("RemoteTimers::RefreshTimers-v1.0", &errorMsg)) {
            esyslog("tvguideng: %s", *errorMsg);
        }
    }

    timeManager = new cTimeManager();
    timeManager->Now();

    epgGrid = new cEpgGrid(rootView, timeManager);

    cChannel *startChannel = Channels.GetByNumber(cDevice::CurrentChannel());
    
    epgGrid->Init(startChannel);
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
                state = ChannelSwitch();
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
        NumericKeyTimeJump(key);
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
    const cChannel *newChannel = Channels.GetByNumber(newChannelNum);
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
        return ChannelSwitch();
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
        return ChannelSwitch();
    } else if (config.blueKeyMode == eBlueKeyFavorites) {
        DetailView(e);
    }
    return osContinue;
}

eOSState cTVGuideOSD::ChannelSwitch(void) {
    const cChannel *currentChannel = epgGrid->GetCurrentChannel();
    if (!currentChannel) {
        return osContinue;
    }
    cDevice::PrimaryDevice()->SwitchChannel(currentChannel, true);
    if (config.closeOnSwitch) {
        return osEnd;
    }
    return osContinue;    
}

void cTVGuideOSD::DetailView(const cEvent *e) {
    if (!e)
        return;
    epgGrid->Deactivate(true);
    if (recMenuView->Active())
        recMenuView->Hide();
    cOsdView *dV = GetOsdView(viRootView, viDetailView);
    detailView = new cDetailView(dV, e);
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
        recMenuView->Activate();
        recMenuView->Flush();        
    }
}

void cTVGuideOSD::KeyRed(void) {
    const cEvent *e = epgGrid->GetCurrentEvent();
    if (!e)
        return;
    epgGrid->Deactivate(false);
    cOsdView *recView = GetOsdView(viRootView, viRecMenu);
    cOsdView *recViewBuffer = GetOsdView(viRootView, viRecMenu);
    cOsdView *recViewBuffer2 = GetOsdView(viRootView, viRecMenu);
    recMenuView->Init(recView, recViewBuffer, recViewBuffer2);
    recMenuView->DisplayRecMenu(e);
    recMenuView->Flush();
}

void cTVGuideOSD::Favorites(void) {
    epgGrid->Deactivate(false);
    cOsdView *recView = GetOsdView(viRootView, viRecMenu);
    cOsdView *recViewBuffer = GetOsdView(viRootView, viRecMenu);
    cOsdView *recViewBuffer2 = GetOsdView(viRootView, viRecMenu);
    recMenuView->Init(recView, recViewBuffer, recViewBuffer2);
    recMenuView->DisplayFavorites();
    recMenuView->Flush();
}
