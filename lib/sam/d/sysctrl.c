#include <libopencm3/cm3/common.h>
#include <libopencm3/sam/d/memorymap.h>
#include <libopencm3/sam/d/nvmctrl.h>

#define SYSCTRL_PCLKSR_DFLLRDY (1 << 4)

#define SYSCTRL_DFLLCTRL_ENABLE   (1 << 1)
#define SYSCTRL_DFLLCTRL_MODE     (1 << 2)
#define SYSCTRL_DFLLCTRL_STABLE   (1 << 3)
#define SYSCTRL_DFLLCTRL_LLAW     (1 << 4)
#define SYSCTRL_DFLLCTRL_USBCRM   (1 << 5)
#define SYSCTRL_DFLLCTRL_RUNSTDBY (1 << 6)
#define SYSCTRL_DFLLCTRL_ONDEMAND (1 << 7)
#define SYSCTRL_DFLLCTRL_CCDIS    (1 << 8)
#define SYSCTRL_DFLLCTRL_QLDIS    (1 << 9)
#define SYSCTRL_DFLLCTRL_BPLCKC   (1 << 10)
#define SYSCTRL_DFLLCTRL_WAITLOCK (1 << 11)

#define SYSCTRL_PCLKSR   MMIO32(SYSCTRL_BASE + 0x0C)
#define SYSCTRL_DFLLCTRL MMIO32(SYSCTRL_BASE + 0x24)
#define SYSCTRL_DFLLVAL  MMIO32(SYSCTRL_BASE + 0x28)
#define SYSCTRL_DFLLMUL  MMIO32(SYSCTRL_BASE + 0x2C)

static inline void sysctrl_wait_dfll_sync(void)
{
    while (!(SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY))
        ;
}

// set dfll register according to errata 9905
static inline void sysctrl_set_dfll(uint16_t dfllctrl, uint32_t dfllval, uint32_t dfllmul)
{
    SYSCTRL_DFLLCTRL &= ~SYSCTRL_DFLLCTRL_ONDEMAND;
    sysctrl_wait_dfll_sync();

    SYSCTRL_DFLLVAL = dfllval;
    SYSCTRL_DFLLMUL = dfllmul;
    SYSCTRL_DFLLCTRL = dfllctrl;

    sysctrl_wait_dfll_sync();
}

void sysctrl_dfll_open_loop(void)
{
    uint16_t dfllctrl = SYSCTRL_DFLLCTRL_ENABLE;

    // generate 48MHz
    uint16_t dfllmul = 0xbb80;

    uint32_t dfllval = nvmctrl_get_dfll_otp();

    sysctrl_set_dfll(dfllctrl, dfllval, dfllmul);
}

/**
 * @brief Enable USB clock recovery mode for DFLL
 * 1. 48MHz clock is generated from USB SOF
 * 2. selected generic clock reference will be ignored
 * 3. DFLLVAL.COARSE will be used as COARSE final value
 */
void sysctrl_dfll_usb(void)
{
    // BPLCKC and STABLE are ignored
    uint16_t dfllctrl = SYSCTRL_DFLLCTRL_ENABLE;
    dfllctrl |= SYSCTRL_DFLLCTRL_USBCRM;
    dfllctrl &= ~SYSCTRL_DFLLCTRL_QLDIS;
    dfllctrl |= SYSCTRL_DFLLCTRL_CCDIS;

    // generate 48MHz
    uint32_t dfllmul = 0xbb80;

    uint32_t dfllval = nvmctrl_get_dfll_otp();

    sysctrl_set_dfll(dfllctrl, dfllval, dfllmul);
}
