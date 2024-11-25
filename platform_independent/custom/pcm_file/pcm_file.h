#ifndef PCM_LIB_PCM_FILE_H_
#define PCM_LIB_PCM_FILE_H_

#include <stdint.h>
#include <stdbool.h>
#include "fatfs.h"

class PCMfile {
private:
    FIL file;
    const char* path;

public:
    PCMfile(const char* file_path);
    virtual ~PCMfile();

    uint32_t getSize();
    bool open();
    bool close();
    bool setOffset(uint32_t val);
    int readMono(int32_t* data_arr, uint32_t samples_num);
    int readMono(float* data_arr, uint32_t samples_num);
    int readMono(int16_t* data_arr, uint32_t samples_num);
    int readStereo(float* ch1_data_arr, float* ch2_data_arr, uint32_t samples_num);
};

#endif /* PCM_LIB_PCM_FILE_H_ */
