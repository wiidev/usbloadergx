#ifndef __RAMDISK_H
#define __RAMDISK_H

#ifdef __cplusplus
extern "C"
{
#endif

    /*
     ramdiskCreate initialize a dynamic RAM-disk.
     return: Handle for ramdiskMount
     */
    void* ramdiskCreate();

    /**************************************************
     * ramdiskDelete
     *
     * destroy all datas
     * if the ramdisk is allready mounted forces ramdiskUnmount
     * IN: a Handle was created witch ramdiskCreate();
     **************************************************/
    void ramdiskDelete(void* Handle);

    /**************************************************
     * ramdiskMount
     *
     * mounts a ramdisk
     * IN: mountpoint e.g. "RAM" or "MEM:"
     *     handle     is a Handle was created ramdiskCreate()
     *                or NULL for auto-create
     * OUT: 0 = Error / !0 = OK
     **************************************************/
    int ramdiskMount(const char *mountpoint, void *handle);

    /**************************************************
     * ramdiskUnmount
     *
     * unmounts a ramdisk
     * IN: mountpoint e.g "RAM" or "MEM:"
     **************************************************/
    void ramdiskUnmount(const char *mountpoint);

/*
 NOTE:
 if the ramdisk not explizit created with ramdiskCreate (e.g ramdiskMount("RAMDISK:", NULL))
 the ramdisk is implizit created. When unmount this "auto-created" ramdisk the ramdisk automitic deleted

 but if the ramdisk explizit created (with ramdiskCreate),
 then we can remount the filesystem without lost all datas.

 Example1:
 =========
 void *ram = ramdiskCreate();        // create ramdisk
 ramdiskMount("RAM", ram);           // mount the ramdisk
 ...
 fopen("RAM:/file", ...);
 ...
 ramdiskMount("MEM", ram);           // remount as MEM: (without lost of data)
 ...
 fopen("RAM:/file", ...);
 ...
 ramdiskUnmount("MEM");              // unmount
 ...
 ramdiskMount("RAM", ram);           // remount as RAM: (without lost of data)
 ...
 ramdiskDelete(ram);                 // unmount and delete

 Example2:
 =========
 ramdiskMount("RAM", NULL);          // create and mount the ramdisk
 ...
 fopen("RAM:/file", ...);
 ...
 ramdiskUnmount("RAM");                  // unmount and delete
 */

#ifdef __cplusplus
}
#endif

#endif /*__RAMDISK_H*/
