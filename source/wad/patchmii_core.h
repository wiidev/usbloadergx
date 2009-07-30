#ifndef _PATCHMII_CORE_
#define _PATCHMII_CORE_


//Patchmii functions
void patchmii_network_init(void);
// Call with version = 0 to get the latest version
// Set "patch" if you want to try to patch title contents
s32 patchmii_install(u32 in_title_h, u32 in_title_l, u32 in_version, u32 out_title_h, u32 out_title_l, u32 out_version, bool patch);
s32 install_temporary_ios(u32 base_ios, u32 base_ver);
s32 load_temporary_ios(void);
s32 cleanup_temporary_ios(void);

//Tools
void forge_tmd(signed_blob *s_tmd);
void forge_tik(signed_blob *s_tik);

void spinner(void);

#define TEMP_IOS

// Basic I/O.

static inline u32 read32(u32 addr)
{
	u32 x;
	asm volatile("lwz %0,0(%1) ; sync" : "=r"(x) : "b"(0xc0000000 | addr));
	return x;
}

static inline void write32(u32 addr, u32 x)
{
	asm("stw %0,0(%1) ; eieio" : : "r"(x), "b"(0xc0000000 | addr));
}

// USB Gecko.

void usb_flush(int chn);
int usb_sendbuffer(int chn,const void *buffer,int size);

// Version string.

extern const char version[];

// Debug: blink the tray led.

static inline void blink(void)
{
	write32(0x0d8000c0, read32(0x0d8000c0) ^ 0x20);
}

void debug_printf(const char *fmt, ...);
void hexdump(FILE *fp, void *d, int len);
void aes_set_key(u8 *key);
void aes_decrypt(u8 *iv, u8 *inbuf, u8 *outbuf, unsigned long long len);
void aes_encrypt(u8 *iv, u8 *inbuf, u8 *outbuf, unsigned long long len);

#define TRACE(x) debug_printf("%s / %d: %d\n", __FUNCTION__, __LINE__, (x))

#define ISFS_ACCESS_READ 1
#define ISFS_ACCESS_WRITE 2

#endif
