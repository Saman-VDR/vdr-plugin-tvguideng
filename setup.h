#ifndef __TVGUIDE_SETUP_H
#define __TVGUIDE_SETUP_H

#include <vdr/plugin.h>
#include "config.h"

extern cTVGuideConfig config;

class cTvGuideSetup : public cMenuSetupPage {
    public:
        cTvGuideSetup(void);
        virtual ~cTvGuideSetup();
    private:
        cTVGuideConfig tmpConfig;
        const char * displayModeItems[2];
        const char * jumpMode[2];
        const char * numMode[2];
        const char * blueMode[3];
        char description1[256];
        char description2[256];
        char description3[256];
        char description4[256];
        const char * recFolderMode[3];
        char fixedFolder[256];
        const char *useSubtitleRerunTexts[3];
        void Setup(void);
        virtual eOSState ProcessKey(eKeys Key);
        virtual void Store(void);
};
#endif //__TVGUIDE_SETUP_H
