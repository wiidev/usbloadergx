#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <unistd.h>

#include "utils.h"
#include "video.h"
#include "wad.h"

#include "nandtitle.h"



#include "prompts/PromptWindows.h"
#include "libwiigui/gui.h"
#include "language/gettext.h"
#include "menu.h"
#include "filelist.h"
/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();
/*** Extern variables ***/
extern GuiWindow * mainWindow;



/* 'WAD Header' structure */
typedef struct {
	/* Header length */
	u32 header_len;

	/* WAD type */
	u16 type;

	u16 padding;

	/* Data length */
	u32 certs_len;
	u32 crl_len;
	u32 tik_len;
	u32 tmd_len;
	u32 data_len;
	u32 footer_len;
} ATTRIBUTE_PACKED wadHeader;

/* Variables */
static u8 wadBuffer[BLOCK_SIZE] ATTRIBUTE_ALIGN(32);


s32 __Wad_ReadFile(FILE *fp, void *outbuf, u32 offset, u32 len)
{
	s32 ret;

	/* Seek to offset */
	fseek(fp, offset, SEEK_SET);

	/* Read data */
	ret = fread(outbuf, len, 1, fp);
	if (ret < 0)
		return ret;

	return 0;
}

s32 __Wad_ReadAlloc(FILE *fp, void **outbuf, u32 offset, u32 len)
{
	void *buffer = NULL;
	s32   ret;

	/* Allocate memory */
	buffer = memalign(32, len);
	if (!buffer)
		return -1;

	/* Read file */
	ret = __Wad_ReadFile(fp, buffer, offset, len);
	if (ret < 0) {
		free(buffer);
		return ret;
	}

	/* Set pointer */
	*outbuf = buffer;
	return 0;
}

s32 __Wad_GetTitleID(FILE *fp, wadHeader *header, u64 *tid)
{
	//signed_blob *p_tik    = NULL;
	void *p_tik    = NULL;
	tik         *tik_data = NULL;

	u32 offset = 0;
	s32 ret;

	/* Ticket offset */
	offset += round_up(header->header_len, 64);
	offset += round_up(header->certs_len,  64);
	offset += round_up(header->crl_len,    64);

	/* Read ticket */
	ret = __Wad_ReadAlloc(fp, &p_tik, offset, header->tik_len);
	if (ret < 0)
		goto out;

	/* Ticket data */
	tik_data = (tik *)SIGNATURE_PAYLOAD((signed_blob *)p_tik);

	/* Copy title ID */
	*tid = tik_data->titleid;

out:
	/* Free memory */
	if (p_tik)
		free(p_tik);

	return ret;
}

s32 Wad_Install(FILE *fp)
{
	//////start the gui shit
	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
	GuiImageData dialogBox(imgPath, dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);}

	GuiText btn1Txt(tr("OK"), 22, THEME.prompttext);
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn1Txt.SetWidescreen(CFG.widescreen);
	btn1Img.SetWidescreen(CFG.widescreen);}
	GuiButton btn1(&btn1Img,&btn1Img, 2, 4, 0, -35, &trigA, &btnSoundOver, btnClick2,1);
	btn1.SetLabel(&btn1Txt);
	btn1.SetState(STATE_SELECTED);

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar_outline.png", CFG.theme_path);
	GuiImageData progressbarOutline(imgPath, progressbar_outline_png);
	GuiImage progressbarOutlineImg(&progressbarOutline);
	if (Settings.wsprompt == yes){
	progressbarOutlineImg.SetWidescreen(CFG.widescreen);}
	progressbarOutlineImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarOutlineImg.SetPosition(25, 50);

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar_empty.png", CFG.theme_path);
	GuiImageData progressbarEmpty(imgPath, progressbar_empty_png);
	GuiImage progressbarEmptyImg(&progressbarEmpty);
	progressbarEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarEmptyImg.SetPosition(25, 50);
	progressbarEmptyImg.SetTile(100);

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar.png", CFG.theme_path);
	GuiImageData progressbar(imgPath, progressbar_png);
	GuiImage progressbarImg(&progressbar);
	progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarImg.SetPosition(25, 50);

    char title[50];
   sprintf(title, "%s", tr("Installing wad"));
	GuiText titleTxt(title, 26, THEME.prompttext);
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,40);
    char msg[50];
    sprintf(msg, " ");
	// sprintf(msg, "%s", tr("Initializing Network"));
	GuiText msg1Txt((char*)NULL, 20, THEME.prompttext);
	msg1Txt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	msg1Txt.SetPosition(50,75);
//	char msg2[50] = " ";
	GuiText msg2Txt((char*)NULL, 20, THEME.prompttext);
	msg2Txt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	msg2Txt.SetPosition(50, 98);

	GuiText msg3Txt((char*)NULL, 20, THEME.prompttext);
	msg3Txt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	msg3Txt.SetPosition(50, 121);

	GuiText msg4Txt((char*)NULL, 20, THEME.prompttext);
	msg4Txt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	msg4Txt.SetPosition(50, 144);

	GuiText msg5Txt((char*)NULL, 20, THEME.prompttext);
	msg5Txt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	msg5Txt.SetPosition(50, 167);

	GuiText prTxt((char*)NULL, 26, THEME.prompttext);
	prTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	prTxt.SetPosition(0, 50);


	if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust for widescreen
		progressbarOutlineImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
		progressbarOutlineImg.SetPosition(0, 50);
		progressbarEmptyImg.SetPosition(80,50);
		progressbarEmptyImg.SetTile(78);
		progressbarImg.SetPosition(80, 50);

		msg1Txt.SetPosition(90,75);
		msg2Txt.SetPosition(90, 98);
		msg3Txt.SetPosition(90, 121);
		msg4Txt.SetPosition(90, 144);
		msg5Txt.SetPosition(90, 167);

	}
	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msg5Txt);
	promptWindow.Append(&msg4Txt);
	promptWindow.Append(&msg3Txt);
	promptWindow.Append(&msg1Txt);
	promptWindow.Append(&msg2Txt);

   //promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	//sleep(1);


	///start the wad shit
	bool fail = false;
	wadHeader   *header  = NULL;
	void		*pvoid;
	signed_blob *p_certs = NULL, *p_crl = NULL, *p_tik = NULL, *p_tmd = NULL;

	tmd *tmd_data  = NULL;

	u32 cnt, offset = 0;
	s32 ret = 666;


	ResumeGui();
	msg1Txt.SetText(tr(">> Reading WAD data..."));
	HaltGui();
#define SetPointer(a, p) a=(typeof(a))p
	// WAD header
	//ret = __Wad_ReadAlloc(fp, (void *)header, offset, sizeof(wadHeader));
	ret = __Wad_ReadAlloc(fp, &pvoid, offset, sizeof(wadHeader));

	if (ret < 0)
		goto err;
	SetPointer(header, pvoid);
	offset += round_up(header->header_len, 64);

	// WAD certificates
	//ret = __Wad_ReadAlloc(fp, (void *)&p_certs, offset, header->certs_len);
	ret = __Wad_ReadAlloc(fp, &pvoid, offset, header->certs_len);
	if (ret < 0)
		goto err;
	SetPointer(p_certs, pvoid);
	offset += round_up(header->certs_len, 64);

	// WAD crl

	if (header->crl_len) {
		//ret = __Wad_ReadAlloc(fp, (void *)&p_crl, offset, header->crl_len);
		ret = __Wad_ReadAlloc(fp, &pvoid, offset, header->crl_len);
		if (ret < 0)
			goto err;
		SetPointer(p_crl, pvoid);
		offset += round_up(header->crl_len, 64);
	}

	// WAD ticket
	//ret = __Wad_ReadAlloc(fp, (void *)&p_tik, offset, header->tik_len);
	ret = __Wad_ReadAlloc(fp, &pvoid, offset, header->tik_len);
	if (ret < 0)
		goto err;
	SetPointer(p_tik, pvoid);
	offset += round_up(header->tik_len, 64);

	// WAD TMD
	//ret = __Wad_ReadAlloc(fp, (void *)&p_tmd, offset, header->tmd_len);
	ret = __Wad_ReadAlloc(fp, &pvoid, offset, header->tmd_len);
	if (ret < 0)
		goto err;
	SetPointer(p_tmd, pvoid);
	offset += round_up(header->tmd_len, 64);

	ResumeGui();
	msg1Txt.SetText(tr("Reading WAD data... Ok!"));
	msg2Txt.SetText(tr(">> Installing ticket..."));
	HaltGui();
	// Install ticket
	ret = ES_AddTicket(p_tik, header->tik_len, p_certs, header->certs_len, p_crl, header->crl_len);
	if (ret < 0)
		goto err;

	ResumeGui();
	msg2Txt.SetText(tr("Installing ticket... Ok!"));
	msg3Txt.SetText(tr(">> Installing title..."));
	//WindowPrompt(">> Installing title...",0,0,0,0,0,200);
	HaltGui();
	// Install title
	ret = ES_AddTitleStart(p_tmd, header->tmd_len, p_certs, header->certs_len, p_crl, header->crl_len);
	if (ret < 0)
		goto err;

	// Get TMD info
	tmd_data = (tmd *)SIGNATURE_PAYLOAD(p_tmd);

	// Install contents
	//ResumeGui();
	//HaltGui();
	promptWindow.Append(&progressbarEmptyImg);
   promptWindow.Append(&progressbarImg);
   promptWindow.Append(&progressbarOutlineImg);
   promptWindow.Append(&prTxt);
	ResumeGui();
	msg3Txt.SetText(tr("Installing title... Ok!"));
	for (cnt = 0; cnt < tmd_data->num_contents; cnt++) {

		tmd_content *content = &tmd_data->contents[cnt];

		u32 idx = 0, len;
		s32 cfd;
		ResumeGui();

		//printf("\r\t\t>> Installing content #%02d...", content->cid);
		// Encrypted content size
		len = round_up(content->size, 64);

		// Install content
		cfd = ES_AddContentStart(tmd_data->title_id, content->cid);
		if (cfd < 0) {
			ret = cfd;
			goto err;
		}
		snprintf(imgPath, sizeof(imgPath), "%s%d...",tr(">> Installing content #"),content->cid);
		msg4Txt.SetText(imgPath);
		// Install content data
		while (idx < len) {

            //VIDEO_WaitVSync ();

			u32 size;

			// Data length
			size = (len - idx);
			if (size > BLOCK_SIZE)
				size = BLOCK_SIZE;

			// Read data
			ret = __Wad_ReadFile(fp, &wadBuffer, offset, size);
			if (ret < 0)
				goto err;

			// Install data
			ret = ES_AddContentData(cfd, wadBuffer, size);
			if (ret < 0)
				goto err;

			// Increase variables
			idx    += size;
			offset += size;
		
		//snprintf(imgPath, sizeof(imgPath), "%s%d (%d)...",tr(">> Installing content #"),content->cid,idx);

		//msg4Txt.SetText(imgPath);

		prTxt.SetTextf("%i%%", 100*(cnt*len+idx)/(tmd_data->num_contents*len));
      if ((Settings.wsprompt == yes) && (CFG.widescreen)) {
         progressbarImg.SetTile(78*(cnt*len+idx)/(tmd_data->num_contents*len));
      } else {
         progressbarImg.SetTile(100*(cnt*len+idx)/(tmd_data->num_contents*len));
      }

		}

		// Finish content installation
		ret = ES_AddContentFinish(cfd);
		if (ret < 0)
			goto err;
	}

	msg4Txt.SetText(tr("Installing content... Ok!"));
	msg5Txt.SetText(tr(">> Finishing installation..."));



	// Finish title install
	ret = ES_AddTitleFinish();
	if (ret >= 0) {
//		printf(" OK!\n");
		goto out;
	}

err:
	//char titties[100];
	ResumeGui();
	prTxt.SetTextf("%s%d", tr("Error..."),ret);
	promptWindow.Append(&prTxt);
	fail = true;
  	//snprintf(titties, sizeof(titties), "%d", ret);
	//printf(" ERROR! (ret = %d)\n", ret);
	//WindowPrompt("ERROR!",titties,"Back",0,0);
	// Cancel install
	ES_AddTitleCancel();
	goto exit;
	//return ret;

out:
	// Free memory
	if (header)
		free(header);
	if (p_certs)
		free(p_certs);
	if (p_crl)
		free(p_crl);
	if (p_tik)
		free(p_tik);
	if (p_tmd)
		free(p_tmd);
	goto exit;


exit:
	if (!fail)msg5Txt.SetText(tr("Finishing installation... Ok!"));
	promptWindow.Append(&btn1);
	while(btn1.GetState() != STATE_CLICKED){
	}


	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();

	return ret;
}


s32 Wad_Uninstall(FILE *fp)
{
	//////start the gui shit
   GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
	GuiImageData dialogBox(imgPath, dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);}

	GuiText btn1Txt(tr("OK"), 22, THEME.prompttext);
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn1Txt.SetWidescreen(CFG.widescreen);
	btn1Img.SetWidescreen(CFG.widescreen);}
	GuiButton btn1(&btn1Img,&btn1Img, 2, 4, 0, -55, &trigA, &btnSoundOver, btnClick2,1);
	btn1.SetLabel(&btn1Txt);
	btn1.SetState(STATE_SELECTED);

    char title[50];
   sprintf(title, "%s", tr("Uninstalling wad"));
	GuiText titleTxt(title, 26, THEME.prompttext);
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,40);

	GuiText msg1Txt((char*)NULL, 18, THEME.prompttext);
	msg1Txt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	msg1Txt.SetPosition(50,75);

	GuiText msg2Txt((char*)NULL, 18, THEME.prompttext);
	msg2Txt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	msg2Txt.SetPosition(50, 98);

	GuiText msg3Txt((char*)NULL, 18, THEME.prompttext);
	msg3Txt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	msg3Txt.SetPosition(50, 121);

	GuiText msg4Txt((char*)NULL, 18, THEME.prompttext);
	msg4Txt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	msg4Txt.SetPosition(50, 144);

	GuiText msg5Txt((char*)NULL, 18, THEME.prompttext);
	msg5Txt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	msg5Txt.SetPosition(50, 167);



	if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust for widescreen

		msg1Txt.SetPosition(70,95);
		msg2Txt.SetPosition(70, 118);
		msg3Txt.SetPosition(70, 141);
		msg4Txt.SetPosition(70, 164);
		msg5Txt.SetPosition(70, 187);

	}
	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msg5Txt);
	promptWindow.Append(&msg4Txt);
	promptWindow.Append(&msg3Txt);
	promptWindow.Append(&msg1Txt);
	promptWindow.Append(&msg2Txt);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();
	//sleep(3);

	///start the wad shit
	wadHeader *header	= NULL;
	void	  *pvoid	= NULL;
	tikview   *viewData = NULL;

	u64 tid;
	u32 viewCnt;
	s32 ret;

	msg1Txt.SetText(tr(">> Reading WAD data..."));

	// WAD header
	ret = __Wad_ReadAlloc(fp, &pvoid, 0, sizeof(wadHeader));
	if (ret < 0) {
		char errTxt[50];
		sprintf(errTxt,"%sret = %d",tr(">> Reading WAD data...ERROR! "),ret);
		msg1Txt.SetText(errTxt);
		//printf(" ERROR! (ret = %d)\n", ret);
		goto out;
	}
	SetPointer(header, pvoid);

	// Get title ID
	ret =  __Wad_GetTitleID(fp, header, &tid);
	if (ret < 0) {
		//printf(" ERROR! (ret = %d)\n", ret);
		char errTxt[50];
		sprintf(errTxt,"%sret = %d",tr(">> Reading WAD data...ERROR! "),ret);
		msg1Txt.SetText(errTxt);
		goto out;
	}

	msg1Txt.SetText(tr(">> Reading WAD data...Ok!"));
	msg2Txt.SetText(tr(">> Deleting tickets..."));

	// Get ticket views
	ret = titles.GetTicketViews(tid, &viewData, &viewCnt);
	if (ret < 0){
		char errTxt[50];
		sprintf(errTxt,"%sret = %d",tr(">> Deleting tickets...ERROR! "),ret);
		msg2Txt.SetText(errTxt);
		//printf(" ERROR! (ret = %d)\n", ret);
		}
	// Delete tickets
	if (ret >= 0) {
		u32 cnt;

		// Delete all tickets
		for (cnt = 0; cnt < viewCnt; cnt++) {
			ret = ES_DeleteTicket(&viewData[cnt]);
			if (ret < 0)
				break;
		}

		if (ret < 0){
			char errTxt[50];
			sprintf(errTxt,"%sret = %d",tr(">> Deleting tickets...ERROR! "),ret);
			msg2Txt.SetText(errTxt);}
			//printf(" ERROR! (ret = %d\n", ret);
		else
			//printf(" OK!\n");
			msg2Txt.SetText(tr(">> Deleting tickets...Ok! "));

	}

	msg3Txt.SetText(tr(">> Deleting title contents..."));
	//WindowPrompt(">> Deleting title contents...",0,"Back",0,0);

	// Delete title contents
	ret = ES_DeleteTitleContent(tid);
	if (ret < 0){
		char errTxt[50];
		sprintf(errTxt,"%sret = %d",tr(">> Deleting title contents...ERROR! "),ret);
		msg3Txt.SetText(errTxt);}
		//printf(" ERROR! (ret = %d)\n", ret);
	else
		//printf(" OK!\n");
		msg3Txt.SetText(tr(">> Deleting title contents...Ok!"));

	msg4Txt.SetText(tr(">> Deleting title..."));
	// Delete title
	ret = ES_DeleteTitle(tid);
	if (ret < 0){
		char errTxt[50];
		sprintf(errTxt,"%sret = %d",tr(">> Deleting title ...ERROR! "),ret);
		msg4Txt.SetText(errTxt);}
		//printf(" ERROR! (ret = %d)\n", ret);
	else
		//printf(" OK!\n");
		msg4Txt.SetText(tr(">> Deleting title ...Ok!"));

out:
	// Free memory
	if (header)
		free(header);

	goto exit;


exit:
	msg5Txt.SetText(tr("Done!"));
	promptWindow.Append(&btn1);
	while(btn1.GetState() != STATE_CLICKED){
	}


	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();

	return ret;
}

