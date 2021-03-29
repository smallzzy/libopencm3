/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2018  Flirc Inc.
 * Written by Jason Kotzin <jasonkotzin@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
/**@{*/

/** 
 * TODO:
 *
 * Will re-write the API to better align with libopencm3, but will
 * continue to use bitfields
 */

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/sam/d/gclk.h>
#include <libopencm3/sam/d/sysctl.h>
#include <libopencm3/sam/d/pm.h>
#include <libopencm3/sam/d/nvmctrl.h>
#include <libopencm3/sam/d/bitfield.h>

static void _gclk_init(void)
{
	/* make sure the gclk peripheral is enable */
	INSERTBF(PM_APBAMASK_GCLK, 1, PM->apbamask);

	/* reset the gclk controller and wait */
	INSERTBF(GCLK_CTRL_SWRST, 1, GCLK->ctrl);
	while (GETBF(GCLK_CTRL_SWRST, GCLK->ctrl));
}

uint8_t get_gclk_source(uint8_t gclk_id)
{
	cm_disable_interrupts();

	uint8_t *gclk_id_p = (uint8_t *)&(GCLK->genctrl);

	while (GETBF(GCLK_STATUS_SYNCBUSY, GCLK->status));

	*gclk_id_p = gclk_id;

	cm_enable_interrupts();

	gclk_id_p++;

	return *(gclk_id_p);
}

static uint16_t get_gclk_div(uint8_t gclk_id)
{
	cm_disable_interrupts();
	uint16_t div;

	/* first write only the gclk_id */
	uint8_t *gclk_id_p = (uint8_t *)&(GCLK->genctrl);
	*gclk_id_p = gclk_id;

	/* wait for chip */
	while (GETBF(GCLK_STATUS_SYNCBUSY, GCLK->status));

	/* set the gclk for the divider register too */
	gclk_id_p = (uint8_t *)&(GCLK->gendiv);
	*gclk_id_p = gclk_id;

	/* wait for chip */
	while (GETBF(GCLK_STATUS_SYNCBUSY, GCLK->status));

	/* grab the divider */
	div = GETBF(GCLK_GENDIV_DIV, GCLK->gendiv);

	if (div == 0) {
		/* zero means on in chip */
		div = 1;
	}

	/* each gclk has a max divider */
	switch (gclk_id) {
	case (GCLK0):
		div &= 0xFF;
		break;
	case (GCLK1):
		div &= 0xFFFF;
		break;
	case (GCLK2):
		div &= 0x1F;
		break;
	case (GCLK3):
	case (GCLK4):
	case (GCLK5):
	case (GCLK6):
	case (GCLK7):
	case (GCLK8):
		div &= 0xFF;
		break;
	default:
		//log_err("error, unknown gclkid\n");
		break;
	}

	/* how is gclk divided */
	if (GETBF(GCLK_GENCTRL_DIVSEL, GCLK->genctrl)) {
		/*  GCLK/(2^(GENDIV.DIV+1)) */
		int i;
		uint32_t div_exp = 2;

		for (i = 0; i < div+1; i++) {
			div_exp *= 2;
		}

		/* gonna max out */
		div = div_exp;
	}

	cm_enable_interrupts();
	return div;
}

uint8_t get_periph_clk_gclk_id(uint8_t peripheral)
{
	cm_disable_interrupts();

	/* set the peripheral first */
	GCLK->clkctrl_id = BF(GCLK_CLKCTRL_ID, peripheral);

	cm_enable_interrupts();

	return GETBF(GCLK_CLKCTRL_GEN,  GCLK->clkctrl);
}

/* Mapping of clock source to Speed */
static struct gclk_src_t {
	uint8_t clock;
	uint32_t speed;
} gclk_src_t [] = {
	{
		.clock = SRC_XOSC,
		.speed = 0, /* should be define */
	},
	{
		.clock = SRC_GCLKIN,
		.speed = 0, /* should be define */
	},
	{
		.clock = SRC_GCLKGEN1,
		.speed = 0, /* should be define */
	},
	{
		.clock = SRC_OSCULP32K,
		.speed = 32,
	},
	{
		.clock = SRC_OSC32K,
		.speed = 32,
	},
	{
		.clock = SRC_XOSC32K,
		.speed = 32,
	},
	{
		.clock = SRC_OSC8M,
		.speed = 8000,
	},
	{
		.clock = SRC_DFLL48M,
		.speed = 48000,
	},
	{
		.clock = SRC_FDPLL96M,
		.speed = 96000,
	},
};

uint32_t get_periph_clk_speed(uint8_t peripheral)
{
	/* find peripheral gclk source*/
	uint8_t gclk = get_periph_clk_gclk_id(peripheral);

	/* get the dividers */
	uint16_t div = get_gclk_div(gclk);

	uint8_t clk_src = get_gclk_source(gclk);
	uint32_t src_speed = gclk_src_t[clk_src].speed;

	if (gclk != GCLK0 && clk_src == SRC_OSC8M)
		src_speed /= 8;

	return src_speed/div;
}

static void errata(void)
{
	cm_enable_interrupts();

	/* following sdk *********************************/
	SYSCTL->intflag = BF(SYSCTL_INTFLAG_BOD33RDY, 1) |
			  BF(SYSCTL_INTFLAG_BOD33DET, 1) |
			  BF(SYSCTL_INTFLAG_DFLLRDY, 1);

	/* set number of wait states */
	nvmctrl_setwaitstates(2);
	/*************************************************/
}

static void setup_osc32k(void)
{
	uint32_t cal = ((*(uint32_t *)FUSES_OSC32K_CAL_ADDR >>
			FUSES_OSC32K_CAL_Pos) & 0x7Ful);

	/* test setup 32k *********************************/
	SYSCTL->osc32k =  BF(SYSCTL_OSC32K_CALIB, cal) |
			  BF(SYSCTL_OSC32K_STARTUP, SYSTEM_OSC32K_STARTUP_10) |
			  BF(SYSCTL_OSC32K_ENABLE, 1) |
			  BF(SYSCTL_OSC32K_ONDEMAND, 1) |
			  BF(SYSCTL_OSC32K_EN32K, 1);
}

int gclk_init(struct gclk_hw *hw)
{
	errata();

	/* initialize 48Mhz clock */
	sysctrl_dfll_usb();

	/* initialize 32kHz clock */
	setup_osc32k();

	/* enable 8Mhz internal oscillator (usually on by default) */
	INSERTBF(SYSCTL_OSC8M_ENABLE, 1, SYSCTL->osc8m);
	INSERTBF(SYSCTL_OSC8M_ONDEMAND, 1, SYSCTL->osc8m);

	/* make sure global system clock peripheral is enable */
	_gclk_init();

	gclk_set_gen(GCLK1, hw->gclk1, hw->gclk1_div, GCLK_MODE_GENEN);	
	gclk_set_gen(GCLK2, hw->gclk2, hw->gclk2_div, GCLK_MODE_GENEN);
	gclk_set_gen(GCLK3, hw->gclk3, hw->gclk3_div, GCLK_MODE_GENEN);
	gclk_set_gen(GCLK4, hw->gclk4, hw->gclk4_div, GCLK_MODE_GENEN);
	gclk_set_gen(GCLK5, hw->gclk5, hw->gclk5_div, GCLK_MODE_GENEN);
	gclk_set_gen(GCLK6, hw->gclk6, hw->gclk6_div, GCLK_MODE_GENEN);
	gclk_set_gen(GCLK7, hw->gclk7, hw->gclk7_div, GCLK_MODE_GENEN);
	gclk_set_gen(GCLK0, hw->gclk0, hw->gclk0_div, GCLK_MODE_GENEN);

	return 0;
}
