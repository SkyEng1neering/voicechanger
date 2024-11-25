#include "string.h"
#include "fatfs.h"
#include "log.h"

#define TESTFILE_NAME           "testfile.txt"

char USERPath[4];
FATFS USERFatFS;

static const char *tag = "FATFS";

void fat_fs_init(void) {
    FATFS_LinkDriver(&USER_Driver, USERPath);
    FRESULT res = f_mount(&USERFatFS, "0:/", 1);
    if(res != FR_OK){
        LOGE(tag, "Error while mounting volume: %lu\n", res);
        return;
    }
    LOGD(tag, "Card has mounted\n");
    SD_test();
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void) {
    return 0;
}

void SD_test() {
    FIL test_file;
    bool test_stat = true;
    uint32_t bytesWritten = 0;
    uint32_t res = 0;

    LOGD(tag, "Running selftest...\n");
    res = f_open(&test_file, TESTFILE_NAME, FA_CREATE_ALWAYS|FA_WRITE);
    if (res != FR_OK) {
        LOGE(tag, "eMMC test failed. f_open error: %lu\n", res);
        test_stat = false;
    }
    else {
        LOGD(tag, "Test file opened\n");
        uint8_t w_text[TEST_BUF_LEN];
        for(int i = 0; i < TEST_BUF_LEN; i++) {
            w_text[i] = TEST_SYMBOL;
        }
        res = f_write(&test_file, w_text, TEST_BUF_LEN, (UINT*)&bytesWritten);
        if ((bytesWritten == 0) || (res != FR_OK)) {
            LOGE(tag, "Written %lu bytes, res = %lu\n", bytesWritten, res);
        }
        LOGD(tag, "Successfully written  %lu bytes, res = %lu\n", bytesWritten, res);
        f_close(&test_file);

        if (f_open(&test_file, TESTFILE_NAME, FA_OPEN_EXISTING|FA_READ) != FR_OK) {
            LOGE(tag, "Reopen file error\n");
            test_stat = false;
        }
        else {
            LOGD(tag, "Try to read written data\n");
            memset(w_text, 0, TEST_BUF_LEN);
            uint32_t bytes_read = 0;
            if ((f_read(&test_file, w_text, TEST_BUF_LEN, (UINT*)&bytes_read) != FR_OK) || (bytes_read != TEST_BUF_LEN)){
                LOGE(tag, "Read failed\n");
                test_stat = false;
            }
            else {
                for(uint32_t i = 0; i < TEST_BUF_LEN; i++){
                    if(w_text[i] != TEST_SYMBOL){
                        test_stat = false;
                        break;
                    }
                }
            }
        }
    }
    if(test_stat == false){
        LOGE(tag, "SD card ERROR\n");
    }
    else {
        LOGD(tag, "SD card OK\n");
    }
    f_close(&test_file);
//	f_unlink(TESTFILE_NAME);
}
