/*
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: <kang.zhao@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <serial.h>
#include <common.h>
#include <lcd.h>
#include <linux/list.h>
#include <linux/fb.h>
#include <asm/types.h>
#include <configs/halley5.h>
#include <asm/arch/tcu.h>
#include <asm/arch/gpio.h>
#include <regulator.h>

#include <jz_lcd/jz_dsim.h>
#include <jz_lcd/lcd_ma0060.h>
#include <jz_lcd/jz_lcd_v14.h>
#include "../jz_mipi_dsi/jz_mipi_dsih_hal.h"

vidinfo_t panel_info = { 720 , 1280, LCD_BPP, };
extern struct jzfb_config_info lcd_config_info;
struct lcd_ma0060_data lcd_ma0060_pdata;


struct fb_videomode jzfb1_videomode = {
	.name = "visionox_ma0060-lcd",
	.refresh = 60,
	.xres = 720,
	.yres = 1280,
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
	.pixclock = 18084,
};

struct jzfb_smart_config ma0060_cfg = {
	.dc_md = 0,
	.wr_md = 1,
	.smart_type = SMART_LCD_TYPE_8080,
	.pix_fmt = SMART_LCD_FORMAT_888,
	.dwidth = SMART_LCD_DWIDTH_24_BIT,
	.te_mipi_switch = 1,
	.te_switch = 1,
};

struct jzfb_config_info jzfb1_init_data = {
        .modes = &jzfb1_videomode,
        .lcd_type = LCD_TYPE_MIPI_SLCD,
	.bpp = 24,

	.smart_config = &ma0060_cfg,
        .pixclk_falling_edge = 0,
        .date_enable_active_low = 0,

        .dither_enable          = 1,
        .dither.dither_red      = 1,
        .dither.dither_green    = 1,
        .dither.dither_blue     = 1,
};

struct dsi_config jz_dsi_config = {
	.max_lanes = 2,
	.max_hs_to_lp_cycles = 40,
	.max_lp_to_hs_cycles = 100,
	.max_bta_cycles = 4095,
	.color_mode_polarity = 1,
	.shut_down_polarity = 1,
};

struct video_config jz_dsi_video_config={
	.no_of_lanes = 2,
	.virtual_channel =0,
	.color_coding = COLOR_CODE_24BIT,
	.video_mode = VIDEO_BURST_WITH_SYNC_PULSES,
	.receive_ack_packets = 0,
	.is_18_loosely = 0,
	.data_en_polarity = 1,
	.byte_clock = 0
};

struct dsi_device jz_dsi = {
	.dsi_config = &jz_dsi_config,
	.video_config = &jz_dsi_video_config,
	.data_lane = 2,
	.bpp_info = 24,
	.max_bps =2750,
};

struct lcd_ma0060_data lcd_ma0060_pdata = {
	.gpio_lcd_vdd  = CONFIG_GPIO_LCD_VDD,
	.gpio_lcd_rst = CONFIG_GPIO_LCD_RST,
	.gpio_lcd_oled  = CONFIG_GPIO_LCD_OLED,
	.gpio_lcd_pwm  = CONFIG_GPIO_LCD_PWM,
	.gpio_lcd_swire = CONFIG_GPIO_LCD_SWIRE,
	.gpio_lcd_bl = CONFIG_GPIO_LCD_BL,
};

static struct dsi_cmd_packet visionox_ma0060_720p_cmd_list1[] =
{
	{0x39, 0x02, 0x00, {0xFE, 0xD0}},
	{0x39, 0x02, 0x00, {0x40, 0x02}},
	{0x39, 0x02, 0x00, {0x4B, 0x4C}},
	{0x39, 0x02, 0x00, {0x49, 0x01}},

	/* {0x39, 0x02, 0x00, {0xFE, 0xD0}}, /1* 【2 lane 设置】 *1/ */
	{0x39, 0x02, 0x00, {0x1E, 0x05}},

	{0x39, 0x02, 0x00, {0xFE, 0x40}},
	{0x39, 0x02, 0x00, {0xC7, 0x85}},
	{0x39, 0x02, 0x00, {0xC8, 0x32}},
	{0x39, 0x02, 0x00, {0xC9, 0x18}},
	{0x39, 0x02, 0x00, {0xCA, 0x09}},
	{0x39, 0x02, 0x00, {0xCB, 0x22}},
	{0x39, 0x02, 0x00, {0xCC, 0x44}},
	{0x39, 0x02, 0x00, {0xCD, 0x11}},
	{0x39, 0x02, 0x00, {0x05, 0x0F}},
	{0x39, 0x02, 0x00, {0x06, 0x09}},
	{0x39, 0x02, 0x00, {0x08, 0x0F}},
	{0x39, 0x02, 0x00, {0x09, 0x09}},
	{0x39, 0x02, 0x00, {0x0A, 0xE6}},
	{0x39, 0x02, 0x00, {0x0B, 0x88}},

	{0x39, 0x02, 0x00, {0x0E, 0x10}},
	{0x39, 0x02, 0x00, {0x20, 0x93}},
	{0x39, 0x02, 0x00, {0x21, 0x93}},
	{0x39, 0x02, 0x00, {0x24, 0x02}},
	{0x39, 0x02, 0x00, {0x26, 0x02}},
	{0x39, 0x02, 0x00, {0x28, 0x05}},
	{0x39, 0x02, 0x00, {0x2A, 0x05}},
	{0x39, 0x02, 0x00, {0x2D, 0x23}},
	{0x39, 0x02, 0x00, {0x2F, 0x23}},
	{0x39, 0x02, 0x00, {0x30, 0x23}},
	{0x39, 0x02, 0x00, {0x31, 0x23}},
	{0x39, 0x02, 0x00, {0x37, 0x80}},
	{0x39, 0x02, 0x00, {0x38, 0x50}},
	{0x39, 0x02, 0x00, {0x39, 0x00}},
	{0x39, 0x02, 0x00, {0x46, 0x27}},
	{0x39, 0x02, 0x00, {0x6F, 0x00}},
	{0x39, 0x02, 0x00, {0x74, 0x2F}},
	{0x39, 0x02, 0x00, {0x75, 0x19}},
	{0x39, 0x02, 0x00, {0x79, 0x00}},
	{0x39, 0x02, 0x00, {0xAD, 0x00}},
	{0x39, 0x02, 0x00, {0xFE, 0x60}},
	{0x39, 0x02, 0x00, {0x00, 0xC4}},
	{0x39, 0x02, 0x00, {0x01, 0x00}},
	{0x39, 0x02, 0x00, {0x02, 0x02}},
	{0x39, 0x02, 0x00, {0x03, 0x00}},
	{0x39, 0x02, 0x00, {0x04, 0x00}},
	{0x39, 0x02, 0x00, {0x05, 0x07}},
	{0x39, 0x02, 0x00, {0x06, 0x00}},
	{0x39, 0x02, 0x00, {0x07, 0x83}},
	{0x39, 0x02, 0x00, {0x09, 0xC4}},
	{0x39, 0x02, 0x00, {0x0A, 0x00}},
	{0x39, 0x02, 0x00, {0x0B, 0x02}},
	{0x39, 0x02, 0x00, {0x0C, 0x00}},
	{0x39, 0x02, 0x00, {0x0D, 0x00}},
	{0x39, 0x02, 0x00, {0x0E, 0x08}},
	{0x39, 0x02, 0x00, {0x0F, 0x00}},
	{0x39, 0x02, 0x00, {0x10, 0x83}},
	{0x39, 0x02, 0x00, {0x12, 0xCC}},
	{0x39, 0x02, 0x00, {0x13, 0x0F}},
	{0x39, 0x02, 0x00, {0x14, 0xFF}},
	{0x39, 0x02, 0x00, {0x15, 0x01}},
	{0x39, 0x02, 0x00, {0x16, 0x00}},
	{0x39, 0x02, 0x00, {0x17, 0x02}},
	{0x39, 0x02, 0x00, {0x18, 0x00}},
	{0x39, 0x02, 0x00, {0x19, 0x00}},
	{0x39, 0x02, 0x00, {0x1B, 0xC4}},
	{0x39, 0x02, 0x00, {0x1C, 0x00}},
	{0x39, 0x02, 0x00, {0x1D, 0x02}},
	{0x39, 0x02, 0x00, {0x1E, 0x00}},
	{0x39, 0x02, 0x00, {0x1F, 0x00}},
	{0x39, 0x02, 0x00, {0x20, 0x08}},
	{0x39, 0x02, 0x00, {0x21, 0x00}},
	{0x39, 0x02, 0x00, {0x22, 0x89}},
	{0x39, 0x02, 0x00, {0x24, 0xC4}},
	{0x39, 0x02, 0x00, {0x25, 0x00}},
	{0x39, 0x02, 0x00, {0x26, 0x02}},
	{0x39, 0x02, 0x00, {0x27, 0x00}},
	{0x39, 0x02, 0x00, {0x28, 0x00}},
	{0x39, 0x02, 0x00, {0x29, 0x07}},
	{0x39, 0x02, 0x00, {0x2A, 0x00}},
	{0x39, 0x02, 0x00, {0x2B, 0x89}},
	{0x39, 0x02, 0x00, {0x2F, 0xC4}},
	{0x39, 0x02, 0x00, {0x30, 0x00}},
	{0x39, 0x02, 0x00, {0x31, 0x02}},
	{0x39, 0x02, 0x00, {0x32, 0x00}},
	{0x39, 0x02, 0x00, {0x33, 0x00}},
	{0x39, 0x02, 0x00, {0x34, 0x06}},
	{0x39, 0x02, 0x00, {0x35, 0x00}},
	{0x39, 0x02, 0x00, {0x36, 0x89}},
	{0x39, 0x02, 0x00, {0x38, 0xC4}},
	{0x39, 0x02, 0x00, {0x39, 0x00}},
	{0x39, 0x02, 0x00, {0x3A, 0x02}},
	{0x39, 0x02, 0x00, {0x3B, 0x00}},
	{0x39, 0x02, 0x00, {0x3D, 0x00}},
	{0x39, 0x02, 0x00, {0x3F, 0x07}},
	{0x39, 0x02, 0x00, {0x40, 0x00}},
	{0x39, 0x02, 0x00, {0x41, 0x89}},
	{0x39, 0x02, 0x00, {0x4C, 0xC4}},
	{0x39, 0x02, 0x00, {0x4D, 0x00}},
	{0x39, 0x02, 0x00, {0x4E, 0x04}},
	{0x39, 0x02, 0x00, {0x4F, 0x01}},
	{0x39, 0x02, 0x00, {0x50, 0x00}},
	{0x39, 0x02, 0x00, {0x51, 0x08}},
	{0x39, 0x02, 0x00, {0x52, 0x00}},
	{0x39, 0x02, 0x00, {0x53, 0x61}},
	{0x39, 0x02, 0x00, {0x55, 0xC4}},
	{0x39, 0x02, 0x00, {0x56, 0x00}},
	{0x39, 0x02, 0x00, {0x58, 0x04}},
	{0x39, 0x02, 0x00, {0x59, 0x01}},
	{0x39, 0x02, 0x00, {0x5A, 0x00}},
	{0x39, 0x02, 0x00, {0x5B, 0x06}},
	{0x39, 0x02, 0x00, {0x5C, 0x00}},
	{0x39, 0x02, 0x00, {0x5D, 0x61}},
	{0x39, 0x02, 0x00, {0x5F, 0xCE}},
	{0x39, 0x02, 0x00, {0x60, 0x00}},
	{0x39, 0x02, 0x00, {0x61, 0x07}},
	{0x39, 0x02, 0x00, {0x62, 0x05}},
	{0x39, 0x02, 0x00, {0x63, 0x00}},
	{0x39, 0x02, 0x00, {0x64, 0x04}},
	{0x39, 0x02, 0x00, {0x65, 0x00}},
	{0x39, 0x02, 0x00, {0x66, 0x60}},
	{0x39, 0x02, 0x00, {0x67, 0x80}},
	{0x39, 0x02, 0x00, {0x9B, 0x03}},
	{0x39, 0x02, 0x00, {0xA9, 0x10}},
	{0x39, 0x02, 0x00, {0xAA, 0x10}},
	{0x39, 0x02, 0x00, {0xAB, 0x02}},
	{0x39, 0x02, 0x00, {0xAC, 0x04}},
	{0x39, 0x02, 0x00, {0xAD, 0x03}},
	{0x39, 0x02, 0x00, {0xAE, 0x10}},
	{0x39, 0x02, 0x00, {0xAF, 0x10}},
	{0x39, 0x02, 0x00, {0xB0, 0x10}},
	{0x39, 0x02, 0x00, {0xB1, 0x10}},
	{0x39, 0x02, 0x00, {0xB2, 0x10}},
	{0x39, 0x02, 0x00, {0xB3, 0x10}},
	{0x39, 0x02, 0x00, {0xB4, 0x10}},
	{0x39, 0x02, 0x00, {0xB5, 0x10}},
	{0x39, 0x02, 0x00, {0xB6, 0x10}},
	{0x39, 0x02, 0x00, {0xB7, 0x10}},
	{0x39, 0x02, 0x00, {0xB8, 0x10}},
	{0x39, 0x02, 0x00, {0xB9, 0x08}},
	{0x39, 0x02, 0x00, {0xBA, 0x09}},
	{0x39, 0x02, 0x00, {0xBB, 0x0A}},
	{0x39, 0x02, 0x00, {0xBC, 0x05}},
	{0x39, 0x02, 0x00, {0xBD, 0x06}},
	{0x39, 0x02, 0x00, {0xBE, 0x02}},
	{0x39, 0x02, 0x00, {0xBF, 0x10}},
	{0x39, 0x02, 0x00, {0xC0, 0x10}},
	{0x39, 0x02, 0x00, {0xC1, 0x03}},
	{0x39, 0x02, 0x00, {0xC4, 0x80}},
	{0x39, 0x02, 0x00, {0xFE, 0x70}},
	{0x39, 0x02, 0x00, {0x48, 0x05}},
	{0x39, 0x02, 0x00, {0x52, 0x00}},
	{0x39, 0x02, 0x00, {0x5A, 0xFF}},
	{0x39, 0x02, 0x00, {0x5C, 0xF6}},
	{0x39, 0x02, 0x00, {0x5D, 0x07}},
	{0x39, 0x02, 0x00, {0x7D, 0x75}},
	{0x39, 0x02, 0x00, {0x86, 0x07}},
	{0x39, 0x02, 0x00, {0xA7, 0x02}},
	{0x39, 0x02, 0x00, {0xA9, 0x2C}},
	{0x39, 0x02, 0x00, {0xFE, 0xA0}},
	{0x39, 0x02, 0x00, {0x2B, 0x18}},

#if 1
	{0x39, 0x02, 0x00, {0xFE, 0x40}}, /* 【scaling up 设置】 */
	{0x39, 0x02, 0x00, {0x36, 0x55}}, //scal_en */
	{0x39, 0x02, 0x00, {0x0D, 0x90}}, //2a2b_en */
	{0x39, 0x02, 0x00, {0x0E, 0x00}},

	{0x39, 0x02, 0x00, {0xFE, 0xD0}},
	{0x39, 0x02, 0x00, {0x07, 0x84}},

	{0x39, 0x02, 0x00, {0xFE, 0x00}},
	{0x39, 0x05, 0x00, {0x2A, 0x00, 0x00, 0x02, 0xCF}},
	{0x39, 0x05, 0x00, {0x2B, 0x00, 0x00, 0x04, 0xFF}},
#endif
};

//#ifndef _1080P
static struct dsi_cmd_packet visionox_ma0060_720p_cmd_list2[] =
{
	{0x15, 0xFE, 0x00},
	{0x15, 0xC2, 0x08},
	{0x15, 0x35, 0x00},
};

static struct dsi_cmd_packet visionox_ma0060_720p_cmd_list11[] =
{
	{0X39, 0X02, 0X00, {0XFE, 0XD0}},
	{0X39, 0X02, 0X00, {0X07, 0X84}},
	{0X39, 0X02, 0X00, {0X40, 0X02}},
	{0X39, 0X02, 0X00, {0X4B, 0X4C}},
	{0X39, 0X02, 0X00, {0X49, 0X01}},
	{0X39, 0X02, 0X00, {0XFE, 0X40}},
	{0X39, 0X02, 0X00, {0XC7, 0X85}},
	{0X39, 0X02, 0X00, {0XC8, 0X32}},
	{0X39, 0X02, 0X00, {0XC9, 0X18}},
	{0X39, 0X02, 0X00, {0XCA, 0X09}},
	{0X39, 0X02, 0X00, {0XCB, 0X22}},
	{0X39, 0X02, 0X00, {0XCC, 0X44}},
	{0X39, 0X02, 0X00, {0XCD, 0X11}},
	{0X39, 0X02, 0X00, {0X05, 0X0F}},
	{0X39, 0X02, 0X00, {0X06, 0X09}},
	{0X39, 0X02, 0X00, {0X08, 0X0F}},
	{0X39, 0X02, 0X00, {0X09, 0X09}},
	{0X39, 0X02, 0X00, {0X0A, 0XE6}},
	{0X39, 0X02, 0X00, {0X0B, 0X88}},
	{0X39, 0X02, 0X00, {0X0D, 0X90}},
	{0X39, 0X02, 0X00, {0X0E, 0X10}},
	{0X39, 0X02, 0X00, {0X20, 0X93}},
	{0X39, 0X02, 0X00, {0X21, 0X93}},
	{0X39, 0X02, 0X00, {0X24, 0X02}},
	{0X39, 0X02, 0X00, {0X26, 0X02}},
	{0X39, 0X02, 0X00, {0X28, 0X05}},
	{0X39, 0X02, 0X00, {0X2A, 0X05}},
	{0X39, 0X02, 0X00, {0X2D, 0X23}},
	{0X39, 0X02, 0X00, {0X2F, 0X23}},
	{0X39, 0X02, 0X00, {0X30, 0X23}},
	{0X39, 0X02, 0X00, {0X31, 0X23}},
	{0X39, 0X02, 0X00, {0X36, 0X55}},
	{0X39, 0X02, 0X00, {0X37, 0X80}},
	{0X39, 0X02, 0X00, {0X38, 0X50}},
	{0X39, 0X02, 0X00, {0X39, 0X00}},
	{0X39, 0X02, 0X00, {0X46, 0X27}},
	{0X39, 0X02, 0X00, {0X6F, 0X00}},
	{0X39, 0X02, 0X00, {0X74, 0X2F}},
	{0X39, 0X02, 0X00, {0X75, 0X19}},
	{0X39, 0X02, 0X00, {0X79, 0X00}},
	{0X39, 0X02, 0X00, {0XAD, 0X00}},
	{0X39, 0X02, 0X00, {0XFE, 0X60}},
	{0X39, 0X02, 0X00, {0X00, 0XC4}},
	{0X39, 0X02, 0X00, {0X01, 0X00}},
	{0X39, 0X02, 0X00, {0X02, 0X02}},
	{0X39, 0X02, 0X00, {0X03, 0X00}},
	{0X39, 0X02, 0X00, {0X04, 0X00}},
	{0X39, 0X02, 0X00, {0X05, 0X07}},
	{0X39, 0X02, 0X00, {0X06, 0X00}},
	{0X39, 0X02, 0X00, {0X07, 0X83}},
	{0X39, 0X02, 0X00, {0X09, 0XC4}},
	{0X39, 0X02, 0X00, {0X0A, 0X00}},
	{0X39, 0X02, 0X00, {0X0B, 0X02}},
	{0X39, 0X02, 0X00, {0X0C, 0X00}},
	{0X39, 0X02, 0X00, {0X0D, 0X00}},
	{0X39, 0X02, 0X00, {0X0E, 0X08}},
	{0X39, 0X02, 0X00, {0X0F, 0X00}},
	{0X39, 0X02, 0X00, {0X10, 0X83}},
	{0X39, 0X02, 0X00, {0X12, 0XCC}},
	{0X39, 0X02, 0X00, {0X13, 0X0F}},
	{0X39, 0X02, 0X00, {0X14, 0XFF}},
	{0X39, 0X02, 0X00, {0X15, 0X01}},
	{0X39, 0X02, 0X00, {0X16, 0X00}},
	{0X39, 0X02, 0X00, {0X17, 0X02}},
	{0X39, 0X02, 0X00, {0X18, 0X00}},
	{0X39, 0X02, 0X00, {0X19, 0X00}},
	{0X39, 0X02, 0X00, {0X1B, 0XC4}},
	{0X39, 0X02, 0X00, {0X1C, 0X00}},
	{0X39, 0X02, 0X00, {0X1D, 0X02}},
	{0X39, 0X02, 0X00, {0X1E, 0X00}},
	{0X39, 0X02, 0X00, {0X1F, 0X00}},
	{0X39, 0X02, 0X00, {0X20, 0X08}},
	{0X39, 0X02, 0X00, {0X21, 0X00}},
	{0X39, 0X02, 0X00, {0X22, 0X89}},
	{0X39, 0X02, 0X00, {0X24, 0XC4}},
	{0X39, 0X02, 0X00, {0X25, 0X00}},
	{0X39, 0X02, 0X00, {0X26, 0X02}},
	{0X39, 0X02, 0X00, {0X27, 0X00}},
	{0X39, 0X02, 0X00, {0X28, 0X00}},
	{0X39, 0X02, 0X00, {0X29, 0X07}},
	{0X39, 0X02, 0X00, {0X2A, 0X00}},
	{0X39, 0X02, 0X00, {0X2B, 0X89}},
	{0X39, 0X02, 0X00, {0X2F, 0XC4}},
	{0X39, 0X02, 0X00, {0X30, 0X00}},
	{0X39, 0X02, 0X00, {0X31, 0X02}},
	{0X39, 0X02, 0X00, {0X32, 0X00}},
	{0X39, 0X02, 0X00, {0X33, 0X00}},
	{0X39, 0X02, 0X00, {0X34, 0X06}},
	{0X39, 0X02, 0X00, {0X35, 0X00}},
	{0X39, 0X02, 0X00, {0X36, 0X89}},
	{0X39, 0X02, 0X00, {0X38, 0XC4}},
	{0X39, 0X02, 0X00, {0X39, 0X00}},
	{0X39, 0X02, 0X00, {0X3A, 0X02}},
	{0X39, 0X02, 0X00, {0X3B, 0X00}},
	{0X39, 0X02, 0X00, {0X3D, 0X00}},
	{0X39, 0X02, 0X00, {0X3F, 0X07}},
	{0X39, 0X02, 0X00, {0X40, 0X00}},
	{0X39, 0X02, 0X00, {0X41, 0X89}},
	{0X39, 0X02, 0X00, {0X4C, 0XC4}},
	{0X39, 0X02, 0X00, {0X4D, 0X00}},
	{0X39, 0X02, 0X00, {0X4E, 0X04}},
	{0X39, 0X02, 0X00, {0X4F, 0X01}},
	{0X39, 0X02, 0X00, {0X50, 0X00}},
	{0X39, 0X02, 0X00, {0X51, 0X08}},
	{0X39, 0X02, 0X00, {0X52, 0X00}},
	{0X39, 0X02, 0X00, {0X53, 0X61}},
	{0X39, 0X02, 0X00, {0X55, 0XC4}},
	{0X39, 0X02, 0X00, {0X56, 0X00}},
	{0X39, 0X02, 0X00, {0X58, 0X04}},
	{0X39, 0X02, 0X00, {0X59, 0X01}},
	{0X39, 0X02, 0X00, {0X5A, 0X00}},
	{0X39, 0X02, 0X00, {0X5B, 0X06}},
	{0X39, 0X02, 0X00, {0X5C, 0X00}},
	{0X39, 0X02, 0X00, {0X5D, 0X61}},
	{0X39, 0X02, 0X00, {0X5F, 0XCE}},
	{0X39, 0X02, 0X00, {0X60, 0X00}},
	{0X39, 0X02, 0X00, {0X61, 0X07}},
	{0X39, 0X02, 0X00, {0X62, 0X05}},
	{0X39, 0X02, 0X00, {0X63, 0X00}},
	{0X39, 0X02, 0X00, {0X64, 0X04}},
	{0X39, 0X02, 0X00, {0X65, 0X00}},
	{0X39, 0X02, 0X00, {0X66, 0X60}},
	{0X39, 0X02, 0X00, {0X67, 0X80}},
	{0X39, 0X02, 0X00, {0X9B, 0X03}},
	{0X39, 0X02, 0X00, {0XA9, 0X10}},
	{0X39, 0X02, 0X00, {0XAA, 0X10}},
	{0X39, 0X02, 0X00, {0XAB, 0X02}},
	{0X39, 0X02, 0X00, {0XAC, 0X04}},
	{0X39, 0X02, 0X00, {0XAD, 0X03}},
	{0X39, 0X02, 0X00, {0XAE, 0X10}},
	{0X39, 0X02, 0X00, {0XAF, 0X10}},
	{0X39, 0X02, 0X00, {0XB0, 0X10}},
	{0X39, 0X02, 0X00, {0XB1, 0X10}},
	{0X39, 0X02, 0X00, {0XB2, 0X10}},
	{0X39, 0X02, 0X00, {0XB3, 0X10}},
	{0X39, 0X02, 0X00, {0XB4, 0X10}},
	{0X39, 0X02, 0X00, {0XB5, 0X10}},
	{0X39, 0X02, 0X00, {0XB6, 0X10}},
	{0X39, 0X02, 0X00, {0XB7, 0X10}},
	{0X39, 0X02, 0X00, {0XB8, 0X10}},
	{0X39, 0X02, 0X00, {0XB9, 0X08}},
	{0X39, 0X02, 0X00, {0XBA, 0X09}},
	{0X39, 0X02, 0X00, {0XBB, 0X0A}},
	{0X39, 0X02, 0X00, {0XBC, 0X05}},
	{0X39, 0X02, 0X00, {0XBD, 0X06}},
	{0X39, 0X02, 0X00, {0XBE, 0X02}},
	{0X39, 0X02, 0X00, {0XBF, 0X10}},
	{0X39, 0X02, 0X00, {0XC0, 0X10}},
	{0X39, 0X02, 0X00, {0XC1, 0X03}},
	{0X39, 0X02, 0X00, {0XC4, 0X80}},
	{0X39, 0X02, 0X00, {0XFE, 0X70}},
	{0X39, 0X02, 0X00, {0X48, 0X05}},
	{0X39, 0X02, 0X00, {0X52, 0X00}},
	{0X39, 0X02, 0X00, {0X5A, 0XFF}},
	{0X39, 0X02, 0X00, {0X5C, 0XF6}},
	{0X39, 0X02, 0X00, {0X5D, 0X07}},
	{0X39, 0X02, 0X00, {0X7D, 0X75}},
	{0X39, 0X02, 0X00, {0X86, 0X07}},
	{0X39, 0X02, 0X00, {0XA7, 0X02}},
	{0X39, 0X02, 0X00, {0XA9, 0X2C}},
	{0X39, 0X02, 0X00, {0XFE, 0XA0}},
	{0X39, 0X02, 0X00, {0X2B, 0X18}},

	{0x39, 0x02, 0x00, {0xFE, 0xD0}}, /* 【2 lane 设置】 */
	{0x39, 0x02, 0x00, {0x1E, 0x05}},

	{0x39, 0x02, 0x00, {0xFE, 0x00}},
	{0x39, 0x05, 0x00, {0x2A, 0x00, 0x00, 0x02, 0xCF}},
	{0x39, 0x05, 0x00, {0x2B, 0x00, 0x00, 0x04, 0xFF}},
};
//#endif

#ifdef _1080P
static struct dsi_cmd_packet visionox_ma0060_1080p_cmd_list1[] =
{
	{0x39, 0x02, 0x00, {0xFE, 0x04}},
	{0x39, 0x02, 0x00, {0x01, 0x08}},
	{0x39, 0x02, 0x00, {0x0D, 0x80}},
	{0x39, 0x02, 0x00, {0x05, 0x07}},
	{0x39, 0x02, 0x00, {0x06, 0x09}},
	{0x39, 0x02, 0x00, {0x0A, 0xDB}},
	{0x39, 0x02, 0x00, {0x0B, 0x8C}},
	{0x39, 0x02, 0x00, {0x0E, 0x10}},
	{0x39, 0x02, 0x00, {0x0F, 0xE0}},
	{0x39, 0x02, 0x00, {0x1A, 0x11}},
	{0x39, 0x02, 0x00, {0x29, 0x93}},
	{0x39, 0x02, 0x00, {0x2A, 0x93}},
	{0x39, 0x02, 0x00, {0x2F, 0x02}},
	{0x39, 0x02, 0x00, {0x31, 0x02}},
	{0x39, 0x02, 0x00, {0x33, 0x05}},
	{0x39, 0x02, 0x00, {0x37, 0x23}},
	{0x39, 0x02, 0x00, {0x38, 0x23}},
	{0x39, 0x02, 0x00, {0x3A, 0x23}},
	{0x39, 0x02, 0x00, {0x3B, 0x23}},
	{0x39, 0x02, 0x00, {0x3D, 0x2C}},
	{0x39, 0x02, 0x00, {0x3F, 0x80}},
	{0x39, 0x02, 0x00, {0x40, 0x50}},
	{0x39, 0x02, 0x00, {0x41, 0x8E}},
	{0x39, 0x02, 0x00, {0x4F, 0x2F}},
	{0x39, 0x02, 0x00, {0x50, 0x19}},
	{0x39, 0x02, 0x00, {0x51, 0x0A}},

	{0x39, 0x02, 0x00, {0xFE, 0x07}},
	{0x39, 0x02, 0x00, {0x03, 0x40}},
	{0x39, 0x02, 0x00, {0x05, 0x00}},
	{0x39, 0x02, 0x00, {0x07, 0x0A}},

	{0x39, 0x02, 0x00, {0xFE, 0x06}},
	{0x39, 0x02, 0x00, {0xB0, 0x11}},
	{0x39, 0x02, 0x00, {0x00, 0xE4}},
	{0x39, 0x02, 0x00, {0x01, 0x0F}},
	{0x39, 0x02, 0x00, {0x02, 0xFF}},
	{0x39, 0x02, 0x00, {0x03, 0x00}},
	{0x39, 0x02, 0x00, {0x04, 0x00}},
	{0x39, 0x02, 0x00, {0x05, 0x02}},
	{0x39, 0x02, 0x00, {0x06, 0x00}},
	{0x39, 0x02, 0x00, {0x07, 0xC0}},
	{0x39, 0x02, 0x00, {0x08, 0xE4}},
	{0x39, 0x02, 0x00, {0x09, 0x00}},
	{0x39, 0x02, 0x00, {0x0A, 0x02}},
	{0x39, 0x02, 0x00, {0x0B, 0x00}},
	{0x39, 0x02, 0x00, {0x0C, 0x00}},
	{0x39, 0x02, 0x00, {0x0D, 0x08}},
	{0x39, 0x02, 0x00, {0x0E, 0x00}},
	{0x39, 0x02, 0x00, {0x0F, 0x83}},
	{0x39, 0x02, 0x00, {0x10, 0xE4}},
	{0x39, 0x02, 0x00, {0x11, 0x00}},
	{0x39, 0x02, 0x00, {0x12, 0x02}},
	{0x39, 0x02, 0x00, {0x13, 0x00}},
	{0x39, 0x02, 0x00, {0x14, 0x00}},
	{0x39, 0x02, 0x00, {0x15, 0x07}},
	{0x39, 0x02, 0x00, {0x16, 0x00}},
	{0x39, 0x02, 0x00, {0x17, 0x67}},
	{0x39, 0x02, 0x00, {0x18, 0xE4}},
	{0x39, 0x02, 0x00, {0x19, 0x00}},
	{0x39, 0x02, 0x00, {0x1A, 0x02}},
	{0x39, 0x02, 0x00, {0x1B, 0x00}},
	{0x39, 0x02, 0x00, {0x1C, 0x00}},
	{0x39, 0x02, 0x00, {0x1D, 0x07}},
	{0x39, 0x02, 0x00, {0x1E, 0x00}},
	{0x39, 0x02, 0x00, {0x1F, 0x67}},
	{0x39, 0x02, 0x00, {0x20, 0xE4}},
	{0x39, 0x02, 0x00, {0x21, 0x00}},
	{0x39, 0x02, 0x00, {0x22, 0x02}},
	{0x39, 0x02, 0x00, {0x23, 0x00}},
	{0x39, 0x02, 0x00, {0x24, 0x00}},
	{0x39, 0x02, 0x00, {0x25, 0x08}},
	{0x39, 0x02, 0x00, {0x26, 0x00}},
	{0x39, 0x02, 0x00, {0x27, 0x83}},
	{0x39, 0x02, 0x00, {0x28, 0xE4}},
	{0x39, 0x02, 0x00, {0x29, 0x00}},
	{0x39, 0x02, 0x00, {0x2A, 0x02}},
	{0x39, 0x02, 0x00, {0x2B, 0x00}},
	{0x39, 0x02, 0x00, {0x2D, 0x00}},
	{0x39, 0x02, 0x00, {0x2F, 0x07}},
	{0x39, 0x02, 0x00, {0x30, 0x00}},
	{0x39, 0x02, 0x00, {0x31, 0x67}},
	{0x39, 0x02, 0x00, {0x32, 0xE4}},
	{0x39, 0x02, 0x00, {0x33, 0x00}},
	{0x39, 0x02, 0x00, {0x34, 0x02}},
	{0x39, 0x02, 0x00, {0x35, 0x00}},
	{0x39, 0x02, 0x00, {0x36, 0x00}},
	{0x39, 0x02, 0x00, {0x37, 0x08}},
	{0x39, 0x02, 0x00, {0x38, 0x00}},
	{0x39, 0x02, 0x00, {0x39, 0x67}},
	{0x39, 0x02, 0x00, {0x44, 0xE4}},
	{0x39, 0x02, 0x00, {0x45, 0x00}},
	{0x39, 0x02, 0x00, {0x46, 0x04}},
	{0x39, 0x02, 0x00, {0x47, 0x01}},
	{0x39, 0x02, 0x00, {0x48, 0x00}},
	{0x39, 0x02, 0x00, {0x49, 0x08}},
	{0x39, 0x02, 0x00, {0x4A, 0x00}},
	{0x39, 0x02, 0x00, {0x4B, 0x64}},
	{0x39, 0x02, 0x00, {0x4C, 0xE4}},
	{0x39, 0x02, 0x00, {0x4D, 0x00}},
	{0x39, 0x02, 0x00, {0x4E, 0x04}},
	{0x39, 0x02, 0x00, {0x4F, 0x01}},
	{0x39, 0x02, 0x00, {0x50, 0x00}},
	{0x39, 0x02, 0x00, {0x51, 0x06}},
	{0x39, 0x02, 0x00, {0x52, 0x00}},
	{0x39, 0x02, 0x00, {0x53, 0x64}},
	{0x39, 0x02, 0x00, {0x8D, 0xEA}},
	{0x39, 0x02, 0x00, {0x8E, 0x0F}},
	{0x39, 0x02, 0x00, {0x8F, 0xFF}},
	{0x39, 0x02, 0x00, {0x90, 0x05}},
	{0x39, 0x02, 0x00, {0x91, 0x00}},
	{0x39, 0x02, 0x00, {0x92, 0x04}},
	{0x39, 0x02, 0x00, {0x93, 0x00}},
	{0x39, 0x02, 0x00, {0x94, 0xAB}},
	{0x39, 0x02, 0x00, {0xAC, 0x00}},
	{0x39, 0x02, 0x00, {0xA6, 0x20}},
	{0x39, 0x02, 0x00, {0xA7, 0x04}},
	{0x39, 0x02, 0x00, {0xA9, 0x58}},
	{0x39, 0x02, 0x00, {0xB1, 0x17}},
	{0x39, 0x02, 0x00, {0xB2, 0x17}},
	{0x39, 0x02, 0x00, {0xB3, 0x00}},
	{0x39, 0x02, 0x00, {0xB4, 0x03}},
	{0x39, 0x02, 0x00, {0xB5, 0x04}},
	{0x39, 0x02, 0x00, {0xB6, 0x17}},
	{0x39, 0x02, 0x00, {0xB7, 0x17}},
	{0x39, 0x02, 0x00, {0xB8, 0x17}},
	{0x39, 0x02, 0x00, {0xB9, 0x17}},
	{0x39, 0x02, 0x00, {0xBA, 0x17}},
	{0x39, 0x02, 0x00, {0xBB, 0x17}},
	{0x39, 0x02, 0x00, {0xBC, 0x17}},
	{0x39, 0x02, 0x00, {0xBD, 0x17}},
	{0x39, 0x02, 0x00, {0xBE, 0x17}},
	{0x39, 0x02, 0x00, {0xBF, 0x17}},
	{0x39, 0x02, 0x00, {0xC0, 0x17}},
	{0x39, 0x02, 0x00, {0xC1, 0x08}},
	{0x39, 0x02, 0x00, {0xC2, 0x09}},
	{0x39, 0x02, 0x00, {0xC3, 0x11}},
	{0x39, 0x02, 0x00, {0xC4, 0x06}},
	{0x39, 0x02, 0x00, {0xC5, 0x05}},
	{0x39, 0x02, 0x00, {0xC6, 0x00}},
	{0x39, 0x02, 0x00, {0xC7, 0x17}},
	{0x39, 0x02, 0x00, {0xC8, 0x17}},

	{0x39, 0x02, 0x00, {0xFE, 0x09}},
	{0x39, 0x02, 0x00, {0x15, 0x92}},
	{0x39, 0x02, 0x00, {0x16, 0x24}},
	{0x39, 0x02, 0x00, {0x17, 0x49}},
	{0x39, 0x02, 0x00, {0x1B, 0x92}},
	{0x39, 0x02, 0x00, {0x1C, 0x24}},
	{0x39, 0x02, 0x00, {0x1D, 0x49}},
	{0x39, 0x02, 0x00, {0x21, 0x92}},
	{0x39, 0x02, 0x00, {0x22, 0x24}},
	{0x39, 0x02, 0x00, {0x23, 0x49}},
	{0x39, 0x02, 0x00, {0x9B, 0x47}},
	{0x39, 0x02, 0x00, {0x9C, 0x81}},
	{0x39, 0x02, 0x00, {0xA3, 0x18}},

	{0x39, 0x02, 0x00, {0xFE, 0x0A}},
	{0x39, 0x02, 0x00, {0x25, 0x66}},
	{0x39, 0x02, 0x00, {0xFE, 0x0E}},
	{0x39, 0x02, 0x00, {0x12, 0x33}},

	{0x39, 0x02, 0x00, {0xFE, 0xD0}}, /* 【2 lane 设置】 */
	{0x39, 0x02, 0x00, {0x1E, 0x05}},

#if 0
	{0x39, 0x02, 0x00, {0xFE, 0x40}}, /* 【scaling up 设置】 */
	{0x39, 0x02, 0x00, {0x36, 0x55}}, //scal_en */
	{0x39, 0x02, 0x00, {0x0D, 0x90}}, //2a2b_en */
	{0x39, 0x02, 0x00, {0x0E, 0x00}},

	{0x39, 0x02, 0x00, {0xFE, 0xD0}},
	{0x39, 0x02, 0x00, {0x07, 0x84}},

	{0x39, 0x02, 0x00, {0xFE, 0x00}},
	{0x39, 0x05, 0x00, {0x2A, 0x00, 0x00, 0x02, 0xCF}},
	{0x39, 0x05, 0x00, {0x2B, 0x00, 0x00, 0x04, 0xFF}},
#endif
	/* {0x39, 0x02, 0x00, {0x3A, 0x55}}, */
};

static struct dsi_cmd_packet visionox_ma0060_1080p_cmd_list2[] =
{
	{0x15, 0xFE, 0x00},
	{0x15, 0xC2, 0x08},
	{0x15, 0x35, 0x00},
};
#endif

void panel_pin_init(void)
{
	int ret = 0;
	ret = gpio_request(lcd_ma0060_pdata.gpio_lcd_vdd,"lcd_vdd");
	if(ret){
	 	/*printf("cannot request gpoi lcd_vdd\n");*/
	}
	ret = gpio_request(lcd_ma0060_pdata.gpio_lcd_rst,"lcd_rst");
	if(ret){
		/*printf("cannot request gpoi lcd_rst\n");*/
	}

	ret = gpio_request(lcd_ma0060_pdata.gpio_lcd_oled,"lcd_oled");
	if(ret){
		/*printf("cannot request gpoi lcd_oled\n");*/
	}

	ret = gpio_request(lcd_ma0060_pdata.gpio_lcd_pwm,"lcd_pwm");
	if(ret){
		/*printf("cannot request gpoi lcd_pwm\n");*/
	}

	ret = gpio_request(lcd_ma0060_pdata.gpio_lcd_swire,"lcd_swire");
	if(ret){
		/*printf("cannot request gpoi lcd_swire\n");*/
	}

	ret = gpio_request(lcd_ma0060_pdata.gpio_lcd_bl,"lcd_bl");
	if(ret){
		/*printf("cannot request gpoi lcd_swire\n");*/
	}

	serial_puts("lcd_ma0060 panel display pin init\n");
}

void open_backlight(void)
{
	gpio_direction_output(lcd_ma0060_pdata.gpio_lcd_bl,1);
	printf("bl = %d\n",lcd_ma0060_pdata.gpio_lcd_bl);

}

void panel_power_on(void)
{

	gpio_direction_output(lcd_ma0060_pdata.gpio_lcd_vdd,1);
	gpio_direction_input(lcd_ma0060_pdata.gpio_lcd_swire);
	gpio_direction_input(lcd_ma0060_pdata.gpio_lcd_oled);
	mdelay(100);

	gpio_direction_output(lcd_ma0060_pdata.gpio_lcd_rst,1);
	mdelay(100);
	gpio_direction_output(lcd_ma0060_pdata.gpio_lcd_rst,0);
	mdelay(150);
	gpio_direction_output(lcd_ma0060_pdata.gpio_lcd_rst,1);
	mdelay(100);

        serial_puts("lcd_ma0060 panel display on\n");
}

void panel_power_off(void)
{

	gpio_direction_output(lcd_ma0060_pdata.gpio_lcd_vdd,0);
	gpio_direction_output(lcd_ma0060_pdata.gpio_lcd_rst,0);
	gpio_direction_output(lcd_ma0060_pdata.gpio_lcd_oled,0);
	gpio_direction_output(lcd_ma0060_pdata.gpio_lcd_pwm,0);
	gpio_direction_output(lcd_ma0060_pdata.gpio_lcd_rst,0);
	serial_puts("lcd_ma0060 panel display off\n");
}


void ma0060_sleep_in(struct dsi_device *dsi)
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x10, 0x00};

	write_command(dsi, data_to_send);
}

void ma0060_sleep_out(struct dsi_device *dsi)
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x11, 0x00};

	write_command(dsi, data_to_send);
}

void ma0060_display_on(struct dsi_device *dsi)
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x29, 0x00};

	write_command(dsi, data_to_send);
}

void ma0060_display_off(struct dsi_device *dsi)
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x28, 0x00};

	write_command(dsi, data_to_send);
}

void ma0060_panel_init(struct dsi_device *dsi)
{
	int i;
/*
#ifdef _1080P
	for (i = 0; i < ARRAY_SIZE(visionox_ma0060_1080p_cmd_list1); i++)
	{
		write_command(dsi,visionox_ma0060_1080p_cmd_list1[i]);
	}
	mdelay(20);
	for (i = 0; i < ARRAY_SIZE(visionox_ma0060_1080p_cmd_list2); i++)
	{
		write_command(dsi,visionox_ma0060_1080p_cmd_list2[i]);
	}
#else
*/
	for (i = 0; i < ARRAY_SIZE(visionox_ma0060_720p_cmd_list11); i++)
	{
		write_command(dsi,visionox_ma0060_720p_cmd_list11[i]);
	}
	mdelay(20);
	for (i = 0; i < ARRAY_SIZE(visionox_ma0060_720p_cmd_list2); i++)
	{
		write_command(dsi,visionox_ma0060_720p_cmd_list2[i]);
	}
}

void panel_init_sequence(struct dsi_device *dsi)
{
	ma0060_panel_init(dsi);
	mdelay(120);
	ma0060_sleep_out(dsi);
        mdelay(120);
	ma0060_display_on(dsi);
	mdelay(80);
}


