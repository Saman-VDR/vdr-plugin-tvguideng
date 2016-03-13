/*
 * tvguideng.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/plugin.h>
#include <libskindesignerapi/skindesignerapi.h>
#define DEFINE_CONFIG 1
#include "config.h"
#include "setup.h"
#include "definitions.h"
#include "tvguidengosd.h"

static const char *VERSION        = "0.3.0";
static const char *DESCRIPTION    = "TV Guide for Skindesigner Skins";
static const char *MAINMENUENTRY  = "TV Guide NG";

class cPluginTvguideng : public cPlugin {
private:
  skindesignerapi::cPluginStructure *plugStruct;
  void SkindesignerRegister(void);
public:
  cPluginTvguideng(void);
  virtual ~cPluginTvguideng();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return DESCRIPTION; }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Stop(void);
  virtual void Housekeeping(void);
  virtual void MainThreadHook(void);
  virtual cString Active(void);
  virtual time_t WakeupTime(void);
  virtual const char *MainMenuEntry(void) { return (config.showMainMenuEntry)?MAINMENUENTRY:NULL; }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  };

cPluginTvguideng::cPluginTvguideng(void) {
}

cPluginTvguideng::~cPluginTvguideng() {
}

const char *cPluginTvguideng::CommandLineHelp(void) {
    return NULL;
}

bool cPluginTvguideng::ProcessArgs(int argc, char *argv[]) {
    return true;
}

bool cPluginTvguideng::Initialize(void) {
    return true;
}

bool cPluginTvguideng::Start(void) {
    SkindesignerRegister();
    return true;
}

void cPluginTvguideng::Stop(void) {
}

void cPluginTvguideng::Housekeeping(void) {
}

void cPluginTvguideng::MainThreadHook(void) {
}

cString cPluginTvguideng::Active(void) {
    return NULL;
}

time_t cPluginTvguideng::WakeupTime(void) {
    return 0;
}

cOsdObject *cPluginTvguideng::MainMenuAction(void) {
    return new cTVGuideOSD(plugStruct);
}

cMenuSetupPage *cPluginTvguideng::SetupMenu(void) {
    return new cTvGuideSetup();
}

bool cPluginTvguideng::SetupParse(const char *Name, const char *Value) {
    return config.SetupParse(Name, Value);
}

bool cPluginTvguideng::Service(const char *Id, void *Data) {
    if (strcmp(Id, "MainMenuHooksPatch-v1.0::osSchedule") == 0 && config.replaceOriginalSchedule != 0) {
        if (Data == NULL)
            return true;
            cOsdObject **guide = (cOsdObject**) Data;
            if (guide)
                *guide = MainMenuAction();
            return true;
    }
    return false;
}

const char **cPluginTvguideng::SVDRPHelpPages(void) {
    return NULL;
}

cString cPluginTvguideng::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode) {
    return NULL;
}

void cPluginTvguideng::SkindesignerRegister(void) {
    plugStruct = new skindesignerapi::cPluginStructure();
    plugStruct->name = "tvguideng";
    plugStruct->libskindesignerAPIVersion = LIBSKINDESIGNERAPIVERSION;
    //Root View
    plugStruct->RegisterRootView("root.xml");
    skindesignerapi::cTokenContainer *tkBackground = new skindesignerapi::cTokenContainer();
    //View Elements Root View
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::backgroundHor, "background_hor", tkBackground);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::backgroundVer, "background_ver", tkBackground);
    skindesignerapi::cTokenContainer *tkHeaderHor = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::headerHor, tkHeaderHor);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::headerHor, "header_hor", tkHeaderHor);
    skindesignerapi::cTokenContainer *tkHeaderVer = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::headerVer, tkHeaderVer);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::headerVer, "header_ver", tkHeaderVer);
    skindesignerapi::cTokenContainer *tkTimerHor = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::timeHor, tkTimerHor);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::timeHor, "time_hor", tkTimerHor);
    skindesignerapi::cTokenContainer *tkTimerVer = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::timeVer, tkTimerVer);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::timeVer, "time_ver", tkTimerVer);
    skindesignerapi::cTokenContainer *tkFooterHor = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::footerHor, tkFooterHor);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::footerHor, "footer_hor", tkFooterHor);
    skindesignerapi::cTokenContainer *tkFooterVer = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::footerVer, tkFooterVer);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::footerVer, "footer_ver", tkFooterVer);
    skindesignerapi::cTokenContainer *tkDateTimelineHor = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::dateTimelineHor, tkDateTimelineHor);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::dateTimelineHor, "datetimeline_hor", tkDateTimelineHor);
    skindesignerapi::cTokenContainer *tkDateTimelineVer = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::dateTimelineVer, tkDateTimelineVer);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::dateTimelineVer, "datetimeline_ver", tkDateTimelineVer);
    skindesignerapi::cTokenContainer *tkTimeIndHor = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::timeIndicatorHor, tkTimeIndHor);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::timeIndicatorHor, "timeindicator_hor", tkTimeIndHor);
    skindesignerapi::cTokenContainer *tkTimeIndVer = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::timeIndicatorVer, tkTimeIndVer);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::timeIndicatorVer, "timeindicator_ver", tkTimeIndVer);
    skindesignerapi::cTokenContainer *tkChanneljump = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::channelJump, tkChanneljump);
    plugStruct->RegisterViewElement((int)eViews::rootView, (int)eViewElementsRoot::channelJump, "channeljump", tkChanneljump);
    //View Grids Root View
    skindesignerapi::cTokenContainer *tkChannelsHor = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewGridsRoot::channelsHor, tkChannelsHor);
    plugStruct->RegisterViewGrid((int)eViews::rootView, (int)eViewGridsRoot::channelsHor, "channels_hor", tkChannelsHor);
    skindesignerapi::cTokenContainer *tkChannelsVer = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewGridsRoot::channelsVer, tkChannelsVer);
    plugStruct->RegisterViewGrid((int)eViews::rootView, (int)eViewGridsRoot::channelsVer, "channels_ver", tkChannelsVer);
    skindesignerapi::cTokenContainer *tkSchedulesHor = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewGridsRoot::schedulesHor, tkSchedulesHor);
    plugStruct->RegisterViewGrid((int)eViews::rootView, (int)eViewGridsRoot::schedulesHor, "schedules_hor", tkSchedulesHor);
    skindesignerapi::cTokenContainer *tkSchedulesVer = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewGridsRoot::schedulesVer, tkSchedulesVer);
    plugStruct->RegisterViewGrid((int)eViews::rootView, (int)eViewGridsRoot::schedulesVer, "schedules_ver", tkSchedulesVer);
    skindesignerapi::cTokenContainer *tkTimelineHor = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewGridsRoot::timelineHor, tkTimelineHor);
    plugStruct->RegisterViewGrid((int)eViews::rootView, (int)eViewGridsRoot::timelineHor, "timeline_hor", tkTimelineHor);
    skindesignerapi::cTokenContainer *tkTimelineVer = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewGridsRoot::timelineVer, tkTimelineVer);
    plugStruct->RegisterViewGrid((int)eViews::rootView, (int)eViewGridsRoot::timelineVer, "timeline_ver", tkTimelineVer);
    skindesignerapi::cTokenContainer *tkChannelGroupsHor = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewGridsRoot::channelGroupsHor, tkChannelGroupsHor);
    plugStruct->RegisterViewGrid((int)eViews::rootView, (int)eViewGridsRoot::channelGroupsHor, "channelgroups_hor", tkChannelGroupsHor);
    skindesignerapi::cTokenContainer *tkChannelGroupsVer = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewGridsRoot::channelGroupsVer, tkChannelGroupsVer);
    plugStruct->RegisterViewGrid((int)eViews::rootView, (int)eViewGridsRoot::channelGroupsVer, "channelgroups_ver", tkChannelGroupsVer);

    //Detail View
    plugStruct->RegisterSubView((int)eViews::detailView, "detail.xml");
    //viewelements Detail View Menus
    skindesignerapi::cTokenContainer *tkBackDetail = new skindesignerapi::cTokenContainer();
    cDetailView::DefineTokens(eViewElementsDetail::background, tkBackDetail);
    plugStruct->RegisterViewElement((int)eViews::detailView, (int)eViewElementsDetail::background, "background", tkBackDetail);
    skindesignerapi::cTokenContainer *tkHeaderDetail = new skindesignerapi::cTokenContainer();
    cDetailView::DefineTokens(eViewElementsDetail::header, tkHeaderDetail);
    plugStruct->RegisterViewElement((int)eViews::detailView, (int)eViewElementsDetail::header, "header", tkHeaderDetail);
    skindesignerapi::cTokenContainer *tkFooterDetail = new skindesignerapi::cTokenContainer();
    cDetailView::DefineTokens(eViewElementsDetail::footer, tkFooterDetail);
    plugStruct->RegisterViewElement((int)eViews::detailView, (int)eViewElementsDetail::footer, "footer", tkFooterDetail);
    skindesignerapi::cTokenContainer *tkTimeDetail = new skindesignerapi::cTokenContainer();
    cTVGuideOSD::DefineTokens(eViewElementsRoot::timeHor, tkTimeDetail);
    plugStruct->RegisterViewElement((int)eViews::detailView, (int)eViewElementsDetail::time, "time", tkTimeDetail);
    //TabbedView
    skindesignerapi::cTokenContainer *tkTab = new skindesignerapi::cTokenContainer();
    cDetailView::DefineTabTokens(tkTab);
    plugStruct->RegisterViewTab((int)eViews::detailView, tkTab);

    //Search & Recording Menus
    plugStruct->RegisterSubView((int)eViews::recMenu, "recmenu.xml");
    //viewelements Recording Menus
    skindesignerapi::cTokenContainer *tkBackRec = new skindesignerapi::cTokenContainer();
    cRecMenu::DefineTokens(eViewElementsRecMenu::background, tkBackRec);
    plugStruct->RegisterViewElement((int)eViews::recMenu, (int)eViewElementsRecMenu::background, "background", tkBackRec);
    skindesignerapi::cTokenContainer *tkScrollbarRec = new skindesignerapi::cTokenContainer();
    cRecMenu::DefineTokens(eViewElementsRecMenu::scrollbar, tkScrollbarRec);
    plugStruct->RegisterViewElement((int)eViews::recMenu, (int)eViewElementsRecMenu::scrollbar, "scrollbar", tkScrollbarRec);
    //grids Recording Menus
    skindesignerapi::cTokenContainer *tkRecMenu = new skindesignerapi::cTokenContainer();
    cRecMenu::DefineTokens(eViewGridsRecMenu::menu, tkRecMenu);
    plugStruct->RegisterViewGrid((int)eViews::recMenu, (int)eViewGridsRecMenu::menu, "recmenu", tkRecMenu);

    plugStruct->RegisterSubView((int)eViews::recMenu2, "recmenu.xml");
    //viewelements Recording Menus
    skindesignerapi::cTokenContainer *tkBackRec2 = new skindesignerapi::cTokenContainer();
    cRecMenu::DefineTokens(eViewElementsRecMenu::background, tkBackRec2);
    plugStruct->RegisterViewElement((int)eViews::recMenu2, (int)eViewElementsRecMenu::background, "background", tkBackRec2);
    skindesignerapi::cTokenContainer *tkScrollbarRec2 = new skindesignerapi::cTokenContainer();
    cRecMenu::DefineTokens(eViewElementsRecMenu::scrollbar, tkScrollbarRec2);
    plugStruct->RegisterViewElement((int)eViews::recMenu2, (int)eViewElementsRecMenu::scrollbar, "scrollbar", tkScrollbarRec2);
    //grids Recording Menus
    skindesignerapi::cTokenContainer *tkRecMenu2 = new skindesignerapi::cTokenContainer();
    cRecMenu::DefineTokens(eViewGridsRecMenu::menu, tkRecMenu2);
    plugStruct->RegisterViewGrid((int)eViews::recMenu2, (int)eViewGridsRecMenu::menu, "recmenu", tkRecMenu2);

    plugStruct->RegisterSubView((int)eViews::recMenu3, "recmenu.xml");
    //viewelements Recording Menus
    skindesignerapi::cTokenContainer *tkBackRec3 = new skindesignerapi::cTokenContainer();
    cRecMenu::DefineTokens(eViewElementsRecMenu::background, tkBackRec3);
    plugStruct->RegisterViewElement((int)eViews::recMenu3, (int)eViewElementsRecMenu::background, "background", tkBackRec3);
    skindesignerapi::cTokenContainer *tkScrollbarRec3 = new skindesignerapi::cTokenContainer();
    cRecMenu::DefineTokens(eViewElementsRecMenu::scrollbar, tkScrollbarRec3);
    plugStruct->RegisterViewElement((int)eViews::recMenu3, (int)eViewElementsRecMenu::scrollbar, "scrollbar", tkScrollbarRec3);
    //grids Recording Menus
    skindesignerapi::cTokenContainer *tkRecMenu3 = new skindesignerapi::cTokenContainer();
    cRecMenu::DefineTokens(eViewGridsRecMenu::menu, tkRecMenu3);
    plugStruct->RegisterViewGrid((int)eViews::recMenu3, (int)eViewGridsRecMenu::menu, "recmenu", tkRecMenu3);

    if (!skindesignerapi::SkindesignerAPI::RegisterPlugin(plugStruct)) {
        esyslog("tvguideng: skindesigner not available");
    } else {
        dsyslog("tvguideng: successfully registered at skindesigner, id %d", plugStruct->id);      
    }
}

VDRPLUGINCREATOR(cPluginTvguideng); // Don't touch this!
