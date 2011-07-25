#include "mload_modules.h"

static u32 ios_36[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022DDAC, // dvd_read_controlling_data
	0x20201010+1, // handle_di_cmd_reentry (thumb)
	0x20200b9c+1, // ios_shared_alloc_aligned (thumb)
	0x20200b70+1, // ios_shared_free (thumb)
	0x20205dc0+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202b4c+1, // ios_doReadHashEncryptedState (thumb)
	0x20203934+1, // ios_printf (thumb)
};

static u32 ios_38[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022cdac, // dvd_read_controlling_data
	0x20200d38+1, // handle_di_cmd_reentry (thumb)
	0x202008c4+1, // ios_shared_alloc_aligned (thumb)
	0x20200898+1, // ios_shared_free (thumb)
	0x20205b80+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202874+1, // ios_doReadHashEncryptedState (thumb)
	0x2020365c+1, // ios_printf (thumb)
};


static u32 ios_37[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022DD60, // dvd_read_controlling_data
	0x20200F04+1, // handle_di_cmd_reentry (thumb)
	0x2020096C+1, // ios_shared_alloc_aligned (thumb)
	0x2020093C+1, // ios_shared_free (thumb)
	0x20205E54+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202A70+1, // ios_doReadHashEncryptedState (thumb)
	0x2020387C+1, // ios_printf (thumb)
};

static u32 ios_57[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022cd60, // dvd_read_controlling_data
	0x20200f04+1, // handle_di_cmd_reentry (thumb)
	0x2020096c+1, // ios_shared_alloc_aligned (thumb) // no usado
	0x2020093C+1, // ios_shared_free (thumb) // no usado
	0x20205EF0+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202944+1, // ios_doReadHashEncryptedState (thumb)
	0x20203750+1, // ios_printf (thumb)
};

static u32 ios_60[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022cd60, // dvd_read_controlling_data
	0x20200f04+1, // handle_di_cmd_reentry (thumb)
	0x2020096c+1, // ios_shared_alloc_aligned (thumb) // no usado
	0x2020093C+1, // ios_shared_free (thumb) // no usado
	0x20205e00+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202944+1, // ios_doReadHashEncryptedState (thumb)
	0x20203750+1, // ios_printf (thumb)
};


static u32 patch_datas[8] ATTRIBUTE_ALIGN(32);
static int my_thread_id = 0;
static data_elf my_data_elf;
static  u8 * dip_plugin = NULL;
static  u32 dip_plugin_size = 0;

int load_modules(const u8 * ehcmodule, int ehcmodule_size, const u8 * dip, int dip_size)
{
	if(mload_init() < 0)
		return -1;

	dip_plugin = (u8 *) dip;
	dip_plugin_size = dip_size;

	mload_elf((u8 *) ehcmodule, &my_data_elf);
	my_thread_id= mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);

	if(my_thread_id < 0)
		return -2;
	usleep(350*1000);

	// Test for IOS
	int is_ios = mload_get_IOS_base();
	u32 dip_address = 0x1377C000;

	switch(is_ios)
	{

		case 36:

			memcpy(ios_36, dip_plugin, 4);		// copy the entry_point
			memcpy(dip_plugin, ios_36, 4*10);	// copy the adresses from the array

			mload_seek(dip_address, SEEK_SET);	// copy dip_plugin in the starlet
			mload_write(dip_plugin, dip_plugin_size);

			// enables DIP plugin
			mload_seek(0x20209040, SEEK_SET);
			mload_write(ios_36, 4);
			break;

		case 37:

			memcpy(ios_37, dip_plugin, 4);		// copy the entry_point
			memcpy(dip_plugin, ios_37, 4*10);   // copy the adresses from the array

			mload_seek(dip_address, SEEK_SET);	// copy dip_plugin in the starlet
			mload_write(dip_plugin,dip_plugin_size);

			// enables DIP plugin
			mload_seek(0x20209030, SEEK_SET);
			mload_write(ios_37, 4);
			break;

		case 38:

			memcpy(ios_38, dip_plugin, 4);		// copy the entry_point
			memcpy(dip_plugin, ios_38, 4*10);   // copy the adresses from the array

			mload_seek(dip_address, SEEK_SET);	// copy dip_plugin in the starlet
			mload_write(dip_plugin,dip_plugin_size);

			// enables DIP plugin
			mload_seek(0x20208030, SEEK_SET);
			mload_write(ios_38, 4);
			break;

		case 57:

			memcpy(ios_57, dip_plugin, 4);		// copy the entry_point
			memcpy(dip_plugin, ios_57, 4*10);   // copy the adresses from the array

			mload_seek(dip_address, SEEK_SET);	// copy dip_plugin in the starlet
			mload_write(dip_plugin,dip_plugin_size);

			// enables DIP plugin
			mload_seek(0x20208030, SEEK_SET);
			mload_write(ios_57, 4);
			break;

		case 60:

			memcpy(ios_60, dip_plugin, 4);		// copy the entry_point
			memcpy(dip_plugin, ios_60, 4*10);   // copy the adresses from the array

			mload_seek(dip_address, SEEK_SET);	// copy dip_plugin in the starlet
			mload_write(dip_plugin,dip_plugin_size);

			// enables DIP plugin
			mload_seek(0x20208030, SEEK_SET);
			mload_write(ios_60, 4);
			break;

	}
	mload_close();
	return 0;
}

void enable_ES_ioctlv_vector(void)
{
	mload_init();
	patch_datas[0]=*((u32 *) (dip_plugin+16*4));
	mload_set_ES_ioctlv_vector((void *) patch_datas[0]);
	mload_close();
}

void Set_DIP_BCA_Datas(u8 *bca_data)
{
	// write in dip_plugin bca data area
	mload_init();
	mload_seek(*((u32 *) (dip_plugin+15*4)), SEEK_SET);	// offset 15 (bca_data area)
	mload_write(bca_data, 64);
	mload_close();
}

u8 *search_for_ehcmodule_cfg(u8 *p, int size)
{
	int n;

	for(n=0;n<size;n++)
	{
		if(!memcmp((void *) &p[n],"EHC_CFG",8) && p[n+8]==0x12 && p[n+9]==0x34 && p[n+10]==0x00 && p[n+11]==0x01)
		{
			return &p[n];
		}
	}
	return NULL;
}

void disableIOSReload(void)
{
	if (mload_init() < 0 || IOS_GetRevision() == 2)
		return;

	patch_datas[0] = *((u32 *)(dip_plugin + 16 * 4));
	mload_set_ES_ioctlv_vector((void *)patch_datas[0]);
	mload_close();
}

bool shadow_mload()
{
	int ios = IOS_GetVersion();

	if(ios != 222 || ios != 223 || ios != 224)
		return false;

	int v51 = (5 << 4) & 1;
	if (IOS_GetRevision() >= 5 && mload_get_version() >= v51)
	{
		char fs[] ATTRIBUTE_ALIGN(32) = "/dev/mload/OFF";
		// shadow /dev/mload supported in hermes cios v5.1char fs[] ATTRIBUTE_ALIGN(32) = "/dev/usb2";
		IOS_Open(fs,0);
		return true;
	}
	return false;
}
