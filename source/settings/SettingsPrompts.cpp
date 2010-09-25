#include <gccore.h>
#include <unistd.h>
#include <string.h>

#include "language/gettext.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "settings/CSettings.h"
#include "themes/CTheme.h"
#include "network/URL_List.h"
#include "FileOperations/fileops.h"
#include "FileOperations/DirList.h"
#include "main.h"
#include "fatmounter.h"
#include "filelist.h"
#include "prompts/filebrowser.h"
#include "sys.h"
#include "menu/menus.h"

/*** Extern variables ***/
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();


/****************************************************************************
 * MenuOGG
 ***************************************************************************/
bool MenuBackgroundMusic()
{
    bool ret = false;
    char entered[1024];
    int result = -1;
    snprintf( entered, sizeof( entered ), "%s", Settings.ogg_path );

    if ( strcmp( entered, "" ) == 0 )
    {
        sprintf( entered, "%s", bootDevice );
    }
    else
    {
        char * pathptr = strrchr( entered, '/' );
        if ( pathptr )
        {
            pathptr++;
            int choice = WindowPrompt( tr( "Playing Music:" ), pathptr, tr( "Play Previous" ), tr( "Play Next" ), tr( "Change Play Path" ), tr( "Cancel" ) );
            if ( choice == 1 )
            {
                return bgMusic->PlayPrevious();
            }
            else if ( choice == 2 )
            {
                return bgMusic->PlayNext();
            }
            else if ( choice == 3 )
            {
                pathptr[0] = 0;
            }
            else
                return true;
        }
        else
            sprintf( entered, "%s", bootDevice );
    }

    result = BrowseDevice( entered, sizeof( entered ), FB_DEFAULT );

    if ( result )
    {
        if ( !bgMusic->Load( entered ) )
        {
            WindowPrompt( tr( "Not supported format!" ), tr( "Loading standard music." ), tr( "OK" ) );
        }
        else
            ret = true;
        bgMusic->Play();
        bgMusic->SetVolume( Settings.volume );
    }

    return ret;
}

/****************************************************************************
 * MenuLanguageSelect
 ***************************************************************************/
int MenuLanguageSelect()
{
    int cnt = 0;
    int ret = 0, choice = 0;
    int scrollon;
    int returnhere = 0;

    GuiSound btnSoundOver( button_over_pcm, button_over_pcm_size, Settings.sfxvolume );
    // because destroy GuiSound must wait while sound playing is finished, we use a global sound
    if ( !btnClick2 ) btnClick2 = new GuiSound( button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume );
    //  GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    char imgPath[100];

    snprintf( imgPath, sizeof( imgPath ), "%sbutton_dialogue_box.png", Settings.theme_path );
    GuiImageData btnOutline( imgPath, button_dialogue_box_png );
    snprintf( imgPath, sizeof( imgPath ), "%ssettings_background.png", Settings.theme_path );
    GuiImageData settingsbg( imgPath, settings_background_png );

    GuiTrigger trigA;
    trigA.SetSimpleTrigger( -1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A );
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger( -1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B );

    char fullpath[100];
    DirList Dir( Settings.languagefiles_path );

    if ( !strcmp( "", Settings.languagefiles_path ) )
    {
        sprintf( fullpath, "%s", tr( "Standard" ) );
    }
    else
    {
        sprintf( fullpath, "%s", Settings.languagefiles_path );
    }

    GuiText titleTxt( fullpath, 24, ( GXColor ) {0, 0, 0, 255} );
    titleTxt.SetAlignment( ALIGN_CENTRE, ALIGN_MIDDLE );
    titleTxt.SetPosition( 0, 0 );
    GuiButton pathBtn( 300, 50 );
    pathBtn.SetAlignment( ALIGN_CENTRE, ALIGN_TOP );
    pathBtn.SetPosition( 0, 28 );
    pathBtn.SetLabel( &titleTxt );
    pathBtn.SetSoundOver( &btnSoundOver );
    pathBtn.SetSoundClick( btnClick2 );
    pathBtn.SetTrigger( &trigA );
    pathBtn.SetEffectGrow();

    GuiImage oggmenubackground( &settingsbg );
    oggmenubackground.SetAlignment( ALIGN_LEFT, ALIGN_TOP );
    oggmenubackground.SetPosition( 0, 0 );

    GuiText backBtnTxt( tr( "Back" ) , 22, Theme.prompttext );
    backBtnTxt.SetMaxWidth( btnOutline.GetWidth() - 30 );
    GuiImage backBtnImg( &btnOutline );
    if ( Settings.wsprompt == yes )
    {
        backBtnTxt.SetWidescreen( Settings.widescreen );
        backBtnImg.SetWidescreen( Settings.widescreen );
    }
    GuiButton backBtn( btnOutline.GetWidth(), btnOutline.GetHeight() );
    backBtn.SetAlignment( ALIGN_CENTRE, ALIGN_TOP );
    backBtn.SetPosition( -190, 400 );
    backBtn.SetLabel( &backBtnTxt );
    backBtn.SetImage( &backBtnImg );
    backBtn.SetSoundOver( &btnSoundOver );
    backBtn.SetSoundClick( btnClick2 );
    backBtn.SetTrigger( &trigA );
    backBtn.SetTrigger( &trigB );
    backBtn.SetEffectGrow();

    GuiText defaultBtnTxt( tr( "Default" ) , 22, Theme.prompttext );
    defaultBtnTxt.SetMaxWidth( btnOutline.GetWidth() - 30 );
    GuiImage defaultBtnImg( &btnOutline );
    if ( Settings.wsprompt == yes )
    {
        defaultBtnTxt.SetWidescreen( Settings.widescreen );
        defaultBtnImg.SetWidescreen( Settings.widescreen );
    }
    GuiButton defaultBtn( btnOutline.GetWidth(), btnOutline.GetHeight() );
    defaultBtn.SetAlignment( ALIGN_CENTRE, ALIGN_TOP );
    defaultBtn.SetPosition( 190, 400 );
    defaultBtn.SetLabel( &defaultBtnTxt );
    defaultBtn.SetImage( &defaultBtnImg );
    defaultBtn.SetSoundOver( &btnSoundOver );
    defaultBtn.SetSoundClick( btnClick2 );
    defaultBtn.SetTrigger( &trigA );
    defaultBtn.SetEffectGrow();

    GuiText updateBtnTxt( tr( "Update Files" ) , 22, Theme.prompttext );
    updateBtnTxt.SetMaxWidth( btnOutline.GetWidth() - 30 );
    GuiImage updateBtnImg( &btnOutline );
    if ( Settings.wsprompt == yes )
    {
        updateBtnTxt.SetWidescreen( Settings.widescreen );
        updateBtnImg.SetWidescreen( Settings.widescreen );
    }
    GuiButton updateBtn( btnOutline.GetWidth(), btnOutline.GetHeight() );
    updateBtn.SetAlignment( ALIGN_CENTRE, ALIGN_TOP );
    updateBtn.SetPosition( 0, 400 );
    updateBtn.SetLabel( &updateBtnTxt );
    updateBtn.SetImage( &updateBtnImg );
    updateBtn.SetSoundOver( &btnSoundOver );
    updateBtn.SetSoundClick( btnClick2 );
    updateBtn.SetTrigger( &trigA );
    updateBtn.SetEffectGrow();

    customOptionList options2( Dir.GetFilecount() );

    for ( cnt = 0; cnt < Dir.GetFilecount(); cnt++ )
    {
        char filename[64];
        strlcpy( filename, Dir.GetFilename( cnt ), sizeof( filename ) );
        char *dot = strchr( filename, '.' );
        if ( dot ) *dot = '\0';
        options2.SetName( cnt, "%s", filename );
        options2.SetValue( cnt, NULL );

    }

    if ( cnt < 9 )
    {
        scrollon = 0;
    }
    else
    {
        scrollon = 1;
    }

    GuiCustomOptionBrowser optionBrowser4( 396, 280, &options2, Settings.theme_path, "bg_options_settings.png", bg_options_settings_png, scrollon, 10 );
    optionBrowser4.SetPosition( 0, 90 );
    optionBrowser4.SetAlignment( ALIGN_CENTRE, ALIGN_TOP );

    HaltGui();
    GuiWindow w( screenwidth, screenheight );
    w.Append( &oggmenubackground );
    w.Append( &pathBtn );
    w.Append( &backBtn );
    w.Append( &defaultBtn );
    w.Append( &updateBtn );
    w.Append( &optionBrowser4 );
    mainWindow->Append( &w );

    w.SetEffect( EFFECT_FADE, 20 );
    ResumeGui();

    while ( w.GetEffect() > 0 ) usleep( 50 );

    while ( !returnhere )
    {

        if ( shutdown == 1 )
            Sys_Shutdown();
        else if ( reset == 1 )
            Sys_Reboot();

        else if ( backBtn.GetState() == STATE_CLICKED )
        {

            backBtn.ResetState();
            break;
        }

        else if ( defaultBtn.GetState() == STATE_CLICKED )
        {
            choice = WindowPrompt( tr( "Loading standard language." ), 0, tr( "OK" ), tr( "Cancel" ) );
            if ( choice == 1 )
            {
                strcpy( Settings.language_path, "" );
                Settings.Save();
                gettextCleanUp();
                HaltGui();
                Settings.Load();
                ResumeGui();
                returnhere = 2;
            }
            defaultBtn.ResetState();
            //optionBrowser4.SetFocus(1); // commented out to prevent crash
        }

        else if ( updateBtn.GetState() == STATE_CLICKED )
        {
            choice = WindowPrompt( tr( "Update all Language Files" ), tr( "Do you wish to update/download all language files?" ), tr( "OK" ), tr( "Cancel" ) );
            if ( choice == 1 )
            {

                bool network = true;
                if ( !IsNetworkInit() )
                {
                    network = NetworkInitPrompt();
                }

                if ( network )
                {
                    const char URL[60] = "http://usbloader-gui.googlecode.com/svn/trunk/Languages/";
                    char fullURL[300];
                    FILE *pfile;

                    URL_List LinkList( URL );
                    int listsize = LinkList.GetURLCount();

                    CreateSubfolder( Settings.languagefiles_path );

                    for ( int i = 0; i < listsize; i++ )
                    {

                        ShowProgress( tr( "Updating Language Files:" ), 0, LinkList.GetURL( i ), i, listsize - 1 );

                        if ( strcasecmp( ".lang", strrchr( LinkList.GetURL( i ), '.' ) ) == 0 )
                        {

                            snprintf( fullURL, sizeof( fullURL ), "%s%s", URL, LinkList.GetURL( i ) );

                            struct block file = downloadfile( fullURL );

                            if ( file.data && file.size )
                            {
                                char filepath[300];

                                snprintf( filepath, sizeof( filepath ), "%s%s", Settings.languagefiles_path, LinkList.GetURL( i ) );
                                pfile = fopen( filepath, "wb" );
                                fwrite( file.data, 1, file.size, pfile );
                                fclose( pfile );

                            }

                            free( file.data );
                        }
                    }
                    ProgressStop();
                    returnhere = 1;
                    break;
                }
            }
            updateBtn.ResetState();
            //optionBrowser4.SetFocus(1); // commented out to prevent crash
        }

        else if ( pathBtn.GetState() == STATE_CLICKED )
        {
            w.Remove( &optionBrowser4 );
            w.Remove( &backBtn );
            w.Remove( &pathBtn );
            w.Remove( &defaultBtn );
            char entered[43] = "";
            strlcpy( entered, Settings.languagefiles_path, sizeof( entered ) );
            int result = OnScreenKeyboard( entered, 43, 0 );
            w.Append( &optionBrowser4 );
            w.Append( &pathBtn );
            w.Append( &backBtn );
            w.Append( &defaultBtn );
            if ( result == 1 )
            {
                int len = ( strlen( entered ) - 1 );
                if ( entered[len] != '/' )
                    strncat ( entered, "/", 1 );
                strlcpy( Settings.languagefiles_path, entered, sizeof( Settings.languagefiles_path ) );
                WindowPrompt( tr( "Languagepath changed." ), 0, tr( "OK" ) );
                if ( isInserted( bootDevice ) )
                {
                    Settings.Save();
                    returnhere = 1;
                    break;
                }
                else
                {
                    WindowPrompt( tr( "No SD-Card inserted!" ), tr( "Insert an SD-Card to save." ), tr( "OK" ) );
                }
            }
            if ( Dir.GetFilecount() > 0 )
            {
                optionBrowser4.SetFocus( 1 );
            }
            pathBtn.ResetState();
        }

        ret = optionBrowser4.GetClickedOption();

        if ( ret >= 0 )
        {
            choice = WindowPrompt( tr( "Do you want to change language?" ), 0, tr( "Yes" ), tr( "Cancel" ) );
            if ( choice == 1 )
            {
                if ( isInserted( bootDevice ) )
                {
                    snprintf( Settings.language_path, sizeof( Settings.language_path ), "%s%s", Settings.languagefiles_path, Dir.GetFilename( ret ) );
                    Settings.Save();
                    if ( !CheckFile( Settings.language_path ) )
                    {
                        sprintf( Settings.language_path, tr( "not set" ) );
                        WindowPrompt( tr( "File not found." ), tr( "Loading standard language." ), tr( "OK" ) );
                    }
                    gettextCleanUp();
                    HaltGui();
                    Settings.Load();
                    ResumeGui();
                    returnhere = 2;
                    break;
                }
                else
                {
                    WindowPrompt( tr( "No SD-Card inserted!" ), tr( "Insert an SD-Card to save." ), tr( "OK" ), 0, 0, 0, -1 );
                }
            }
            optionBrowser4.SetFocus( 1 );
        }

    }

    w.SetEffect( EFFECT_FADE, -20 );
    while ( w.GetEffect() > 0 ) usleep( 50 );

    HaltGui();
    mainWindow->Remove( &w );
    ResumeGui();

    return returnhere;
}

