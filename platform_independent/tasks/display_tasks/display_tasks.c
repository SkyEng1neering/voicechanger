#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "log.h"
#include "display_tasks.h"
#include "display_ll.h"
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "lvgl.h"

#define DISPLAY_TASK_MAIN_STACK_SIZE            1024
#define DISPLAY_TASK_LVGL_TIM_STACK_SIZE        256
#define DISPLAY_TASK_LVGL_DRAW_STACK_SIZE       256

static osThreadId                               mainTaskHandle;
static uint32_t                                 mainTaskBuffer[DISPLAY_TASK_MAIN_STACK_SIZE];
static osStaticThreadDef_t                      mainTaskControlBlock;

static osThreadId                               lvglTimTaskHandle;
static uint32_t                                 lvglTimTaskBuffer[DISPLAY_TASK_LVGL_TIM_STACK_SIZE];
static osStaticThreadDef_t                      lvglTimTaskControlBlock;

static osThreadId                               lvglDrawTaskHandle;
static uint32_t                                 lvglDrawTaskBuffer[DISPLAY_TASK_LVGL_DRAW_STACK_SIZE];
static osStaticThreadDef_t                      lvglDrawTaskControlBlock;

static const char*                              tag = "DISPLAY";

/* LVGL data */
static lv_disp_draw_buf_t                       draw_buf;
static lv_color_t                               buf1[DISPLAY_BUFFER_SIZE / 10];  /*Declare a buffer for 1/10 screen size*/
static lv_disp_drv_t                            disp_drv;        /*Descriptor of a display driver*/

static void disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p) {
    display_flush(area->x1, area->x2, area->y1, area->y2, color_p);
    lv_disp_flush_ready(disp);
}

static void lvgl_tim_task(void const * argument) {
    LOGI(tag, "LVGL timer task started\n");
    uint32_t lvgl_tick_delay = 10;
    for (;;) {
        lv_tick_inc(lvgl_tick_delay);
        osDelay(lvgl_tick_delay);
    }
}

static void lvgl_draw_task(void const * argument) {
    LOGI(tag, "LVGL handler task started\n", tag);
    for (;;) {
        lv_timer_handler();
        osDelay(10);
    }
}

static void display_main_task(void const * argument) {
    LOGI(tag, "%s task started\n", tag);

    /* Init HW display */
    if (display_i2c_init() != true) {
        LOGI(tag, "Display i2c init error\n");
        return;
    }
    display_init();

    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, DISPLAY_BUFFER_SIZE / 10); /*Initialize the display buffer.*/
    lv_disp_drv_init(&disp_drv);                                            /*Basic initialization*/
    disp_drv.flush_cb = disp_flush;                                         /*Set your driver function*/
    disp_drv.draw_buf = &draw_buf;                                          /*Assign the buffer to the display*/
    disp_drv.hor_res = DISPLAY_WIDTH;                                       /*Set the horizontal resolution of the display*/
    disp_drv.ver_res = DISPLAY_HEIGHT;                                      /*Set the vertical resolution of the display*/
    lv_disp_drv_register(&disp_drv);                                        /*Finally register the driver*/

    for (;;) {
//        ssd1306_TestAll();
//        display_flush(5, 10, 5, 10, buf1);
//        osDelay(1000);
//        display_flush(5, 10, 5, 10, buf2);
        osDelay(1000);
    }
}

void display_tasks_init() {
    osThreadStaticDef(display_main_task, display_main_task, osPriorityNormal, 0,
            DISPLAY_TASK_MAIN_STACK_SIZE, mainTaskBuffer, &mainTaskControlBlock);
    mainTaskHandle = osThreadCreate(osThread(display_main_task), NULL);

    osThreadStaticDef(lvgl_tim_task, lvgl_tim_task, osPriorityNormal, 0,
            DISPLAY_TASK_LVGL_TIM_STACK_SIZE, lvglTimTaskBuffer, &lvglTimTaskControlBlock);
    lvglTimTaskHandle = osThreadCreate(osThread(lvgl_tim_task), NULL);

    osThreadStaticDef(lvgl_draw_task, lvgl_draw_task, osPriorityNormal, 0,
            DISPLAY_TASK_LVGL_DRAW_STACK_SIZE, lvglDrawTaskBuffer, &lvglDrawTaskControlBlock);
    lvglDrawTaskHandle = osThreadCreate(osThread(lvgl_draw_task), NULL);
}
