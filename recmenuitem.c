#include "recmenuitem.h"
#include "recmenus.h"
#include "config.h"
#include "helpers.h"
#include <math.h>
#include <wctype.h>
#include <vdr/remote.h>

long cRecMenuItem::idCounter;

// --- cRecMenuItem  -------------------------------------------------------------

cRecMenuItem::cRecMenuItem(void) {
    id = idCounter;
    idCounter++;
    init = true;
    active = false;
    selectable = true;
    action = rmsNotConsumed;
    height = 0;
    text = "";
}

cRecMenuItem::~cRecMenuItem(void) {
}

bool cRecMenuItem::IsNew(void) {
    if (init) {
        init = false;
        return true;
    }
    return false;
}

// --- cRecMenuItemInfo  -------------------------------------------------------

cRecMenuItemInfo::cRecMenuItemInfo(string line1, int numLines, string line2, string line3, string line4) {
    selectable = false;
    this->numLines = numLines;
    this->line1 = line1;
    this->line2 = line2;
    this->line3 = line3;
    this->line4 = line4;
    if (numLines == 1)
        height = 12;
    else if (numLines == 2)
        height = 16;
    else if (numLines == 3)
        height = 20;
    else if (numLines == 4)
        height = 24;
    this->active = false;
}

cRecMenuItemInfo::~cRecMenuItemInfo(void) {
}

void cRecMenuItemInfo::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::info, 1);
    menu->AddIntToken((int)eRecMenuIT::lines, numLines);
    menu->AddStringToken((int)eRecMenuST::line1, line1.c_str());
    menu->AddStringToken((int)eRecMenuST::line2, line2.c_str());
    menu->AddStringToken((int)eRecMenuST::line3, line3.c_str());
    menu->AddStringToken((int)eRecMenuST::line4, line4.c_str());
}

// --- cRecMenuItemButton  -------------------------------------------------------

cRecMenuItemButton::cRecMenuItemButton(string text, eRecMenuState action, bool active) {
    selectable = true;
    height = 8;
    this->text = text;
    this->action = action;
    this->active = active;
}

cRecMenuItemButton::~cRecMenuItemButton(void) {
}

void cRecMenuItemButton::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::button, 1);
    menu->AddStringToken((int)eRecMenuST::buttontext, text.c_str());
}

eRecMenuState cRecMenuItemButton::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kOk:
            return action;
            break;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemButtonYesNo  -------------------------------------------------------
cRecMenuItemButtonYesNo::cRecMenuItemButtonYesNo(string textYes,
                                                 string textNo,
                                                 eRecMenuState actionYes, 
                                                 eRecMenuState actionNo,
                                                 bool active) {
    height = 8;
    selectable = true;
    this->textYes = textYes;
    this->textNo = textNo;
    this->action = actionYes;
    this->actionNo = actionNo;
    this->active = active;
    yesActive = true;
}

cRecMenuItemButtonYesNo::~cRecMenuItemButtonYesNo(void) {
}

void cRecMenuItemButtonYesNo::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::buttonyesno, 1);
    menu->AddIntToken((int)eRecMenuIT::yes, yesActive);
    menu->AddStringToken((int)eRecMenuST::textyes, textYes.c_str());
    menu->AddStringToken((int)eRecMenuST::textno, textNo.c_str());
}

eRecMenuState cRecMenuItemButtonYesNo::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kLeft:
            if (!yesActive) {
                yesActive = true;
                return rmsRefresh;
            } else
                return rmsNotConsumed;
            break;
        case kRight:
            if (yesActive) {
                yesActive = false;
                return rmsRefresh;
            } else
                return rmsNotConsumed;
            break;
        case kOk:
            if (yesActive)
                return action;
            return actionNo;
            break;
        default:
            break;
    }
    return rmsNotConsumed;
}


// --- cRecMenuItemInt  -------------------------------------------------------
cRecMenuItemInt::cRecMenuItemInt(string text,
                                int initialVal,
                                int minVal,
                                int maxVal,
                                bool active, 
                                int *callback,
                                eRecMenuState action) {

    height = 8;
    selectable = true;
    this->text = text;
    this->currentVal = initialVal;
    this->minVal = minVal;
    this->maxVal = maxVal;
    this->active = active;
    this->callback = callback;
    this->action = action;
    fresh = true;
}

cRecMenuItemInt::~cRecMenuItemInt(void) {
}

void cRecMenuItemInt::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::intselector, 1);
    menu->AddIntToken((int)eRecMenuIT::value, currentVal);
    menu->AddStringToken((int)eRecMenuST::text, text.c_str());    
}

eRecMenuState cRecMenuItemInt::ProcessKey(eKeys Key) {
    int oldValue = currentVal;
    switch (Key & ~k_Repeat) {
        case kLeft:
            fresh = true;
            if (currentVal > minVal) {
                currentVal--;
                if (callback)
                    *callback = currentVal;
            }
            return rmsRefresh;
            break;
        case kRight:
            fresh = true;
            if (currentVal < maxVal) {
                currentVal++;
                if (callback)
                    *callback = currentVal;
            }
            return rmsRefresh;
            break;
        case k0 ... k9:
            if (fresh) {
               currentVal = 0;
               fresh = false;
            }
            currentVal = currentVal * 10 + (Key - k0);
            if (!((currentVal >= minVal) &&  (currentVal <= maxVal)))
                currentVal = oldValue;
            if (callback)
                *callback = currentVal;
            return rmsRefresh;
            break;
        case kOk:
            return action;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemBool  -------------------------------------------------------
cRecMenuItemBool::cRecMenuItemBool(string text,
                                   bool initialVal,
                                   bool active,
                                   bool *callback,
                                   eRecMenuState action) {
    height = 8;
    selectable = true;
    this->text = text;
    this->yes = initialVal;
    this->active = active;
    this->callback = callback;
    this->action = action;
}

cRecMenuItemBool::~cRecMenuItemBool(void) {
}

void cRecMenuItemBool::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::boolselector, 1);
    menu->AddIntToken((int)eRecMenuIT::value, yes);
    menu->AddStringToken((int)eRecMenuST::text, text.c_str());    
}

eRecMenuState cRecMenuItemBool::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kLeft:
        case kRight:
            yes = !yes;
            if (callback)
                *callback = yes;
            return rmsRefresh;
            break;
        case kOk:
            return action;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemSelect  -------------------------------------------------------
cRecMenuItemSelect::cRecMenuItemSelect(string text,
                                       vector<string> strings,
                                       int initialVal,
                                       bool active,
                                       int *callback,
                                       eRecMenuState action) {
    height = 8;
    selectable = true;
    this->text = text;
    this->strings = strings;
    numValues = strings.size();
    if ((initialVal < 0) || (initialVal > numValues-1))
        this->currentVal = 0;
    else
        this->currentVal = initialVal;
    this->active = active;
    this->callback = callback;
    this->action = action;
}

cRecMenuItemSelect::~cRecMenuItemSelect(void) {
}

void cRecMenuItemSelect::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::stringselector, 1);
    menu->AddStringToken((int)eRecMenuST::text, text.c_str());    
    menu->AddStringToken((int)eRecMenuST::value, strings[currentVal].c_str());
}

eRecMenuState cRecMenuItemSelect::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kLeft:
            currentVal--;
            if (currentVal<0)
                currentVal = numValues - 1;
            if (callback)
                *callback = currentVal;
            return rmsRefresh;
            break;
        case kRight:
            currentVal = (currentVal+1)%numValues;
            if (callback)
                *callback = currentVal;
            return rmsRefresh;
            break;
        case kOk:
            return action;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemText  -------------------------------------------------------
cRecMenuItemText::cRecMenuItemText(string text,
                                   char *initialVal,
                                   int length,
                                   bool active,
                                   char *callback) {

    height = 16;
    selectable = true;
    this->text = text;
    value = initialVal;
    this->active = active;
    this->callback = callback;
    this->length = length;
    allowed = trVDR(FileNameChars);
    pos = -1;
    offset = 0;
    insert = uppercase = false;
    newchar = true;
    lengthUtf8 = 0;
    valueUtf8 = NULL;
    allowedUtf8 = NULL;
    charMapUtf8 = NULL;
    currentCharUtf8 = NULL;
    lastKey = kNone;
    buffer = "";
}

cRecMenuItemText::~cRecMenuItemText(void) {
    delete[] valueUtf8;
    delete[] allowedUtf8;
    delete[] charMapUtf8;
}

void cRecMenuItemText::EnterEditMode(void) {
    if (!valueUtf8) {
        valueUtf8 = new uint[length];
        lengthUtf8 = Utf8ToArray(value, valueUtf8, length);
        int l = strlen(allowed) + 1;
        allowedUtf8 = new uint[l];
        Utf8ToArray(allowed, allowedUtf8, l);
        const char *charMap = trVDR("CharMap$ 0\t-.,1#~\\^$[]|()*+?{}/:%@&\tabc2\tdef3\tghi4\tjkl5\tmno6\tpqrs7\ttuv8\twxyz9");
        l = strlen(charMap) + 1;
        charMapUtf8 = new uint[l];
        Utf8ToArray(charMap, charMapUtf8, l);
        currentCharUtf8 = charMapUtf8;
        AdvancePos();
    }
}

void cRecMenuItemText::LeaveEditMode(bool SaveValue) {
    if (valueUtf8) {
        if (SaveValue) {
            Utf8FromArray(valueUtf8, value, length);
            stripspace(value);
            if (callback) {
                strncpy(callback, value, TEXTINPUTLENGTH); 
            }
        }
        lengthUtf8 = 0;
        delete[] valueUtf8;
        valueUtf8 = NULL;
        delete[] allowedUtf8;
        allowedUtf8 = NULL;
        delete[] charMapUtf8;
        charMapUtf8 = NULL;
        pos = -1;
        offset = 0;
        newchar = true;
    }
}

void cRecMenuItemText::AdvancePos(void) {
    if (pos < length - 2 && pos < lengthUtf8) {
        if (++pos >= lengthUtf8) {
            if (pos >= 2 && valueUtf8[pos - 1] == ' ' && valueUtf8[pos - 2] == ' ')
                pos--; // allow only two blanks at the end
            else {
                valueUtf8[pos] = ' ';
                valueUtf8[pos + 1] = 0;
                lengthUtf8++;
            }
        }
    }
    newchar = true;
    if (!insert && Utf8is(alpha, valueUtf8[pos]))
        uppercase = Utf8is(upper, valueUtf8[pos]);
}

uint cRecMenuItemText::Inc(uint c, bool Up) {
    uint *p = IsAllowed(c);
    if (!p)
        p = allowedUtf8;
    if (Up) {
        if (!*++p)
            p = allowedUtf8;
    } else if (--p < allowedUtf8) {
        p = allowedUtf8;
        while (*p && *(p + 1))
            p++;
    }
    return *p;
}

void cRecMenuItemText::Type(uint c) {
    if (insert && lengthUtf8 < length - 1)
        Insert();
    valueUtf8[pos] = c;
    if (pos < length - 2)
        pos++;
    if (pos >= lengthUtf8) {
        valueUtf8[pos] = ' ';
        valueUtf8[pos + 1] = 0;
        lengthUtf8 = pos + 1;
    }
}

void cRecMenuItemText::Insert(void) {
    memmove(valueUtf8 + pos + 1, valueUtf8 + pos, (lengthUtf8 - pos + 1) * sizeof(*valueUtf8));
    lengthUtf8++;
    valueUtf8[pos] = ' ';
}

void cRecMenuItemText::Delete(void) {
    memmove(valueUtf8 + pos, valueUtf8 + pos + 1, (lengthUtf8 - pos) * sizeof(*valueUtf8));
    lengthUtf8--;
}

uint *cRecMenuItemText::IsAllowed(uint c) {
    if (allowedUtf8) {
        for (uint *a = allowedUtf8; *a; a++) {
            if (c == *a)
                return a;
        }
    }
    return NULL;
}

void cRecMenuItemText::SetText(void) {
    if (InEditMode()) {
        int textAreaWidth = 800;
        if (pos < offset)
            offset = pos;
        int WidthFromOffset = 0;
        int EndPos = lengthUtf8;
        for (int i = offset; i < lengthUtf8; i++) {
            WidthFromOffset += 20;
            if (WidthFromOffset > textAreaWidth) {
                if (pos >= i) {
                    do {
                        WidthFromOffset -= 20;
                        offset++;
                    } while (WidthFromOffset > textAreaWidth && offset < pos);
                    EndPos = pos + 1;
                } else {
                    EndPos = i;
                    break;
                }
            }
        }
        char buf[1000];
        char *p = buf;
        if (offset)
            *p++ = '<';
        p += Utf8FromArray(valueUtf8 + offset, p, sizeof(buf) - (p - buf), pos - offset);
        *p++ = '[';
        if (insert && newchar)
            *p++ = ']';
        p += Utf8FromArray(&valueUtf8[pos], p, sizeof(buf) - (p - buf), 1);
        if (!(insert && newchar))
            *p++ = ']';
        p += Utf8FromArray(&valueUtf8[pos + 1], p, sizeof(buf) - (p - buf), EndPos - pos - 1);
        if (EndPos != lengthUtf8)
            *p++ = '>';
        *p = 0;
        buffer = buf;
    } else {
        buffer = "";
    }
}

void cRecMenuItemText::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::textinput, 1);
    menu->AddIntToken((int)eRecMenuIT::editmode, InEditMode());
    menu->AddStringToken((int)eRecMenuST::text, text.c_str());
    if (buffer.size() > 0) {
        menu->AddStringToken((int)eRecMenuST::value, buffer.c_str());
    } else {        
        menu->AddStringToken((int)eRecMenuST::value, value);
    }
}

eRecMenuState cRecMenuItemText::ProcessKey(eKeys Key) {
    bool consumed = false;
    bool SameKey = NORMALKEY(Key) == lastKey;

    if (Key != kNone) {
        lastKey = NORMALKEY(Key);
    } else if (!newchar && k0 <= lastKey && lastKey <= k9 && autoAdvanceTimeout.TimedOut()) {
        AdvancePos();
        newchar = true;
        currentCharUtf8 = NULL;
        SetText();
        return rmsRefresh;
    }

    switch ((int)Key) {
        case kRed:   // Switch between upper- and lowercase characters
            if (InEditMode()) {
                if (!insert || !newchar) {
                    uppercase = !uppercase;
                    valueUtf8[pos] = uppercase ? Utf8to(upper, valueUtf8[pos]) : Utf8to(lower, valueUtf8[pos]);
                }
                consumed = true;
            }
            break;
        case kGreen: // Toggle insert/overwrite modes
            if (InEditMode()) {
                insert = !insert;
                newchar = true;
                consumed = true;
            }
            break;
        case kYellow|k_Repeat:
        case kYellow: // Remove the character at the current position; in insert mode it is the character to the right of the cursor
            if (InEditMode()) {
                if (lengthUtf8 > 1) {
                    if (!insert || pos < lengthUtf8 - 1)
                        Delete();
                    else if (insert && pos == lengthUtf8 - 1)
                        valueUtf8[pos] = ' '; // in insert mode, deleting the last character replaces it with a blank to keep the cursor position
                    // reduce position, if we removed the last character
                    if (pos == lengthUtf8)
                        pos--;
                } else if (lengthUtf8 == 1)
                    valueUtf8[0] = ' '; // This is the last character in the string, replace it with a blank
                if (Utf8is(alpha, valueUtf8[pos]))
                    uppercase = Utf8is(upper, valueUtf8[pos]);
                newchar = true;
                consumed = true;
            }
            break;
        case kLeft|k_Repeat:
        case kLeft:
            
            if (pos > 0) {
                if (!insert || newchar)
                    pos--;
                newchar = true;
                if (!insert && Utf8is(alpha, valueUtf8[pos]))
                    uppercase = Utf8is(upper, valueUtf8[pos]);
            }
            consumed = true;
            break;
        case kRight|k_Repeat:
        case kRight: 
            if (InEditMode()) {
                AdvancePos();
            } else {
                EnterEditMode();
            }
            consumed = true;
            break;
        case kUp|k_Repeat:
        case kUp:
        case kDown|k_Repeat:
        case kDown:  
            if (InEditMode()) {
                if (insert && newchar) {
                // create a new character in insert mode
                    if (lengthUtf8 < length - 1)
                        Insert();
                    }
                    if (uppercase)
                        valueUtf8[pos] = Utf8to(upper, Inc(Utf8to(lower, valueUtf8[pos]), NORMALKEY(Key) == kUp));
                    else
                        valueUtf8[pos] =               Inc(              valueUtf8[pos],  NORMALKEY(Key) == kUp);
                        newchar = false;
                consumed = true;
            }
            break;
        case k0|k_Repeat ... k9|k_Repeat:
        case k0 ... k9: {
            if (InEditMode()) {
                if (Setup.NumberKeysForChars) {
                    if (!SameKey) {
                        if (!newchar)
                            AdvancePos();
                        currentCharUtf8 = NULL;
                    }
                    if (!currentCharUtf8 || !*currentCharUtf8 || *currentCharUtf8 == '\t') {
                    // find the beginning of the character map entry for Key
                        int n = NORMALKEY(Key) - k0;
                        currentCharUtf8 = charMapUtf8;
                        while (n > 0 && *currentCharUtf8) {
                            if (*currentCharUtf8++ == '\t')
                                n--;
                        }
                        // find first allowed character
                        while (*currentCharUtf8 && *currentCharUtf8 != '\t' && !IsAllowed(*currentCharUtf8))
                            currentCharUtf8++;
                    }
                    if (*currentCharUtf8 && *currentCharUtf8 != '\t') {
                        if (insert && newchar) {
                        // create a new character in insert mode
                            if (lengthUtf8 < length - 1)
                                Insert();
                        }
                        valueUtf8[pos] = *currentCharUtf8;
                        if (uppercase)
                            valueUtf8[pos] = Utf8to(upper, valueUtf8[pos]);
                        // find next allowed character
                        do {
                            currentCharUtf8++;
                        } while (*currentCharUtf8 && *currentCharUtf8 != '\t' && !IsAllowed(*currentCharUtf8));
                        newchar = false;
                        autoAdvanceTimeout.Set(AUTO_ADVANCE_TIMEOUT);
                    }
                } else {
                    Type('0' + NORMALKEY(Key) - k0);
                }
                consumed = true;
            }                
            break; }
        case kBack:
        case kOk:    
            if (InEditMode()) {
                LeaveEditMode(Key == kOk);
            } else {
                EnterEditMode();
            }
            consumed = true;
            break;
        default:
            if (InEditMode() && BASICKEY(Key) == kKbd) {
                int c = KEYKBD(Key);
                if (c <= 0xFF) { 
                    if (IsAllowed(Utf8to(lower, c)))
                        Type(c);
                    else {
                        switch (c) {
                            case 0x7F: // backspace
                                if (pos > 0) {
                                    pos--;
                                    ProcessKey(kYellow);
                                }
                                break;
                            default: ;
                        }
                    }
                } else {
                    switch (c) {
                        case kfHome: pos = 0; break;
                        case kfEnd:  pos = lengthUtf8 - 1; break;
                        case kfIns:  ProcessKey(kGreen);
                        case kfDel:  ProcessKey(kYellow);
                        default: ;
                    }
                }
                consumed = true;
            }
            break;
    }
    SetText();
    if (consumed)
        return rmsRefresh;
    return rmsNotConsumed;
}

// --- cRecMenuItemTime  -------------------------------------------------------
cRecMenuItemTime::cRecMenuItemTime(string text,
                                   int initialVal,
                                   bool active,
                                   int *callback,
                                   eRecMenuState action) {
    height = 8;
    selectable = true;
    this->text = text;
    this->value = initialVal;
    hh = value / 100;
    mm = value % 100;
    pos = 0;
    fresh = true;
    this->active = active;
    this->callback = callback;
    this->action = action;
}

cRecMenuItemTime::~cRecMenuItemTime(void) {
}

void cRecMenuItemTime::SetTokens(skindesignerapi::cViewGrid *menu) {
    char buf[10];
    switch (pos) {
        case 1:  snprintf(buf, sizeof(buf), "%01d-:--", hh / 10); break;
        case 2:  snprintf(buf, sizeof(buf), "%02d:--", hh); break;
        case 3:  snprintf(buf, sizeof(buf), "%02d:%01d-", hh, mm / 10); break;
        default: snprintf(buf, sizeof(buf), "%02d:%02d", hh, mm);
    }
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::timeselector, 1);
    menu->AddStringToken((int)eRecMenuST::text, text.c_str());
    menu->AddStringToken((int)eRecMenuST::value, buf);
}

eRecMenuState cRecMenuItemTime::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kLeft|k_Repeat:
        case kLeft: {
            if (--mm < 0) {
                mm = 59;
                if (--hh < 0)
                    hh = 23;
            }
            fresh = true;
            value = hh * 100 + mm;
            if (callback)
                *callback = value;
            return rmsRefresh;
            break; }
        case kRight|k_Repeat:
        case kRight: {
            if (++mm > 59) {
                mm = 0;
                if (++hh > 23)
                    hh = 0;
            }
            fresh = true;
            value = hh * 100 + mm;
            if (callback)
                *callback = value;
            return rmsRefresh;
            break; }
        case k0|k_Repeat ... k9|k_Repeat:
        case k0 ... k9: {
            if (fresh || pos > 3) {
                pos = 0;
                fresh = false;
            }
            int n = Key - k0;
            switch (pos) {
                case 0: 
                    if (n <= 2) {
                        hh = n * 10;
                        mm = 0;
                        pos++;
                    }
                    break;
                case 1: 
                    if (hh + n <= 23) {
                        hh += n;
                        pos++;
                    }
                  break;
                case 2: 
                    if (n <= 5) {
                        mm += n * 10;
                        pos++;
                    }
                    break;
                case 3: 
                    if (mm + n <= 59) {
                        mm += n;
                        pos++;
                    }
                    break;
                default: ;
            }
            value = hh * 100 + mm;
            if (callback)
                *callback = value;
            return rmsRefresh;
            break; }
        case kOk:
            return action;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemDay  -------------------------------------------------------
cRecMenuItemDay::cRecMenuItemDay(string text,
                                 time_t initialVal,
                                 bool active,
                                 time_t *callback,
                                 eRecMenuState action) {

    height = 8;
    selectable = true;
    this->text = text;
    this->currentVal = cTimer::SetTime(initialVal, 0);
    this->active = active;
    this->callback = callback;
    this->action = action;
}

cRecMenuItemDay::~cRecMenuItemDay(void) {
}

void cRecMenuItemDay::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::dayselector, 1);
    menu->AddStringToken((int)eRecMenuST::text, text.c_str());
    menu->AddStringToken((int)eRecMenuST::value, *DateString(currentVal));
}

eRecMenuState cRecMenuItemDay::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kLeft:
            currentVal -= 60*60*24;
            if (callback)
                *callback = currentVal;
            return rmsRefresh;
            break;
        case kRight:
            currentVal += 60*60*24;
            if (callback)
                *callback = currentVal;
            return rmsRefresh;
            break;
        case kOk:
            return action;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemChannelChooser -------------------------------------------------------
cRecMenuItemChannelChooser::cRecMenuItemChannelChooser(string text,
                                                       const cChannel *initialChannel,
                                                       bool active,
                                                       int *callback,
                                                       eRecMenuState action) {

    height = 8;
    selectable = true;
    this->text = text;
    this->channel = initialChannel;
    if (initialChannel)
        initialChannelSet = true;
    else
        initialChannelSet = false;
    channelNumber = 0;
    fresh = true;
    this->active = active;
    this->callback = callback;
    this->action = action;
}

cRecMenuItemChannelChooser::~cRecMenuItemChannelChooser(void) {
}

void cRecMenuItemChannelChooser::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::channelselector, 1);
    menu->AddStringToken((int)eRecMenuST::text, text.c_str());
    if (channel) {
        menu->AddIntToken((int)eRecMenuIT::channelnumber, channel->Number());
        menu->AddStringToken((int)eRecMenuST::channelname, channel->Name());
        cString channelId = channel->GetChannelID().ToString();
        menu->AddStringToken((int)eRecMenuST::channelid, *channelId);
        menu->AddIntToken((int)eRecMenuIT::channellogoexisis, menu->ChannelLogoExists(*channelId));
    } else {
        menu->AddIntToken((int)eRecMenuIT::channelnumber, 0);
        menu->AddStringToken((int)eRecMenuST::channelname, tr("all Channels"));
        menu->AddStringToken((int)eRecMenuST::channelid, "");
        menu->AddIntToken((int)eRecMenuIT::channellogoexisis, false);        
    }
}

eRecMenuState cRecMenuItemChannelChooser::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kLeft: {
            fresh = true;
            if (!channel)
                return rmsConsumed;
            const cChannel *prev = channel;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
            LOCK_CHANNELS_READ;
            const cChannels* channels = Channels;
#else
            const cChannels* channels = &Channels;
#endif
            const cChannel *firstChannel = channels->First();
            if (firstChannel->GroupSep())
                firstChannel = channels->Next(firstChannel);
            if (prev == firstChannel) {
                if (!initialChannelSet)
                    channel = NULL;
            } else {
                while (prev = channels->Prev(prev)) {
                    if(!prev->GroupSep()) {
                        channel = prev;
                        break;
                    }    
                }
            }
            if (callback) {
                if (channel)
                    *callback = channel->Number();
                else
                    *callback = 0;
            }
            return rmsRefresh;
            break; }
        case kRight: {
            fresh = true;
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
            LOCK_CHANNELS_READ;
            const cChannels* channels = Channels;
#else
            const cChannels* channels = &Channels;
#endif
            if (!channel) {
                channel = channels->First();
                if(channel->GroupSep())
                    channel = channels->Next(channel);
            } else {
                const cChannel *next = channel;
                while (next = channels->Next(next)) {
                    if(!next->GroupSep()) {
                        channel = next;
                        break;
                    }
                }
            }
            if (callback) {
                if (channel)
                    *callback = channel->Number();
                else
                    *callback = 0;
            }
            return rmsRefresh;
            break; }
        case k0 ... k9: {
            if (fresh) {
               channelNumber = 0;
               fresh = false;
            }
            channelNumber = channelNumber * 10 + (Key - k0);
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
            LOCK_CHANNELS_READ;
            const cChannel *chanNew = Channels->GetByNumber(channelNumber);
#else
            const cChannel *chanNew = Channels.GetByNumber(channelNumber);
#endif
            if (chanNew) {
                channel = chanNew;
                if (callback)
                    *callback = channel->Number();
            }
            return rmsRefresh;
            break; }
        case kOk:
            return action;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemDayChooser -------------------------------------------------------
cRecMenuItemDayChooser::cRecMenuItemDayChooser(string text,
                                               int weekdays,
                                               bool active,
                                               int *callback) {
    height = 8;
    selectable = true;
    this->text = text;
    if (weekdays < 1)
        weekdays *= -1;
    this->weekdays = weekdays;
    this->active = active;
    this->callback = callback;
    selectedDay = 0;
}

cRecMenuItemDayChooser::~cRecMenuItemDayChooser(void) {
}

void cRecMenuItemDayChooser::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::weekdayselector, 1);
    menu->AddStringToken((int)eRecMenuST::text, text.c_str());
    menu->AddIntToken((int)eRecMenuIT::dayselected, selectedDay);

    string days = trVDR("MTWTFSS");
    for (unsigned day=0; day<days.length(); ++day) {
        cString strDay = cString::sprintf("%c", days.at(day));
        menu->AddStringToken((int)eRecMenuST::day0abbr + day, *strDay);
        menu->AddIntToken((int)eRecMenuIT::day0set + day, WeekDaySet(day));
    }
}

bool cRecMenuItemDayChooser::WeekDaySet(unsigned day) {
    return weekdays & (1 << day);
}

void cRecMenuItemDayChooser::ToggleDay(void) {
    bool dayActive = WeekDaySet(selectedDay);
    int dayBit = pow(2, selectedDay);
    if (dayActive) {
        weekdays -= dayBit;
    } else {
        weekdays += dayBit;
    }
    if (callback) {
        *callback = weekdays;
    }
}

eRecMenuState cRecMenuItemDayChooser::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kLeft: {
            selectedDay--;
            if (selectedDay<0)
                selectedDay += 7;
            return rmsRefresh;
            break; }
        case kRight: {
            selectedDay = (selectedDay+1)%7;
            return rmsRefresh;
            break; }
        case kOk:
            ToggleDay();
            return rmsRefresh;
            break;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemSelectDirectory  -------------------------------------------------------
cRecMenuItemSelectDirectory::cRecMenuItemSelectDirectory(string text,
                                                         string originalFolder,
                                                         bool active,
                                                         char *callback,
                                                         eRecMenuState action,
                                                         bool isSearchTimer) {

    height = 8;
    selectable = true;
    this->text = text;
    this->originalFolder = originalFolder;
    this->active = active;
    this->callback = callback;
    this->action = action;
    folders.push_back(tr("root video folder"));
    if (isSearchTimer && config.instRecFixedFolder.size() > 0)
        folders.push_back(config.instRecFixedFolder);
    ReadRecordingDirectories(&folders, NULL, "");
    numValues = folders.size();
    this->currentVal = GetInitial();
}

void cRecMenuItemSelectDirectory::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::directoryselector, 1);
    menu->AddStringToken((int)eRecMenuST::text, text.c_str());
    menu->AddStringToken((int)eRecMenuST::folder, folders[currentVal].c_str());
}

eRecMenuState cRecMenuItemSelectDirectory::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kLeft:
            currentVal--;
            if (currentVal<0)
                currentVal = numValues - 1;
            if (callback) {
                SetCallback();
            }
            return rmsRefresh;
            break;
        case kRight: {
            currentVal = (currentVal+1)%numValues;
            if (callback) {
                SetCallback();
            }
            return rmsRefresh;
            break; }
        case kOk:
            return action;
        default:
            break;
    }
    return rmsNotConsumed;
}

void cRecMenuItemSelectDirectory::SetCallback(void) {
    string newFolder = folders[currentVal];
    if (!newFolder.compare(tr("root video folder")))
        newFolder = "";
    strncpy(callback, newFolder.c_str(), TEXTINPUTLENGTH);
}

int cRecMenuItemSelectDirectory::GetInitial(void) {
    if (originalFolder.size() == 0)
        return 0;
    for (int i=0; i < numValues; i++) {
        if (!folders[i].compare(originalFolder)) {
            return i;
        }
    }
    return 0;
}

// --- cRecMenuItemTimerConflictHeader -------------------------------------------------------
cRecMenuItemTimerConflictHeader::cRecMenuItemTimerConflictHeader(time_t conflictStart,
                                                                 time_t conflictStop,
                                                                 time_t overlapStart,
                                                                 time_t overlapStop) {
    height = 10;
    selectable = false;
    active = false;
    this->conflictStart = conflictStart;
    this->conflictStop = conflictStop;
    this->overlapStart = overlapStart;
    this->overlapStop = overlapStop;
}

cRecMenuItemTimerConflictHeader::~cRecMenuItemTimerConflictHeader(void) {
}

void cRecMenuItemTimerConflictHeader::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::timerconflictheader, 1);
    menu->AddStringToken((int)eRecMenuST::text, tr("Timer Conflict"));
    menu->AddStringToken((int)eRecMenuST::conflictstart, *TimeString(conflictStart));
    menu->AddStringToken((int)eRecMenuST::conflictstop, *TimeString(conflictStop));
    menu->AddStringToken((int)eRecMenuST::overlapstart, *TimeString(overlapStart));
    menu->AddStringToken((int)eRecMenuST::overlapstop, *TimeString(overlapStop));

    int olStart = (double)(overlapStart - conflictStart) / (double)(conflictStop - conflictStart) * 100; 
    int olWidth = (double)(overlapStop - overlapStart) / (double)(conflictStop - conflictStart) * 100;
    menu->AddIntToken((int)eRecMenuIT::overlapstartpercent, olStart);
    menu->AddIntToken((int)eRecMenuIT::overlapwidthpercent, olWidth);
}

// --- cRecMenuItemTimer  -------------------------------------------------------
cRecMenuItemTimer::cRecMenuItemTimer(const cTimer *timer, 
                                     eRecMenuState action1, 
                                     eRecMenuState action2,
                                     eRecMenuState action3,
                                     eRecMenuState action4,
                                     time_t conflictStart,
                                     time_t conflictStop,
                                     time_t overlapStart,
                                     time_t overlapStop,
                                     bool active) {
    height = 12;
    selectable = true;
    this->timer = timer;
    this->action = action1;
    this->action2 = action2;
    this->action3 = action3;
    this->action4 = action4;
    iconActive = 0;
    this->conflictStart = conflictStart;
    this->conflictStop = conflictStop;
    this->overlapStart = overlapStart;
    this->overlapStop = overlapStop;
    this->active = active;
}

cRecMenuItemTimer::~cRecMenuItemTimer(void) {
}

void cRecMenuItemTimer::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::timerconflict, 1);
    
    const cChannel *channel = timer->Channel();
    int channelTransponder = 0;
    const char *channelName = NULL;
    cString channelId = "";
    if (channel) {
        channelTransponder = channel->Transponder();
        channelName = channel->Name() ? channel->Name() : "";
        channelId = channel->GetChannelID().ToString();
    }
    menu->AddIntToken((int)eRecMenuIT::channeltransponder, channelTransponder);
    menu->AddStringToken((int)eRecMenuST::channelname, channelName);    
    menu->AddStringToken((int)eRecMenuST::channelid, *channelId);

    const cEvent *event = timer->Event();
    string timerTitle = "";
    if (event && event->Title()) {
        timerTitle = event->Title();
    }
    menu->AddStringToken((int)eRecMenuST::timertitle, timerTitle.c_str());
    menu->AddStringToken((int)eRecMenuST::starttime, *TimeString(timer->StartTime()));
    menu->AddStringToken((int)eRecMenuST::stoptime, *TimeString(timer->StopTime()));
    menu->AddStringToken((int)eRecMenuST::date, *ShortDateString(timer->StartTime()));
    menu->AddStringToken((int)eRecMenuST::weekday, *WeekDayName(timer->StartTime()));

    menu->AddIntToken((int)eRecMenuIT::infoactive, (iconActive==0)?true:false);
    menu->AddIntToken((int)eRecMenuIT::deleteactive, (iconActive==1)?true:false);
    menu->AddIntToken((int)eRecMenuIT::editactive, (iconActive==2)?true:false);
    menu->AddIntToken((int)eRecMenuIT::searchactive, (iconActive==3)?true:false);


    int conflictTime = conflictStop - conflictStart;
    int timerStart = (double)(timer->StartTime() - conflictStart) / (double)conflictTime * 100;
    int timerWidth = (double)(timer->StopTime() - timer->StartTime()) / (double)conflictTime * 100;
    int olStart = (double)(overlapStart - conflictStart) / (double)(conflictStop - conflictStart) * 100; 
    int olWidth = (double)(overlapStop - overlapStart) / (double)(conflictStop - conflictStart) * 100;
    menu->AddIntToken((int)eRecMenuIT::timerstartpercent, timerStart);
    menu->AddIntToken((int)eRecMenuIT::timerwidthpercent, timerWidth);
    menu->AddIntToken((int)eRecMenuIT::overlapstartpercent, olStart);
    menu->AddIntToken((int)eRecMenuIT::overlapwidthpercent, olWidth);
}

eRecMenuState cRecMenuItemTimer::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kLeft:
            if (iconActive > 0) {
                iconActive--;
                return rmsRefresh;
            } else 
                return rmsNotConsumed;
            break;
        case kRight:
            if (iconActive < 3) {
                iconActive++;
                return rmsRefresh;
            } else 
                return rmsNotConsumed;
            break;
        case kOk:
            if (iconActive == 0)
                return action;
            else if (iconActive == 1)
                return action2;
            else if (iconActive == 2)
                return action3;
            else if (iconActive == 3)
                return action4;
            break;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemEvent  -------------------------------------------------------
cRecMenuItemEvent::cRecMenuItemEvent(const cEvent *event, 
                                     eRecMenuState action1, 
                                     eRecMenuState action2, 
                                     bool active) {
    height = 8;
    selectable = true;
    this->event = event;
    this->action = action1;
    this->action2 = action2;
    iconActive = 0;
    this->active = active;
}

cRecMenuItemEvent::~cRecMenuItemEvent(void) {
}

void cRecMenuItemEvent::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::event, 1);
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
    LOCK_CHANNELS_READ;
    const cChannel *channel = Channels->GetByChannelID(event->ChannelID());
#else
    const cChannel *channel = Channels.GetByChannelID(event->ChannelID());
#endif
    const char *channelName = NULL;
    cString channelId = "";
    if (channel) {
        channelName = channel->Name();
        channelId = channel->GetChannelID().ToString();
        menu->AddIntToken((int)eRecMenuIT::channelnumber, channel->Number());
    }

    menu->AddStringToken((int)eRecMenuST::channelname, channelName);    
    menu->AddStringToken((int)eRecMenuST::channelid, *channelId);
    menu->AddIntToken((int)eRecMenuIT::channellogoexisis, menu->ChannelLogoExists(*channelId));
    menu->AddStringToken((int)eRecMenuST::title, event->Title());
    menu->AddStringToken((int)eRecMenuST::shorttext, event->ShortText());
    menu->AddStringToken((int)eRecMenuST::date, *ShortDateString(event->StartTime()));
    menu->AddStringToken((int)eRecMenuST::weekday, *WeekDayName(event->StartTime()));
    menu->AddStringToken((int)eRecMenuST::starttime, *TimeString(event->StartTime()));
    menu->AddStringToken((int)eRecMenuST::stoptime, *TimeString(event->EndTime()));
    menu->AddIntToken((int)eRecMenuIT::hastimer, event->HasTimer());
}

eRecMenuState cRecMenuItemEvent::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kOk:
            return action;
            break;
        case kRed:
            if (!event->HasTimer())
                return action2;
            break;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemRecording  -------------------------------------------------------
cRecMenuItemRecording::cRecMenuItemRecording(const cRecording *recording, bool active) {
    height = 8;
    selectable = true;
    this->recording = recording;
    this->active = active;
}

void cRecMenuItemRecording::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::recording, 1);
    if (!recording)
        return;
    const cRecordingInfo *recInfo = recording->Info();
    const cChannel *channel = NULL;
    if (recInfo) {
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
        LOCK_CHANNELS_READ;
        channel = Channels->GetByChannelID(recInfo->ChannelID());
#else
        channel = Channels.GetByChannelID(recInfo->ChannelID());
#endif
    }
    string channelName = tr("unknown channel");
    string channelId = "";
    if (channel && channel->Name()) {
        channelName = channel->Name();
        channelId = *(channel->GetChannelID().ToString());
        menu->AddIntToken((int)eRecMenuIT::channelnumber, channel->Number());
    }
    menu->AddStringToken((int)eRecMenuST::channelname, channelName.c_str());    
    menu->AddStringToken((int)eRecMenuST::channelid, channelId.c_str());
    menu->AddIntToken((int)eRecMenuIT::channellogoexisis, menu->ChannelLogoExists(channelId));

    string recName = recording->Name() ? recording->Name() : "";
    string recDate = *ShortDateString(recording->Start());
    string recStartTime = *TimeString(recording->Start());
    int recDuration = recording->LengthInSeconds() / 60;

    menu->AddStringToken((int)eRecMenuST::recname, recName.c_str());
    menu->AddStringToken((int)eRecMenuST::recdate, recDate.c_str());
    menu->AddStringToken((int)eRecMenuST::recstarttime, recStartTime.c_str());
    menu->AddIntToken((int)eRecMenuIT::recduration, recDuration);
}

// --- cRecMenuItemSearchTimer  -------------------------------------------------------
cRecMenuItemSearchTimer::cRecMenuItemSearchTimer(cTVGuideSearchTimer timer, 
                                                 eRecMenuState action1,
                                                 eRecMenuState action2,
                                                 eRecMenuState action3,
                                                 bool active) {
    height = 8;
    this->timer = timer;
    this->action = action1;
    this->action2 = action2;
    this->action3 = action3;
    selectable = true;
    this->active = active;
    iconActive = 0;
}

cRecMenuItemSearchTimer::~cRecMenuItemSearchTimer(void) {
}

void cRecMenuItemSearchTimer::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::searchtimer, 1);
    menu->AddIntToken((int)eRecMenuIT::timeractive, timer.Active());
    menu->AddStringToken((int)eRecMenuST::searchstring, timer.SearchString().c_str());
    menu->AddIntToken((int)eRecMenuIT::activetimers, timer.GetNumTimers());
    menu->AddIntToken((int)eRecMenuIT::recordingsdone, timer.GetNumRecordings());
    menu->AddIntToken((int)eRecMenuIT::searchactive, (iconActive==0)?true:false);
    menu->AddIntToken((int)eRecMenuIT::editactive, (iconActive==1)?true:false);
    menu->AddIntToken((int)eRecMenuIT::deleteactive, (iconActive==2)?true:false);
}

eRecMenuState cRecMenuItemSearchTimer::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kLeft:
            if (iconActive > 0) {
                iconActive--;
                return rmsRefresh;
            }
            return rmsNotConsumed;
            break;
        case kRight: {
            if (iconActive < 2) {
                iconActive++;
                return rmsRefresh;
            }   
            return rmsNotConsumed;
            break; }
        case kOk:
            if (iconActive == 0)
                return action;
            else if (iconActive == 1)
                return action2;
            else if (iconActive == 2)
                return action3;
            break;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemTimelineHeader  -------------------------------------------------------
cRecMenuItemTimelineHeader::cRecMenuItemTimelineHeader(time_t day) {
    height = 15;
    timer = NULL;
    this->day = day;
    selectable = false;
    active = false;
}

cRecMenuItemTimelineHeader::~cRecMenuItemTimelineHeader(void) {
}

void cRecMenuItemTimelineHeader::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::timelineheader, 1);
    string daydate = *DateString(day);
    menu->AddStringToken((int)eRecMenuST::date, daydate.c_str());
    if (!timer) {
        menu->AddIntToken((int)eRecMenuIT::timerset, false);
        return;
    }
    menu->AddIntToken((int)eRecMenuIT::timerset, true);

    const cChannel *channel = timer->Channel();
    const char *channelName = NULL;
    cString channelId = "";
    int channelNumber = 0;
    int transponder = 0;
    if (channel) {
        channelName = channel->Name();
        channelId = channel->GetChannelID().ToString();
        channelNumber = channel->Number();
        transponder = channel->Transponder();
    }
    menu->AddStringToken((int)eRecMenuST::channelname, channelName);
    menu->AddStringToken((int)eRecMenuST::channelid, *channelId);
    menu->AddIntToken((int)eRecMenuIT::channelnumber, channelNumber);
    menu->AddIntToken((int)eRecMenuIT::channeltransponder, transponder);
    menu->AddIntToken((int)eRecMenuIT::channellogoexisis, menu->ChannelLogoExists(*channelId));

    menu->AddStringToken((int)eRecMenuST::timerstart, *TimeString(timer->StartTime()));
    menu->AddStringToken((int)eRecMenuST::timerstop, *TimeString(timer->StopTime()));

    const cEvent *event = timer->Event();
    const char *eventTitle = NULL;
    const char *eventShortText = "";
    cString eventStart = "";
    cString eventStop = "";
    if (event) {
        eventTitle = event->Title();
        eventShortText = event->ShortText();
        eventStart = event->GetTimeString();
        eventStop = event->GetEndTimeString();
    }
    menu->AddStringToken((int)eRecMenuST::eventtitle, eventTitle);
    menu->AddStringToken((int)eRecMenuST::eventshorttext, eventShortText);
    menu->AddStringToken((int)eRecMenuST::eventstart, *eventStart);
    menu->AddStringToken((int)eRecMenuST::eventstop, *eventStop);
}

// --- cRecMenuItemTimelineTimer  -------------------------------------------------------
cRecMenuItemTimelineTimer::cRecMenuItemTimelineTimer(const cTimer *timer, time_t start, time_t stop, bool active) {
    height = 8;
    this->timer = timer;
    this->start = start;
    this->stop = stop;
    selectable = true;
    this->active = active;
}

cRecMenuItemTimelineTimer::~cRecMenuItemTimelineTimer(void) {
}

void cRecMenuItemTimelineTimer::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::timelinetimer, 1);

    int secsPerDay = 24*60*60;    
    time_t timerStart = timer->StartTime() - start;
    time_t timerStop = timer->StopTime() - start;
    if (timerStart < 0)
        timerStart = 0;
    if (timerStop > secsPerDay)
        timerStop = secsPerDay;

    int percentStart = ((double)timerStart / (double)secsPerDay) * 1000;
    int percentWidth = ((double)(timerStop - timerStart) / (double)secsPerDay) * 1000;
    menu->AddIntToken((int)eRecMenuIT::timerstart, percentStart);
    menu->AddIntToken((int)eRecMenuIT::timerwidth, percentWidth);
}

const cTimer *cRecMenuItemTimelineTimer::GetTimer(void) {
    return timer;    
}

eRecMenuState cRecMenuItemTimelineTimer::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kOk:
            return rmsTimelineTimerEdit;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemFavorite  -------------------------------------------------------
cRecMenuItemFavorite::cRecMenuItemFavorite(cTVGuideSearchTimer favorite, 
                                           eRecMenuState action1,
                                           bool active) {
    height = 8;
    this->favorite = favorite;
    this->action = action1;
    selectable = true;
    this->active = active;
}

void cRecMenuItemFavorite::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::favorite, 1);
    menu->AddStringToken((int)eRecMenuST::favdesc, favorite.SearchString().c_str());
}

eRecMenuState cRecMenuItemFavorite::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kOk:
            return action;
        default:
            break;
    }
    return rmsNotConsumed;
}

// --- cRecMenuItemFavoriteStatic  -------------------------------------------------------
cRecMenuItemFavoriteStatic::cRecMenuItemFavoriteStatic(string text, eRecMenuState action, bool active) {
    height = 8;
    this->text = text;
    this->action = action;
    selectable = true;
    this->active = active;
}

void cRecMenuItemFavoriteStatic::SetTokens(skindesignerapi::cViewGrid *menu) {
    menu->ClearTokens();
    menu->AddIntToken((int)eRecMenuIT::favorite, 1);
    menu->AddStringToken((int)eRecMenuST::favdesc, text.c_str());
}

eRecMenuState cRecMenuItemFavoriteStatic::ProcessKey(eKeys Key) {
    switch (Key & ~k_Repeat) {
        case kOk:
            return action;
        default:
            break;
    }
    return rmsNotConsumed;
}

