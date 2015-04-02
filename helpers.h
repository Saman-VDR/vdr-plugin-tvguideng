#ifndef __TVGUIDE_HELPERS_H
#define __TVGUIDE_HELPERS_H

#include <string>
#include <vector>
#include <vdr/font.h>
#include <vdr/recording.h>
#include <vdr/plugin.h>

using namespace std;

cPlugin *GetScraperPlugin(void);

bool FileExists(const string &fullpath);
bool FileExists(const string &path, const string &name, const string &ext);

class cStopWatch {
private:
	uint64_t start;
	uint64_t last;
public:
	cStopWatch(const char* message = NULL);
	~cStopWatch(void) {};
	void Report(const char* message);
	void Stop(const char* message);
};

std::string CutText(std::string text, int width, const cFont *font);
std::string StrToLowerCase(std::string str);
std::string GetDirectoryFromTimer(std::string file);
void ReadRecordingDirectories(std::vector<std::string> *folders, cList<cNestedItem> *rootFolders, cString path);
void DrawRoundedCorners(cPixmap *p, int posX, int posY, int width, int height, int radius, int borderWidth, tColor borderColor);

class splitstring : public std::string {
    std::vector<std::string> flds;
public:
    splitstring(const char *s) : std::string(s) { };
    std::vector<std::string>& split(char delim, int rep=0);
};

int FindIgnoreCase(const std::string& expr, const std::string& query);

char* GetAuxValue(const char* aux, const char* name);
char* GetAuxValue(const cRecording *recording, const char* name);
char* GetAuxValue(const cTimer* timer, const char* name);

#ifndef _AFUZZY_H
#define _AFUZZY_H

// source from:
/*
  Leonid Boitsov 2002. (itman@narod.ru)
  C version of Stas Namin.
  This code is a GPL software and is distributed under GNU
  public licence without any warranty.
*/

typedef unsigned int Uint;

#define MaxPatSize (sizeof(Uint) * 8)

typedef struct
{
    Uint        *R,
                *R1,
                *RP,
                *S,
                *RI;
    Uint        *FilterS;

    int         Map[256];
    int         FilterMap[256];
    int         k;
    Uint        mask_ok;
    Uint        filter_ok;
    Uint        filter_shift;
    int         r_size;
    int         FilterSet;
} AFUZZY;

void afuzzy_init(const char *p, int kerr, int UseFilter, AFUZZY *fuzzy);
void afuzzy_free(AFUZZY *fuzzy);
int afuzzy_checkSUB(const char *t, AFUZZY *fuzzy);
int afuzzy_checkFLT(const char *t, AFUZZY *fuzzy);
#endif

#endif // __TVGUIDE_HELPERS_H
