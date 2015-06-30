#include "helpers.h"
#include "tvguidengosd.h"
#include "detailview.h"
#include "services/scraper2vdr.h"
#include "services/epgsearch.h"

cDetailView::cDetailView(skindesignerapi::cOsdView *detailView, const cEvent *event) {
    init = true;
    lastSecond = -1;
    this->detailView = detailView;
    this->event = event;
    back = detailView->GetViewElement(vedBackground);
    header = detailView->GetViewElement(vedHeader);
    footer = detailView->GetViewElement(vedFooter);
    watch = detailView->GetViewElement(vedTime);
    tabs = detailView->GetViewTabs();
}

cDetailView::~cDetailView() {
    delete back;
    delete header;
    delete footer;
    delete watch;
    delete detailView;
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

void cDetailView::DrawBackground(void) {
    back->Display();
}

void cDetailView::DrawHeader(void) {
    if (!event)
        return;
    header->ClearTokens();
    static cPlugin *pScraper = GetScraperPlugin();
    if (!pScraper) {
        header->AddIntToken("ismovie", false);
        header->AddIntToken("isseries", false);
        header->AddIntToken("posteravailable", false);
        header->AddIntToken("banneravailable", false);
    } else {
        ScraperGetEventType getType;
        getType.event = event;
        if (!pScraper->Service("GetEventType", &getType)) {
            header->AddIntToken("ismovie", false);
            header->AddIntToken("isseries", false);
            header->AddIntToken("posteravailable", false);
            header->AddIntToken("banneravailable", false);
        } else {
            if (getType.type == tMovie) {
                cMovie movie;
                movie.movieId = getType.movieId;
                pScraper->Service("GetMovie", &movie);
                header->AddIntToken("ismovie", true);
                header->AddIntToken("isseries", false);
                header->AddIntToken("posteravailable", true);
                header->AddIntToken("banneravailable", false);
                header->AddStringToken("posterpath", movie.poster.path);
                header->AddIntToken("posterwidth", movie.poster.width);
                header->AddIntToken("posterheight", movie.poster.height);
            } else if (getType.type == tSeries) {
                cSeries series;
                series.seriesId = getType.seriesId;
                series.episodeId = getType.episodeId;
                pScraper->Service("GetSeries", &series);
                header->AddIntToken("ismovie", false);
                header->AddIntToken("isseries", true);
                vector<cTvMedia>::iterator poster = series.posters.begin();
                if (poster != series.posters.end()) {
                    header->AddIntToken("posterwidth", (*poster).width);
                    header->AddIntToken("posterheight", (*poster).height);
                    header->AddStringToken("posterpath", (*poster).path);
                    header->AddIntToken("posteravailable", true);
                } else {
                    header->AddIntToken("posterwidth", 0);
                    header->AddIntToken("posterheight", 0);
                    header->AddStringToken("posterpath", "");                    
                    header->AddIntToken("posteravailable", false);
                }
                vector<cTvMedia>::iterator banner = series.banners.begin();
                if (banner != series.banners.end()) {
                    header->AddIntToken("bannerwidth", (*banner).width);
                    header->AddIntToken("bannerheight", (*banner).height);
                    header->AddStringToken("bannerpath", (*banner).path);
                    header->AddIntToken("banneravailable", true);
                } else {
                    header->AddIntToken("bannerwidth", 0);
                    header->AddIntToken("bannerheight", 0);
                    header->AddStringToken("bannerpath", "");                    
                    header->AddIntToken("banneravailable", false);
                }
            } else {
                header->AddIntToken("ismovie", false);
                header->AddIntToken("isseries", false);        
                header->AddIntToken("posteravailable", false);
                header->AddIntToken("banneravailable", false);
            }
        }
    }

    header->AddStringToken("title", event->Title() ? event->Title() : "");
    header->AddStringToken("shorttext", event->ShortText() ? event->ShortText() : "");
    header->AddStringToken("start", *(event->GetTimeString()));
    header->AddStringToken("stop", *(event->GetEndTimeString()));
    
    time_t startTime = event->StartTime();
    header->AddStringToken("day", *WeekDayName(startTime));
    header->AddStringToken("date", *ShortDateString(startTime));
    struct tm * sStartTime = localtime(&startTime);
    header->AddIntToken("year", sStartTime->tm_year + 1900);
    header->AddIntToken("daynumeric", sStartTime->tm_mday);
    header->AddIntToken("month", sStartTime->tm_mon+1);

    const cChannel *channel = Channels.GetByChannelID(event->ChannelID());
    if (channel) {
        header->AddStringToken("channelname", channel->Name() ? channel->Name() : "");
        header->AddIntToken("channelnumber", channel->Number());
    } else {
        header->AddStringToken("channelname", "");            
        header->AddIntToken("channelnumber", 0);
    }
    string channelID = *(channel->GetChannelID().ToString());
    header->AddStringToken("channelid", channelID);
    header->AddIntToken("channellogoexists", header->ChannelLogoExists(channelID));

    bool isRunning = false;
    time_t now = time(NULL);
    if ((now >= event->StartTime()) && (now <= event->EndTime()))
        isRunning = true;
    header->AddIntToken("running", isRunning);
    if (isRunning) {
        header->AddIntToken("elapsed", (now - event->StartTime())/60);
    } else {
        header->AddIntToken("elapsed", 0);
    }
    header->AddIntToken("duration", event->Duration() / 60);
    header->AddIntToken("durationhours", event->Duration() / 3600);
    header->AddStringToken("durationminutes", *cString::sprintf("%.2d", (event->Duration() / 60)%60));
    if (event->Vps())
        header->AddStringToken("vps", *event->GetVpsString());
    else
        header->AddStringToken("vps", "");

    stringstream epgImageName;
    epgImageName << event->EventID();
    string epgImagePath = header->GetEpgImagePath();

    bool epgPicAvailable = FileExists(epgImagePath, epgImageName.str(), "jpg");
    if (epgPicAvailable) {
        header->AddIntToken("epgpicavailable", true);
        header->AddStringToken("epgpicpath", *cString::sprintf("%s%s.jpg", epgImagePath.c_str(), epgImageName.str().c_str()));
    } else {
        epgImageName << "_0";
        epgPicAvailable = FileExists(epgImagePath, epgImageName.str(), "jpg");
        if (epgPicAvailable) {
            header->AddIntToken("epgpicavailable", true);
            header->AddStringToken("epgpicpath", *cString::sprintf("%s%s.jpg", epgImagePath.c_str(), epgImageName.str().c_str()));
        } else {
            header->AddIntToken("epgpicavailable", false);
            header->AddStringToken("epgpicpath", "");                
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

    footer->AddStringToken("red", textRed);
    footer->AddStringToken("green", textGreen);
    footer->AddStringToken("yellow", textYellow);
    footer->AddStringToken("blue", textBlue);

    for (int button = 1; button < 5; button++) {
        string red = *cString::sprintf("red%d", button);
        string green = *cString::sprintf("green%d", button);
        string yellow = *cString::sprintf("yellow%d", button);
        string blue = *cString::sprintf("blue%d", button);
        bool isRed = false;
        bool isGreen = false;
        bool isYellow = false;
        bool isBlue = false;
        switch (colorKeys[button-1]) {
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
        footer->AddIntToken(red, isRed);
        footer->AddIntToken(green, isGreen);
        footer->AddIntToken(yellow, isYellow);
        footer->AddIntToken(blue, isBlue);
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
    watch->AddIntToken("sec", sec);
    watch->AddIntToken("min", min);
    watch->AddIntToken("hour", hour);
    watch->AddIntToken("hmins", hourMinutes);
    watch->AddIntToken("year", now->tm_year + 1900);
    watch->AddIntToken("day", now->tm_mday);
    watch->AddStringToken("time", *TimeString(t));
    watch->AddStringToken("monthname", monthname);
    watch->AddStringToken("monthnameshort", monthshort);
    watch->AddStringToken("month", *cString::sprintf("%02d", now->tm_mon + 1));
    watch->AddStringToken("dayleadingzero", *cString::sprintf("%02d", now->tm_mday));
    watch->AddStringToken("dayname", *WeekDayNameFull(now->tm_wday));
    watch->AddStringToken("daynameshort", *WeekDayName(now->tm_wday));
    watch->Display();

    lastSecond = sec;
    return true;
}

void cDetailView::SetTabTokens(void) {
    tabs->ClearTokens();

    tabs->AddStringToken("title", event->Title() ? event->Title() : "");
    tabs->AddStringToken("shorttext", event->ShortText() ? event->ShortText() : "");
    tabs->AddStringToken("description", event->Description() ? event->Description() : "");
    tabs->AddStringToken("start", *(event->GetTimeString()));
    tabs->AddStringToken("stop", *(event->GetEndTimeString()));
    time_t startTime = event->StartTime();
    tabs->AddStringToken("day", *WeekDayName(startTime));
    tabs->AddStringToken("date", *ShortDateString(startTime));
    struct tm * sStartTime = localtime(&startTime);
    tabs->AddIntToken("year", sStartTime->tm_year + 1900);
    tabs->AddIntToken("daynumeric", sStartTime->tm_mday);
    tabs->AddIntToken("month", sStartTime->tm_mon+1);

    string channelID = *(event->ChannelID().ToString());
    tabs->AddStringToken("channelid", channelID);
    tabs->AddIntToken("channellogoexists", tabs->ChannelLogoExists(channelID));

    bool isRunning = false;
    time_t now = time(NULL);
    if ((now >= event->StartTime()) && (now <= event->EndTime()))
        isRunning = true;
    tabs->AddIntToken("running", isRunning);
    if (isRunning) {
        tabs->AddIntToken("elapsed", (now - event->StartTime())/60);
    } else {
        tabs->AddIntToken("elapsed", 0);
    }
    tabs->AddIntToken("duration", event->Duration() / 60);    
    tabs->AddIntToken("durationhours", event->Duration() / 3600);
    tabs->AddStringToken("durationminutes", *cString::sprintf("%.2d", (event->Duration() / 60)%60));
    if (event->Vps())
        tabs->AddStringToken("vps", *event->GetVpsString());
    else
        tabs->AddStringToken("vps", "");


    bool hasReruns = LoadReruns();
    tabs->AddIntToken("hasreruns", hasReruns);   

    SetScraperTokens();
    SetEpgPictures(event->EventID());
}

bool cDetailView::LoadReruns(void) {
    if (!event)
        return false;
    
    cPlugin *epgSearchPlugin = cPluginManager::GetPlugin("epgsearch");
    if (!epgSearchPlugin)
        return false;

    if (isempty(event->Title()))
        return false;

    int maxNumReruns = config.rerunAmount;
    int rerunDistance = config.rerunDistance * 3600;
    int rerunNaxChannel = config.rerunMaxChannel;

    Epgsearch_searchresults_v1_0 data;
    string strQuery = (event->Title()) ? event->Title() : "";
    data.query = (char *)strQuery.c_str();
    data.mode = 0;
    data.channelNr = 0;
    data.useTitle = true;
    data.useSubTitle = true;
    data.useDescription = false;

    bool foundRerun = false;
    if (epgSearchPlugin->Service("Epgsearch-searchresults-v1.0", &data)) {
        cList<Epgsearch_searchresults_v1_0::cServiceSearchResult>* list = data.pResultList;
        if (list && (list->Count() > 1)) {
            foundRerun = true;
            int i = 0;
            for (Epgsearch_searchresults_v1_0::cServiceSearchResult *r = list->First(); r && i < maxNumReruns; r = list->Next(r)) {
                time_t eventStart = event->StartTime();
                time_t rerunStart = r->event->StartTime();
                cChannel *channel = Channels.GetByChannelID(r->event->ChannelID(), true, true);
                //check for identical event
                if ((event->ChannelID() == r->event->ChannelID()) && (eventStart == rerunStart))
                    continue;
                //check for timely distance
                if (rerunDistance > 0) {
                    if (rerunStart - eventStart < rerunDistance) {
                        continue;
                    }
                }
                //check for maxchannel
                if (rerunNaxChannel > 0) {
                    if (channel && channel->Number() > rerunNaxChannel) {
                        continue;
                    }
                }
                i++;
                map< string, string > rerun;
                rerun.insert(pair<string, string>("reruns[title]", r->event->Title() ? r->event->Title() : ""));
                rerun.insert(pair<string, string>("reruns[shorttext]", r->event->ShortText() ? r->event->ShortText() : ""));
                rerun.insert(pair<string, string>("reruns[start]", *(r->event->GetTimeString())));
                rerun.insert(pair<string, string>("reruns[start]", *(r->event->GetTimeString())));
                rerun.insert(pair<string, string>("reruns[stop]", *(r->event->GetEndTimeString())));
                rerun.insert(pair<string, string>("reruns[date]", *ShortDateString(r->event->StartTime())));
                rerun.insert(pair<string, string>("reruns[day]", *WeekDayName(r->event->StartTime())));
                string channelID = *(r->event->ChannelID().ToString());
                rerun.insert(pair<string, string>("reruns[channelid]", channelID));
                bool logoExists = tabs->ChannelLogoExists(channelID);
                rerun.insert(pair<string, string>("reruns[channellogoexists]", logoExists ? "1" : "0"));

                if (channel) {
                    stringstream channelNumber;
                    channelNumber << channel->Number();
                    rerun.insert(pair<string, string>("reruns[channelname]", channel->ShortName(true)));
                    rerun.insert(pair<string, string>("reruns[channelnumber]", channelNumber.str()));
                } else {
                    rerun.insert(pair<string, string>("reruns[channelname]", ""));
                    rerun.insert(pair<string, string>("reruns[channelnumber]", ""));                    
                }
                tabs->AddLoopToken("reruns", rerun);
            }
            delete list;
        }
    }
    return foundRerun;
}

void cDetailView::SetScraperTokens(void) {
    static cPlugin *pScraper = GetScraperPlugin();
    if (!pScraper || !event) {
        tabs->AddIntToken("ismovie", false);
        tabs->AddIntToken("isseries", false);
        return;
    }

    ScraperGetEventType getType;
    getType.event = event;
    getType.recording = NULL;
    if (!pScraper->Service("GetEventType", &getType)) {
        tabs->AddIntToken("ismovie", false);
        tabs->AddIntToken("isseries", false);
        return;
    }

    if (getType.type == tMovie) {
        cMovie movie;
        movie.movieId = getType.movieId;
        pScraper->Service("GetMovie", &movie);
        tabs->AddIntToken("ismovie", true);
        tabs->AddIntToken("isseries", false);

        tabs->AddStringToken("movietitle", movie.title);
        tabs->AddStringToken("movieoriginalTitle", movie.originalTitle);
        tabs->AddStringToken("movietagline", movie.tagline);
        tabs->AddStringToken("movieoverview", movie.overview);
        tabs->AddStringToken("moviegenres", movie.genres);
        tabs->AddStringToken("moviehomepage", movie.homepage);
        tabs->AddStringToken("moviereleasedate", movie.releaseDate);
        stringstream pop;
        pop << movie.popularity;
        tabs->AddStringToken("moviepopularity", pop.str());
        stringstream vote;
        vote << movie.voteAverage;
        tabs->AddStringToken("movievoteaverage", pop.str());
        tabs->AddStringToken("posterpath", movie.poster.path);
        tabs->AddStringToken("fanartpath", movie.fanart.path);
        tabs->AddStringToken("collectionposterpath", movie.collectionPoster.path);
        tabs->AddStringToken("collectionfanartpath", movie.collectionFanart.path);

        tabs->AddIntToken("movieadult", movie.adult);
        tabs->AddIntToken("moviebudget", movie.budget);
        tabs->AddIntToken("movierevenue", movie.revenue);
        tabs->AddIntToken("movieruntime", movie.runtime);
        tabs->AddIntToken("posterwidth", movie.poster.width);
        tabs->AddIntToken("posterheight", movie.poster.height);
        tabs->AddIntToken("fanartwidth", movie.fanart.width);
        tabs->AddIntToken("fanartheight", movie.fanart.height);
        tabs->AddIntToken("collectionposterwidth", movie.collectionPoster.width);
        tabs->AddIntToken("collectionposterheight", movie.collectionPoster.height);
        tabs->AddIntToken("collectionfanartwidth", movie.collectionFanart.width);
        tabs->AddIntToken("collectionfanartheight", movie.collectionFanart.height);

        for (vector<cActor>::iterator act = movie.actors.begin(); act != movie.actors.end(); act++) {
            map< string, string > actor;
            actor.insert(pair<string, string>("actors[name]", (*act).name));
            actor.insert(pair<string, string>("actors[role]", (*act).role));
            actor.insert(pair<string, string>("actors[thumb]", (*act).actorThumb.path));
            stringstream actWidth, actHeight;
            actWidth << (*act).actorThumb.width;
            actHeight << (*act).actorThumb.height;
            actor.insert(pair<string, string>("actors[thumbwidth]", actWidth.str()));
            actor.insert(pair<string, string>("actors[thumbheight]", actHeight.str()));
            tabs->AddLoopToken("actors", actor);
        }

    } else if (getType.type == tSeries) {
        cSeries series;
        series.seriesId = getType.seriesId;
        series.episodeId = getType.episodeId;
        pScraper->Service("GetSeries", &series);
        tabs->AddIntToken("ismovie", false);
        tabs->AddIntToken("isseries", true);
        //Series Basics
        tabs->AddStringToken("seriesname", series.name);
        tabs->AddStringToken("seriesoverview", series.overview);
        tabs->AddStringToken("seriesfirstaired", series.firstAired);
        tabs->AddStringToken("seriesnetwork", series.network);
        tabs->AddStringToken("seriesgenre", series.genre);
        stringstream rating;
        rating << series.rating;
        tabs->AddStringToken("seriesrating", rating.str());
        tabs->AddStringToken("seriesstatus", series.status);
        //Episode Information
        tabs->AddIntToken("episodenumber", series.episode.number);
        tabs->AddIntToken("episodeseason", series.episode.season);
        tabs->AddStringToken("episodetitle", series.episode.name);
        tabs->AddStringToken("episodefirstaired", series.episode.firstAired);
        tabs->AddStringToken("episodegueststars", series.episode.guestStars);
        tabs->AddStringToken("episodeoverview", series.episode.overview);
        stringstream eprating;
        eprating << series.episode.rating;
        tabs->AddStringToken("episoderating", eprating.str());
        tabs->AddIntToken("episodeimagewidth", series.episode.episodeImage.width);
        tabs->AddIntToken("episodeimageheight", series.episode.episodeImage.height);
        tabs->AddStringToken("episodeimagepath", series.episode.episodeImage.path);
        //Seasonposter
        tabs->AddIntToken("seasonposterwidth", series.seasonPoster.width);
        tabs->AddIntToken("seasonposterheight", series.seasonPoster.height);
        tabs->AddStringToken("seasonposterpath", series.seasonPoster.path);

        //Posters
        int current = 1;
        for(vector<cTvMedia>::iterator poster = series.posters.begin(); poster != series.posters.end(); poster++) {
            stringstream labelWidth, labelHeight, labelPath;
            labelWidth << "seriesposter" << current << "width";
            labelHeight << "seriesposter" << current << "height";
            labelPath << "seriesposter" << current << "path";

            tabs->AddIntToken(labelWidth.str(), (*poster).width);
            tabs->AddIntToken(labelHeight.str(), (*poster).height);
            tabs->AddStringToken(labelPath.str(), (*poster).path);
            current++;
        }
        if (current < 3) {
            for (; current < 4; current++) {
                stringstream labelWidth, labelHeight, labelPath;
                labelWidth << "seriesposter" << current << "width";
                labelHeight << "seriesposter" << current << "height";
                labelPath << "seriesposter" << current << "path";
                
                tabs->AddIntToken(labelWidth.str(), 0);
                tabs->AddIntToken(labelHeight.str(), 0);
                tabs->AddStringToken(labelPath.str(), "");
            }
        }

        //Banners
        current = 1;
        for(vector<cTvMedia>::iterator banner = series.banners.begin(); banner != series.banners.end(); banner++) {
            stringstream labelWidth, labelHeight, labelPath;
            labelWidth << "seriesbanner" << current << "width";
            labelHeight << "seriesbanner" << current << "height";
            labelPath << "seriesbanner" << current << "path";
            
            tabs->AddIntToken(labelWidth.str(), (*banner).width);
            tabs->AddIntToken(labelHeight.str(), (*banner).height);
            tabs->AddStringToken(labelPath.str(), (*banner).path);
            current++;
        }
        if (current < 3) {
            for (; current < 4; current++) {
                stringstream labelWidth, labelHeight, labelPath;
                labelWidth << "seriesbanner" << current << "width";
                labelHeight << "seriesbanner" << current << "height";
                labelPath << "seriesbanner" << current << "path";
                
                tabs->AddIntToken(labelWidth.str(), 0);
                tabs->AddIntToken(labelHeight.str(), 0);
                tabs->AddStringToken(labelPath.str(), "");
            }
        }
        
        //Fanarts
        current = 1;
        for(vector<cTvMedia>::iterator fanart = series.fanarts.begin(); fanart != series.fanarts.end(); fanart++) {
            stringstream labelWidth, labelHeight, labelPath;
            labelWidth << "seriesfanart" << current << "width";
            labelHeight << "seriesfanart" << current << "height";
            labelPath << "seriesfanart" << current << "path";
            
            tabs->AddIntToken(labelWidth.str(), (*fanart).width);
            tabs->AddIntToken(labelHeight.str(), (*fanart).height);
            tabs->AddStringToken(labelPath.str(), (*fanart).path);
            current++;
        }
        if (current < 3) {
            for (; current < 4; current++) {
                stringstream labelWidth, labelHeight, labelPath;
                labelWidth << "seriesfanart" << current << "width";
                labelHeight << "seriesfanart" << current << "height";
                labelPath << "seriesfanart" << current << "path";
                
                tabs->AddIntToken(labelWidth.str(), 0);
                tabs->AddIntToken(labelHeight.str(), 0);
                tabs->AddStringToken(labelPath.str(), "");
            }
        }
        
        //Actors
        for (vector<cActor>::iterator act = series.actors.begin(); act != series.actors.end(); act++) {
            map< string, string > actor;
            actor.insert(pair<string, string>("actors[name]", (*act).name));
            actor.insert(pair<string, string>("actors[role]", (*act).role));
            actor.insert(pair<string, string>("actors[thumb]", (*act).actorThumb.path));
            stringstream actWidth, actHeight;
            actWidth << (*act).actorThumb.width;
            actHeight << (*act).actorThumb.height;
            actor.insert(pair<string, string>("actors[thumbwidth]", actWidth.str()));
            actor.insert(pair<string, string>("actors[thumbheight]", actHeight.str()));
            tabs->AddLoopToken("actors", actor);
        }

    } else {
        tabs->AddIntToken("ismovie", false);
        tabs->AddIntToken("isseries", false);
    }
}

void cDetailView::SetEpgPictures(int eventId) {
    string epgImagePath = tabs->GetEpgImagePath();
    for (int i=0; i<3; i++) {
        stringstream picName;
        picName << eventId << "_" << i;
        bool epgPicAvailable = FileExists(epgImagePath, picName.str(), "jpg");
        stringstream available;
        stringstream path;
        available << "epgpic" << i+1 << "avaialble";
        path << "epgpic" << i+1 << "path";
        if (epgPicAvailable) {
            tabs->AddIntToken(available.str(), true);
            tabs->AddStringToken(path.str(), *cString::sprintf("%s%s.jpg", epgImagePath.c_str(), picName.str().c_str()));
        } else {
            tabs->AddIntToken(available.str(), false);
            tabs->AddStringToken(path.str(), "");            
        }
    }
}