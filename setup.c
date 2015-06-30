#include "setup.h"

cTvGuideSetup::cTvGuideSetup() {
    tmpConfig = config;
    displayModeItems[0] = "vertical";
    displayModeItems[1] = "horizontal";
    jumpMode[0] = tr("x channels back / forward");
    jumpMode[1] = tr("previous / next channel group");
    numMode[0] = tr("Timely Jump");
    numMode[1] = tr("Jump to specific channel");
    blueMode[0] = tr("Blue: Channel Switch, Ok: Detailed EPG");
    blueMode[1] = tr("Blue: Detailed EPG, Ok: Channel Switch");
    blueMode[2] = tr("Blue: Favorites / Switch, Ok: Detailed EPG");
    strn0cpy(description1, tmpConfig.descUser1.c_str(), sizeof(description1));
    strn0cpy(description2, tmpConfig.descUser2.c_str(), sizeof(description2));
    strn0cpy(description3, tmpConfig.descUser3.c_str(), sizeof(description3));
    strn0cpy(description4, tmpConfig.descUser4.c_str(), sizeof(description4));
    recFolderMode[0] = tr("Always use root video folder");
    recFolderMode[1] = tr("Select from folder list");
    recFolderMode[2] = tr("Use fixed folder");
    strn0cpy(fixedFolder, tmpConfig.instRecFixedFolder.c_str(), sizeof(fixedFolder));
    useSubtitleRerunTexts[0] = tr("never");
    useSubtitleRerunTexts[1] = tr("if exists");
    useSubtitleRerunTexts[2] = tr("always");
    Setup();
}

cTvGuideSetup::~cTvGuideSetup() {
}

void cTvGuideSetup::Setup(void) {
    int current = Current();
    Clear();

    cString indent = "    ";

    Add(new cMenuEditBoolItem(tr("Show Main Menu Entry"), &tmpConfig.showMainMenuEntry));
    Add(new cMenuEditBoolItem(tr("Replace VDR Schedules Menu"), &tmpConfig.replaceOriginalSchedule));
    Add(new cMenuEditStraItem(tr("Display Mode"), &tmpConfig.displayMode, 2,  displayModeItems));
    Add(new cMenuEditIntItem(tr("Channels to display in horizontal View"), &tmpConfig.channelsPerPageHorizontal, 3, 12));
    Add(new cMenuEditIntItem(tr("Channels to display in vertical View"), &tmpConfig.channelsPerPageVertical, 3, 12));
    Add(new cMenuEditIntItem(tr("Number of Hours to display"), &tmpConfig.displayHours, 3, 12));
    Add(new cMenuEditIntItem(tr("Big Step (Keys 1 / 3) in hours"), &tmpConfig.bigStepHours, 1, 12));
    Add(new cMenuEditIntItem(tr("Huge Step (Keys 4 / 6) in hours"), &tmpConfig.hugeStepHours, 13, 48));
    Add(new cMenuEditBoolItem(tr("Hide last Channel Group"), &tmpConfig.hideLastChannelGroup));
    Add(new cMenuEditStraItem(tr("Channel Jump Mode (Keys Green / Yellow)"), &tmpConfig.channelJumpMode, 2,  jumpMode));
    Add(new cMenuEditBoolItem(tr("Close TVGuide after channel switch"), &tmpConfig.closeOnSwitch));
    Add(new cMenuEditStraItem(tr("Functionality of numeric Keys"), &tmpConfig.numKeyMode, 2,  numMode));
    Add(new cMenuEditStraItem(tr("Keys Blue and OK"), &tmpConfig.blueKeyMode, 3,  blueMode));
    Add(new cMenuEditBoolItem(tr("\"Intelligent\" channel switch mode"), &tmpConfig.intelligentSwitch));
    Add(new cMenuEditIntItem(tr("Maximum number of reruns to display"), &tmpConfig.rerunAmount, 1, 100));
    Add(new cMenuEditIntItem(tr("Minimum timely distance of rerun (in hours)"), &tmpConfig.rerunDistance, 0, 1000));
    Add(new cMenuEditIntItem(tr("Limit Channel Numbers for reruns"), &tmpConfig.rerunMaxChannel, 0, 1000, tr("no limit")));

    Add(new cMenuEditStraItem(tr("Folder for instant Recordings"), &tmpConfig.instRecFolderMode, 3,  recFolderMode));
    if (tmpConfig.instRecFolderMode == eFolderFixed) {
        Add(new cMenuEditStrItem(cString::sprintf("%s%s", *indent, tr("Folder")), fixedFolder, sizeof(fixedFolder), trVDR(FileNameChars)));
    }
    if (pRemoteTimers)
        Add(new cMenuEditBoolItem(tr("Use Remotetimers"), &tmpConfig.useRemoteTimers));

    Add(new cMenuEditBoolItem(tr("Use \"What's on now\" in favorites"), &tmpConfig.favWhatsOnNow));
    Add(new cMenuEditBoolItem(tr("Use \"What's on next\" in favorites"), &tmpConfig.favWhatsOnNext));
    Add(new cMenuEditBoolItem(tr("Use user defined time 1 in favorites"), &tmpConfig.favUseTime1));
    if (tmpConfig.favUseTime1) {
        Add(new cMenuEditStrItem(cString::sprintf("%s%s", *indent, tr("Description")), description1, sizeof(description1), trVDR(FileNameChars)));
        Add(new cMenuEditTimeItem(cString::sprintf("%s%s", *indent, tr("Time")), &tmpConfig.favTime1));
    }
    Add(new cMenuEditBoolItem(tr("Use user defined time 2 in favorites"), &tmpConfig.favUseTime2));
    if (tmpConfig.favUseTime2) {
        Add(new cMenuEditStrItem(cString::sprintf("%s%s", *indent, tr("Description")), description2, sizeof(description2), trVDR(FileNameChars)));
        Add(new cMenuEditTimeItem(cString::sprintf("%s%s", *indent, tr("Time")), &tmpConfig.favTime2));
    }
    Add(new cMenuEditBoolItem(tr("Use user defined time 3 in favorites"), &tmpConfig.favUseTime3));
    if (tmpConfig.favUseTime3) {
        Add(new cMenuEditStrItem(cString::sprintf("%s%s", *indent, tr("Description")), description3, sizeof(description3), trVDR(FileNameChars)));
        Add(new cMenuEditTimeItem(cString::sprintf("%s%s", *indent, tr("Time")), &tmpConfig.favTime3));
    }
    Add(new cMenuEditBoolItem(tr("Use user defined time 4 in favorites"), &tmpConfig.favUseTime4));
    if (tmpConfig.favUseTime4) {
        Add(new cMenuEditStrItem(cString::sprintf("%s%s", *indent, tr("Description")), description4, sizeof(description4), trVDR(FileNameChars)));
        Add(new cMenuEditTimeItem(cString::sprintf("%s%s", *indent, tr("Time")), &tmpConfig.favTime4));
    }
    Add(new cMenuEditBoolItem(tr("Limit channels in favorites"), &tmpConfig.favLimitChannels));
    if (tmpConfig.favLimitChannels) {
        Add(new cMenuEditChanItem(tr("Start Channel"), &tmpConfig.favStartChannel));
        Add(new cMenuEditChanItem(tr("Stop Channel"), &tmpConfig.favStopChannel));
    }
    Add(new cMenuEditIntItem(tr("Minutes a switchtimer switches before start of a show"), &tmpConfig.switchMinsBefore, 0, 10));

    SetCurrent(Get(current));
    Display();
}

void cTvGuideSetup::Store(void) {
    config = tmpConfig;
    SetupStore("showMainMenuEntry", config.showMainMenuEntry);
    SetupStore("replaceOriginalSchedule", config.replaceOriginalSchedule);
    SetupStore("displayMode", config.displayMode);
    SetupStore("channelsPerPageHorizontal", config.channelsPerPageHorizontal);
    SetupStore("channelsPerPageVertical", config.channelsPerPageVertical);
    SetupStore("displayHours", config.displayHours);
    SetupStore("bigStepHours", config.bigStepHours);
    SetupStore("hugeStepHours", config.hugeStepHours);
    SetupStore("hideLastChannelGroup", config.hideLastChannelGroup);
    SetupStore("channelJumpMode", config.channelJumpMode);
    SetupStore("closeOnSwitch", config.closeOnSwitch);
    SetupStore("numKeyMode", config.numKeyMode);
    SetupStore("blueKeyMode", config.blueKeyMode);
    SetupStore("intelligentSwitch", config.intelligentSwitch);
    SetupStore("rerunAmount", config.rerunAmount);
    SetupStore("rerunDistance", config.rerunDistance);
    SetupStore("rerunMaxChannel", config.rerunMaxChannel);
    SetupStore("useRemoteTimers", config.useRemoteTimers);
    SetupStore("instRecFolderMode", config.instRecFolderMode);
    SetupStore("instRecFixedFolder", config.instRecFixedFolder.c_str());
    SetupStore("favWhatsOnNow", config.favWhatsOnNow);
    SetupStore("favWhatsOnNext", config.favWhatsOnNext);
    SetupStore("favUseTime1", config.favUseTime1);
    SetupStore("favUseTime2", config.favUseTime2);
    SetupStore("favUseTime3", config.favUseTime3);
    SetupStore("favUseTime4", config.favUseTime4);
    SetupStore("favTime1", config.favTime1);
    SetupStore("favTime2", config.favTime2);
    SetupStore("favTime3", config.favTime3);
    SetupStore("favTime4", config.favTime4);
    SetupStore("descUser1", config.descUser1.c_str());
    SetupStore("descUser2", config.descUser2.c_str());
    SetupStore("descUser3", config.descUser3.c_str());
    SetupStore("descUser4", config.descUser4.c_str());
    SetupStore("favLimitChannels", config.favLimitChannels);
    SetupStore("favStartChannel", config.favStartChannel);
    SetupStore("favStopChannel", config.favStopChannel);
    SetupStore("switchMinsBefore", config.switchMinsBefore);
}

eOSState cTvGuideSetup::ProcessKey(eKeys Key) {
    int tmpFavUseTime1 = tmpConfig.favUseTime1;
    int tmpFavUseTime2 = tmpConfig.favUseTime2;
    int tmpFavUseTime3 = tmpConfig.favUseTime3;
    int tmpFavUseTime4 = tmpConfig.favUseTime4;
    int tmpFavLimitChannels = tmpConfig.favLimitChannels;
    int tmpFolderMode = tmpConfig.instRecFolderMode;
    eOSState state = cOsdMenu::ProcessKey(Key);
    if (Key == kOk) {
        tmpConfig.descUser1 = description1;
        tmpConfig.descUser2 = description2;
        tmpConfig.descUser3 = description3;
        tmpConfig.descUser4 = description4;
        tmpConfig.instRecFixedFolder = fixedFolder;
        Store();
    } else if ((Key == kLeft)||(Key == kRight)) {
        if ((tmpFavUseTime1 != tmpConfig.favUseTime1) ||
            (tmpFavUseTime2 != tmpConfig.favUseTime2) ||
            (tmpFavUseTime3 != tmpConfig.favUseTime3) ||
            (tmpFavUseTime4 != tmpConfig.favUseTime4) ||
            (tmpFavLimitChannels != tmpConfig.favLimitChannels) ||
            (tmpFolderMode != tmpConfig.instRecFolderMode) )
            Setup();
    }
    return state;
}
