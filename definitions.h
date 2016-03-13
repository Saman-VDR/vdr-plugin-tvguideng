#ifndef __DEFINITIONS_H
#define __DEFINITIONS_H

/******************************************************************
* Menus
*******************************************************************/
enum class eViews {
    rootView = 0,
    detailView,
    recMenu,
    recMenu2,
    recMenu3,
    count
};

/******************************************************************
* Viewelements
*******************************************************************/
enum class eViewElementsRoot {
    backgroundHor = 0,
    backgroundVer,
    headerHor,
    headerVer,
    footerHor,
    footerVer,
    timeHor,
    timeVer,
    dateTimelineHor,
    dateTimelineVer,
    timeIndicatorHor,
    timeIndicatorVer,
    channelJump,
    count
};

enum class eViewElementsDetail {
    background = 0,
    header,
    footer,
    time,
    count
};

enum class eViewElementsRecMenu {
    background = 0,
    scrollbar,
    count
};

/******************************************************************
* Viewgrids
*******************************************************************/
enum class eViewGridsRoot {
    channelsHor = 0,
    channelsVer,
    schedulesHor,
    schedulesVer,
    channelGroupsHor,
    channelGroupsVer,
    timelineHor,
    timelineVer,
    count
};

enum class eViewGridsRecMenu {
    menu = 0,
    count
};

/******************************************************************
* Tokens Rootview Viewelements
*******************************************************************/
enum class eHeaderST {
    title = 0,
    shorttext,
    description,
    start,
    stop,
    day,
    date,
    durationminutes,
    channelname,
    channelid,
    posterpath,
    count
};

enum class eHeaderIT {
    isdummy = 0,
    daynumeric,
    month,
    year,
    running,
    elapsed,
    duration,
    durationhours,
    channelnumber,
    channellogoexists,
    hasposter,
    posterwidth,
    posterheight,
    count
};

enum class eFooterIT {
    red1 = 0, red2, red3, red4,
    green1, green2, green3, green4,
    yellow1, yellow2, yellow3, yellow4,
    blue1, blue2, blue3, blue4,
    count
};

enum class eFooterST {
    red = 0,
    green,
    yellow,
    blue,
    count
};

enum class eTimeST {
    time = 0,
    monthname,
    monthnameshort,
    month,
    dayleadingzero,
    dayname,
    daynameshort,
    count
};

enum class eTimeIT {
    sec = 0,
    min,
    hour,
    hmins,
    year,
    day,
    count
};

enum class eDateTimeST {
    weekday = 0,
    date,
    count
};

enum class eTimeIndicatorIT {
    percenttotal = 0,
    count
};

enum class eChannelJumpST {
    channel = 0,
    count
};

/******************************************************************
* Tokens Rootview Grids
*******************************************************************/
enum class eChannelGridST {
    name = 0,
    channelid,
    count
};

enum class eChannelGridIT {
    number = 0,
    channellogoexists,
    count
};

enum class eSchedulesGridST {
    title = 0,
    shorttext,
    start,
    stop,
    count
};

enum class eSchedulesGridIT {
    color = 0,
    dummy,
    timer,
    switchtimer,
    count
};

enum class eTimelineGridST {
    timestring = 0,
    count
};

enum class eTimelineGridIT {
    fullhour = 0,
    count
};

enum class eChannelgroupsGridST {
    group = 0,
    count
};

enum class eChannelgroupsGridIT {
    color = 0,
    count
};

/******************************************************************
* Tokens Detailview ViewElements
*******************************************************************/
enum class eScrollbarIT {
    height = 0,
    offset,
    count
};

enum class eScraperHeaderST {
    posterpath = 0,
    bannerpath,
    count
};

enum class eScraperHeaderIT {
    ismovie = 0,
    isseries,
    posteravailable,
    posterwidth,
    posterheight,
    banneravailable,
    bannerwidth,
    bannerheight,
    count
};

enum class eDetailedHeaderST {
    title = eScraperHeaderST::count,
    shorttext,
    start,
    stop,
    day,
    date,
    durationminutes,
    vps,
    channelname,
    channelid,
    epgpicpath,
    count
};

enum class eDetailedHeaderIT {
    daynumeric = eScraperHeaderIT::count,
    month,
    year,
    running,
    elapsed,
    duration,
    durationhours,
    channelnumber,
    channellogoexists,
    epgpicavailable,
    count
};

enum class eTabsIT {
    count = 0,
};

enum class eTabsST {
    currenttab = 0,
    prevtab,
    nexttab,
    count
};

enum class eTabsLT {
    title = 0,
    current,
    count
};

enum class eScraperST {
    movietitle = 0,
    movieoriginalTitle,
    movietagline,
    movieoverview,
    moviegenres,
    moviehomepage,
    moviereleasedate,
    moviepopularity,
    movievoteaverage,
    posterpath,
    fanartpath,
    moviecollectionName,
    collectionposterpath,
    collectionfanartpath,
    seriesname,
    seriesoverview,
    seriesfirstaired,
    seriesnetwork,
    seriesgenre,
    seriesrating,
    seriesstatus,
    episodetitle,
    episodefirstaired,
    episodegueststars,
    episodeoverview,
    episoderating,
    episodeimagepath,
    seasonposterpath,
    seriesposter1path,
    seriesposter2path,
    seriesposter3path,
    seriesfanart1path,
    seriesfanart2path,
    seriesfanart3path,
    seriesbanner1path,
    seriesbanner2path,
    seriesbanner3path,
    count
};

enum class eScraperIT {
    ismovie = 0,
    moviebudget,
    movierevenue,
    movieadult,
    movieruntime,
    isseries,
    posterwidth,
    posterheight,
    fanartwidth,
    fanartheight,
    movieiscollection,
    collectionposterwidth,
    collectionposterheight,
    collectionfanartwidth,
    collectionfanartheight,
    epgpicavailable,
    episodenumber,
    episodeseason,
    episodeimagewidth,
    episodeimageheight,
    seasonposterwidth,
    seasonposterheight,
    seriesposter1width,
    seriesposter1height,
    seriesposter2width,
    seriesposter2height,
    seriesposter3width,
    seriesposter3height,
    seriesfanart1width,
    seriesfanart1height,
    seriesfanart2width,
    seriesfanart2height,
    seriesfanart3width,
    seriesfanart3height,
    seriesbanner1width,
    seriesbanner1height,
    seriesbanner2width,
    seriesbanner2height,
    seriesbanner3width,
    seriesbanner3height,
    count
};

enum class eScraperLT {
    //actors
    name = 0,
    role,
    thumb,
    thumbwidth,
    thumbheight,
    count
};

enum class eDetailedEpgST {
    title = eScraperST::count,
    shorttext,
    description,
    start,
    stop,
    day,
    date,
    durationminutes,
    vps,
    channelname,
    channelid,
    epgpic1path,
    epgpic2path,
    epgpic3path,
    count
};

enum class eDetailedEpgIT {
    daynumeric = eScraperIT::count,
    month,
    year,
    running,
    elapsed,
    duration,
    durationhours,
    channelnumber,
    channellogoexists,
    hasreruns,
    epgpic1avaialble,
    epgpic2avaialble,
    epgpic3avaialble,
    count
};

enum class eRerunsLT {
    title = 0,
    shorttext,
    date,
    day,
    start,
    stop,
    channelname,
    channelnumber,
    channelid,
    channellogoexists,
    count
};

/******************************************************************
* Tokens RecMenu ViewElements
*******************************************************************/
enum class eBackgroundRecMenuIT {
    menuwidth = 0,
    menuheight,
    hasscrollbar,
    count
};

enum class eScrollbarRecMenuIT {
    menuwidth = 0,
    posy,
    totalheight,
    height,
    offset,
    count
};

enum class eRecMenuIT {
    info = 0,
    lines,
    button,
    buttonyesno,
    yes,
    intselector,
    value,
    boolselector,
    stringselector,
    textinput,
    editmode,
    timeselector,
    dayselector,
    channelselector,
    channelnumber,
    channellogoexisis,
    weekdayselector,
    dayselected,
    day0set,
    day1set,
    day2set,
    day3set,
    day4set,
    day5set,
    day6set,
    directoryselector,
    timerconflictheader,
    overlapstartpercent,
    overlapwidthpercent,
    timerconflict,
    infoactive,
    deleteactive,
    editactive,
    searchactive,
    timerstartpercent,
    timerwidthpercent,
    event,
    hastimer,
    recording,
    recduration,
    searchtimer,
    timeractive,
    activetimers,
    recordingsdone,
    timelineheader,
    timerset,
    channeltransponder,
    timelinetimer,
    timerstart,
    timerwidth,
    favorite,
    count
};

enum class eRecMenuST {
    line1 = 0,
    line2,
    line3,
    line4,
    value,
    buttontext,
    textyes,
    textno,
    text,
    channelname,
    channelid,
    transponder,
    day0abbr,
    day1abbr,
    day2abbr,
    day3abbr,
    day4abbr,
    day5abbr,
    day6abbr,
    folder,
    conflictstart,
    conflictstop,
    overlapstart,
    overlapstop,
    timertitle,
    starttime,
    stoptime,
    date,
    weekday,
    title,
    shorttext,
    recname,
    recstarttime,
    recdate,
    searchstring,
    timerstart,
    timerstop,
    eventtitle,
    eventshorttext,
    eventstart,
    eventstop,
    favdesc,
    count
};

#endif //__DEFINITIONS_H