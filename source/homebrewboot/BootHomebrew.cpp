#if 0
#include <gccore.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <ogc/machine/processor.h>
#include <wiiuse/wpad.h>
#include <vector>
#include <string>

#include "usbloader/usbstorage2.h"
#include "dolloader.h"
#include "fatmounter.h"
#include "sys.h"

extern const u8	app_booter_dol[];
extern const u32 app_booter_dol_size;

static u8 *homebrewbuffer = (u8 *) 0x92000000;
static int homebrewsize = 0;
static std::vector<std::string> Arguments;

void AddBootArgument(const char * argv)
{
    std::string arg(argv);
    Arguments.push_back(arg);
}

int CopyHomebrewMemory(u8 *temp, u32 pos, u32 len)
{
    homebrewsize += len;
    memcpy((homebrewbuffer)+pos, temp, len);

    return 1;
}

void FreeHomebrewBuffer()
{
    homebrewbuffer = (u8 *)0x92000000;
    homebrewsize = 0;
}

static int SetupARGV(struct __argv * args)
{
    if(!args)
        return -1;

    bzero(args, sizeof(struct __argv));
    args->argvMagic = ARGV_MAGIC;

    u32 stringlength = 1;

    /** Append Arguments **/
    for(u32 i = 0; i < Arguments.size(); i++)
    {
        stringlength += Arguments[i].size()+1;
    }

    args->length = stringlength;
    args->commandLine = (char*) malloc(args->length);
    if (!args->commandLine)
        return -1;

    u32 argc = 0;
    u32 position = 0;

    /** Append Arguments **/
    for(u32 i = 0; i < Arguments.size(); i++)
    {
        strcpy(&args->commandLine[position], Arguments[i].c_str());
        position += Arguments[i].size()+1;
        argc++;
    }

    args->argc = argc;

    args->commandLine[args->length - 1] = '\0';
    args->argv = &args->commandLine;
    args->endARGV = args->argv + 1;

    Arguments.clear();

    return 0;
}

int BootHomebrew()
{
    if(homebrewsize <= 0)
        Sys_BackToLoader();

    SDCard_deInit();
    USBDevice_deInit();
    USBStorage2_Deinit();

    struct __argv args;
    SetupARGV(&args);

    u32 cpu_isr;

    entrypoint entry = (entrypoint) load_dol(app_booter_dol, &args);

    if (!entry)
        Sys_BackToLoader();

    VIDEO_SetBlack(true);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
    _CPU_ISR_Disable (cpu_isr);
    __exception_closeall();
    entry();
    _CPU_ISR_Restore (cpu_isr);

    return 0;
}

int BootHomebrew(const char * filepath)
{
    FILE * file = fopen(filepath, "rb");
    if(!file)
        Sys_BackToLoader();

    fseek(file, 0, SEEK_END);

    int size = ftell(file);
    rewind(file);

    homebrewsize = fread(homebrewbuffer, 1, size, file);
    fclose(file);

    AddBootArgument(filepath);

    return BootHomebrew();
}
#endif

#include <gccore.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ogc/machine/processor.h>
#include <wiiuse/wpad.h>
#include <vector>
#include <string>
#include "../lstub.h"
#include "../sys.h"
#include "../gecko.h"

#include "fatmounter.h"
#include "dolloader.h"


void *innetbuffer = NULL;
static u8 *homebrewbuffer =( u8 * )0x92000000;
u32 homebrewsize = 0;
static std::vector<std::string> Arguments;

extern const u8		app_booter_dol[];

int AllocHomebrewMemory( u32 filesize )
{

    innetbuffer = malloc( filesize );

    if ( !innetbuffer )
	return -1;

    homebrewsize = filesize;
    return 1;
}

void AddBootArgument( const char * argv )
{
    std::string arg( argv );
    Arguments.push_back( arg );
}

int CopyHomebrewMemory( u8 *temp, u32 pos, u32 len )
{
    homebrewsize += len;
    memcpy(( homebrewbuffer ) + pos, temp, len );

    return 1;
}

void FreeHomebrewBuffer()
{
    homebrewbuffer = ( u8 * ) 0x92000000;
    homebrewsize = 0;

    if ( innetbuffer )
    {
	free( innetbuffer );
	innetbuffer = NULL;
    }

    Arguments.clear();
}

static int SetupARGV( struct __argv * args )
{
    if ( !args )
	return -1;

    bzero( args, sizeof( struct __argv ) );
    args->argvMagic = ARGV_MAGIC;

    u32 stringlength = 1;

    /** Append Arguments **/
    for ( u32 i = 0; i < Arguments.size(); i++ )
    {
	stringlength += Arguments[i].size() + 1;
    }

    args->length = stringlength;
    args->commandLine = ( char* ) malloc( args->length );

    if ( !args->commandLine )
	return -1;

    u32 argc = 0;
    u32 position = 0;

    /** Append Arguments **/
    for ( u32 i = 0; i < Arguments.size(); i++ )
    {
	strcpy( &args->commandLine[position], Arguments[i].c_str() );
	position += Arguments[i].size() + 1;
	argc++;
    }

    args->argc = argc;

    args->commandLine[args->length - 1] = '\0';
    args->argv = &args->commandLine;
    args->endARGV = args->argv + 1;

    Arguments.clear();

    return 0;
}

static int RunAppbooter()
{
    if ( homebrewsize == 0 )
	return -1;

    struct __argv args;
    SetupARGV( &args );

    u32 cpu_isr;

    entrypoint entry = ( entrypoint ) load_dol(( void* ) app_booter_dol, &args );

    if ( !entry )
    {
	FreeHomebrewBuffer();
	return -1;
    }

    u64 currentStub = getStubDest();
    loadStub();

    if ( Set_Stub_Split( 0x00010001, "UNEO" ) < 0 )
    {
	if ( Set_Stub_Split( 0x00010001, "ULNR" ) < 0 )
	{
	    if ( !currentStub )
		currentStub = 0x100000002ULL;

	    Set_Stub( currentStub );
	}
    }

    SDCard_deInit();
    USBDevice_deInit();

    WPAD_Flush( 0 );
    WPAD_Disconnect( 0 );
    WPAD_Shutdown();

    SYS_ResetSystem( SYS_SHUTDOWN, 0, 0 );
    _CPU_ISR_Disable( cpu_isr );
    __exception_closeall();
    entry();
    _CPU_ISR_Restore( cpu_isr );

    return 0;
}

int BootHomebrew( char * filepath )
{
    void *buffer = NULL;
    u32 filesize = 0;

    FILE *file = fopen( filepath, "rb" );

    if ( !file )
	Sys_BackToLoader();

    fseek( file, 0, SEEK_END );
    filesize = ftell( file );
    rewind( file );

    buffer = malloc( filesize );

    if ( fread( buffer, 1, filesize, file ) != filesize )
    {
	fclose( file );
	free( buffer );
	SDCard_deInit();
	USBDevice_deInit();
	Sys_BackToLoader();
    }

    fclose( file );

    CopyHomebrewMemory(( u8* ) buffer, 0, filesize );

    if ( buffer )
    {
	free( buffer );
	buffer = NULL;
    }

    AddBootArgument( filepath );
    return RunAppbooter();
}

int BootHomebrewFromMem()
{
    gprintf( "BootHomebrewFromMem()\n %p, %08x\n", innetbuffer, homebrewsize );

    if ( !innetbuffer )
    {
	gprintf( "!innetbuffer\n" );
	SDCard_deInit();
	USBDevice_deInit();
	Sys_BackToLoader();
    }

    CopyHomebrewMemory(( u8* ) innetbuffer, 0, homebrewsize );
    free( innetbuffer );

    return RunAppbooter();
}
