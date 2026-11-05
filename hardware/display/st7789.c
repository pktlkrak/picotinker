#include "st7789.h"
#include "hardware/dma.h"
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <limits.h>
#include <pico/time.h>
#include <stdint.h>

static int disp_spi_dma_tx;

static inline void display_send_bytes(uint8_t *bytes, int length) {
    spi_write_blocking(spi1, bytes, length);
}

#define SEND_CMDS(...)                                                         \
    do {                                                                       \
        uint8_t temp[] = {__VA_ARGS__};                                        \
        display_send_bytes(temp, sizeof(temp));                                \
    } while (0);

static inline void display_mode_set_data_mode(bool data_mode) {
    gpio_put(PIN_DISP_DC, data_mode);
}

void display_init() {
    disp_spi_dma_tx = dma_claim_unused_channel(true);

    spi_init(spi1, INT_MAX);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_DISP_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_DISP_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_DISP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_DISP_CS, GPIO_FUNC_SPI);

    gpio_init(PIN_DISP_DC);
    gpio_init(PIN_DISP_RST);
    gpio_set_dir(PIN_DISP_DC, GPIO_OUT);
    gpio_set_dir(PIN_DISP_RST, GPIO_OUT);

    gpio_put(PIN_DISP_RST, 0);
    sleep_ms(100);
    gpio_put(PIN_DISP_RST, 1);

    display_mode_set_data_mode(false);

    SEND_CMDS(ST77XX_SWRESET);
    sleep_ms(150);
    SEND_CMDS(ST77XX_SLPOUT);
    sleep_ms(10);
    SEND_CMDS(ST77XX_COLMOD, 0x66);
    sleep_ms(10);
    SEND_CMDS(ST77XX_MADCTL, 0x08);
    SEND_CMDS(ST77XX_CASET, 0x00, 0, 0, 240);
    SEND_CMDS(ST77XX_RASET, 0x00, 0, 0, 240);
    SEND_CMDS(ST77XX_INVON);
    sleep_ms(10);
    SEND_CMDS(ST77XX_NORON);
    sleep_ms(10);
    SEND_CMDS(ST77XX_DISPON);
    sleep_ms(10);
}

void display_send_framebuffer_no_dma(uint8_t *data, int length) {
    display_mode_set_data_mode(false);
    SEND_CMDS(ST77XX_CASET, 0x00, 0, 0, 240);
    sleep_ms(10);
    SEND_CMDS(ST77XX_RASET, 0x00, 0, 0, 240);
    sleep_ms(10);
    SEND_CMDS(ST77XX_RAMWR);
    sleep_ms(10);

    display_mode_set_data_mode(true);
    sleep_ms(10);
    display_send_bytes(data, length);
}

void display_send_framebuffer_dma(uint8_t *data, int length) {
    display_mode_set_data_mode(false);
    SEND_CMDS(ST77XX_CASET, 0x00, 0, 0, 240);
    SEND_CMDS(ST77XX_RASET, 0x00, 0, 0, 240);
    SEND_CMDS(ST77XX_RAMWR);

    display_mode_set_data_mode(true);

    dma_channel_config c = dma_channel_get_default_config(disp_spi_dma_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi1, true));
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    dma_channel_configure(
        disp_spi_dma_tx, &c,
        &spi_get_hw(spi1)->dr, // write address
        data,                  // read address
        length, // element count (each element is of size transfer_data_size)
        true);
    dma_channel_wait_for_finish_blocking(disp_spi_dma_tx);
}