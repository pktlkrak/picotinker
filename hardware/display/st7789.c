#include "st7789.h"

#include "../pin_config.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hil.h"
#include "pico/time.h"
#include <limits.h>
#include <stdint.h>

#define DISPLAY_COMMAND_MODE 0
#define DISPLAY_DATA_MODE 1

#define SEND_CMDS(...)                                                         \
    do {                                                                       \
        uint8_t temp[] = {__VA_ARGS__};                                        \
        display_send_bytes(temp, sizeof(temp));                                \
    } while (0);

typedef uint8_t display_mode_t;

static int disp_spi_dma_tx;

static inline void display_send_bytes(uint8_t *bytes, int length) {
    spi_write_blocking(SPI_DISPLAY, bytes, length);
}

static inline void display_mode_set_data_mode(display_mode_t mode) {
    gpio_put(PIN_DISPLAY_DC, mode);
}

void display_init() {
    disp_spi_dma_tx = dma_claim_unused_channel(true);

    spi_init(SPI_DISPLAY, INT_MAX);
    spi_set_format(SPI_DISPLAY, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_DISPLAY_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_DISPLAY_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_DISPLAY_CS, GPIO_FUNC_SPI);

    gpio_init(PIN_DISPLAY_DC);
    gpio_init(PIN_DISPLAY_RST);
    gpio_set_dir(PIN_DISPLAY_DC, GPIO_OUT);
    gpio_set_dir(PIN_DISPLAY_RST, GPIO_OUT);

    gpio_put(PIN_DISPLAY_RST, 0);
    sleep_ms(100);
    gpio_put(PIN_DISPLAY_RST, 1);

    display_mode_set_data_mode(DISPLAY_COMMAND_MODE);

    SEND_CMDS(ST7789_SWRESET);
    sleep_ms(150);
    SEND_CMDS(ST7789_SLPOUT);
    sleep_ms(10);
    SEND_CMDS(ST7789_COLMOD, 0x66);
    sleep_ms(10);
    SEND_CMDS(ST7789_MADCTL, 0x08);
    SEND_CMDS(ST7789_CASET, 0x00, 0, SCREEN_SIZE_X >> 8, SCREEN_SIZE_X & 0xFF);
    SEND_CMDS(ST7789_RASET, 0x00, 0, SCREEN_SIZE_Y >> 8, SCREEN_SIZE_Y & 0xFF);
    SEND_CMDS(ST7789_INVON);
    sleep_ms(10);
    SEND_CMDS(ST7789_NORON);
    sleep_ms(10);
    SEND_CMDS(ST7789_DISPON);
    sleep_ms(10);
}

void display_send_framebuffer_dma(uint8_t *data, int length) {
    display_mode_set_data_mode(DISPLAY_COMMAND_MODE);
    sleep_us(ST7789_DC_SLEEP_US);
    SEND_CMDS(ST7789_CASET, 0x00, 0, SCREEN_SIZE_X >> 8, SCREEN_SIZE_X & 0xFF);
    SEND_CMDS(ST7789_RASET, 0x00, 0, SCREEN_SIZE_Y >> 8, SCREEN_SIZE_Y & 0xFF);
    SEND_CMDS(ST7789_RAMWR);
    sleep_us(ST7789_DC_SLEEP_US);

    display_mode_set_data_mode(DISPLAY_DATA_MODE);
    sleep_us(ST7789_DC_SLEEP_US);

    dma_channel_config c = dma_channel_get_default_config(disp_spi_dma_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(SPI_DISPLAY, true));
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    dma_channel_configure(disp_spi_dma_tx, &c, &spi_get_hw(SPI_DISPLAY)->dr,
                          data, length, true);
    dma_channel_wait_for_finish_blocking(disp_spi_dma_tx);
    sleep_us(ST7789_DC_SLEEP_US);
}