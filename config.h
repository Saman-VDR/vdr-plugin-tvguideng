#ifndef __TVGUIDE_CONFIG_H
#define __TVGUIDE_CONFIG_H

#include "string"
#include <vdr/plugin.h>

using namespace std;

enum eDisplayMode {
    eVertical,
    eHorizontal
};

enum eChannelJumpMode {
    eNumJump,
    eGroupJump
};

enum eNumKeyMode {
    eTimeJump,
    eChannelJump
};

enum eBlueKeyMode {
    eBlueKeySwitch = 0,
    eBlueKeyEPG,
    eBlueKeyFavorites
};

enum eInstRecFolderMode {
    eFolderRoot = 0,
    eFolderSelect,
    eFolderFixed
};

class cTVGuideConfig {
private:
public:
    cTVGuideConfig(void);
    ~cTVGuideConfig() {};
    int showMainMenuEntry;
    int replaceOriginalSchedule;
    int displayMode;
    int channelsPerPageHorizontal;
    int channelsPerPageVertical;
    int displayHours;
    int timeStep;
    int bigStepHours;
    int hugeStepHours;
    int hideLastChannelGroup;
    int channelJumpMode;
    int closeOnSwitch;
    int numKeyMode;
    int blueKeyMode;
    int intelligentSwitch;
    int rerunAmount;
    int rerunDistance;
    int rerunMaxChannel;
    int useRemoteTimers;
    int instRecFolderMode;
    string instRecFixedFolder;
    int favWhatsOnNow;
    int favWhatsOnNext;
    int favUseTime1;
    int favUseTime2;
    int favUseTime3;
    int favUseTime4;
    int favTime1;
    int favTime2;
    int favTime3;
    int favTime4;
    string descUser1;
    string descUser2;
    string descUser3;
    string descUser4;
    int favLimitChannels;
    int favStartChannel;
    int favStopChannel;
    int useSubtitleRerun;
    int switchMinsBefore;
    bool SetupParse(const char *Name, const char *Value);
};

#ifdef DEFINE_CONFIG
    cTVGuideConfig config;
    cPlugin *pRemoteTimers = NULL;
#else
    extern cTVGuideConfig config;
    extern cPlugin *pRemoteTimers;
#endif

#endif //__TVGUIDE_CONFIG_H