#ifndef __TVGUIDENGOSD_H
#define __TVGUIDENGOSD_H

#include <vdr/osdbase.h>
#include <vdr/plugin.h>
#include <libskindesignerapi/skindesignerapi.h>
#include <libskindesignerapi/skindesignerosdbase.h>
#include "definitions.h"
#include "config.h"
#include "timemanager.h"
#include "epggrid.h"
#include "channeljump.h"
#include "detailview.h"
#include "recmenuview.h"
#include "recmanager.h"

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
    eOSState ChannelSwitch(const cEvent *e);
    void CreateSwitchTimer(const cEvent *e);
    void Favorites(void);
    void CreateInstantTimer(void);
    void DisplaySearchRecordings(void);
    void DisplaySearchEPG(void);
public:
    cTVGuideOSD(skindesignerapi::cPluginStructure *plugStruct);
    virtual ~cTVGuideOSD(void);
    virtual void Show(void);
  	virtual eOSState ProcessKey(eKeys Key);
    static void DefineTokens(eViewElementsRoot ve, skindesignerapi::cTokenContainer *tk);
    static void DefineTokens(eViewGridsRoot vg, skindesignerapi::cTokenContainer *tk);
};

#endif //__TVGUIDENGOSD_H
