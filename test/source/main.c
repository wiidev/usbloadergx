/************************************************************/
/*                   USB speed test                         */
/*                                                          */
/*                                                          */
/************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <sys/dir.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <fcntl.h>
#include <debug.h>

#include <sdcard/gcsd.h>
#include <sdcard/wiisd_io.h>
#include <sdcard/gcsd.h>
#include <ogc/usbstorage.h>

#include <ogc/lwp_watchdog.h>

#include "mload.h"
#include "../build/ehcmodule_elf.h"
//#include "../build/fat_elf.h"
data_elf my_data_elf;
int my_thread_id=0;


#include <fat.h>

#include <sdcard/wiisd_io.h>
#include <sdcard/gcsd.h>
#include <ogc/usbstorage.h>
#include "usbstorage2.h"



const DISC_INTERFACE* sd = &__io_wiisd;

//const DISC_INTERFACE* usb1 = &__io_usb1storage;

static bool reset_pressed = false;
static bool power_pressed = false;

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;
static const u32 CACHE_PAGES = 8;

static u8 memsector[4*1024];


void initialise_video() 
{
    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync(); 
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync(); 
    printf("\x1b[2;0H");
    VIDEO_WaitVSync();
}

#include<ogc/lwp_watchdog.h>
//#define ticks_to_secs(ticks)		((u32)((u64)(ticks)/(u64)(TB_TIMER_CLOCK*1000)))
//#define ticks_to_msecs(ticks)		((u32)((u64)(ticks)/(u64)(TB_TIMER_CLOCK)))
/*
void showdir(char *path)
{
  char filename[MAXPATHLEN];
  DIR_ITER *dp;
  struct stat fstat;
  //chdir("usb:/");
  dp = diropen(path);

  if(dp==NULL) 
  {
    printf("Error diropen\n");
    return;
  }
  else printf("Ok diropen\n");
  
  int cnt=0;
  while( dirnext( dp, filename, &fstat ) == 0 )
  {
    //if(cnt==20) break;
    cnt++;

    if( fstat.st_mode & S_IFDIR ) printf("/"); 
    printf("%s\n",filename);	  
    	
  }
  dirclose(dp);
  printf("\nEnd show dirrectory (%d).\n",cnt);
}
*/
static void reset_cb (void) {
	reset_pressed = true;
}
static void power_cb (void) {
	power_pressed = true;
}

#include <errno.h>


#define VERSION "2.1"


void save_log() // at now all data is sent to usbgecko, sd log disabled
{
	FILE *fp;
	u8 *temp_data= NULL;
	u32 level,len;
	
	temp_data=memalign(32,256*1024);


	mload_init();
	len=mload_get_log();

	if(len>0)
		mload_read(temp_data, len);
	
	
	mload_close();
	
	for(len=0;len <2*4096 && temp_data[len]!=0;len++);
	/*
	fatUnmount("sd");
	if(fatMount("sd",sd,0,128,2) )
	{

		fp=fopen("sd:/log_usb2.txt","wb");

		if(fp!=0)
		{
			fprintf(fp,"USB2 device test program. v%s\n------------------------------\n",VERSION);
			fwrite(temp_data,1, len ,fp);
				
			fclose(fp);
		}
	}
	*/
	if (usb_isgeckoalive(1)) {
		level = IRQ_Disable();
		usb_sendbuffer(1, temp_data, len);
		IRQ_Restore(level);
	}
	
	//printf("\nlog:\n-------\n%s",	temp_data);
	free(temp_data);
}

void TryUSBDevice(int port)
{
	printf("Initializing usb device port: %i.\n",port);
	bool s;
	DISC_INTERFACE* usb;

	if(port==0) usb= &__io_usbstorage2_port0;
	else  usb= &__io_usbstorage2_port1;
	
	//USB2Enable(true);
	s=usb->startup();
	usleep(10000);
	/*
	if(s)
	{
		s = __usbstorage_ReadSectors(0, 1, memsector);
	}
	usleep(10000);*/

	
	if(s>0)
	{
	
		printf("usb device detected, your device is fully compatible with this usb2 driver(%i)\n",s);		
	}
	else 
	{
		//(s==-1) printf("usb device NOT INSERTED, log file not created\n");
		//se
		{
			printf("usb device NOT detected or not inserted, report log file(%i)\n",s);
			printf("Log saved to sd:/log_usb2.txt file\n");
		}
	}
	save_log();
}

void init_mload()
{
	int ret;
	ret=mload_init();
  
	if(ret<0)
	{
		printf("fail to get dev/mload, mload cios not detected. Quit\n");
		sleep(3);
		exit(0);
		//goto out;
	}
}
void loadehcimodule()
{
		u32 addr;
		int len;

		mload_get_load_base(&addr, &len);
		
		if(((u32) ehcmodule_elf) & 3) {printf("Unaligned elf!\n"); return;}
		mload_elf((void *) ehcmodule_elf, &my_data_elf);
		my_thread_id=mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
		//my_thread_id=mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
		if(my_thread_id<0)
		{
			printf("EHCI module can NOT be loaded (%i).. Quit\n",my_thread_id);
			sleep(3);
			exit(0);
		}
		
		printf("EHCI module loaded (%i).\n",my_thread_id);
		VIDEO_WaitVSync();
}

int main(int argc, char **argv) 
{
	int ret;
	bool usblan;
	int usblanport;
   
	initialise_video(); 
	CON_EnableGecko(1,0);

  	

	IOS_ReloadIOS(222);
  	init_mload();
	//mload_reset_log();
	loadehcimodule();
	
  	
  SYS_SetResetCallback (reset_cb);
  SYS_SetPowerCallback (power_cb);
  WPAD_Init();
//  usleep(10000);
//WIIDVD_Init(false);
//  usleep(10000);

//	TryUSBDevice(); // init usb2 device
//	usleep(10000);
//	save_log();
//	usleep(10000);
/*
	if(WIIDVD_DiscPresent()) printf("dvd inserted\n");
	else printf("dvd NOT inserted\n");
	usleep(10000);
	save_log();

	if(fatMount("usb",usb,0,3,128))
	{
		showdir("usb:/");
	}
	else printf("can't mount usb\n");	
	
	sleep(6);
	return 0;
	wait_for_network_initialisation(); // check if lan is working
	sleep(1);
*/	
  	printf("Press Home or reset to exit. Press A or power to try to detect usb device again.\n");

  	VIDEO_WaitVSync();
  		VIDEO_WaitVSync();
  	//exit(0);	
	while(1) //wait home press to exit
	{
		WPAD_ScanPads();
		if(reset_pressed) break;
		u32 pressed = WPAD_ButtonsDown(0);
		if ( pressed & WPAD_BUTTON_HOME ) 
		{
			mload_close();
			printf ("\n\nExit ...");
			exit(0);
		}
		if ( (pressed & WPAD_BUTTON_A) || power_pressed) 
		{
			power_pressed=false;
			
			TryUSBDevice(0);
			//usleep(5000);
			power_pressed=false;	
			/*
			if(fatMount("usb",usb,0,3,128))
			{
				showdir("usb:/");
			}
			else printf("can't mount usb\n");	
			*/
			//TryUSBDevice();
			/*
			printf("deinit (5secs)\n");
			sleep(1);
			net_deinit();
			sleep(4);
			printf("init\n");
			wait_for_network_initialisation();	
			*/	
			//power_pressed=false;			
		}
		if ( (pressed & WPAD_BUTTON_B) || power_pressed) 
		{
			power_pressed=false;
			
			TryUSBDevice(1);
			//usleep(5000);
			power_pressed=false;	
			/*
			if(fatMount("usb",usb,0,3,128))
			{
				showdir("usb:/");
			}
			else printf("can't mount usb\n");	
			*/
			//TryUSBDevice();
			/*
			printf("deinit (5secs)\n");
			sleep(1);
			net_deinit();
			sleep(4);
			printf("init\n");
			wait_for_network_initialisation();	
			*/	
			//power_pressed=false;			
		}
		usleep(500);
		VIDEO_WaitVSync();
   }
out:
	mload_close();
	printf ("\n\nExit ...");
	exit(0);		
}
