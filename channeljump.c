#include <vdr/channels.h>
#include "channeljump.h"

cChannelJump::cChannelJump(skindesignerapi::cViewElement *channelJump, int lastValidChannel) {
    this->channelJump = channelJump;
    channel = 0;
    maxChannels = lastValidChannel;
    startTime = 0;
    timeout = Setup.ChannelEntryTimeout;
}

cChannelJump::~cChannelJump(void) {
    channelJump->Clear();
    delete channelJump;
}

void cChannelJump::Draw(void) {
    channelJump->Clear();
    channelJump->ClearTokens();
    channelJump->AddStringToken((int)eChannelJumpST::channel, *BuildChannelString());
    channelJump->Display();
}

void cChannelJump::Set(int key) {
    startTime = cTimeMs::Now();
    if (channel == 0) {
	    channel = key;
	    return;
    }
    int newChannel = channel * 10 + key;
    if (newChannel <= maxChannels)
    	channel = newChannel;
}

cString cChannelJump::BuildChannelString(void) {
	if (channel*10 <= maxChannels)
		return cString::sprintf("%d-", channel);
	else
		return cString::sprintf("%d", channel); 
}

 bool cChannelJump::TimeOut(void) {
 	if ((int)(cTimeMs::Now() - startTime) > timeout)
	 	return true;
	return false;
 }