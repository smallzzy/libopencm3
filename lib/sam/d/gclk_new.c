#include <libopencm3/sam/d/gclk_new.h>

#define GCLK_CTRL    MMIO8(GCLK_BASE + 0x00)
#define GCLK_STATUS  MMIO8(GCLK_BASE + 0x01)
#define GCLK_CLKCTRL MMIO16(GCLK_BASE + 0x02)
#define GCLK_GENCTRL MMIO32(GCLK_BASE + 0x04)
#define GCLK_GENDIV  MMIO32(GCLK_BASE + 0x08)

// read access
#define GCLK_CLKCTRL_ID MMIO8(GCLK_BASE + 0x02)
#define GCLK_GENCTRL_ID MMIO8(GCLK_BASE + 0x04)
#define GCLK_GENDIV_ID  MMIO8(GCLK_BASE + 0x08)

#define GCLK_STATUS_SYNCBUSY (1 << 7)

#define GCLK_CLKCTRL_GEN_SHIFT 8

#define GCLK_GENCTRL_SRC_SHIFT  8
#define GCLK_GENCTRL_MODE_SHIFT 16
// mode here

#define GCLK_GENDIV_DIV_SHIFT 8

// sync needed for GENCTRL, GENDIV, CTRL
static inline void gclk_wait_sync(void)
{
    while (GCLK_STATUS & GCLK_STATUS_SYNCBUSY);
}

void gclk_reset(void) {
    GCLK_CTRL = 1;
    gclk_wait_sync();
}

// for variant-a device, gclk frequency is capped to 32MHz when div > 1
void gclk_set_gen(uint8_t gclk_id, enum gclk_src source, uint16_t div, uint8_t gen_mode)
{
    GCLK_GENDIV = gclk_id |
                  (div << GCLK_GENDIV_DIV_SHIFT);    
    GCLK_GENCTRL = gclk_id |
                   (source << GCLK_GENCTRL_SRC_SHIFT) |
                   (gen_mode << GCLK_GENCTRL_MODE_SHIFT);
    gclk_wait_sync();
}

void gclk_set_periph(enum gclk_periph periph_id, uint8_t gclk_id, uint8_t periph_mode)
{
    GCLK_CLKCTRL = periph_id |
                   ((gclk_id | periph_mode) << GCLK_CLKCTRL_GEN_SHIFT);
}
