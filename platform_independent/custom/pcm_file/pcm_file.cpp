#include "pcm_file.h"
#include "log.h"

static const char* tag = "PCM file";

PCMfile::PCMfile(const char* file_path) {
    path = file_path;
}

PCMfile::~PCMfile() {
    close();
}

uint32_t PCMfile::getSize() {
    return f_size(&file);
}

bool PCMfile::setOffset(uint32_t val) {
    uint32_t res = f_lseek(&file, val);
    if (res != FR_OK) {
        LOGE(tag, "File %s set carriage error: %lu\n", path, res);
        return false;
    }
    return true;
}

bool PCMfile::open() {
    uint32_t res = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
    if (res != FR_OK) {
        LOGE(tag, "File %s open error: %lu\n", path, res);
        return false;
    }
    return true;
}

bool PCMfile::close() {
    uint32_t res = f_close(&file);
    if (res != FR_OK) {
        LOGE(tag, "File %s close error: %lu\n", path, res);
        return false;
    }
    return true;
}

int PCMfile::readMono(int32_t* data_arr, uint32_t samples_num) {
    uint32_t bytes_read = 0;
    uint32_t res = f_read(&file, data_arr, samples_num * sizeof(int32_t), (UINT*)&bytes_read);
    if (res != FR_OK) {
        LOGE(tag, "File %s read error: %lu\n", path, res);
    }
    return bytes_read / sizeof(int32_t);
}

int PCMfile::readMono(float* data_arr, uint32_t samples_num) {
    uint32_t bytes_read = 0;
    uint32_t res = f_read(&file, data_arr, samples_num * sizeof(float), (UINT*)&bytes_read);
    if (res != FR_OK) {
        LOGE(tag, "File %s read error: %lu\n", path, res);
    }
    return bytes_read / sizeof(float);
}

int PCMfile::readMono(int16_t* data_arr, uint32_t samples_num) {
    uint32_t bytes_read = 0;
    uint32_t res = f_read(&file, data_arr, samples_num * sizeof(uint16_t), (UINT*)&bytes_read);
    if (res != FR_OK) {
        LOGE(tag, "File %s read error: %lu\n", path, res);
    }
    return bytes_read / sizeof(uint16_t);
}

int PCMfile::readStereo(float* ch1_data_arr, float* ch2_data_arr, uint32_t samples_num) {
    uint32_t bytes_read_total = 0;
    uint32_t bytes_read = 0;
    uint32_t arr_offset = 0;
    uint32_t res = 0;
    for (uint32_t i = 0; i < samples_num; i++) {
        res = f_read(&file, &ch1_data_arr[arr_offset], sizeof(float), (UINT*)&bytes_read);
        if (res != FR_OK) {
            LOGE(tag, "File %s read error: %lu\n", path, res);
            break;
        }
        res = f_read(&file, &ch2_data_arr[arr_offset], sizeof(float), (UINT*)&bytes_read);
        if (res != FR_OK) {
            LOGE(tag, "File %s read error: %lu\n", path, res);
            break;
        }
        bytes_read_total += bytes_read * 2;
        arr_offset += sizeof(float);
    }
    return bytes_read / sizeof(float) * 2;
}
