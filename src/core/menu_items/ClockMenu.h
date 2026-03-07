#ifndef __CLOCK_MENU_H__
#define __CLOCK_MENU_H__

#include <MenuItemInterface.h>

class ClockMenu : public MenuItemInterface {
public:
    ClockMenu() : MenuItemInterface("Clock") {}

    void optionsMenu(void);
    void showSubMenu(void);
    void drawIcon(float scale);
    bool showClockViewSubMenu(void);
    void showClockToolsSubMenu(void);
    void showClockSettingsSubMenu(void);
    void showAnalogSettingsSubMenu(void);
    void showAnalogFaceSettingsSubMenu(void);
    void showAnalogHandSettingsSubMenu(void);
    void showAnalogComplicationsSubMenu(void);
    void showChimeSettingsSubMenu(void);
    void showAlarmSettingsSubMenu(void);
    void runWorldClockLoop(bool showMenuHint = false);
    bool hasTheme() { return bruceConfig.theme.clock; }
    String themePath() { return bruceConfig.theme.paths.clock; }

private:
    enum class ViewMode : uint8_t {
        Analog = 0,
        Digital,
        Stopwatch,
    };
    ViewMode viewMode = ViewMode::Analog;
};

#endif
