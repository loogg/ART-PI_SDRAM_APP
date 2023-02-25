/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-02     RT-Thread    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "agile_upgrade.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define BOOT_APP_ADDR          ((uint32_t)0xC0000000)

#define LED_PIN GET_PIN(I, 8)

extern const struct agile_upgrade_ops agile_upgrade_file_ops;
extern const struct agile_upgrade_ops agile_upgrade_sdram_ops;

typedef void (*boot_app_func)(void);
int main(void)
{
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    rt_thread_mdelay(3000);

    agile_upgrade_t src_agu = {0};
    src_agu.name = "src";
    src_agu.user_data = "./sdcard/app.rbl";
    src_agu.ops = &agile_upgrade_file_ops;

    uint32_t dst_addr = 0;
    agile_upgrade_t dst_agu = {0};
    dst_agu.name = "app";
    dst_agu.len = 0x100000;
    dst_agu.user_data = &dst_addr;
    dst_agu.ops = &agile_upgrade_sdram_ops;

    rt_device_t sd_dev = rt_device_find("sd0");
    if (sd_dev != RT_NULL)
    {
        int rc = agile_upgrade_release(&src_agu, &dst_agu, 0);
        if (rc != AGILE_UPGRADE_EOK)
        {
            LOG_E("release firmware to sdram failed.");
            return -RT_ERROR;
        }
    }

    LOG_I("Jump to application running.");

    __disable_irq();
    __set_CONTROL(0);
    SysTick->CTRL = 0;

    SCB_DisableICache();
    SCB_DisableDCache();

    SD_HandleTypeDef hsd = {0};
    hsd.Instance = SDMMC1;
    HAL_SD_MspDeInit(&hsd);

    UART_HandleTypeDef huart = {0};
    huart.Instance = UART4;
    HAL_UART_DeInit(&huart);

    for(int i=0; i<150; i++)
    {
        HAL_NVIC_DisableIRQ(i);
        HAL_NVIC_ClearPendingIRQ(i);
    }

    boot_app_func app_func = NULL;
    uint32_t app_addr = BOOT_APP_ADDR;

    app_func = (boot_app_func) * (__IO uint32_t *)(app_addr + 4);
    /* Configure main stack */
    __set_MSP(*(__IO uint32_t *)app_addr);
    /* jump to application */
    app_func();

    return RT_EOK;
}

