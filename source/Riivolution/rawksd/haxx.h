#pragma once

#include <vector>

#include <gctypes.h>

typedef struct
{
	u8 boot1_hash[20];
	u8 common_key[16];
	u32 ng_id;
	union {
		struct {
			u8 ng_priv[30];
			u8 _wtf1[18];
		};
		struct {
			u8 _wtf2[28];
			u8 nand_hmac[20];
		};
	};
	u8 nand_key[16];
	u8 prng_key[16];
	u32 unk1;
	u32 unk2; // 0x00000007
} otp_t;

typedef union {
	struct {
		u32 ms_id;             // 0x00
		u32 ca_id;             // 0x04
		u32 ng_key_id;         // 0x08
		u8 ng_sig[60];         // 0x0C
		struct {               // 0x48
			union {
				struct {
					 u8 boot2version;
					 u8 unknown1;
					 u8 unknown2;
					 u8 pad;
					 u32 update_tag;
				} __attribute__((packed));
				u16 data[4];
			};
			u16 checksum; // sum of data[] elements?
		} boot2_counters[2];
		struct {
			union {
				u32 nand_gen; // matches offset 0x8 in nand SFFS blocks
				u16 data[2];
			} __attribute__((packed));
			u16 checksum; // sum of data[] elements?
		} nand_counters[3];
		u8 pad0[6];            // 0x6E
		u8 korean_key[16];     // 0x74
		u8 pad1[116];          // 0x84
		u16 prng_seed[2];      // 0xF8 u32 with lo word stored first, incremented every time IOS starts.
		u8 pad2[4];            // 0xFC
	};
	u16 data[128];

} seeprom_t;

extern otp_t otp;
extern seeprom_t seeprom;
#define SYS_CERTS_SIZE      2560
extern u8 sys_certs[SYS_CERTS_SIZE];
extern u32 is_wiiu;

int get_certs();
int Haxx_Init();
void Haxx_Mount(std::vector<int>* mounted);
bool forge_sig(u8 *data, u32 length);
int check_cert_chain(const u8 *data, const u32 data_len);
void RunBootmii();
int seeprom_write(const void *src, unsigned int offset, unsigned int size);
int seeprom_read(void *dst, unsigned int offset, unsigned int size);

#define HAXX_IOS 0x0000000100000025ULL
#define HAXX_IOS_MAXIMUM 5919
#define HAXX_IOS_MINIMUM 3869