#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <vdr/channels.h>
#include <vdr/device.h>
#include "helpers.h"
#include "searchtimer.h"

// -- cTVGuideSearchTimer -----------------------------------------------------------------
cTVGuideSearchTimer::cTVGuideSearchTimer(void) {
    strTimer = "";
    ID = -1;
    searchString = "";
    useTime = false;
    startTime = 0000;
    stopTime = 2359;
    useChannel = false;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    channelMin = Channels->GetByNumber(cDevice::CurrentChannel());
    channelMax = Channels->GetByNumber(cDevice::CurrentChannel());
#else
    channelMin = Channels.GetByNumber(cDevice::CurrentChannel());
    channelMax = Channels.GetByNumber(cDevice::CurrentChannel());
#endif
    channelGroup = "";
    useCase = false;
    mode = 0;
    useTitle = true;
    useSubtitle = true;
    useDescription = true;
    useDuration = false;
    minDuration = 0;
    maxDuration = 2359;
    useAsSearchTimer = true;
    useDayOfWeek = false;
    dayOfWeek = 0;
    directory = "";
    useEpisode = 0;
    priority = 99;
    lifetime = 99;
    marginStart = 5;
    marginStop = 5;
    useVPS = false;
    action = 0;
    useExtEPGInfo = 0;
    extEPGInfoValues = "";
    avoidRepeats = 1;
    allowedRepeats = 1;
    compareTitle = 1;
    compareSubtitle = 2;
    compareSummary = 1;
    catvaluesAvoidRepeat = 0;
    repeatsWithinDays = 0;
    delAfterDays = 0;
    recordingsKeep = 0;
    switchMinsBefore = 0;
    pauseOnNrRecordings = 0;
    blacklistMode = 0;
    blacklists = "";
    fuzzyTolerance = 0;
    useInFavorites = 0;
    menuTemplate = 0;
    delMode = 0;
    delAfterCountRecs = 0;
    delAfterDaysOfFirstRec = 0;
    useAsSearchTimerFrom = 0;
    useAsSearchTimerTil = 0;
    ignoreMissingEPGCats = 0;
    unmuteSoundOnSwitch = 0;
    compareSummaryMatchInPercent = 0;
    contentsFilter = "";
    compareDate = 0;
}

cTVGuideSearchTimer::~cTVGuideSearchTimer(void) {
}

bool cTVGuideSearchTimer::operator < (const cTVGuideSearchTimer& other) const {
    std::string searchStringOther = other.SearchString();
    searchStringOther = StrToLowerCase(searchStringOther);
    std::string thisSearchString = StrToLowerCase(searchString);
    int comp = thisSearchString.compare(searchStringOther);
    if (comp < 0)
        return true;
    return false;
}


void cTVGuideSearchTimer::SetTemplate(std::string tmpl) {
    std::stringstream searchTimerString;
    searchTimerString << "0:";
    searchTimerString << tmpl;
    strTimer = searchTimerString.str();
}

int cTVGuideSearchTimer::DayOfWeek(void) {
    int vdrDayOfWeek = 0;
    if (dayOfWeek >= 0) {
        vdrDayOfWeek = pow(2, (dayOfWeek+6)%7);
    } else if (dayOfWeek < 0) {
        int absDayOfWeek = abs(dayOfWeek);
        for (int i=0; i < 7; i++) {
            if (absDayOfWeek & (1 << i)) {
                vdrDayOfWeek += pow(2, (i+6)%7);
            }
        }
    }
    return vdrDayOfWeek; 
}

void cTVGuideSearchTimer::SetDayOfWeek(int VDRDayOfWeek) { 
    int epgSearchDayOfWeek = 0;
    for (int i=0; i < 7; i++) {
        if (VDRDayOfWeek & (1 << i)) {
            epgSearchDayOfWeek += pow(2, (i+1)%7);
        }
    }
    this->dayOfWeek = epgSearchDayOfWeek * (-1);
}

/*
    0 - unique search timer id
    1 - the search term
    2 - use time? 0/1
    3 - start time in HHMM
    4 - stop time in HHMM
    5 - use channel? 0 = no,  1 = Interval, 2 = Channel group, 3 = FTA only
    6 - if 'use channel' = 1 then channel id[|channel id] in VDR format,
        one entry or min/max entry separated with |, if 'use channel' = 2
        then the channel group name
    7 - match case? 0/1
    8 - search mode:
        0 - the whole term must appear as substring
        1 - all single terms (delimiters are blank,',', ';', '|' or '~')
            must exist as substrings.
        2 - at least one term (delimiters are blank, ',', ';', '|' or '~')
            must exist as substring.
        3 - matches exactly
        4 - regular expression   
    9 - use title? 0/1
    10 - use subtitle? 0/1
    11 - use description? 0/1
    12 - use duration? 0/1
    13 - min duration in hhmm
    14 - max duration in hhmm
    15 - use as search timer? 0/1
    16 - use day of week? 0/1
    17 - day of week (0 = Sunday, 1 = Monday...;
         -1 Sunday, -2 Monday, -4 Tuesday, ...; -7 Sun, Mon, Tue)
    18 - use series recording? 0/1
    19 - directory for recording
    20 - priority of recording
    21 - lifetime of recording
    22 - time margin for start in minutes
    23 - time margin for stop in minutes
    24 - use VPS? 0/1
    25 - action:
         0 = create a timer
         1 = announce only via OSD (no timer)
         2 = switch only (no timer)
         3 = announce via OSD and switch (no timer)
         4 = announce via mail
    26 - use extended EPG info? 0/1
    27 - extended EPG info values. This entry has the following format
         (delimiter is '|' for each category, '#' separates id and value):
         1 - the id of the extended EPG info category as specified in
             epgsearchcats.conf
         2 - the value of the extended EPG info category
             (a ':' will be translated to "!^colon^!", e.g. in "16:9")
    28 - avoid repeats? 0/1
    29 - allowed repeats
    30 - compare title when testing for a repeat? 0/1
    31 - compare subtitle when testing for a repeat? 0/1/2
         0 - no
         1 - yes
         2 - yes, if present
    32 - compare description when testing for a repeat? 0/1
    33 - compare extended EPG info when testing for a repeat?
         This entry is a bit field of the category IDs.
    34 - accepts repeats only within x days
    35 - delete a recording automatically after x days
    36 - but keep this number of recordings anyway
    37 - minutes before switch (if action = 2)
    38 - pause if x recordings already exist
    39 - blacklist usage mode (0 none, 1 selection, 2 all)
    40 - selected blacklist IDs separated with '|'
    41 - fuzzy tolerance value for fuzzy searching
    42 - use this search in favorites menu (0 no, 1 yes)
    43 - id of a menu search template
    44 - auto deletion mode (0 don't delete search timer, 1 delete after given
         count of recordings, 2 delete after given days after first recording)
    45 - count of recordings after which to delete the search timer
    46 - count of days after the first recording after which to delete the search
         timer
    47 - first day where the search timer is active (see parameter 16)
    48 - last day where the search timer is active (see parameter 16)
    49 - ignore missing EPG categories? 0/1
    50 - unmute sound if off when used as switch timer
    51 - percentage of match when comparing the summary of two events (with 'avoid repeats')
    52 - HEX representation of the content descriptors, each descriptor ID is represented with 2 chars
    53 - compare date when testing for a repeat? (0=no, 1=same day, 2=same week, 3=same month)
*/
bool cTVGuideSearchTimer::Parse(bool readTemplate) {
    splitstring s(strTimer.c_str());
    std::vector<std::string> values = s.split(':', 1);
    int numValues = values.size();
    if (numValues < 12)
        return false;
    for (int value = 0; value < numValues; value++) {
    	switch (value) {
    		case 0:
                if (!readTemplate)
                    ID = atoi(values[value].c_str());
                break;
    		case 1:
                if (!readTemplate) {
                    std::string searchStringMasked = values[value];
                    std::replace(searchStringMasked.begin(), searchStringMasked.end(), '|', ':');
                    searchString = searchStringMasked;
                }
                break;
    		case 2:
                useTime = atoi(values[value].c_str());
                break;
            case 3:
                if (useTime) {
				    startTime = atoi(values[value].c_str());
				}
				break;
            case 4:
                if (useTime) {
				    stopTime = atoi(values[value].c_str());
				}
				break;
    		case 5:
                useChannel = atoi(values[value].c_str());
                break;
            case 6:
                if (useChannel == 0) {
                    channelMin = NULL;
                    channelMax = NULL;
                } else if (useChannel == 1) {
                    char *channelMinbuffer = NULL;
                    char *channelMaxbuffer = NULL;
                    int channels = sscanf(values[value].c_str(), "%m[^|]|%m[^|]", &channelMinbuffer, &channelMaxbuffer);
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
                    LOCK_CHANNELS_READ;
		            const cChannels* channelsP = Channels;
#else
                    cChannels* channelsP = &Channels;
#endif
		            channelMin = channelsP->GetByChannelID(tChannelID::FromString(channelMinbuffer), true, true);
                    if (!channelMin) {
                    	channelMin = channelMax = NULL;
                        useChannel = 0;
                    }
                    if (channels == 1)
                        channelMax = channelMin;
                    else {
                        channelMax = channelsP->GetByChannelID(tChannelID::FromString(channelMaxbuffer), true, true);
                        if (!channelMax) {
                            channelMin = channelMax = NULL;
                            useChannel = 0;
                        }
                    }
                    free(channelMinbuffer);
                    free(channelMaxbuffer);
                } else if (useChannel == 2) {
                	channelGroup = values[value];
                }
                break;
           	case 7:
           		useCase = atoi(values[value].c_str());
                break;
            case 8:  
                mode = atoi(values[value].c_str());
                break;
            case 9: 
                useTitle = atoi(values[value].c_str());
                break;
            case 10: 
                useSubtitle = atoi(values[value].c_str());
                break;
            case 11: 
                useDescription = atoi(values[value].c_str());
                break;
            case 12: 
                useDuration = atoi(values[value].c_str());
                break;
            case 13: 
                minDuration = atoi(values[value].c_str());
                break;
            case 14: 
                maxDuration = atoi(values[value].c_str());
                break;
            case 15: 
                useAsSearchTimer = atoi(values[value].c_str());
                break;
            case 16: 
                useDayOfWeek = atoi(values[value].c_str());
                break;
            case 17: 
                dayOfWeek = atoi(values[value].c_str());
                break;
            case 18: 
                useEpisode = atoi(values[value].c_str());
                break;
            case 19:  
                directory = values[value];
                break;
            case 20: 
                priority = atoi(values[value].c_str());
                break;
            case 21: 
                lifetime = atoi(values[value].c_str());
                break;
            case 22: 
                marginStart = atoi(values[value].c_str());
                break;
            case 23: 
                marginStop = atoi(values[value].c_str());
                break;
            case 24: 
                useVPS = atoi(values[value].c_str());
                break;
            case 25: 
                action = atoi(values[value].c_str());
                break;
            case 26: 
                useExtEPGInfo = atoi(values[value].c_str());
                break;
            case 27: 
                extEPGInfoValues = values[value];
                break;
            case 28: 
                avoidRepeats = atoi(values[value].c_str());
                break;
            case 29: 
                allowedRepeats = atoi(values[value].c_str());
                break;
            case 30: 
                compareTitle = atoi(values[value].c_str());
                break;
            case 31: 
                compareSubtitle = atoi(values[value].c_str());
                break;
            case 32: 
                compareSummary = atoi(values[value].c_str());
                break;
            case 33: 
                catvaluesAvoidRepeat = atol(values[value].c_str());
                break;
            case 34: 
                repeatsWithinDays = atoi(values[value].c_str());
                break;
            case 35: 
                delAfterDays = atoi(values[value].c_str());
                break;
            case 36:  
                recordingsKeep = atoi(values[value].c_str());
                break;
            case 37: 
                switchMinsBefore = atoi(values[value].c_str());
                break;
            case 38: 
                pauseOnNrRecordings = atoi(values[value].c_str());
                break;
            case 39: 
                blacklistMode = atoi(values[value].c_str());
                break;
            case 40:
                blacklists = values[value];
                break;
            case 41: 
                fuzzyTolerance = atoi(values[value].c_str());
                break;
            case 42: 
                useInFavorites = atoi(values[value].c_str());
                break;
            case 43: 
                menuTemplate = atoi(values[value].c_str());
                break;
            case 44: 
                delMode = atoi(values[value].c_str());
                break;
            case 45: 
                delAfterCountRecs = atoi(values[value].c_str());
                break;
            case 46: 
                delAfterDaysOfFirstRec = atoi(values[value].c_str());
                break;
            case 47:
                useAsSearchTimerFrom = atol(values[value].c_str());
                break;
            case 48:
                useAsSearchTimerTil = atol(values[value].c_str());
                break;
            case 49:
                ignoreMissingEPGCats = atoi(values[value].c_str());
                break;
            case 50:
                unmuteSoundOnSwitch = atoi(values[value].c_str());
                break;
            case 51:
                compareSummaryMatchInPercent = atoi(values[value].c_str());
                break;
            case 52:
                contentsFilter = values[value];
                break;
            case 53:
                compareDate = atoi(values[value].c_str());
                break;
            default:
                break;
        }
    }
    return true;
}

std::string cTVGuideSearchTimer::BuildSearchString(void) {
    std::stringstream search;
    // 0 - 2
    if (ID > -1)
        search << ID << ":";
    else
        search << ":";
    std::string searchStringMasked = searchString;
    std::replace(searchStringMasked.begin(), searchStringMasked.end(), ':', '|');
    search << searchStringMasked << ":"; 
    search << useTime << ":";
    
    // 3 - 6
    if (useTime) {
        search << *cString::sprintf("%04d", startTime) << ":";
        search << *cString::sprintf("%04d", stopTime) << ":";
    } else {
        search << "::";
    }

    search << useChannel << ":";
    if (useChannel == 1) {
        if (channelMin && channelMax) {
            if (channelMin->Number() < channelMax->Number())
                search << std::string(channelMin->GetChannelID().ToString()) << "|" << std::string(channelMax->GetChannelID().ToString()) << ":";
            else
                search << std::string(channelMin->GetChannelID().ToString()) << ":";
        } else {
            search << "0:";
        }
    } else if (useChannel == 2) {
        search << channelGroup << ":";
    } else {
        search << "0:";
    }
    // 7 - 14
    search << useCase << ":";
    search << mode << ":";
    search << useTitle << ":";
    search << useSubtitle << ":";
    search << useDescription << ":";
    search << useDuration << ":";
    if (useDuration) {
        search << *cString::sprintf("%04d", minDuration) << ":";
        search << *cString::sprintf("%04d", maxDuration) << ":";
    } else {
        search << "::";
    }
    //15 - 53
    search << useAsSearchTimer << ":";
    search << useDayOfWeek << ":";
    search << dayOfWeek << ":";
    search << useEpisode << ":";
    search << directory << ":";
    search << priority << ":";
    search << lifetime << ":";
    search << marginStart << ":";
    search << marginStop << ":";
    search << useVPS << ":";
    search << action << ":";
    search << useExtEPGInfo << ":";
    search << extEPGInfoValues << ":";
    search << avoidRepeats << ":";
    search << allowedRepeats << ":";
    search << compareTitle << ":";
    search << compareSubtitle << ":";
    search << compareSummary << ":";
    search << catvaluesAvoidRepeat << ":";
    search << repeatsWithinDays << ":";
    search << delAfterDays << ":";
    search << recordingsKeep << ":";
    search << switchMinsBefore << ":";
    search << pauseOnNrRecordings << ":";
    search << blacklistMode << ":";
    search << blacklists << ":";
    search << fuzzyTolerance << ":";
    search << useInFavorites << ":";
    search << menuTemplate << ":";
    search << delMode << ":";
    search << delAfterCountRecs << ":";
    search << delAfterDaysOfFirstRec << ":";
    search << useAsSearchTimerFrom << ":";
    search << useAsSearchTimerTil << ":";
    search << ignoreMissingEPGCats << ":";
    search << unmuteSoundOnSwitch << ":";
    search << compareSummaryMatchInPercent << ":";
    search << contentsFilter << ":";
    search << compareDate;

    strTimer = search.str();
    return strTimer;
}

bool cTVGuideSearchTimer::Active(void) {
    if (useAsSearchTimer)
        return true;
    return false;
}

void cTVGuideSearchTimer::SetStartChannel(int startChannel) { 
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    channelMin = Channels->GetByNumber(startChannel); 
#else
    channelMin = Channels.GetByNumber(startChannel); 
#endif
}

void cTVGuideSearchTimer::SetStopChannel(int stopChannel) { 
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    channelMax = Channels->GetByNumber(stopChannel); 
#else
    channelMax = Channels.GetByNumber(stopChannel); 
#endif
}

int cTVGuideSearchTimer::GetNumTimers(void) {
    int numTimers = 0;
    if (ID < 0)
    	return numTimers;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
     LOCK_TIMERS_READ;
     const cTimers* timers = Timers;
#else
     const cTimers* timers = &Timers;
#endif
    for (const cTimer *timer = timers->First(); timer; timer = timers->Next(timer)) {
        char* searchID = GetAuxValue(timer, "s-id");
        if (!searchID) continue;
      	if (ID == atoi(searchID))
        	numTimers++;
	    free(searchID);
    }
    return numTimers;
}

int cTVGuideSearchTimer::GetNumRecordings(void) {
    int numRecordings = 0;
    if (ID < 0)
        return numRecordings;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_RECORDINGS_READ;
    const cRecordings* recordings = Recordings;
#else
    cRecordings* recordings = &Recordings;
#endif
    for (const cRecording *recording = recordings->First(); recording; recording = recordings->Next(recording)) {
        if (recording->IsEdited()) 
            continue;
        if (!recording->Info()) 
            continue;
        char* searchID = GetAuxValue(recording, "s-id");
        if (!searchID) continue;
        if (ID == atoi(searchID))
            numRecordings++;
        free(searchID);
    }
    return numRecordings;
}

void cTVGuideSearchTimer::GetSearchModes(std::vector<std::string> *searchModes) {
    searchModes->push_back(tr("whole term must appear"));
    searchModes->push_back(tr("all terms must exist"));
    searchModes->push_back(tr("one term must exist"));
    searchModes->push_back(tr("exact match"));
    searchModes->push_back(tr("regular expression"));
}


void cTVGuideSearchTimer::Dump(void) {
    esyslog("tvguide searchtimer: strTimer: %s", strTimer.c_str());
    esyslog("tvguide searchtimer: ID: %d", ID);
    esyslog("tvguide searchtimer: searchString: %s", searchString.c_str());
    esyslog("tvguide searchtimer: useTime: %d", useTime);
    esyslog("tvguide searchtimer: startTime: %d", startTime);
    esyslog("tvguide searchtimer: stopTime: %d", stopTime);
    esyslog("tvguide searchtimer: useChannel: %d", useChannel);
    if (channelMin)
	    esyslog("tvguide searchtimer: channelMin: %s", channelMin->Name());
    if (channelMax)
	    esyslog("tvguide searchtimer: channelMax: %s", channelMax->Name());
    esyslog("tvguide searchtimer: channelGroup: %s", channelGroup.c_str());
    esyslog("tvguide searchtimer: useCase: %d", useCase);
    esyslog("tvguide searchtimer: mode: %d", mode);
    esyslog("tvguide searchtimer: useTitle: %d", useTitle);
    esyslog("tvguide searchtimer: useSubtitle: %d", useSubtitle);
    esyslog("tvguide searchtimer: useDescription: %d", useDescription);
}

