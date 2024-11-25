#include "log.h"
#include "string.h"
#include "ff_gen_drv.h"
#include "user_diskio.h"
#include "sd_card_ll.h"

#define DEFAULT_BLOCK_SIZE 512

static volatile DSTATUS stat = STA_NOINIT;

DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);

Diskio_drvTypeDef  USER_Driver =
{
    USER_initialize,
    USER_status,
    USER_read,
    USER_write,
    USER_ioctl
};

DSTATUS USER_initialize(BYTE pdrv) {
    UNUSED(pdrv);
    stat = STA_NOINIT;
    if (sd_init_ll() == true)
        stat = 0;

    return stat;
}

DSTATUS USER_status(BYTE pdrv) {
    UNUSED(pdrv);
    stat = STA_NOINIT;
    if (sd_get_state() == SD_STAT_TRANSFER)
        stat = 0;

    return stat;
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (
    BYTE pdrv,      /* Physical drive nmuber to identify the drive */
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Sector address in LBA */
    UINT count      /* Number of sectors to read */
) {
    UNUSED(pdrv);
    if (sd_read_blocks_dma(buff, (uint32_t)sector, (uint32_t)count, SD_RW_TIMEOUT_MS) == true) {
        while (sd_get_state() != SD_STAT_TRANSFER);
        return RES_OK;
    }

    return RES_ERROR;
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_write (
    BYTE pdrv,          /* Physical drive nmuber to identify the drive */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Sector address in LBA */
    UINT count          /* Number of sectors to write */
) {
    UNUSED(pdrv);
    if (sd_write_blocks_dma((uint8_t *)buff, (uint32_t)sector, (uint32_t)count, SD_RW_TIMEOUT_MS) == true) {
        while(sd_get_state() != SD_STAT_TRANSFER);
        return (RES_OK);
    }

    return RES_ERROR;
}

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
DRESULT USER_ioctl (
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
) {
    UNUSED(pdrv);
    DRESULT res = RES_ERROR;

    if (stat & STA_NOINIT) return RES_NOTRDY;

    switch (cmd) {
        /* Make sure that no pending write process */
        case CTRL_SYNC :
            res = RES_OK;
            break;

        /* Get number of sectors on the disk (DWORD) */
        case GET_SECTOR_COUNT :
            *(LBA_t*) buff = sd_get_log_blocks();
            res = RES_OK;
            break;

        /* Get R/W sector size (WORD) */
        case GET_SECTOR_SIZE :
            *(WORD*) buff = sd_get_log_blk_size();
            res = RES_OK;
            break;

            /* Get erase block size in unit of sector (DWORD) */
        case GET_BLOCK_SIZE :
            *(DWORD*) buff = sd_get_log_blk_size() / 512;
            res = RES_OK;
            break;

        default:
            res = RES_PARERR;
    }
    return res;
}
