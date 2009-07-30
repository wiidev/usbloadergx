#include <string.h>

#include "settings/cfg.h"
#include "usbloader/wbfs.h"
#include "main.h"



struct discHdr * gameList;
u32 gameCnt;
s32 gameSelected, gameStart;

/****************************************************************************
 * EntryCmp
 ***************************************************************************/
s32 __Menu_EntryCmp(const void *a, const void *b)

{

    struct discHdr *hdr1 = (struct discHdr *)a;

    struct discHdr *hdr2 = (struct discHdr *)b;



    /* Compare strings */

    return stricmp(get_title(hdr1), get_title(hdr2));

}

s32 __Menu_EntryCmpCount(const void *a, const void *b) {
    s32 ret;

    struct discHdr *hdr1 = (struct discHdr *)a;

    struct discHdr *hdr2 = (struct discHdr *)b;

    /* Compare Play Count */
    u16 count1 = 0;
    u16 count2 = 0;
    struct Game_NUM* game_num1 = CFG_get_game_num(hdr1->id);
    struct Game_NUM* game_num2 = CFG_get_game_num(hdr2->id);



    if (game_num1) count1 = game_num1->count;
    if (game_num2) count2 = game_num2->count;

    ret = (s32) (count2-count1);
    if (ret == 0) return stricmp(get_title(hdr1), get_title(hdr2));

    return ret;
}

/****************************************************************************
 * Get Gamelist
 ***************************************************************************/

int __Menu_GetEntries(int t) {
    struct discHdr *buffer = NULL;
    struct discHdr *buffer2 = NULL;
    struct discHdr *header = NULL;

    u32 cnt, len;
    s32 ret;

    /* Get list length */
    ret = WBFS_GetCount(&cnt);
    if (ret < 0)
        return ret;

    /* Buffer length */
    len = sizeof(struct discHdr) * cnt;

    /* Allocate memory */
    buffer = (struct discHdr *)memalign(32, len);
    if (!buffer)
        return -1;

    /* Clear buffer */
    memset(buffer, 0, len);

    /* Get header list */
    ret = WBFS_GetHeaders(buffer, cnt, sizeof(struct discHdr));
    if (ret < 0) {
        if (buffer) free(buffer);
        return ret;
    }

    /* Filters */
    if (Settings.fave && t==0) {
        u32 cnt2 = 0;

        for (u32 i = 0; i < cnt; i++) {
            header = &buffer[i];
            u8 favorite = 0;
            struct Game_NUM* game_num = CFG_get_game_num(header->id);
            if (game_num) {
                favorite = game_num->favorite;
            }
            if (favorite==1) {
                buffer2 = (discHdr *) realloc(buffer2, (cnt2+1) * sizeof(struct discHdr));
                if (!buffer2) {
                    free(buffer);
                    return -1;
                }

                memcpy((buffer2 + cnt2), (buffer + i), sizeof(struct discHdr));
                cnt2++;
            }
        }
        if (buffer2) {
            free(buffer);
            buffer = buffer2;
            buffer2 = NULL;
        } else {
            memset(buffer, 0, len);
        }
        cnt = cnt2;
    }

    if (Settings.parentalcontrol && !Settings.godmode && t==0) {
        u32 cnt2 = 0;

        for (u32 i = 0; i < cnt; i++) {
            header = &buffer[i];
            if (get_block(header) < Settings.parentalcontrol) {
                buffer2 = (discHdr *) realloc(buffer2, (cnt2+1) * sizeof(struct discHdr));
                if (!buffer2) {
                    free(buffer);
                    return -1;
                }

                memcpy((buffer2 + cnt2), (buffer + i), sizeof(struct discHdr));
                cnt2++;
            }
        }
        if (buffer2) {
            free(buffer);
            buffer = buffer2;
            buffer2 = NULL;
        } else {
            memset(buffer, 0, len);
        }
        cnt = cnt2;
    }

    if (Settings.sort==pcount) {
        qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmpCount);
    } else {
        qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmp);
    }

    /* Free memory */
    if (gameList)
        free(gameList);

    /* Set values */
    gameList = buffer;
    buffer = NULL;
    gameCnt  = cnt;

    /* Reset variables */
    gameSelected = gameStart = 0;

    return 0;
}
