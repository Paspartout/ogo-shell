#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"

#include "sdcard.h"


#define SDCARD_IO_MISO GPIO_NUM_19
#define SDCARD_IO_MOSI GPIO_NUM_23
#define SDCARD_IO_CLK GPIO_NUM_18
#define SDCARD_IO_CS GPIO_NUM_22

static sdmmc_card_t *sdcard = NULL;

esp_err_t sdcard_init(const char *mount_path)
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = HSPI_HOST;
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;

    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = SDCARD_IO_MISO;
    slot_config.gpio_mosi = SDCARD_IO_MOSI;
    slot_config.gpio_sck = SDCARD_IO_CLK;
    slot_config.gpio_cs = SDCARD_IO_CS;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = { 0 };
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 5;

    return esp_vfs_fat_sdmmc_mount(mount_path, &host, &slot_config, &mount_config, &sdcard);
}

esp_err_t sdcard_deinit()
{
    if (!sdcard) {
        return ESP_FAIL;
    }

    return esp_vfs_fat_sdmmc_unmount();
}

bool sdcard_present(void)
{
    return sdcard != NULL;
}
