#include "helpers.h"
#include "tvguidengosd.h"
#include "detailview.h"

cDetailView::cDetailView(skindesignerapi::cOsdView *detailView, const cEvent *event) {
    init = true;
    lastSecond = -1;
    this->detailView = detailView;
    this->event = event;
    back = detailView->GetViewElement((int)eViewElementsDetail::background);
    header = detailView->GetViewElement((int)eViewElementsDetail::header);
    footer = detailView->GetViewElement((int)eViewElementsDetail::footer);
    watch = detailView->GetViewElement((int)eViewElementsDetail::time);
    tabs = detailView->GetViewTabs();
    movie = NULL;
    series = NULL;
}

cDetailView::~cDetailView() {
    delete back;
    delete header;
    delete footer;
    delete watch;
    delete tabs;
    delete detailView;
    delete movie;
    delete series;
}

void cDetailView::Draw(void) {
    if (!event) {
        return;
    }
    if (init) {
        DrawBackground();
        DrawHeader();
        DrawFooter();
        Flush();
        SetTabTokens();
        tabs->Init();
        init = false;
    }
    tabs->Display();
}

void cDetailView::Left(void) {
    tabs->Left();
    tabs->Display();
}

void cDetailView::Right(void) {
    tabs->Right();
    tabs->Display();
}

void cDetailView::Up(void) {
    tabs->Up();
    tabs->Display();
}

void cDetailView::Down(void) {
    tabs->Down();
    tabs->Display();
}

void cDetailView::DefineTokens(eViewElementsDetail ve, skindesignerapi::cTokenContainer *tk) {
    switch (ve) {
        case eViewElementsDetail::header:
            tk->DefineIntToken("{ismovie}", (int)eScraperHeaderIT::ismovie);
            tk->DefineIntToken("{isseries}", (int)eScraperHeaderIT::isseries);
            tk->DefineIntToken("{posteravailable}", (int)eScraperHeaderIT::posteravailable);
            tk->DefineIntToken("{posterwidth}", (int)eScraperHeaderIT::posterwidth);
            tk->DefineIntToken("{posterheight}", (int)eScraperHeaderIT::posterheight);
            tk->DefineIntToken("{banneravailable}", (int)eScraperHeaderIT::banneravailable);
            tk->DefineIntToken("{bannerwidth}", (int)eScraperHeaderIT::bannerwidth);
            tk->DefineIntToken("{bannerheight}", (int)eScraperHeaderIT::bannerheight);
            tk->DefineIntToken("{daynumeric}", (int)eDetailedHeaderIT::daynumeric);
            tk->DefineIntToken("{month}", (int)eDetailedHeaderIT::month);
            tk->DefineIntToken("{year}", (int)eDetailedHeaderIT::year);
            tk->DefineIntToken("{running}", (int)eDetailedHeaderIT::running);
            tk->DefineIntToken("{elapsed}", (int)eDetailedHeaderIT::elapsed);
            tk->DefineIntToken("{duration}", (int)eDetailedHeaderIT::duration);
            tk->DefineIntToken("{durationhours}", (int)eDetailedHeaderIT::durationhours);
            tk->DefineIntToken("{channelnumber}", (int)eDetailedHeaderIT::channelnumber);
            tk->DefineIntToken("{channellogoexists}", (int)eDetailedHeaderIT::channellogoexists);
            tk->DefineIntToken("{epgpicavailable}", (int)eDetailedHeaderIT::epgpicavailable);
            tk->DefineStringToken("{posterpath}", (int)eScraperHeaderST::posterpath);
            tk->DefineStringToken("{bannerpath}", (int)eScraperHeaderST::bannerpath);
            tk->DefineStringToken("{title}", (int)eDetailedHeaderST::title);
            tk->DefineStringToken("{shorttext}", (int)eDetailedHeaderST::shorttext);
            tk->DefineStringToken("{start}", (int)eDetailedHeaderST::start);
            tk->DefineStringToken("{stop}", (int)eDetailedHeaderST::stop);
            tk->DefineStringToken("{day}", (int)eDetailedHeaderST::day);
            tk->DefineStringToken("{date}", (int)eDetailedHeaderST::date);
            tk->DefineStringToken("{durationminutes}", (int)eDetailedHeaderST::durationminutes);
            tk->DefineStringToken("{vps}", (int)eDetailedHeaderST::vps);
            tk->DefineStringToken("{channelname}", (int)eDetailedHeaderST::channelname);
            tk->DefineStringToken("{channelid}", (int)eDetailedHeaderST::channelid);
            tk->DefineStringToken("{epgpicpath}", (int)eDetailedHeaderST::epgpicpath);
            break;
        case eViewElementsDetail::footer:
            tk->DefineIntToken("{red1}", (int)eFooterIT::red1);
            tk->DefineIntToken("{red2}", (int)eFooterIT::red2);
            tk->DefineIntToken("{red3}", (int)eFooterIT::red3);
            tk->DefineIntToken("{red4}", (int)eFooterIT::red4);
            tk->DefineIntToken("{green1}", (int)eFooterIT::green1);
            tk->DefineIntToken("{green2}", (int)eFooterIT::green2);
            tk->DefineIntToken("{green3}", (int)eFooterIT::green3);
            tk->DefineIntToken("{green4}", (int)eFooterIT::green4);
            tk->DefineIntToken("{yellow1}", (int)eFooterIT::yellow1);
            tk->DefineIntToken("{yellow2}", (int)eFooterIT::yellow2);
            tk->DefineIntToken("{yellow3}", (int)eFooterIT::yellow3);
            tk->DefineIntToken("{yellow4}", (int)eFooterIT::yellow4);
            tk->DefineIntToken("{blue1}", (int)eFooterIT::blue1);
            tk->DefineIntToken("{blue2}", (int)eFooterIT::blue2);
            tk->DefineIntToken("{blue3}", (int)eFooterIT::blue3);
            tk->DefineIntToken("{blue4}", (int)eFooterIT::blue4);
            tk->DefineStringToken("{red}", (int)eFooterST::red);
            tk->DefineStringToken("{green}", (int)eFooterST::green);
            tk->DefineStringToken("{yellow}", (int)eFooterST::yellow);
            tk->DefineStringToken("{blue}", (int)eFooterST::blue);
            break;
        default:
            break;
    }
}

void cDetailView::DefineTabTokens(skindesignerapi::cTokenContainer *tk) {
    tk->DefineStringToken("{title}", (int)eDetailedEpgST::title);
    tk->DefineStringToken("{shorttext}", (int)eDetailedEpgST::shorttext);
    tk->DefineStringToken("{description}", (int)eDetailedEpgST::description);
    tk->DefineStringToken("{start}", (int)eDetailedEpgST::start);
    tk->DefineStringToken("{stop}", (int)eDetailedEpgST::stop);
    tk->DefineStringToken("{day}", (int)eDetailedEpgST::day);
    tk->DefineStringToken("{date}", (int)eDetailedEpgST::date);
    tk->DefineStringToken("{durationminutes}", (int)eDetailedEpgST::durationminutes);
    tk->DefineStringToken("{vps}", (int)eDetailedEpgST::vps);
    tk->DefineStringToken("{channelname}", (int)eDetailedEpgST::channelname);
    tk->DefineStringToken("{channelid}", (int)eDetailedEpgST::channelid);
    tk->DefineStringToken("{epgpic1path}", (int)eDetailedEpgST::epgpic1path);
    tk->DefineStringToken("{epgpic2path}", (int)eDetailedEpgST::epgpic2path);
    tk->DefineStringToken("{epgpic3path}", (int)eDetailedEpgST::epgpic3path);
    tk->DefineStringToken("{movietitle}", (int)eScraperST::movietitle);
    tk->DefineStringToken("{movieoriginalTitle}", (int)eScraperST::movieoriginalTitle);
    tk->DefineStringToken("{movietagline}", (int)eScraperST::movietagline);
    tk->DefineStringToken("{movieoverview}", (int)eScraperST::movieoverview);
    tk->DefineStringToken("{moviegenres}", (int)eScraperST::moviegenres);
    tk->DefineStringToken("{moviehomepage}", (int)eScraperST::moviehomepage);
    tk->DefineStringToken("{moviereleasedate}", (int)eScraperST::moviereleasedate);
    tk->DefineStringToken("{moviepopularity}", (int)eScraperST::moviepopularity);
    tk->DefineStringToken("{movievoteaverage}", (int)eScraperST::movievoteaverage);
    tk->DefineStringToken("{posterpath}", (int)eScraperST::posterpath);
    tk->DefineStringToken("{fanartpath}", (int)eScraperST::fanartpath);
    tk->DefineStringToken("{moviecollectionName}", (int)eScraperST::moviecollectionName);
    tk->DefineStringToken("{collectionposterpath}", (int)eScraperST::collectionposterpath);
    tk->DefineStringToken("{collectionfanartpath}", (int)eScraperST::collectionfanartpath);
    tk->DefineStringToken("{seriesname}", (int)eScraperST::seriesname);
    tk->DefineStringToken("{seriesoverview}", (int)eScraperST::seriesoverview);
    tk->DefineStringToken("{seriesfirstaired}", (int)eScraperST::seriesfirstaired);
    tk->DefineStringToken("{seriesnetwork}", (int)eScraperST::seriesnetwork);
    tk->DefineStringToken("{seriesgenre}", (int)eScraperST::seriesgenre);
    tk->DefineStringToken("{seriesrating}", (int)eScraperST::seriesrating);
    tk->DefineStringToken("{seriesstatus}", (int)eScraperST::seriesstatus);
    tk->DefineStringToken("{episodetitle}", (int)eScraperST::episodetitle);
    tk->DefineStringToken("{episodefirstaired}", (int)eScraperST::episodefirstaired);
    tk->DefineStringToken("{episodegueststars}", (int)eScraperST::episodegueststars);
    tk->DefineStringToken("{episodeoverview}", (int)eScraperST::episodeoverview);
    tk->DefineStringToken("{episoderating}", (int)eScraperST::episoderating);
    tk->DefineStringToken("{episodeimagepath}", (int)eScraperST::episodeimagepath);
    tk->DefineStringToken("{seasonposterpath}", (int)eScraperST::seasonposterpath);
    tk->DefineStringToken("{seriesposter1path}", (int)eScraperST::seriesposter1path);
    tk->DefineStringToken("{seriesposter2path}", (int)eScraperST::seriesposter2path);
    tk->DefineStringToken("{seriesposter3path}", (int)eScraperST::seriesposter3path);
    tk->DefineStringToken("{seriesfanart1path}", (int)eScraperST::seriesfanart1path);
    tk->DefineStringToken("{seriesfanart2path}", (int)eScraperST::seriesfanart2path);
    tk->DefineStringToken("{seriesfanart3path}", (int)eScraperST::seriesfanart3path);
    tk->DefineStringToken("{seriesbanner1path}", (int)eScraperST::seriesbanner1path);
    tk->DefineStringToken("{seriesbanner2path}", (int)eScraperST::seriesbanner2path);
    tk->DefineStringToken("{seriesbanner3path}", (int)eScraperST::seriesbanner3path);
    tk->DefineIntToken("{daynumeric}", (int)eDetailedEpgIT::daynumeric);
    tk->DefineIntToken("{month}", (int)eDetailedEpgIT::month);
    tk->DefineIntToken("{year}", (int)eDetailedEpgIT::year);
    tk->DefineIntToken("{running}", (int)eDetailedEpgIT::running);
    tk->DefineIntToken("{elapsed}", (int)eDetailedEpgIT::elapsed);
    tk->DefineIntToken("{duration}", (int)eDetailedEpgIT::duration);
    tk->DefineIntToken("{durationhours}", (int)eDetailedEpgIT::durationhours);
    tk->DefineIntToken("{channelnumber}", (int)eDetailedEpgIT::channelnumber);
    tk->DefineIntToken("{channellogoexists}", (int)eDetailedEpgIT::channellogoexists);
    tk->DefineIntToken("{hasreruns}", (int)eDetailedEpgIT::hasreruns);
    tk->DefineIntToken("{epgpic1avaialble}", (int)eDetailedEpgIT::epgpic1avaialble);
    tk->DefineIntToken("{epgpic2avaialble}", (int)eDetailedEpgIT::epgpic2avaialble);
    tk->DefineIntToken("{epgpic3avaialble}", (int)eDetailedEpgIT::epgpic3avaialble);
    tk->DefineIntToken("{ismovie}", (int)eScraperIT::ismovie);
    tk->DefineIntToken("{moviebudget}", (int)eScraperIT::moviebudget);
    tk->DefineIntToken("{movierevenue}", (int)eScraperIT::movierevenue);
    tk->DefineIntToken("{movieadult}", (int)eScraperIT::movieadult);
    tk->DefineIntToken("{movieruntime}", (int)eScraperIT::movieruntime);
    tk->DefineIntToken("{isseries}", (int)eScraperIT::isseries);
    tk->DefineIntToken("{posterwidth}", (int)eScraperIT::posterwidth);
    tk->DefineIntToken("{posterheight}", (int)eScraperIT::posterheight);
    tk->DefineIntToken("{fanartwidth}", (int)eScraperIT::fanartwidth);
    tk->DefineIntToken("{fanartheight}", (int)eScraperIT::fanartheight);
    tk->DefineIntToken("{movieiscollection}", (int)eScraperIT::movieiscollection);
    tk->DefineIntToken("{collectionposterwidth}", (int)eScraperIT::collectionposterwidth);
    tk->DefineIntToken("{collectionposterheight}", (int)eScraperIT::collectionposterheight);
    tk->DefineIntToken("{collectionfanartwidth}", (int)eScraperIT::collectionfanartwidth);
    tk->DefineIntToken("{collectionfanartheight}", (int)eScraperIT::collectionfanartheight);
    tk->DefineIntToken("{epgpicavailable}", (int)eScraperIT::epgpicavailable);
    tk->DefineIntToken("{episodenumber}", (int)eScraperIT::episodenumber);
    tk->DefineIntToken("{episodeseason}", (int)eScraperIT::episodeseason);
    tk->DefineIntToken("{episodeimagewidth}", (int)eScraperIT::episodeimagewidth);
    tk->DefineIntToken("{episodeimageheight}", (int)eScraperIT::episodeimageheight);
    tk->DefineIntToken("{seasonposterwidth}", (int)eScraperIT::seasonposterwidth);
    tk->DefineIntToken("{seasonposterheight}", (int)eScraperIT::seasonposterheight);
    tk->DefineIntToken("{seriesposter1width}", (int)eScraperIT::seriesposter1width);
    tk->DefineIntToken("{seriesposter1height}", (int)eScraperIT::seriesposter1height);
    tk->DefineIntToken("{seriesposter2width}", (int)eScraperIT::seriesposter2width);
    tk->DefineIntToken("{seriesposter2height}", (int)eScraperIT::seriesposter2height);
    tk->DefineIntToken("{seriesposter3width}", (int)eScraperIT::seriesposter3width);
    tk->DefineIntToken("{seriesposter3height}", (int)eScraperIT::seriesposter3height);
    tk->DefineIntToken("{seriesfanart1width}", (int)eScraperIT::seriesfanart1width);
    tk->DefineIntToken("{seriesfanart1height}", (int)eScraperIT::seriesfanart1height);
    tk->DefineIntToken("{seriesfanart2width}", (int)eScraperIT::seriesfanart2width);
    tk->DefineIntToken("{seriesfanart2height}", (int)eScraperIT::seriesfanart2height);
    tk->DefineIntToken("{seriesfanart3width}", (int)eScraperIT::seriesfanart3width);
    tk->DefineIntToken("{seriesfanart3height}", (int)eScraperIT::seriesfanart3height);
    tk->DefineIntToken("{seriesbanner1width}", (int)eScraperIT::seriesbanner1width);
    tk->DefineIntToken("{seriesbanner1height}", (int)eScraperIT::seriesbanner1height);
    tk->DefineIntToken("{seriesbanner2width}", (int)eScraperIT::seriesbanner2width);
    tk->DefineIntToken("{seriesbanner2height}", (int)eScraperIT::seriesbanner2height);
    tk->DefineIntToken("{seriesbanner3width}", (int)eScraperIT::seriesbanner3width);
    tk->DefineIntToken("{seriesbanner3height}", (int)eScraperIT::seriesbanner3height);
    tk->DefineLoopToken("{reruns[title]}", (int)eRerunsLT::title);
    tk->DefineLoopToken("{reruns[shorttext]}", (int)eRerunsLT::shorttext);
    tk->DefineLoopToken("{reruns[date]}", (int)eRerunsLT::date);
    tk->DefineLoopToken("{reruns[day]}", (int)eRerunsLT::day);
    tk->DefineLoopToken("{reruns[start]}", (int)eRerunsLT::start);
    tk->DefineLoopToken("{reruns[stop]}", (int)eRerunsLT::stop);
    tk->DefineLoopToken("{reruns[channelname]}", (int)eRerunsLT::channelname);
    tk->DefineLoopToken("{reruns[channelnumber]}", (int)eRerunsLT::channelnumber);
    tk->DefineLoopToken("{reruns[channelid]}", (int)eRerunsLT::channelid);
    tk->DefineLoopToken("{reruns[channellogoexists]}", (int)eRerunsLT::channellogoexists);
    tk->DefineLoopToken("{actors[name]}", (int)eScraperLT::name);
    tk->DefineLoopToken("{actors[role]}", (int)eScraperLT::role);
    tk->DefineLoopToken("{actors[thumb]}", (int)eScraperLT::thumb);
    tk->DefineLoopToken("{actors[thumbwidth]}", (int)eScraperLT::thumbwidth);
    tk->DefineLoopToken("{actors[thumbheight]}", (int)eScraperLT::thumbheight);
}
/********************************************************************
* Private Functions
********************************************************************/
void cDetailView::DrawBackground(void) {
    back->Display();
}

void cDetailView::DrawHeader(void) {
    if (!event)
        return;
    header->ClearTokens();
    static cPlugin *pScraper = GetScraperPlugin();
    if (pScraper) {
        ScraperGetEventType getType;
        getType.event = event;
        if (!pScraper->Service("GetEventType", &getType)) {
            if (getType.type == tMovie) {
                cMovie movie;
                movie.movieId = getType.movieId;
                pScraper->Service("GetMovie", &movie);
                header->AddIntToken((int)eScraperHeaderIT::banneravailable, true);
                header->AddIntToken((int)eScraperHeaderIT::isseries, false);
                header->AddIntToken((int)eScraperHeaderIT::posteravailable, true);
                header->AddIntToken((int)eScraperHeaderIT::banneravailable, false);
                header->AddStringToken((int)eScraperHeaderST::posterpath, movie.poster.path.c_str());
                header->AddIntToken((int)eScraperHeaderIT::posterwidth, movie.poster.width);
                header->AddIntToken((int)eScraperHeaderIT::posterheight, movie.poster.height);
            } else if (getType.type == tSeries) {
                cSeries series;
                series.seriesId = getType.seriesId;
                series.episodeId = getType.episodeId;
                pScraper->Service("GetSeries", &series);
                header->AddIntToken((int)eScraperHeaderIT::ismovie, false);
                header->AddIntToken((int)eScraperHeaderIT::isseries, true);
                vector<cTvMedia>::iterator poster = series.posters.begin();
                if (poster != series.posters.end()) {
                    header->AddIntToken((int)eScraperHeaderIT::posterwidth, (*poster).width);
                    header->AddIntToken((int)eScraperHeaderIT::posterheight, (*poster).height);
                    header->AddStringToken((int)eScraperHeaderST::posterpath, (*poster).path.c_str());
                    header->AddIntToken((int)eScraperHeaderIT::posteravailable, true);
                }
                vector<cTvMedia>::iterator banner = series.banners.begin();
                if (banner != series.banners.end()) {
                    header->AddIntToken((int)eScraperHeaderIT::bannerwidth, (*banner).width);
                    header->AddIntToken((int)eScraperHeaderIT::bannerheight, (*banner).height);
                    header->AddStringToken((int)eScraperHeaderST::bannerpath, (*banner).path.c_str());
                    header->AddIntToken((int)eScraperHeaderIT::banneravailable, true);
                }
            }
        }
    }

    header->AddStringToken((int)eDetailedHeaderST::title, event->Title());
    header->AddStringToken((int)eDetailedHeaderST::shorttext, event->ShortText());
    header->AddStringToken((int)eDetailedHeaderST::start, *(event->GetTimeString()));
    header->AddStringToken((int)eDetailedHeaderST::stop, *(event->GetEndTimeString()));
    
    time_t startTime = event->StartTime();
    header->AddStringToken((int)eDetailedHeaderST::day, *WeekDayName(startTime));
    header->AddStringToken((int)eDetailedHeaderST::date, *ShortDateString(startTime));
    struct tm * sStartTime = localtime(&startTime);
    header->AddIntToken((int)eDetailedHeaderIT::year, sStartTime->tm_year + 1900);
    header->AddIntToken((int)eDetailedHeaderIT::daynumeric, sStartTime->tm_mday);
    header->AddIntToken((int)eDetailedHeaderIT::month, sStartTime->tm_mon+1);
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannel *channel = Channels->GetByChannelID(event->ChannelID());
#else
    const cChannel *channel = Channels.GetByChannelID(event->ChannelID());
#endif
    if (channel) {
        header->AddStringToken((int)eDetailedHeaderST::channelname, channel->Name());
        header->AddIntToken((int)eDetailedHeaderIT::channelnumber, channel->Number());
    } else {
        header->AddIntToken((int)eDetailedHeaderIT::channelnumber, 0);
    }
    cString channelID = channel->GetChannelID().ToString();
    header->AddStringToken((int)eDetailedHeaderST::channelid, *channelID);
    header->AddIntToken((int)eDetailedHeaderIT::channellogoexists, header->ChannelLogoExists(*channelID));

    bool isRunning = false;
    time_t now = time(NULL);
    if ((now >= event->StartTime()) && (now <= event->EndTime()))
        isRunning = true;
    header->AddIntToken((int)eDetailedHeaderIT::running, isRunning);
    if (isRunning) {
        header->AddIntToken((int)eDetailedHeaderIT::elapsed, (now - event->StartTime())/60);
    } else {
        header->AddIntToken((int)eDetailedHeaderIT::elapsed, 0);
    }
    header->AddIntToken((int)eDetailedHeaderIT::duration, event->Duration() / 60);
    header->AddIntToken((int)eDetailedHeaderIT::durationhours, event->Duration() / 3600);
    header->AddStringToken((int)eDetailedHeaderST::durationminutes, *cString::sprintf("%.2d", (event->Duration() / 60)%60));
    if (event->Vps())
        header->AddStringToken((int)eDetailedHeaderST::vps, *event->GetVpsString());

    stringstream epgImageName;
    epgImageName << event->EventID();
    string epgImagePath = header->GetEpgImagePath();

    bool epgPicAvailable = FileExists(epgImagePath, epgImageName.str(), "jpg");
    if (epgPicAvailable) {
        header->AddIntToken((int)eDetailedHeaderIT::epgpicavailable, true);
        header->AddStringToken((int)eDetailedHeaderST::epgpicpath, *cString::sprintf("%s%s.jpg", epgImagePath.c_str(), epgImageName.str().c_str()));
    } else {
        epgImageName << "_0";
        epgPicAvailable = FileExists(epgImagePath, epgImageName.str(), "jpg");
        if (epgPicAvailable) {
            header->AddIntToken((int)eDetailedHeaderIT::epgpicavailable, true);
            header->AddStringToken((int)eDetailedHeaderST::epgpicpath, *cString::sprintf("%s%s.jpg", epgImagePath.c_str(), epgImageName.str().c_str()));
        }
    }
    
    header->Display();
}

void cDetailView::DrawFooter(void) {
    string textGreen = "";
    string textYellow = "";
    string textRed = tr("Search & Record");
    string textBlue = tr("Switch");

    int colorKeys[4] = { Setup.ColorKey0, Setup.ColorKey1, Setup.ColorKey2, Setup.ColorKey3 };

    footer->Clear();
    footer->ClearTokens();

    footer->AddStringToken((int)eFooterST::red, textRed.c_str());
    footer->AddStringToken((int)eFooterST::green, textGreen.c_str());
    footer->AddStringToken((int)eFooterST::yellow, textYellow.c_str());
    footer->AddStringToken((int)eFooterST::blue, textBlue.c_str());

    for (int button = 0; button < 4; button++) {
        bool isRed = false;
        bool isGreen = false;
        bool isYellow = false;
        bool isBlue = false;
        switch (colorKeys[button]) {
            case 0:
                isRed = true;
                break;
            case 1:
                isGreen = true;
                break;
            case 2:
                isYellow = true;
                break;
            case 3:
                isBlue = true;
                break;
            default:
                break;
        }
        footer->AddIntToken(0  + button, isRed);
        footer->AddIntToken(4  + button, isGreen);
        footer->AddIntToken(8  + button, isYellow);
        footer->AddIntToken(12 + button, isBlue);
    }

    footer->Display();
}

bool cDetailView::DrawTime(void) {
    time_t t = time(0);   // get time now
    struct tm * now = localtime(&t);
    int sec = now->tm_sec;
    if (sec == lastSecond)
        return false;

    int min = now->tm_min;
    int hour = now->tm_hour;
    int hourMinutes = hour%12 * 5 + min / 12;

    char monthname[20];
    char monthshort[10];
    strftime(monthshort, sizeof(monthshort), "%b", now);
    strftime(monthname, sizeof(monthname), "%B", now);
    
    watch->Clear();
    watch->ClearTokens();
    watch->AddIntToken((int)eTimeIT::sec, sec);
    watch->AddIntToken((int)eTimeIT::min, min);
    watch->AddIntToken((int)eTimeIT::hour, hour);
    watch->AddIntToken((int)eTimeIT::hmins, hourMinutes);
    watch->AddIntToken((int)eTimeIT::year, now->tm_year + 1900);
    watch->AddIntToken((int)eTimeIT::day, now->tm_mday);
    watch->AddStringToken((int)eTimeST::time, *TimeString(t));
    watch->AddStringToken((int)eTimeST::monthname, monthname);
    watch->AddStringToken((int)eTimeST::monthnameshort, monthshort);
    watch->AddStringToken((int)eTimeST::month, *cString::sprintf("%02d", now->tm_mon + 1));
    watch->AddStringToken((int)eTimeST::dayleadingzero, *cString::sprintf("%02d", now->tm_mday));
    watch->AddStringToken((int)eTimeST::dayname, *WeekDayNameFull(now->tm_wday));
    watch->AddStringToken((int)eTimeST::daynameshort, *WeekDayName(now->tm_wday));
    watch->Display();

    lastSecond = sec;
    return true;
}

void cDetailView::SetTabTokens(void) {
    tabs->ClearTokens();
    tabs->AddStringToken((int)eDetailedEpgST::title, event->Title());
    tabs->AddStringToken((int)eDetailedEpgST::shorttext, event->ShortText());
    tabs->AddStringToken((int)eDetailedEpgST::description, event->Description());
    tabs->AddStringToken((int)eDetailedEpgST::start, *(event->GetTimeString()));
    tabs->AddStringToken((int)eDetailedEpgST::stop, *(event->GetEndTimeString()));
    time_t startTime = event->StartTime();
    tabs->AddStringToken((int)eDetailedEpgST::day, *WeekDayName(startTime));
    tabs->AddStringToken((int)eDetailedEpgST::date, *ShortDateString(startTime));
    struct tm * sStartTime = localtime(&startTime);
    tabs->AddIntToken((int)eDetailedEpgIT::year, sStartTime->tm_year + 1900);
    tabs->AddIntToken((int)eDetailedEpgIT::daynumeric, sStartTime->tm_mday);
    tabs->AddIntToken((int)eDetailedEpgIT::month, sStartTime->tm_mon+1);

    cString channelID = event->ChannelID().ToString();
    tabs->AddStringToken((int)eDetailedEpgST::channelid, *channelID);
    tabs->AddIntToken((int)eDetailedEpgIT::channellogoexists, tabs->ChannelLogoExists(*channelID));

    bool isRunning = false;
    time_t now = time(NULL);
    if ((now >= event->StartTime()) && (now <= event->EndTime()))
        isRunning = true;
    tabs->AddIntToken((int)eDetailedEpgIT::running, isRunning);
    if (isRunning) {
        tabs->AddIntToken((int)eDetailedEpgIT::elapsed, (now - event->StartTime())/60);
    } else {
        tabs->AddIntToken((int)eDetailedEpgIT::elapsed, 0);
    }
    tabs->AddIntToken((int)eDetailedEpgIT::duration, event->Duration() / 60);    
    tabs->AddIntToken((int)eDetailedEpgIT::durationhours, event->Duration() / 3600);
    tabs->AddStringToken((int)eDetailedEpgST::durationminutes, *cString::sprintf("%.2d", (event->Duration() / 60)%60));
    if (event->Vps())
        tabs->AddStringToken((int)eDetailedEpgST::vps, *event->GetVpsString());
    else
        tabs->AddStringToken((int)eDetailedEpgST::vps, "");


    bool scrapInfoAvailable = LoadScrapInfo(event);
    int numActors = NumActors();

    cList<Epgsearch_searchresults_v1_0::cServiceSearchResult> *reruns = LoadReruns();
    int numReruns = NumReruns(reruns);
    
    vector<int> loopInfo;
    loopInfo.push_back(numReruns);
    loopInfo.push_back(numActors);
    tabs->SetLoop(loopInfo);

    if (numReruns > 0) {
        tabs->AddIntToken((int)eDetailedEpgIT::hasreruns, 1);
        SetReruns(reruns);
    }
    if (scrapInfoAvailable) {
        SetScraperTokens();
    }

    SetEpgPictures(event->EventID());
}

cList<Epgsearch_searchresults_v1_0::cServiceSearchResult> *cDetailView::LoadReruns(void) {
    cPlugin *epgSearchPlugin = cPluginManager::GetPlugin("epgsearch");
    if (!epgSearchPlugin)
        return NULL;

    if (isempty(event->Title()))
        return NULL;

    Epgsearch_searchresults_v1_0 data;
    data.query = (char*)event->Title();
    data.mode = 0;
    data.channelNr = 0;
    data.useTitle = true;
    data.useSubTitle = true;
    data.useDescription = false;

    cList<Epgsearch_searchresults_v1_0::cServiceSearchResult> *result = NULL;
    if (epgSearchPlugin->Service("Epgsearch-searchresults-v1.0", &data))
        result = data.pResultList;
    return result;
}

int cDetailView::NumReruns(cList<Epgsearch_searchresults_v1_0::cServiceSearchResult> *reruns) {
    if (!reruns || reruns->Count() < 2)
        return 0;

    int maxNumReruns = config.rerunAmount;
    int rerunDistance = config.rerunDistance * 3600;
    int rerunMaxChannel = config.rerunMaxChannel;

    int i = 0;
    for (Epgsearch_searchresults_v1_0::cServiceSearchResult *r = reruns->First(); r && i < maxNumReruns; r = reruns->Next(r)) {
        time_t eventStart = event->StartTime();
        time_t rerunStart = r->event->StartTime();
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
        LOCK_CHANNELS_READ;
        const cChannel *channel = Channels->GetByChannelID(r->event->ChannelID(), true, true);
#else
        const cChannel *channel = Channels.GetByChannelID(r->event->ChannelID(), true, true);
#endif
        //check for identical event
        if ((event->ChannelID() == r->event->ChannelID()) && (eventStart == rerunStart))
            continue;
        //check for timely distance
        if (rerunDistance > 0)
            if (rerunStart - eventStart < rerunDistance)
                continue;
        //check for maxchannel
        if (rerunMaxChannel > 0)
            if (channel && channel->Number() > rerunMaxChannel)
                continue;
        i++;
    }
    return i;
}

void cDetailView::SetReruns(cList<Epgsearch_searchresults_v1_0::cServiceSearchResult> *reruns) {
    if (!reruns || reruns->Count() < 2)
        return;
    int rerunsIndex = tabs->GetLoopIndex("reruns");

    int maxNumReruns = config.rerunAmount;
    int rerunDistance = config.rerunDistance * 3600;
    int rerunMaxChannel = config.rerunMaxChannel;

    int i = 0;
    for (Epgsearch_searchresults_v1_0::cServiceSearchResult *r = reruns->First(); r && i < maxNumReruns; r = reruns->Next(r)) {
        time_t eventStart = event->StartTime();
        time_t rerunStart = r->event->StartTime();
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
        LOCK_CHANNELS_READ;
        const cChannel *channel = Channels->GetByChannelID(r->event->ChannelID(), true, true);
#else
        const cChannel *channel = Channels.GetByChannelID(r->event->ChannelID(), true, true);
#endif
        //check for identical event
        if ((event->ChannelID() == r->event->ChannelID()) && (eventStart == rerunStart))
            continue;
        //check for timely distance
        if (rerunDistance > 0)
            if (rerunStart - eventStart < rerunDistance)
                continue;
        //check for maxchannel
        if (rerunMaxChannel > 0)
            if (channel && channel->Number() > rerunMaxChannel)
                continue;
        tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::title, r->event->Title());
        tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::shorttext, r->event->ShortText());
        tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::start, *(r->event->GetTimeString()));
        tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::stop, *(r->event->GetEndTimeString()));
        tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::date, *ShortDateString(r->event->StartTime()));
        tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::day, *WeekDayName(r->event->StartTime()));
        cString channelID = r->event->ChannelID().ToString();
        tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::channelid, *channelID);
        bool logoExists = tabs->ChannelLogoExists(*channelID);
        tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::channellogoexists, logoExists ? "1" : "0");
        if (channel) {
            cString channelNumber = cString::sprintf("%d", channel->Number());
            tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::channelname, channel->ShortName(true));
            tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::channelnumber, *channelNumber);
        } else {
            tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::channelname, "");
            tabs->AddLoopToken(rerunsIndex, i, (int)eRerunsLT::channelnumber, "");
        }
        i++;
    }
}

bool cDetailView::LoadScrapInfo(const cEvent *event) {
    static cPlugin *pScraper = GetScraperPlugin();
    if (!pScraper || !event) {
        return false;
    }
    delete movie;
    movie = NULL;
    delete series;
    series = NULL;
    
    ScraperGetEventType getType;
    getType.event = event;
    getType.recording = NULL;
    if (!pScraper->Service("GetEventType", &getType)) {
        return false;
    }
    if (getType.type == tMovie) {
        movie = new cMovie();
        movie->movieId = getType.movieId;
        pScraper->Service("GetMovie", movie);
        return true;
    } else if (getType.type == tSeries) {
        series = new cSeries();
        series->seriesId = getType.seriesId;
        series->episodeId = getType.episodeId;
        pScraper->Service("GetSeries", series);
        return true;
    }
    return false;
}

int cDetailView::NumActors(void) {
    if (series) {
        return series->actors.size();
    } else if (movie) {
        return movie->actors.size();
    }    
    return 0;
}

void cDetailView::SetScraperTokens(void) {
    if (movie) {
        tabs->AddIntToken((int)eScraperIT::ismovie, true);
        tabs->AddIntToken((int)eScraperIT::isseries, false);

        tabs->AddStringToken((int)eScraperST::movietitle, movie->title.c_str());
        tabs->AddStringToken((int)eScraperST::movieoriginalTitle, movie->originalTitle.c_str());
        tabs->AddStringToken((int)eScraperST::movietagline, movie->tagline.c_str());
        tabs->AddStringToken((int)eScraperST::movieoverview, movie->overview.c_str());
        tabs->AddStringToken((int)eScraperST::moviegenres, movie->genres.c_str());
        tabs->AddStringToken((int)eScraperST::moviehomepage, movie->homepage.c_str());
        tabs->AddStringToken((int)eScraperST::moviereleasedate, movie->releaseDate.c_str());
        stringstream pop;
        pop << movie->popularity;
        tabs->AddStringToken((int)eScraperST::moviepopularity, pop.str().c_str());
        stringstream vote;
        vote << movie->voteAverage;
        tabs->AddStringToken((int)eScraperST::movievoteaverage, pop.str().c_str());
        tabs->AddStringToken((int)eScraperST::posterpath, movie->poster.path.c_str());
        tabs->AddStringToken((int)eScraperST::fanartpath, movie->fanart.path.c_str());
        tabs->AddStringToken((int)eScraperST::collectionposterpath, movie->collectionPoster.path.c_str());
        tabs->AddStringToken((int)eScraperST::collectionfanartpath, movie->collectionFanart.path.c_str());

        tabs->AddIntToken((int)eScraperIT::movieadult, movie->adult);
        tabs->AddIntToken((int)eScraperIT::moviebudget, movie->budget);
        tabs->AddIntToken((int)eScraperIT::movierevenue, movie->revenue);
        tabs->AddIntToken((int)eScraperIT::movieruntime, movie->runtime);
        tabs->AddIntToken((int)eScraperIT::posterwidth, movie->poster.width);
        tabs->AddIntToken((int)eScraperIT::posterheight, movie->poster.height);
        tabs->AddIntToken((int)eScraperIT::fanartwidth, movie->fanart.width);
        tabs->AddIntToken((int)eScraperIT::fanartheight, movie->fanart.height);
        tabs->AddIntToken((int)eScraperIT::collectionposterwidth, movie->collectionPoster.width);
        tabs->AddIntToken((int)eScraperIT::collectionposterheight, movie->collectionPoster.height);
        tabs->AddIntToken((int)eScraperIT::collectionfanartwidth, movie->collectionFanart.width);
        tabs->AddIntToken((int)eScraperIT::collectionfanartheight, movie->collectionFanart.height);

        int actorsIndex = tabs->GetLoopIndex("actors");
        int i=0;
        for (vector<cActor>::iterator act = movie->actors.begin(); act != movie->actors.end(); act++) {
            tabs->AddLoopToken(actorsIndex, i, (int)eScraperLT::name, (*act).name.c_str());
            tabs->AddLoopToken(actorsIndex, i, (int)eScraperLT::role, (*act).role.c_str());
            tabs->AddLoopToken(actorsIndex, i, (int)eScraperLT::thumb, (*act).actorThumb.path.c_str());
            tabs->AddLoopToken(actorsIndex, i, (int)eScraperLT::thumbwidth, *cString::sprintf("%d", (*act).actorThumb.width));
            tabs->AddLoopToken(actorsIndex, i, (int)eScraperLT::thumbheight, *cString::sprintf("%d", (*act).actorThumb.height));
            i++;
        }
    } else if (series) {
        tabs->AddIntToken((int)eScraperIT::ismovie, false);
        tabs->AddIntToken((int)eScraperIT::isseries, true);
        //Series Basics
        tabs->AddStringToken((int)eScraperST::seriesname, series->name.c_str());
        tabs->AddStringToken((int)eScraperST::seriesoverview, series->overview.c_str());
        tabs->AddStringToken((int)eScraperST::seriesfirstaired, series->firstAired.c_str());
        tabs->AddStringToken((int)eScraperST::seriesnetwork, series->network.c_str());
        tabs->AddStringToken((int)eScraperST::seriesgenre, series->genre.c_str());
        stringstream rating;
        rating << series->rating;
        tabs->AddStringToken((int)eScraperST::seriesrating, rating.str().c_str());
        tabs->AddStringToken((int)eScraperST::seriesstatus, series->status.c_str());
        //Episode Information
        tabs->AddIntToken((int)eScraperIT::episodenumber, series->episode.number);
        tabs->AddIntToken((int)eScraperIT::episodeseason, series->episode.season);
        tabs->AddStringToken((int)eScraperST::episodetitle, series->episode.name.c_str());
        tabs->AddStringToken((int)eScraperST::episodefirstaired, series->episode.firstAired.c_str());
        tabs->AddStringToken((int)eScraperST::episodegueststars, series->episode.guestStars.c_str());
        tabs->AddStringToken((int)eScraperST::episodeoverview, series->episode.overview.c_str());
        stringstream eprating;
        eprating << series->episode.rating;
        tabs->AddStringToken((int)eScraperST::episoderating, eprating.str().c_str());
        tabs->AddIntToken((int)eScraperIT::episodeimagewidth, series->episode.episodeImage.width);
        tabs->AddIntToken((int)eScraperIT::episodeimageheight, series->episode.episodeImage.height);
        tabs->AddStringToken((int)eScraperST::episodeimagepath, series->episode.episodeImage.path.c_str());
        //Seasonposter
        tabs->AddIntToken((int)eScraperIT::seasonposterwidth, series->seasonPoster.width);
        tabs->AddIntToken((int)eScraperIT::seasonposterheight, series->seasonPoster.height);
        tabs->AddStringToken((int)eScraperST::seasonposterpath, series->seasonPoster.path.c_str());

        //Posters
        int indexInt = (int)eScraperIT::seriesposter1width;
        int indexStr = (int)eScraperST::seriesposter1path;
        for(vector<cTvMedia>::iterator poster = series->posters.begin(); poster != series->posters.end(); poster++) {
            tabs->AddIntToken(indexInt, (*poster).width);
            tabs->AddIntToken(indexInt+1, (*poster).height);
            tabs->AddStringToken(indexStr, (*poster).path.c_str());
            indexInt += 2;
            indexStr++;
        }
        //Banners
        indexInt = (int)eScraperIT::seriesbanner1width;
        indexStr = (int)eScraperST::seriesbanner1path;
        for(vector<cTvMedia>::iterator banner = series->banners.begin(); banner != series->banners.end(); banner++) {
            tabs->AddIntToken(indexInt, (*banner).width);
            tabs->AddIntToken(indexInt+1, (*banner).height);
            tabs->AddStringToken(indexStr, (*banner).path.c_str());
            indexInt += 2;
            indexStr++;
        }
        //Fanarts
        indexInt = (int)eScraperIT::seriesfanart1width;
        indexStr = (int)eScraperST::seriesfanart1path;
        for(vector<cTvMedia>::iterator fanart = series->fanarts.begin(); fanart != series->fanarts.end(); fanart++) {
            tabs->AddIntToken(indexInt, (*fanart).width);
            tabs->AddIntToken(indexInt+1, (*fanart).height);
            tabs->AddStringToken(indexStr, (*fanart).path.c_str());
            indexInt += 2;
            indexStr++;
        }
        //Actors
        int actorsIndex = tabs->GetLoopIndex("actors");
        int i=0;
        for (vector<cActor>::iterator act = series->actors.begin(); act != series->actors.end(); act++) {
            tabs->AddLoopToken(actorsIndex, i, (int)eScraperLT::name, (*act).name.c_str());
            tabs->AddLoopToken(actorsIndex, i, (int)eScraperLT::role, (*act).role.c_str());
            tabs->AddLoopToken(actorsIndex, i, (int)eScraperLT::thumb, (*act).actorThumb.path.c_str());
            tabs->AddLoopToken(actorsIndex, i, (int)eScraperLT::thumbwidth, *cString::sprintf("%d", (*act).actorThumb.width));
            tabs->AddLoopToken(actorsIndex, i, (int)eScraperLT::thumbheight, *cString::sprintf("%d", (*act).actorThumb.height));
            i++;
        }
    } else {
        tabs->AddIntToken((int)eScraperIT::ismovie, false);
        tabs->AddIntToken((int)eScraperIT::isseries, false);
    }
}

void cDetailView::SetEpgPictures(int eventId) {
    string epgImagePath = tabs->GetEpgImagePath();
    for (int i=0; i<3; i++) {
        stringstream picName;
        picName << eventId << "_" << i;
        bool epgPicAvailable = FileExists(epgImagePath, picName.str(), "jpg");
        if (epgPicAvailable) {
            tabs->AddIntToken((int)eDetailedEpgIT::epgpic1avaialble + i, true);
            tabs->AddStringToken((int)eDetailedEpgST::epgpic1path + i, *cString::sprintf("%s%s.jpg", epgImagePath.c_str(), picName.str().c_str()));
        } else {
            tabs->AddIntToken((int)eDetailedEpgIT::epgpic1avaialble + i, false);
            tabs->AddStringToken((int)eDetailedEpgST::epgpic1path + i, "");            
        }
    }
}