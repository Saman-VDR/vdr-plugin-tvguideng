#ifndef __TVGUIDENGOSD_H
#define __TVGUIDENGOSD_H

#include <vdr/osdbase.h>
#include <vdr/plugin.h>
#include <libskindesignerapi/skindesignerapi.h>
#include <libskindesignerapi/skindesignerosdbase.h>
#include "config.h"
#include "timemanager.h"
#include "epggrid.h"
#include "channeljump.h"
#include "detailview.h"
#include "recmenuview.h"

enum eViews {
    viRootView,
    viDetailView,
    viRecMenu
};

enum eViewElementsRoot {
    verBackgroundHor,
    verBackgroundVer,
    verHeaderHor,
    verHeaderVer,
    verFooterHor,
    verFooterVer,
    verTimeHor,
    verTimeVer,
    verDateTimelineHor,
    verDateTimelineVer,
    verTimeIndicatorHor,
    verTimeIndicatorVer,
    verChannelJump
};

enum eViewGrids {
    vgChannelsHor,
    vgChannelsVer,
    vgSchedulesHor,
    vgSchedulesVer,
    vgChannelGroupsHor,
    vgChannelGroupsVer,
    vgTimelineHor,
    vgTimelineVer,
    vgRecordingMenu
};

enum eViewElementsDetail {
    vedBackground,
    vedHeader,
    vedFooter,
    vedTime
};

enum eViewElementsRecMenu {
    vemBackground,
    vemScrollbar
};

class cTVGuideOSD : public skindesignerapi::cSkindesignerOsdObject {
private:
    cTimeManager *timeManager;
    cEpgGrid *epgGrid;
    cChannelJump *channelJumper;
    cDetailView *detailView;
    cRecMenuView *recMenuView;
    void KeyLeft(void);
    void KeyRight(void);
    void KeyUp(void);
    void KeyDown(void);
    void TimeForward(void);
    void TimeBack(void);
    void ChannelsForward(void);
    void ChannelsBack(void);
    void NumericKey(int key);
    void NumericKeyTimeJump(int key);
    void NumericKeyChannelJump(int key);
    void CheckTimeout(void);
    void KeyGreen(void);
    void KeyYellow(void);
    eOSState KeyBlue(const cEvent *e);
    eOSState KeyOk(const cEvent *e);
    void KeyRed(void);
    void DetailView(const cEvent *e);
    void CloseDetailedView(void);
    eOSState ChannelSwitch(void);
    void Favorites(void);
public:
    cTVGuideOSD(void);
    virtual ~cTVGuideOSD(void);
    virtual void Show(void);
  	virtual eOSState ProcessKey(eKeys Key);
};

#endif //__TVGUIDENGOSD_H
