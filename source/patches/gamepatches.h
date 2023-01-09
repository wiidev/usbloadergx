#ifndef GAMEPATCHES_H_
#define GAMEPATCHES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gccore.h>

void RegisterDOL(u8 *dst, int len);
void ClearDOLList();
void gamepatches(u8 videoSelected, u8 videoPatchDol, u8 aspectForce, u8 languageChoice, u8 patchcountrystring,
                 u8 vipatch, u8 deflicker, u8 sneekVideoPatch, u8 hooktype, u8 videoWidth, u64 returnTo, u8 privateServer, const char *serverAddr);
void anti_002_fix(u8 *addr, u32 len);
void deflicker_patch(u8 *addr, u32 len);
void patch_vfilters(u8 *addr, u32 len, u8 *vfilter);
void patch_vfilters_rogue(u8 *addr, u32 len, u8 *vfilter);
void patch_width(u8 *addr, u32 len);
void PrivateServerPatcher(void *addr, u32 len, u8 privateServer, const char *serverAddr);
void PatchFix480p();
s8 do_new_wiimmfi();
s8 do_new_wiimmfi_nonMKWii(void *addr, u32 len);
void domainpatcher(void *addr, u32 len, const char *domain);
bool patch_nsmb(u8 *gameid);
bool patch_pop(u8 *gameid);
void patch_error_codes(u8 *gameid);
void VideoModePatcher(u8 *dst, int len, u8 videoSelected, u8 VideoPatchDol);
void sneek_video_patch(void *addr, u32 len);
bool PatchReturnTo(void *Address, int Size, u32 id);
int PatchNewReturnTo(int es_fd, u64 title);
int BlockIOSReload(int es_fd, u32 gameIOS);
void PatchAspectRatio(void *addr, u32 len, u8 aspect);

// Revolution SDK Render Modes
extern GXRModeObj TVPal528Prog_RVL;
extern GXRModeObj TVPal528ProgSoft_RVL;
extern GXRModeObj TVPal524ProgAa_RVL;
extern GXRModeObj TVPal528Int_RVL;
extern GXRModeObj TVPal528IntDf_RVL;
extern GXRModeObj TVEurgb60Hz480Prog_RVL;
extern GXRModeObj TVEurgb60Hz480ProgSoft_RVL;
extern GXRModeObj TVEurgb60Hz480ProgAa_RVL;
extern GXRModeObj TVPal524IntAa_RVL;
extern GXRModeObj TVPal264Int_RVL;
extern GXRModeObj TVPal264IntAa_RVL;
extern GXRModeObj TVPal264Ds_RVL;
extern GXRModeObj TVPal264DsAa_RVL;
extern GXRModeObj TVMpal240Int_RVL;
extern GXRModeObj TVMpal240IntAa_RVL;
extern GXRModeObj TVMpal480Int_RVL;
extern GXRModeObj TVMpal480ProgSoft_RVL;
extern GXRModeObj TVMpal480ProgAa_RVL;

#ifdef __cplusplus
}
#endif

#endif
