#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vdr/osd.h>
#include <vdr/plugin.h>
#include <vdr/skins.h>
#include "services/epgsearch.h"
#include "helpers.h"

cPlugin *GetScraperPlugin(void) {
    static cPlugin *pScraper = cPluginManager::GetPlugin("scraper2vdr");
    if( !pScraper ) // if it doesn't exit, try tvscraper
        pScraper = cPluginManager::GetPlugin("tvscraper");
    return pScraper;
}

bool FileExists(const string &fullpath) {
    struct stat buffer;
    return (stat (fullpath.c_str(), &buffer) == 0); 
}

bool FileExists(const string &path, const string &name, const string &ext) {
    stringstream fileName;
    fileName << path << name << "." << ext;
    struct stat buffer;
    return (stat (fileName.str().c_str(), &buffer) == 0); 
}

/****************************************************************************************
*            STOPWATCH
****************************************************************************************/

cStopWatch::cStopWatch(const char* message) {
    start = cTimeMs::Now();
    last = start;
    if (message) {
        dsyslog("tvguideng: Starting StopWatch %s", message);        
    }
}

void cStopWatch::Report(const char* message) {
    dsyslog("tvguideng: %s - needed %d ms", message, (int)(cTimeMs::Now() - last));
    last = cTimeMs::Now();
}

void cStopWatch::Stop(const char* message) {
    dsyslog("tvguideng: %s - needed %d ms", message, (int)(cTimeMs::Now() - start));
}

/****************************************************************************************
*            CUTTEXT
****************************************************************************************/
std::string CutText(std::string text, int width, const cFont *font) {
    if (width <= font->Size())
        return text.c_str();
    if (font->Width(text.c_str()) < width)
        return text.c_str();
    cTextWrapper twText;
    twText.Set(text.c_str(), font, width);
    std::string cuttedTextNative = twText.GetLine(0);
    std::stringstream sstrText;
    sstrText << cuttedTextNative << "...";
    std::string cuttedText = sstrText.str();
    int actWidth = font->Width(cuttedText.c_str());
    if (actWidth > width) {
        int overlap = actWidth - width;
        int charWidth = font->Width(".");
        if (charWidth == 0)
            charWidth = 1;
        int cutChars = overlap / charWidth;
        if (cutChars > 0) {
            cuttedTextNative = cuttedTextNative.substr(0, cuttedTextNative.length() - cutChars);
            std::stringstream sstrText2;
            sstrText2 << cuttedTextNative << "...";
            cuttedText = sstrText2.str();
        }
    }
    return cuttedText;
}

/****************************************************************************************
*            StrToLowerCase
****************************************************************************************/
std::string StrToLowerCase(std::string str) {
    std::string lowerCase = str;
    const int length = lowerCase.length();
    for(int i=0; i < length; ++i) {
        lowerCase[i] = std::tolower(lowerCase[i]);
    }
    return lowerCase;
}

/****************************************************************************************
*            GetDirectoryFromTimer
****************************************************************************************/
std::string GetDirectoryFromTimer(std::string file) {
    std::string dir = "";
    size_t found = file.find_last_of('~');
    if (found != std::string::npos) {
        dir = file.substr(0, found);
    }
    return dir;
}

/****************************************************************************************
*            GetDirectoryFromTimer
****************************************************************************************/
void ReadRecordingDirectories(std::vector<std::string> *folders, cList<cNestedItem> *rootFolders, cString path) {
    cPlugin *epgSearchPlugin = NULL;
    epgSearchPlugin = cPluginManager::GetPlugin("epgsearch");
    if (epgSearchPlugin) {
        Epgsearch_services_v1_0 *epgSearch = new Epgsearch_services_v1_0;
        if (epgSearchPlugin->Service("Epgsearch-services-v1.0", epgSearch)) {
            std::set<std::string> epgSearchDirs = epgSearch->handler->DirectoryList();
            std::set<std::string>::iterator it;
            for (it = epgSearchDirs.begin(); it != epgSearchDirs.end(); it++) {
                std::string newFolder = *it;
                std::replace(newFolder.begin(), newFolder.end(), '/', '~');
                folders->push_back(newFolder);
            }
        }
    } else {
        cList<cNestedItem> *foldersLevel = NULL;
        if (rootFolders) {
            foldersLevel = rootFolders;
        } else {
            foldersLevel = &Folders;
        }
        for (cNestedItem *folder = foldersLevel->First(); folder; folder = foldersLevel->Next(folder)) {
            std::string strFolder = *cString::sprintf("%s%s", *path, folder->Text());
            std::replace(strFolder.begin(), strFolder.end(), '/', '~');
            folders->push_back(strFolder);
            cList<cNestedItem> *subItems = folder->SubItems();
            if (subItems) {
                std::string strFolder2 = *cString::sprintf("%s%s", *path, folder->Text());
                std::replace(strFolder2.begin(), strFolder2.end(), '/', '~');
                ReadRecordingDirectories(folders, subItems, strFolder2.c_str());
            }
        }
    }
}


/****************************************************************************************
*            DrawRoundedCorners
****************************************************************************************/
void DrawRoundedCorners(cPixmap *p, int posX, int posY, int width, int height, int radius, int borderWidth, tColor borderColor) {
    if( height > 2*radius) {
        p->DrawEllipse(cRect(posX, posY, radius, radius), borderColor, -2);
        p->DrawEllipse(cRect(posX - borderWidth, posY - borderWidth, radius, radius), clrTransparent, -2);

        p->DrawEllipse(cRect(posX+width - radius, posY, radius, radius), borderColor, -1);
        p->DrawEllipse(cRect(posX+width - radius + borderWidth, posY - borderWidth, radius, radius), clrTransparent, -1);
        
        p->DrawEllipse(cRect(posX, posY + height - radius, radius, radius), borderColor, -3);
        p->DrawEllipse(cRect(posX - borderWidth, posY + height - radius + borderWidth, radius, radius), clrTransparent, -3);
        
        p->DrawEllipse(cRect(posX + width - radius, posY + height - radius, radius, radius), borderColor, -4);
        p->DrawEllipse(cRect(posX + width - radius + borderWidth, posY + height - radius + borderWidth, radius, radius), clrTransparent, -4);
    }
}


/****************************************************************************************
*            SPLTSTRING
****************************************************************************************/
// split: receives a char delimiter; returns a vector of strings
// By default ignores repeated delimiters, unless argument rep == 1.
std::vector<std::string>& splitstring::split(char delim, int rep) {
    if (!flds.empty()) flds.clear();  // empty vector if necessary
    std::string work = data();
    std::string buf = "";
    int i = 0;
    while (i < (int)work.length()) {
        if (work[i] != delim)
            buf += work[i];
        else if (rep == 1) {
            flds.push_back(buf);
            buf = "";
        } else if (buf.length() > 0) {
            flds.push_back(buf);
            buf = "";
        }
        i++;
    }
    if (!buf.empty())
        flds.push_back(buf);
    return flds;
}

/****************************************************************************************
*            FINDIGNORECASE
****************************************************************************************/
int FindIgnoreCase(const std::string& expr, const std::string& query)
{
  const char *p = expr.c_str();
  const char *r = strcasestr(p, query.c_str());

  if (!r)
     return -1;
  return r - p;
}


/****************************************************************************************
*            GetAuxValue
****************************************************************************************/
char* GetAuxValue(const char* aux, const char* name) {
    if (isempty(aux))
        return NULL;
    
    char* descr = strdup(aux);
    char* beginaux = strstr(descr, "<epgsearch>");
    char* endaux = strstr(descr, "</epgsearch>");
    if (!beginaux || !endaux) {
        free(descr);
        return NULL;
    }

    beginaux +=  11;  // strlen("<epgsearch>");
    endaux[0] = 0;
    memmove(descr, beginaux, endaux - beginaux + 1);

    if (strcmp(name, "epgsearch") == 0) 
        return descr; // full aux

    int namelen = strlen(name);
    char catname[100] = "";
    catname[0] = '<';
    memcpy(catname + 1, name, namelen);
    catname[1 + namelen] = '>';
    catname[2 + namelen] = 0;

    char* cat = strcasestr(descr, catname);
    if (!cat) {
        free(descr);
        return NULL;
    }
      
    cat += namelen + 2;
    char* end = strstr(cat, "</");
    if (!end) {
        free(descr);    
        return NULL;
    }
    end[0] = 0;

    int catlen = end - cat + 1;
    char* value = (char *) malloc(catlen);
    memcpy(value, cat, catlen);

    free(descr);
    return value;
}

char* GetAuxValue(const cRecording *recording, const char* name) {
    if (!recording || !recording->Info()) 
        return NULL;
    return GetAuxValue(recording->Info()->Aux(), name);
}

char* GetAuxValue(const cTimer *timer, const char* name) {
    if (!timer || !timer->Aux()) 
        return NULL;
    return GetAuxValue(timer->Aux(), name);
}

/****************************************************************************************
*            FUZZYSEARCH
****************************************************************************************/

/******************************************************************************
FUNCTION afuzzy_init() 
    Initialization of the fuzzy search routine. This applies to the consequent
    calls of the afuzzy_CheckRTR (whole string matching) and afuzzy_CheckSUB
    (substring match) routines. afuzzy_init() should be called for each
    new pattern or error length. The search is case sensitive

ARGUMENTS: 
    p           Pattern
    kerr        Number of possible errors. Shouldn't exceed pattern length
    UseFilter   Use agrep filter algorithm that speeds up search.
    fuzzy       pointer to the structure that will be later passes to Check*
                    (the first 6 elements should be NULLs for the first call)
    
RETURN VALUE:
    none    

ALGORITHM
    see. the article on agrep algorithms.
    The only change is accounting transpositions as one edit operation .
******************************************************************************/
void afuzzy_init(const char *p, int kerr, int UseFilter, AFUZZY *fuzzy)
{
    int cnt, p_len, i, j, l, d, m, dd;
    char PatFilter[sizeof(Uint)*8 + 1];

    fuzzy->k = kerr;
    m = strlen(p);
    fuzzy->FilterSet = 0;
    memset(fuzzy->Map, 0 , sizeof(fuzzy->Map) );

    if (fuzzy->S)
        free(fuzzy->S);
    if (fuzzy->R)
        free(fuzzy->R);
    if (fuzzy->R1)
        free(fuzzy->R1);
    if (fuzzy->RP)
        free(fuzzy->RP);
    if (fuzzy->RI)
        free(fuzzy->RI);
    if (fuzzy->FilterS)
        free(fuzzy->FilterS);

    fuzzy->FilterS = NULL;
    fuzzy->S = (Uint *)calloc(m + 1, sizeof(Uint));
    fuzzy->R = (Uint *)calloc(fuzzy->k + 1, sizeof(Uint));
    fuzzy->R1 = (Uint *)calloc(fuzzy->k + 1, sizeof(Uint));
    fuzzy->RI = (Uint *)calloc(fuzzy->k + 1, sizeof(Uint));
    fuzzy->RP = (Uint *)calloc(fuzzy->k + 1, sizeof(Uint));

    for (i = 0, cnt = 0; i < m; i++)
    {
        l = fuzzy->Map[(unsigned char)p[i]];
        if (!l)
        {
            l = fuzzy->Map[(unsigned char)p[i]] = ++cnt;
            fuzzy->S[l] = 0;
        }
        fuzzy->S[l] |= 1 << i;
    }


    for (d = 0; d <= fuzzy->k; d++)
        fuzzy->RI[d] = (1 << d) - 1;

    fuzzy->mask_ok = (1 << (m - 1));
    fuzzy->r_size  = sizeof(Uint) * (fuzzy->k + 1);
    p_len = m;

    if (p_len > (int) sizeof(Uint)*8)
        p_len = (int) sizeof(Uint)*8;

    /* If k is zero then no filter is needed! */
    if (fuzzy->k && (p_len >= 2*(fuzzy->k + 1)) )
    {
        if (UseFilter)
        {
            fuzzy->FilterSet = 1;
            memset(fuzzy->FilterMap, 0 , sizeof(fuzzy->FilterMap) );
            fuzzy->FilterS = (Uint *)calloc(m + 1, sizeof(Uint));

            /* Not let's fill the interleaved pattern */
            dd = p_len / (fuzzy->k + 1);
            p_len  = dd * (fuzzy->k + 1);

            for (i = 0, cnt = 0; i < dd; i++)
                for (j = 0; j < fuzzy->k + 1; j++, cnt++)
                    PatFilter[cnt] = (unsigned char)p[j*dd + i];
            PatFilter[p_len] = 0;

            for (i = 0, cnt = 0; i < p_len; i++)
            {
                l = fuzzy->FilterMap[(unsigned char)PatFilter[i]];
                if (!l)
                {
                    l = fuzzy->FilterMap[(unsigned char)PatFilter[i]] = ++cnt;
                    fuzzy->FilterS[l] = 0;
                }
                fuzzy->FilterS[l] |= 1 << i;
            }
            fuzzy->filter_ok = 0;
            for (i = p_len - fuzzy->k - 1; i <= p_len - 1; i++) /* k+1 times */
                fuzzy->filter_ok |= 1 << i;

             /* k+1 first bits set to 1 */
            fuzzy->filter_shift = (1 << (fuzzy->k + 2)) - 1;
        }
    }
}

/******************************************************************************
FUNCTION afuzzy_free() 
    Cleaning up after previous afuzzy_init() call.

ARGUMENTS: 
    fuzzy       pointer to the afuzzy parameters structure
    
RETURN VALUE:
    none    
******************************************************************************/
void afuzzy_free(AFUZZY *fuzzy)
{
    if (fuzzy->S)
    {
        free(fuzzy->S);
        fuzzy->S = NULL;
    }
    if (fuzzy->R)
    {
        free(fuzzy->R);
        fuzzy->R = NULL;
    }
    if (fuzzy->R1)
    {
        free(fuzzy->R1);
        fuzzy->R1 = NULL;
    }
    if (fuzzy->RP)
    {
        free(fuzzy->RP);
        fuzzy->RP = NULL;
    }
    if (fuzzy->RI)
    {
        free(fuzzy->RI);
        fuzzy->RI = NULL;
    }
    if (fuzzy->FilterS)
    {
        free(fuzzy->FilterS);
        fuzzy->FilterS = NULL;
    }
}


/******************************************************************************
FUNCTION afuzzy_CheckSUB() 
    Perform a fuzzy pattern substring matching. afuzzy_init() should be 
    called previously to initialize the pattern and error length. 
    Positive result means that some part of the string given matches the 
    pattern with no more than afuzzy->k errors (1 error = 1 letter 
    replacement or transposition)

ARGUMENTS: 
    t           the string to test
    fuzzy       pointer to the afuzzy parameters structure

RETURN VALUE:
    0   - no match
    > 0 - strings match

ALGORITHM
    ????????????????
******************************************************************************/
int afuzzy_checkSUB(const char *t, AFUZZY *fuzzy)
{
    register char c;
    register int j, d;

    /* For eficciency this case should be little bit optimized */
    if (!fuzzy->k)
    {
        Uint R = 0, R1;

        for (j = 0; (c = t[j]) != '\0'; j++)
        {
            R1 = ( ((R<<1) | 1) & fuzzy->S[fuzzy->Map[(unsigned char)c]]);
            R = R1;

            if (R1 & fuzzy->mask_ok)
                return 1;
        } /* end for (register int j = 0 ... */
        return 0;
    }

    if (fuzzy->FilterSet && !afuzzy_checkFLT(t, fuzzy))
        return 0;

    memcpy(fuzzy->R, fuzzy->RI, fuzzy->r_size); /* R = RI */

    for (j = 0; (c = t[j]); j++)
    {
        for (d = 0; d <= fuzzy->k; d++)
        {
            fuzzy->R1[d] = (((fuzzy->R[d]<<1) | 1) & 
                                        fuzzy->S[fuzzy->Map[(unsigned char)c]]);
            if (d > 0)
                fuzzy->R1[d] |= ((fuzzy->R[d-1] | fuzzy->R1[d-1])<<1) | 1 | 
                                                            fuzzy->R[d-1];
        }
        if (fuzzy->R1[fuzzy->k] & fuzzy->mask_ok)
            return j;

        memcpy(fuzzy->R, fuzzy->R1, fuzzy->r_size);

    } /* end for (register int j = 0 ... */

    return 0;
}

int afuzzy_checkFLT(const char *t, AFUZZY *fuzzy)
{
    register Uint FilterR = 0;
    register Uint FilterR1;
    register int j;

    for (j = 0; t[j] != '\0'; j++)
    {
        FilterR1 = ( ((FilterR<<(fuzzy->k+1)) | fuzzy->filter_shift) & 
                        fuzzy->FilterS[fuzzy->FilterMap[(unsigned char)t[j]]]);
        if (FilterR1 & fuzzy->filter_ok)
            return 1;
        FilterR = FilterR1;
    } /* end for (register int j = 0 ... */

    return 0;
}
