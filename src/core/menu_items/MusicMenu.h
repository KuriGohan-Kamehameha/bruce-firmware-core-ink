#ifndef __MUSIC_MENU_H__
#define __MUSIC_MENU_H__

#include <MenuItemInterface.h>

class MusicMenu : public MenuItemInterface {
public:
    MusicMenu() : MenuItemInterface("Music") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return bruceConfig.theme.music; }
    String themePath() { return bruceConfig.theme.paths.music; }
};

#endif
