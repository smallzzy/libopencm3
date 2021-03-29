#ifndef LIBOPENCM3_GCLK_H
#define LIBOPENCM3_GCLK_H

#include <libopencm3/cm3/common.h>
#include <libopencm3/sam/memorymap.h>

#define GCLK_MODE_GENEN    (1 << 0)
#define GCLK_MODE_IDC      (1 << 1)
#define GCLK_MODE_OOV      (1 << 2)
#define GCLK_MODE_OE       (1 << 3)
#define GCLK_MODE_DIVSEL   (1 << 4)
#define GCLK_MODE_RUNSTDBY (1 << 5)

#define GCLK_PERIPH_CLKEN   (1 << 6)
#define GCLK_PERIPH_WRTLOCK (1 << 7)

enum gclk_src
{
    GCLK_SRC_XOSC = 0,
    GCLK_SRC_GCLKIN = 1,
    GCLK_SRC_GLCKGEN1 = 2,
    GCLK_SRC_OSCULP32K = 3,
    GCLK_SRC_OSC32K = 4,
    GCLK_SRC_XOSC32K = 5,
    GCLK_SRC_OSC8M = 6,
    GCLK_SRC_DFLL48M = 7,
    GCLK_SRC_FDPLL96M = 8,
};

enum gclk_periph
{
    GCLK_DFLL48M = 0,
    GCLK_FDPLL96M,
    GCLK_FDPLL96M_32khz,
    GCLK_WDT,
    GCLK_RTC,
    GCLK_EIC,
    GCLK_USB,
    GCLK_EVSYS_CHANNEL_0,
    GCLK_EVSYS_CHANNEL_1,
    GCLK_EVSYS_CHANNEL_2,
    GCLK_EVSYS_CHANNEL_3,
    GCLK_EVSYS_CHANNEL_4,
    GCLK_EVSYS_CHANNEL_5,
    GCLK_EVSYS_CHANNEL_6,
    GCLK_EVSYS_CHANNEL_7,
    GCLK_EVSYS_CHANNEL_8,
    GCLK_EVSYS_CHANNEL_9,
    GCLK_EVSYS_CHANNEL_10,
    GCLK_EVSYS_CHANNEL_11,
    GCLK_SERCOMX_SLOW,
    GCLK_SERCOM0_CORE,
    GCLK_SERCOM1_CORE,
    GCLK_SERCOM2_CORE,
    GCLK_SERCOM3_CORE,
    GCLK_SERCOM4_CORE,
    GCLK_SERCOM5_CORE,
    GCLK_TCC0_TCC1,
    GCLK_TCC2_TC3,
    GCLK_TC4_TC5,
    GCLK_TC6_TC7,
    GCLK_ADC,
    GCLK_AC_DIG,
    GCLK_AC_ANA,
    GCLK_DAC,
    GCLK_PTC,
    GCLK_I2S_0,
    GCLK_I2S_1
};

void gclk_reset(void);
void gclk_set_gen(uint8_t gclk_id, enum gclk_src source, uint16_t div, uint8_t gen_mode);
void gclk_set_periph(enum gclk_periph periph_id, uint8_t gclk_id, uint8_t periph_mode);

#endif
