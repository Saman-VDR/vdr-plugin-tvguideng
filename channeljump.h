#ifndef __TVGUIDE_CHANNELJUMP_H
#define __TVGUIDE_CHANNELJUMP_H

#include <libskindesignerapi/skindesignerosdbase.h>
#include "definitions.h"

// --- cChannelJump  -------------------------------------------------------------

class cChannelJump {
private:
    skindesignerapi::cViewElement *channelJump;
    int channel;
    int maxChannels;
    int startTime;
    int timeout;
    cString BuildChannelString(void);
public:
    cChannelJump(skindesignerapi::cViewElement *channelJump, int lastValidChannel);
    virtual ~cChannelJump(void);
    void Set(int key);
    bool TimeOut(void);
    void Draw(void);
    int GetChannel(void) { return channel; };
};

#endif //__TVGUIDE_CHANNELJUMP_H