#ifndef AUDIO_TASK_H_
#define AUDIO_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum DacState {
    DAC_HALF_SENT_STATE = 0,
    DAC_FULL_SENT_STATE,
    DAC_STOPPED_STATE
};

void audio_tasks_init();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AUDIO_TASK_H_ */
