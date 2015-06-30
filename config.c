#include <vdr/plugin.h>
#include "config.h"

cTVGuideConfig::cTVGuideConfig(void) {
    showMainMenuEntry = 1;
    replaceOriginalSchedule = 0;
    displayMode = eHorizontal;
    channelsPerPageHorizontal = 8;
    channelsPerPageVertical = 6;
    displayHours = 4;
    timeStep = 60;
    bigStepHours = 3;
    hugeStepHours = 24;
    hideLastChannelGroup = 0;
    channelJumpMode = eGroupJump;
    closeOnSwitch = 0;
    numKeyMode = eChannelJump;
    blueKeyMode = eBlueKeyFavorites;
    intelligentSwitch = 0;
    //settings for rerun display
    rerunAmount = 10;
    rerunDistance = 2;
    rerunMaxChannel = 0;
    useRemoteTimers = 1;
    instRecFolderMode = eFolderRoot;
    instRecFixedFolder = "";
    favWhatsOnNow = 1;
    favWhatsOnNext = 1;
    favUseTime1 = 0;
    favUseTime2 = 0;
    favUseTime3 = 0;
    favUseTime4 = 0;
    favTime1 = 0;
    favTime2 = 0;
    favTime3 = 0;
    favTime4 = 0;
    descUser1 = "";
    descUser2 = "";
    descUser3 = "";
    descUser4 = "";
    favLimitChannels = 0;
    favStartChannel = 0;
    favStopChannel = 0;
    useSubtitleRerun = 1;
    switchMinsBefore = 2;
}

bool cTVGuideConfig::SetupParse(const char *Name, const char *Value) {

    if      (!strcasecmp(Name, "showMainMenuEntry"))         showMainMenuEntry = atoi(Value);
    else if (!strcasecmp(Name, "replaceOriginalSchedule"))   replaceOriginalSchedule = atoi(Value);
    else if (!strcasecmp(Name, "displayMode"))               displayMode = atoi(Value);
    else if (!strcasecmp(Name, "channelsPerPageHorizontal")) channelsPerPageHorizontal = atoi(Value);
    else if (!strcasecmp(Name, "channelsPerPageVertical"))   channelsPerPageVertical = atoi(Value);
    else if (!strcasecmp(Name, "displayHours"))              displayHours = atoi(Value);
    else if (!strcasecmp(Name, "bigStepHours"))              bigStepHours = atoi(Value);
    else if (!strcasecmp(Name, "hugeStepHours"))             hugeStepHours = atoi(Value);
    else if (!strcasecmp(Name, "hideLastChannelGroup"))      hideLastChannelGroup = atoi(Value);
    else if (!strcasecmp(Name, "channelJumpMode"))           channelJumpMode = atoi(Value);
    else if (!strcasecmp(Name, "closeOnSwitch"))             closeOnSwitch = atoi(Value);
    else if (!strcasecmp(Name, "numKeyMode"))                numKeyMode = atoi(Value);
    else if (!strcasecmp(Name, "blueKeyMode"))               blueKeyMode = atoi(Value);
    else if (!strcasecmp(Name, "intelligentSwitch"))         intelligentSwitch = atoi(Value);
    else if (!strcasecmp(Name, "rerunAmount"))               rerunAmount = atoi(Value);
    else if (!strcasecmp(Name, "rerunDistance"))             rerunDistance = atoi(Value);
    else if (!strcasecmp(Name, "rerunMaxChannel"))           rerunMaxChannel = atoi(Value);
    else if (!strcasecmp(Name, "useRemoteTimers"))           useRemoteTimers = atoi(Value);
    else if (!strcasecmp(Name, "instRecFolderMode"))         instRecFolderMode = atoi(Value);
    else if (!strcasecmp(Name, "instRecFixedFolder"))        instRecFixedFolder = atoi(Value);
    else if (!strcasecmp(Name, "favWhatsOnNow"))             favWhatsOnNow = atoi(Value);
    else if (!strcasecmp(Name, "favWhatsOnNext"))            favWhatsOnNext = atoi(Value);
    else if (!strcasecmp(Name, "favUseTime1"))               favUseTime1 = atoi(Value);
    else if (!strcasecmp(Name, "favUseTime2"))               favUseTime2 = atoi(Value);
    else if (!strcasecmp(Name, "favUseTime3"))               favUseTime3 = atoi(Value);
    else if (!strcasecmp(Name, "favUseTime4"))               favUseTime4 = atoi(Value);
    else if (!strcasecmp(Name, "favTime1"))                  favTime1 = atoi(Value);
    else if (!strcasecmp(Name, "favTime2"))                  favTime2 = atoi(Value);
    else if (!strcasecmp(Name, "favTime3"))                  favTime3 = atoi(Value);
    else if (!strcasecmp(Name, "favTime4"))                  favTime4 = atoi(Value);
    else if (!strcasecmp(Name, "descUser1"))                 descUser1 = Value;
    else if (!strcasecmp(Name, "descUser2"))                 descUser2 = Value;
    else if (!strcasecmp(Name, "descUser3"))                 descUser3 = Value;
    else if (!strcasecmp(Name, "descUser4"))                 descUser4 = Value;
    else if (!strcasecmp(Name, "favLimitChannels"))          favLimitChannels = atoi(Value);
    else if (!strcasecmp(Name, "favStartChannel"))           favStartChannel = atoi(Value);
    else if (!strcasecmp(Name, "favStopChannel"))            favStopChannel = atoi(Value);
    else if (!strcasecmp(Name, "switchMinsBefore"))          switchMinsBefore = atoi(Value);
    else return false;

    return true;
}