#pragma once
#include "types.h"

class EventHandler {
public:
    virtual bool IsActive() = 0;
    virtual void OnKeyDown(char keybuffer, byte scancode, byte modifiers) = 0;
    virtual void OnKeyUp(char keybuffer, byte scancode, byte modifiers) = 0;
    virtual void OnMouseMoveClick(byte buttons, dword x, dword y) = 0;
};
