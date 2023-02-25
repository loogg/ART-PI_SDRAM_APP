#include <stddef.h>
#include <agile_upgrade.h>
#include "sdram_port.h"
#include <stdint.h>

static void _sdram_read(uint32_t addr, uint8_t *buf, int size)
{
    for (int i = 0; i < size; i++)
    {
        buf[i] = *(uint8_t *)(SDRAM_BANK_ADDR + addr + i);
    }
}

static void _sdram_write(uint32_t addr, const uint8_t *buf, int size)
{
    for (int i = 0; i < size; i++)
    {
        *(uint8_t *)(SDRAM_BANK_ADDR + addr + i) = buf[i];
    }
}

static int adapter_init(agile_upgrade_t *agu) {
    agu->ops_data = NULL;
    if (agu->len <= 0) return AGILE_UPGRADE_ERR;
    if (agu->user_data == NULL) return AGILE_UPGRADE_ERR;

    agu->ops_data = agu->user_data;

    return AGILE_UPGRADE_EOK;
}

static int adapter_read(agile_upgrade_t *agu, int offset, uint8_t *buf, int size) {
    if (agu->ops_data == NULL) return AGILE_UPGRADE_ERR;

    uint32_t addr = *(const uint32_t *)agu->ops_data;
    if (offset < 0) {
        addr = addr + agu->len + offset;
    } else {
        addr += offset;
    }

    _sdram_read(addr, buf, size);

    return size;
}

static int adapter_write(agile_upgrade_t *agu, int offset, const uint8_t *buf, int size) {
    if (agu->ops_data == NULL) return AGILE_UPGRADE_ERR;

    uint32_t addr = *(const uint32_t *)agu->ops_data;
    if (offset < 0) {
        addr = addr + agu->len + offset;
    } else {
        addr += offset;
    }

    _sdram_write(addr, buf, size);

    return size;
}

static int adapter_deinit(agile_upgrade_t *agu) {
    agu->ops_data = NULL;

    return AGILE_UPGRADE_EOK;
}

const struct agile_upgrade_ops agile_upgrade_sdram_ops = {adapter_init, adapter_read, adapter_write,
                                                         NULL, adapter_deinit};
