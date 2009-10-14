#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>

/* Constants */
#define IOCTL_DI_READID		0x70
#define IOCTL_DI_READ		0x71
#define IOCTL_DI_WAITCVRCLOSE	0x79
#define IOCTL_DI_GETCOVER	0x88
#define IOCTL_DI_RESET		0x8A
#define IOCTL_DI_OPENPART	0x8B
#define IOCTL_DI_CLOSEPART	0x8C
#define IOCTL_DI_UNENCREAD	0x8D
#define IOCTL_DI_SEEK		0xAB
#define IOCTL_DI_STOPLASER	0xD2
#define IOCTL_DI_OFFSET		0xD9
#define IOCTL_DI_STOPMOTOR	0xE3
#define IOCTL_DI_SETUSBMODE	0xF4
#define IOCTL_DI_DISABLERESET	0xF6

/** Hermes IOS222 **/
#define DI_SETWBFSMODE	0xfe
#define DI_SETOFFSETBASE 0xf1

/* Variables */
static u32 inbuf[8]  ATTRIBUTE_ALIGN(32);
static u32 outbuf[8] ATTRIBUTE_ALIGN(32);

static const char di_fs[] ATTRIBUTE_ALIGN(32) = "/dev/di";
static s32 di_fd = -1;

s32 WDVD_Init(void) {
    /* Open "/dev/di" */
    if (di_fd < 0) {
        di_fd = IOS_Open(di_fs, 0);
        if (di_fd < 0)
            return di_fd;
    }

    return 0;
}

s32 WDVD_Close(void) {
    /* Close "/dev/di" */
    if (di_fd >= 0) {
        IOS_Close(di_fd);
        di_fd = -1;
    }

    return 0;
}

s32 WDVD_GetHandle(void) {
    /* Return di handle */
    return di_fd;
}

s32 WDVD_Reset(void) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Reset drive */
    inbuf[0] = IOCTL_DI_RESET << 24;
    inbuf[1] = 1;

    ret = IOS_Ioctl(di_fd, IOCTL_DI_RESET, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;
}

s32 WDVD_ReadDiskId(void *id) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Read disc ID */
    inbuf[0] = IOCTL_DI_READID << 24;

    ret = IOS_Ioctl(di_fd, IOCTL_DI_READID, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret < 0)
        return ret;

    if (ret == 1) {
        memcpy(id, outbuf, sizeof(dvddiskid));
        return 0;
    }

    return -ret;
}

s32 WDVD_Seek(u64 offset) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Drive seek */
    inbuf[0] = IOCTL_DI_SEEK << 24;
    inbuf[1] = (u32)(offset >> 2);

    ret = IOS_Ioctl(di_fd, IOCTL_DI_SEEK, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret!=1) {
        // Try old cIOS 222
        /* Drive seek */
        inbuf[0] = DI_SETOFFSETBASE << 24;
        ret = IOS_Ioctl(di_fd, DI_SETOFFSETBASE, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    }
    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;

}

s32 WDVD_Offset(u64 offset) {
    //u32 *off = (u32 *)((void *)&offset);
	union { u64 off64; u32 off32[2]; } off;off.off64 = offset;
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Set offset */
    inbuf[0] = IOCTL_DI_OFFSET << 24;
    inbuf[1] = (off.off32[0]) ? 1: 0;
    inbuf[2] = (off.off32[1] >> 2);

    ret = IOS_Ioctl(di_fd, IOCTL_DI_OFFSET, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;
}

s32 WDVD_StopLaser(void) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Stop laser */
    inbuf[0] = IOCTL_DI_STOPLASER << 24;

    ret = IOS_Ioctl(di_fd, IOCTL_DI_STOPLASER, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;
}

s32 WDVD_StopMotor(void) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Stop motor */
    inbuf[0] = IOCTL_DI_STOPMOTOR << 24;

    ret = IOS_Ioctl(di_fd, IOCTL_DI_STOPMOTOR, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;
}

s32 WDVD_OpenPartition(u64 offset) {
    u8 *vector = NULL;

    u32 *buffer = NULL;
    s32 ret;

    /* Allocate memory */
    buffer = (u32 *)memalign(32, 0x5000);
    if (!buffer)
        return -1;

    /* Set vector pointer */
    vector = (u8 *)buffer;

    memset(buffer, 0, 0x5000);

    /* Open partition */
    buffer[0] = (u32)(buffer + 0x10);
    buffer[1] = 0x20;
    buffer[3] = 0x024A;
    buffer[6] = (u32)(buffer + 0x380);
    buffer[7] = 0x49E4;
    buffer[8] = (u32)(buffer + 0x360);
    buffer[9] = 0x20;

    buffer[(0x40 >> 2)]     = IOCTL_DI_OPENPART << 24;
    buffer[(0x40 >> 2) + 1] = offset >> 2;

    ret = IOS_Ioctlv(di_fd, IOCTL_DI_OPENPART, 3, 2, (ioctlv *)vector);

    /* Free memory */
    free(buffer);

    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;
}

s32 WDVD_ClosePartition(void) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Close partition */
    inbuf[0] = IOCTL_DI_CLOSEPART << 24;

    ret = IOS_Ioctl(di_fd, IOCTL_DI_CLOSEPART, inbuf, sizeof(inbuf), NULL, 0);
    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;
}

s32 WDVD_UnencryptedRead(void *buf, u32 len, u64 offset) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Unencrypted read */
    inbuf[0] = IOCTL_DI_UNENCREAD << 24;
    inbuf[1] = len;
    inbuf[2] = (u32)(offset >> 2);

    ret = IOS_Ioctl(di_fd, IOCTL_DI_UNENCREAD, inbuf, sizeof(inbuf), buf, len);
    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;
}

s32 WDVD_Read(void *buf, u32 len, u64 offset) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Disc read */
    inbuf[0] = IOCTL_DI_READ << 24;
    inbuf[1] = len;
    inbuf[2] = (u32)(offset >> 2);

    ret = IOS_Ioctl(di_fd, IOCTL_DI_READ, inbuf, sizeof(inbuf), buf, len);
    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;
}

s32 WDVD_WaitForDisc(void) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Wait for disc */
    inbuf[0] = IOCTL_DI_WAITCVRCLOSE << 24;

    ret = IOS_Ioctl(di_fd, IOCTL_DI_WAITCVRCLOSE, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;
}

s32 WDVD_GetCoverStatus(u32 *status) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Get cover status */
    inbuf[0] = IOCTL_DI_GETCOVER << 24;

    ret = IOS_Ioctl(di_fd, IOCTL_DI_GETCOVER, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret < 0)
        return ret;

    if (ret == 1) {
        /* Copy cover status */
        memcpy(status, outbuf, sizeof(u32));

        return 0;
    }

    return -ret;
}

s32 WDVD_DisableReset(u8 val) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Disable/Enable reset */
    inbuf[0] = IOCTL_DI_DISABLERESET << 24;
    inbuf[1] = val;

    ret = IOS_Ioctl(di_fd, IOCTL_DI_DISABLERESET, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;
}

/** Hermes **/
s32 WDVD_SetUSBMode(u8 *id) {
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Set USB mode */
    inbuf[0] = IOCTL_DI_SETUSBMODE << 24;
    inbuf[1] = (id) ? 1 : 0;


    /* Copy ID */
    if (id) {
        memcpy(&inbuf[2], id, 6);
    }

    ret = IOS_Ioctl(di_fd, IOCTL_DI_SETUSBMODE, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret!=1) {
        // Try old cIOS 222
        /* Set USB mode */
        inbuf[0] = DI_SETWBFSMODE << 24;
        ret = IOS_Ioctl(di_fd, DI_SETWBFSMODE, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    }

    if (ret < 0)
        return ret;

    return (ret == 1) ? 0 : -ret;
}
