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
#include "tvguidengosd.h"

static const char *VERSION        = "0.1.5";
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
    skindesignerapi::cPluginStructure plugStruct;
    plugStruct.name = "tvguideng";
    plugStruct.libskindesignerAPIVersion = LIBSKINDESIGNERAPIVERSION;
    plugStruct.SetView(viRootView, "root.xml");
    plugStruct.SetViewElement(viRootView, verBackgroundHor, "background_hor");
    plugStruct.SetViewElement(viRootView, verBackgroundVer, "background_ver");
    plugStruct.SetViewElement(viRootView, verHeaderHor, "header_hor");
    plugStruct.SetViewElement(viRootView, verHeaderVer, "header_ver");
    plugStruct.SetViewElement(viRootView, verTimeHor, "time_hor");
    plugStruct.SetViewElement(viRootView, verTimeVer, "time_ver");
    plugStruct.SetViewElement(viRootView, verFooterHor, "footer_hor");
    plugStruct.SetViewElement(viRootView, verFooterVer, "footer_ver");
    plugStruct.SetViewElement(viRootView, verDateTimelineHor, "datetimeline_hor");
    plugStruct.SetViewElement(viRootView, verDateTimelineVer, "datetimeline_ver");
    plugStruct.SetViewElement(viRootView, verTimeIndicatorHor, "timeindicator_hor");
    plugStruct.SetViewElement(viRootView, verTimeIndicatorVer, "timeindicator_ver");
    plugStruct.SetViewElement(viRootView, verChannelJump, "channeljump");
    plugStruct.SetViewGrid(viRootView, vgChannelsHor, "channels_hor");
    plugStruct.SetViewGrid(viRootView, vgChannelsVer, "channels_ver");
    plugStruct.SetViewGrid(viRootView, vgSchedulesHor, "schedules_hor");
    plugStruct.SetViewGrid(viRootView, vgSchedulesVer, "schedules_ver");
    plugStruct.SetViewGrid(viRootView, vgTimelineHor, "timeline_hor");
    plugStruct.SetViewGrid(viRootView, vgTimelineVer, "timeline_ver");
    plugStruct.SetViewGrid(viRootView, vgChannelGroupsHor, "channelgroups_hor");
    plugStruct.SetViewGrid(viRootView, vgChannelGroupsVer, "channelgroups_ver");
    //Detail View
    plugStruct.SetSubView(viRootView, viDetailView, "detail.xml");
    plugStruct.SetViewElement(viDetailView, vedBackground, "background");
    plugStruct.SetViewElement(viDetailView, vedHeader, "header");
    plugStruct.SetViewElement(viDetailView, vedFooter, "footer");
    plugStruct.SetViewElement(viDetailView, vedTime, "time");
    //Search & Recording Menus
    plugStruct.SetSubView(viRootView, viRecMenu, "recmenu.xml");
    plugStruct.SetViewElement(viRecMenu, vemBackground, "background");
    plugStruct.SetViewElement(viRecMenu, vemScrollbar, "scrollbar");
    plugStruct.SetViewGrid(viRecMenu, vgRecordingMenu, "recmenu");

    if (!skindesignerapi::SkindesignerAPI::RegisterPlugin(&plugStruct)) {
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
