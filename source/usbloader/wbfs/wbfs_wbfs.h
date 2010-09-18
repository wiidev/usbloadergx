#ifndef _WBFS_WBFS_H
#define _WBFS_WBFS_H

#include "wbfs_base.h"
#include "libwbfs/libwbfs.h"

class Wbfs_Wbfs : public Wbfs
{
    public:
        Wbfs_Wbfs( u32 device, u32 lba, u32 size ) : Wbfs( device, lba, size ) {}

        s32 Open();
        wbfs_disc_t* OpenDisc( u8 * );
        void CloseDisc( wbfs_disc_t * );

        s32 Format();
        s32 GetCount( u32 * );
        s32 GetHeaders( struct discHdr *, u32, u32 );

        s32 AddGame();
        s32 RemoveGame( u8 * );

        s32 DiskSpace( f32 *, f32 * );

        s32 RenameGame( u8 *, const void * );
        s32 ReIDGame( u8 *, const void * );

        f32 EstimateGameSize();
};

#endif //_WBFS_WBFS_H
