#ifndef __fatfs_H
#define __fatfs_H
#ifdef __cplusplus
extern "C" {
#endif

#include "ff.h"
#include "ff_gen_drv.h"
#include "user_diskio.h"                /* defines USER_Driver as external */
#include "stdbool.h"

#define TEST_BUF_LEN                    100
#define TEST_SYMBOL                     'M'
#define MAX_VOLNAME_LEN                 32
#define MAX_VOL_ID_LEN                  8
#define MAX_VOL_PATH_LEN                9

#define SECTORS_PER_VOLUME_INDEX        8

extern char USERPath[4];                /* USER logical drive path */
extern FATFS USERFatFS;                 /* File system object for USER logical drive */

void fat_fs_init(void);
void SD_test();

#ifdef __cplusplus
}
#endif
#endif /*__fatfs_H */
