#include <malloc.h>
#include <string.h>
#include "FileOperations/fileops.h"
#include "filelist.h"
#include "Resources.h"

RecourceFile Resources::RecourceFiles[] =
{
    {"closebutton.png", closebutton_png, closebutton_png_size, NULL, 0},
    {"gxlogo.png", gxlogo_png, gxlogo_png_size, NULL, 0},
    {"sdcard.png", sdcard_png, sdcard_png_size, NULL, 0},
    {"sdcard_over.png", sdcard_over_png, sdcard_over_png_size, NULL, 0},
    {"Wifi_btn.png", Wifi_btn_png, Wifi_btn_png_size, NULL, 0},
    {"wiimote.png", wiimote_png, wiimote_png_size, NULL, 0},
    {"gameinfo1.png", gameinfo1_png, gameinfo1_png_size, NULL, 0},
    {"gameinfo2.png", gameinfo2_png, gameinfo2_png_size, NULL, 0},
    {"gameinfo1a.png", gameinfo1a_png, gameinfo1a_png_size, NULL, 0},
    {"gameinfo2a.png", gameinfo2a_png, gameinfo2a_png_size, NULL, 0},
    {"credits_button.png", credits_button_png, credits_button_png_size, NULL, 0},
    {"credits_button_over.png", credits_button_over_png, credits_button_over_png_size, NULL, 0},
    {"tooltip_left.png", tooltip_left_png, tooltip_left_png_size, NULL, 0},
    {"tooltip_tile.png", tooltip_tile_png, tooltip_tile_png_size, NULL, 0},
    {"tooltip_right.png", tooltip_right_png, tooltip_right_png_size, NULL, 0},
    {"startgame_arrow_left.png", startgame_arrow_left_png, startgame_arrow_left_png_size, NULL, 0},
    {"startgame_arrow_right.png", startgame_arrow_right_png, startgame_arrow_right_png_size, NULL, 0},
    {"credits_bg.png", credits_bg_png, credits_bg_png_size, NULL, 0},
    {"little_star.png", little_star_png, little_star_png_size, NULL, 0},
    {"background.png", background_png, background_png_size, NULL, 0},
    {"wbackground.png", wbackground_png, wbackground_png_size, NULL, 0},
    {"bg_options_settings.png", bg_options_settings_png, bg_options_settings_png_size, NULL, 0},
    {"settings_background.png", settings_background_png, settings_background_png_size, NULL, 0},
    {"bg_browser.png", bg_browser_png, bg_browser_png_size, NULL, 0},
    {"icon_folder.png", icon_folder_png, icon_folder_png_size, NULL, 0},
    {"bg_browser_selection.png", bg_browser_selection_png, bg_browser_selection_png_size, NULL, 0},
    {"addressbar_textbox.png", addressbar_textbox_png, addressbar_textbox_png_size, NULL, 0},
    {"browser.png", browser_png, browser_png_size, NULL, 0},
    {"browser_over.png", browser_over_png, browser_over_png_size, NULL, 0},
    {"nocover.png", nocover_png, nocover_png_size, NULL, 0},
    {"nocoverFlat.png", nocoverFlat_png, nocoverFlat_png_size, NULL, 0},
    {"nodisc.png", nodisc_png, nodisc_png_size, NULL, 0},
    {"theme_dialogue_box.png", theme_dialogue_box_png, theme_dialogue_box_png_size, NULL, 0},
    {"button_install.png", button_install_png, button_install_png_size, NULL, 0},
    {"button_install_over.png", button_install_over_png, button_install_over_png_size, NULL, 0},
    {"dialogue_box_startgame.png", dialogue_box_startgame_png, dialogue_box_startgame_png_size, NULL, 0},
    {"wdialogue_box_startgame.png", wdialogue_box_startgame_png, wdialogue_box_startgame_png_size, NULL, 0},
    {"button_dialogue_box.png", button_dialogue_box_png, button_dialogue_box_png_size, NULL, 0},
    {"keyboard_textbox.png", keyboard_textbox_png, keyboard_textbox_png_size, NULL, 0},
    {"keyboard_key.png", keyboard_key_png, keyboard_key_png_size, NULL, 0},
    {"keyboard_key_over.png", keyboard_key_over_png, keyboard_key_over_png_size, NULL, 0},
    {"keyboard_mediumkey_over.png", keyboard_mediumkey_over_png, keyboard_mediumkey_over_png_size, NULL, 0},
    {"keyboard_largekey_over.png", keyboard_largekey_over_png, keyboard_largekey_over_png_size, NULL, 0},
    {"keyboard_backspace_over.png", keyboard_backspace_over_png, keyboard_backspace_over_png_size, NULL, 0},
    {"keyboard_clear_over.png", keyboard_clear_over_png, keyboard_clear_over_png_size, NULL, 0},
    {"menu_button.png", menu_button_png, menu_button_png_size, NULL, 0},
    {"menu_button_over.png", menu_button_over_png, menu_button_over_png_size, NULL, 0},
    {"settings_button.png", settings_button_png, settings_button_png_size, NULL, 0},
    {"settings_button_over.png", settings_button_over_png, settings_button_over_png_size, NULL, 0},
    {"settings_menu_button.png", settings_menu_button_png, settings_menu_button_png_size, NULL, 0},
    {"wiimote_poweroff.png", wiimote_poweroff_png, wiimote_poweroff_png_size, NULL, 0},
    {"dialogue_box.png", dialogue_box_png, dialogue_box_png_size, NULL, 0},
    {"theme_box.png", theme_box_png, theme_box_png_size, NULL, 0},
    {"wiimote_poweroff_over.png", wiimote_poweroff_over_png, wiimote_poweroff_over_png_size, NULL, 0},
    {"bg_options.png", bg_options_png, bg_options_png_size, NULL, 0},
    {"bg_options_entry.png", bg_options_entry_png, bg_options_entry_png_size, NULL, 0},
    {"scrollbar.png", scrollbar_png, scrollbar_png_size, NULL, 0},
    {"scrollbar_arrowup.png", scrollbar_arrowup_png, scrollbar_arrowup_png_size, NULL, 0},
    {"scrollbar_arrowdown.png", scrollbar_arrowdown_png, scrollbar_arrowdown_png_size, NULL, 0},
    {"scrollbar_box.png", scrollbar_box_png, scrollbar_box_png_size, NULL, 0},
    {"progressbar.png", progressbar_png, progressbar_png_size, NULL, 0},
    {"progressbar_empty.png", progressbar_empty_png, progressbar_empty_png_size, NULL, 0},
    {"progressbar_outline.png", progressbar_outline_png, progressbar_outline_png_size, NULL, 0},
    {"player1_point.png", player1_point_png, player1_point_png_size, NULL, 0},
    {"player2_point.png", player2_point_png, player2_point_png_size, NULL, 0},
    {"player3_point.png", player3_point_png, player3_point_png_size, NULL, 0},
    {"player4_point.png", player4_point_png, player4_point_png_size, NULL, 0},
    {"rplayer1_point.png", rplayer1_point_png, rplayer1_point_png_size, NULL, 0},
    {"rplayer2_point.png", rplayer2_point_png, rplayer2_point_png_size, NULL, 0},
    {"rplayer3_point.png", rplayer3_point_png, rplayer3_point_png_size, NULL, 0},
    {"rplayer4_point.png", rplayer4_point_png, rplayer4_point_png_size, NULL, 0},
    {"battery.png", battery_png, battery_png_size, NULL, 0},
    {"battery_bar.png", battery_bar_png, battery_bar_png_size, NULL, 0},
    {"battery_white.png", battery_white_png, battery_white_png_size, NULL, 0},
    {"battery_red.png", battery_red_png, battery_red_png_size, NULL, 0},
    {"battery_bar_white.png", battery_bar_white_png, battery_bar_white_png_size, NULL, 0},
    {"battery_bar_red.png", battery_bar_red_png, battery_bar_red_png_size, NULL, 0},
    {"arrow_next.png", arrow_next_png, arrow_next_png_size, NULL, 0},
    {"arrow_previous.png", arrow_previous_png, arrow_previous_png_size, NULL, 0},
    {"mp3_pause.png", mp3_pause_png, mp3_pause_png_size, NULL, 0},
    {"mp3_stop.png", mp3_stop_png, mp3_stop_png_size, NULL, 0},
    {"exit_top.png", exit_top_png, exit_top_png_size, NULL, 0},
    {"exit_top_over.png", exit_top_over_png, exit_top_over_png_size, NULL, 0},
    {"exit_bottom.png", exit_bottom_png, exit_bottom_png_size, NULL, 0},
    {"exit_bottom_over.png", exit_bottom_over_png, exit_bottom_over_png_size, NULL, 0},
    {"exit_button.png", exit_button_png, exit_button_png_size, NULL, 0},
    {"favorite.png", favorite_png, favorite_png_size, NULL, 0},
    {"not_favorite.png", not_favorite_png, not_favorite_png_size, NULL, 0},
    {"favIcon.png", favIcon_png, favIcon_png_size, NULL, 0},
    {"favIcon_gray.png", favIcon_gray_png, favIcon_gray_png_size, NULL, 0},
    {"searchIcon.png", searchIcon_png, searchIcon_png_size, NULL, 0},
    {"searchIcon_gray.png", searchIcon_gray_png, searchIcon_gray_png_size, NULL, 0},
    {"abcIcon.png", abcIcon_png, abcIcon_png_size, NULL, 0},
    {"abcIcon_gray.png", abcIcon_gray_png, abcIcon_gray_png_size, NULL, 0},
    {"rankIcon.png", rankIcon_png, rankIcon_png_size, NULL, 0},
    {"rankIcon_gray.png", rankIcon_gray_png, rankIcon_gray_png_size, NULL, 0},
    {"playCountIcon.png", playCountIcon_png, playCountIcon_png_size, NULL, 0},
    {"playCountIcon_gray.png", playCountIcon_gray_png, playCountIcon_gray_png_size, NULL, 0},
    {"arrangeList.png", arrangeList_png, arrangeList_png_size, NULL, 0},
    {"arrangeList_gray.png", arrangeList_gray_png, arrangeList_gray_png_size, NULL, 0},
    {"arrangeGrid.png", arrangeGrid_png, arrangeGrid_png_size, NULL, 0},
    {"arrangeGrid_gray.png", arrangeGrid_gray_png, arrangeGrid_gray_png_size, NULL, 0},
    {"arrangeCarousel.png", arrangeCarousel_png, arrangeCarousel_png_size, NULL, 0},
    {"arrangeCarousel_gray.png", arrangeCarousel_gray_png, arrangeCarousel_gray_png_size, NULL, 0},
    {"settings_title.png", settings_title_png, settings_title_png_size, NULL, 0},
    {"settings_title_over.png", settings_title_over_png, settings_title_over_png_size, NULL, 0},
    {"pageindicator.png", pageindicator_png, pageindicator_png_size, NULL, 0},
    {"Wiimote1.png", Wiimote1_png, Wiimote1_png_size, NULL, 0},
    {"Wiimote2.png", Wiimote2_png, Wiimote1_png_size, NULL, 0},
    {"Wiimote4.png", Wiimote4_png, Wiimote4_png_size, NULL, 0},
    {"wifi1.png", wifi1_png, wifi1_png_size, NULL, 0},
    {"wifi2.png", wifi2_png, wifi2_png_size, NULL, 0},
    {"wifi3.png", wifi3_png, wifi3_png_size, NULL, 0},
    {"wifi4.png", wifi4_png, wifi4_png_size, NULL, 0},
    {"wifi8.png", wifi8_png, wifi8_png_size, NULL, 0},
    {"wifi12.png", wifi12_png, wifi12_png_size, NULL, 0},
    {"wifi16.png", wifi16_png, wifi16_png_size, NULL, 0},
    {"wifi32.png", wifi32_png, wifi32_png_size, NULL, 0},
    {"norating.png", norating_png, norating_png_size, NULL, 0},
    {"guitar.png", guitar_png, guitar_png_size, NULL, 0},
    {"guitarR.png", guitarR_png, guitarR_png_size, NULL, 0},
    {"microphone.png", microphone_png, microphone_png_size, NULL, 0},
    {"microphoneR.png", microphoneR_png, microphoneR_png_size, NULL, 0},
    {"gcncontroller.png", gcncontroller_png, gcncontroller_png_size, NULL, 0},
    {"gcncontrollerR.png", gcncontrollerR_png, gcncontrollerR_png_size, NULL, 0},
    {"classiccontroller.png", classiccontroller_png, classiccontroller_png_size, NULL, 0},
    {"classiccontrollerR.png", classiccontrollerR_png, classiccontrollerR_png_size, NULL, 0},
    {"nunchuk.png", nunchuk_png, nunchuk_png_size, NULL, 0},
    {"nunchukR.png", nunchukR_png, nunchukR_png_size, NULL, 0},
    {"dancepad.png", dancepad_png, dancepad_png_size, NULL, 0},
    {"dancepadR.png", dancepadR_png, dancepadR_png_size, NULL, 0},
    {"balanceboard.png", balanceboard_png, balanceboard_png_size, NULL, 0},
    {"balanceboardR.png", balanceboardR_png, balanceboardR_png_size, NULL, 0},
    {"drums.png", drums_png, drums_png_size, NULL, 0},
    {"drumsR.png", drumsR_png, drumsR_png_size, NULL, 0},
    {"motionplus.png", motionplus_png, motionplus_png_size, NULL, 0},
    {"motionplusR.png", motionplusR_png, motionplusR_png_size, NULL, 0},
    {"wheel.png", wheel_png, wheel_png_size, NULL, 0},
    {"wheelR.png", wheelR_png, wheelR_png_size, NULL, 0},
    {"zapper.png", zapper_png, zapper_png_size, NULL, 0},
    {"zapperR.png", zapperR_png, zapperR_png_size, NULL, 0},
    {"wiispeak.png", wiispeak_png, wiispeak_png_size, NULL, 0},
    {"wiispeakR.png", wiispeakR_png, wiispeakR_png_size, NULL, 0},
    {"nintendods.png", nintendods_png, nintendods_png_size, NULL, 0},
    {"nintendodsR.png", nintendodsR_png, nintendodsR_png_size, NULL, 0},
    {"esrb_ec.png", esrb_ec_png, esrb_ec_png_size, NULL, 0},
    {"esrb_e.png", esrb_e_png, esrb_e_png_size, NULL, 0},
    {"esrb_eten.png", esrb_eten_png, esrb_eten_png_size, NULL, 0},
    {"esrb_t.png", esrb_t_png, esrb_t_png_size, NULL, 0},
    {"esrb_m.png", esrb_m_png, esrb_m_png_size, NULL, 0},
    {"esrb_ao.png", esrb_ao_png, esrb_ao_png_size, NULL, 0},
    {"cero_a.png", cero_a_png, cero_a_png_size, NULL, 0},
    {"cero_b.png", cero_b_png, cero_b_png_size, NULL, 0},
    {"cero_c.png", cero_c_png, cero_c_png_size, NULL, 0},
    {"cero_d.png", cero_d_png, cero_d_png_size, NULL, 0},
    {"cero_z.png", cero_z_png, cero_z_png_size, NULL, 0},
    {"pegi_3.png", pegi_3_png, pegi_3_png_size, NULL, 0},
    {"pegi_7.png", pegi_7_png, pegi_7_png_size, NULL, 0},
    {"pegi_12.png", pegi_12_png, pegi_12_png_size, NULL, 0},
    {"pegi_16.png", pegi_16_png, pegi_16_png_size, NULL, 0},
    {"pegi_18.png", pegi_18_png, pegi_18_png_size, NULL, 0},
    {"dvd.png", dvd_png, dvd_png_size, NULL, 0},
    {"dvd_gray.png", dvd_gray_png, dvd_gray_png_size, NULL, 0},
    {"new.png", new_png, new_png_size, NULL, 0},
    {"lock.png", lock_png, lock_png_size, NULL, 0},
    {"lock_gray.png", lock_gray_png, lock_gray_png_size, NULL, 0},
    {"unlock.png", unlock_png, unlock_png_size, NULL, 0},
    {"unlock_gray.png", unlock_gray_png, unlock_gray_png_size, NULL, 0},
    {NULL, NULL, 0, NULL, 0}
};

void Resources::Clear()
{
    for(int i = 0; RecourceFiles[i].filename != NULL; ++i)
    {
        if(RecourceFiles[i].CustomFile)
        {
            free(RecourceFiles[i].CustomFile);
            RecourceFiles[i].CustomFile = NULL;
        }

        if(RecourceFiles[i].CustomFileSize != 0)
            RecourceFiles[i].CustomFileSize = 0;
    }
}

void Resources::LoadFiles(const char * path)
{
    if(!path)
        return;

    Clear();

    char fullpath[1024];

    for(int i = 0; RecourceFiles[i].filename != NULL; ++i)
    {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, RecourceFiles[i].filename);

        if(CheckFile(fullpath))
        {
            u8 * buffer = NULL;
            u64 filesize = 0;

            LoadFileToMem(fullpath, &buffer, &filesize);

            RecourceFiles[i].CustomFile = buffer;
            RecourceFiles[i].CustomFileSize = (u32) filesize;
        }
    }
}

const u8 * Resources::GetFile(const char * filename)
{
    for(int i = 0; RecourceFiles[i].filename != NULL; ++i)
    {
        if(strcasecmp(filename, RecourceFiles[i].filename) == 0)
        {
            return (RecourceFiles[i].CustomFile ? RecourceFiles[i].CustomFile : RecourceFiles[i].DefaultFile);
        }
    }

    return NULL;
}

const u32 Resources::GetFileSize(const char * filename)
{
    for(int i = 0; RecourceFiles[i].filename != NULL; ++i)
    {
        if(strcasecmp(filename, RecourceFiles[i].filename) == 0)
        {
            return (RecourceFiles[i].CustomFile ? RecourceFiles[i].CustomFileSize : RecourceFiles[i].DefaultFileSize);
        }
    }

    return NULL;
}

GuiImageData * Resources::GetImageData(const char * filename)
{
    for(int i = 0; RecourceFiles[i].filename != NULL; ++i)
    {
        if(strcasecmp(filename, RecourceFiles[i].filename) == 0)
        {
            const u8 * buff = RecourceFiles[i].CustomFile ? RecourceFiles[i].CustomFile : RecourceFiles[i].DefaultFile;
            const u32 size = RecourceFiles[i].CustomFile ? RecourceFiles[i].CustomFileSize : RecourceFiles[i].DefaultFileSize;
            return (new GuiImageData(buff, size));
        }
    }

    return NULL;
}
