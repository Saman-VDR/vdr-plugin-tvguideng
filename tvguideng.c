/*
 * tvguideng.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/plugin.h>
#include "libskindesigner/services.h"
#define DEFINE_CONFIG 1
#include "config.h"
#include "setup.h"
#include "tvguidengosd.h"

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = "TV Guide for Skindesigner Skins";
static const char *MAINMENUENTRY  = "TV Guide NG";

class cPluginTvguideng : public cPlugin {
private:
  // Add any member variables or functions you may need here.
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
    RegisterPlugin reg;
    reg.name = "tvguideng";
    reg.SetView(viRootView, "root.xml");
    reg.SetViewElement(viRootView, verBackgroundHor, "background_hor");
    reg.SetViewElement(viRootView, verBackgroundVer, "background_ver");
    reg.SetViewElement(viRootView, verHeader, "header");
    reg.SetViewElement(viRootView, verTime, "time");
    reg.SetViewElement(viRootView, verFooter, "footer");
    reg.SetViewElement(viRootView, verDateTimelineHor, "datetimeline_hor");
    reg.SetViewElement(viRootView, verDateTimelineVer, "datetimeline_ver");
    reg.SetViewElement(viRootView, verTimeIndicatorHor, "timeindicator_hor");
    reg.SetViewElement(viRootView, verTimeIndicatorVer, "timeindicator_ver");
    reg.SetViewElement(viRootView, verChannelJump, "channeljump");
    reg.SetViewGrid(viRootView, vgChannelsHor, "channels_hor");
    reg.SetViewGrid(viRootView, vgChannelsVer, "channels_ver");
    reg.SetViewGrid(viRootView, vgSchedulesHor, "schedules_hor");
    reg.SetViewGrid(viRootView, vgSchedulesVer, "schedules_ver");
    reg.SetViewGrid(viRootView, vgTimelineHor, "timeline_hor");
    reg.SetViewGrid(viRootView, vgTimelineVer, "timeline_ver");
    reg.SetViewGrid(viRootView, vgChannelGroupsHor, "channelgroups_hor");
    reg.SetViewGrid(viRootView, vgChannelGroupsVer, "channelgroups_ver");
    //Detail View
    reg.SetSubView(viRootView, viDetailView, "detail.xml");
    reg.SetViewElement(viDetailView, vedBackground, "background");
    reg.SetViewElement(viDetailView, vedHeader, "header");
    reg.SetViewElement(viDetailView, vedFooter, "footer");
    reg.SetViewElement(viDetailView, vedTime, "time");
    //Search & Recording Menus
    reg.SetSubView(viRootView, viRecMenu, "recmenu.xml");
    reg.SetViewElement(viRecMenu, vemBackground, "background");
    reg.SetViewElement(viRecMenu, vemScrollbar, "scrollbar");
    reg.SetViewGrid(viRecMenu, vgRecordingMenu, "recmenu");
    static cPlugin *pSkinDesigner = cPluginManager::GetPlugin("skindesigner");
    if (pSkinDesigner) {
        pSkinDesigner->Service("RegisterPlugin", &reg);
    } else {
        esyslog("tvguideng: skindesigner not available");
    }

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
  return new cTVGuideOSD();
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

VDRPLUGINCREATOR(cPluginTvguideng); // Don't touch this!
