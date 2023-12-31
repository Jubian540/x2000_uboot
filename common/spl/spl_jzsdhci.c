#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <fat.h>
#include <version.h>
#include <mmc.h>
#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>
#include <asm/io.h>
#include "spl_gpt_partition.h"
#include "spl_rtos.h"
#ifdef CONFIG_JZSD_OTA_VERSION20
#include "spl_ota_jzsd.h"
#endif

//#define DEBUG_MSC
//#define DEBUG_DDR_CONTENT

#ifdef DEBUG_MSC
#define msc_debug	printf
#else
#define msc_debug(fmt, args...) do { }while(0)
#endif

/* global variables */
static uint32_t io_base = MSC0_BASE;
static int highcap = 0;

#define MSC_BUS_WIDTH_1            0
#define MSC_BUS_WIDTH_4            2
#define MSC_BUS_WIDTH_8            3

#if defined(CONFIG_SPL_JZ_MSC_BUS_8BIT)
static uint32_t bus_width = MSC_BUS_WIDTH_8;
#elif defined(CONFIG_SPL_JZ_MSC_BUS_4BIT)
static uint32_t bus_width = MSC_BUS_WIDTH_4;
#else // CONFIG_SPL_JZ_MSC_BUS_1BIT
static uint32_t bus_width = MSC_BUS_WIDTH_1;
#endif

static uint32_t msc_readl(uint32_t off)
{
	return readl(io_base + off);
}
static uint16_t msc_readw(uint32_t off)
{
	return readw(io_base + off);
}
static uint16_t msc_readb(uint32_t off)
{
	return readb(io_base + off);
}

static void msc_writel(uint32_t off, uint32_t value)
{
	writel(value, io_base + off);
}
static void msc_writew(uint32_t off, uint32_t value)
{
	writew(value, io_base + off);
}
static void msc_writeb(uint32_t off, uint32_t value)
{
	writeb(value, io_base + off);
}

#ifdef DEBUG_MSC
static void dump_error_status(void)
{
	msc_debug("PSTATE_REG				= %x\n", msc_readl(MSC_PSTATE_REG));
	msc_debug("XFER_MODE_R				= %x\n", msc_readw(MSC_XFER_MODE_R));
	msc_debug("NORMAL_INT_STAT_EN_R	= %x\n", msc_readw(MSC_NORMAL_INT_STAT_EN_R));
	msc_debug("NORMAL_INT_STAT_R		= %x\n", msc_readw(MSC_NORMAL_INT_STAT_R));
	msc_debug("ERROR_INT_STAT_EN_R		= %x\n", msc_readw(MSC_ERROR_INT_STAT_EN_R));
	msc_debug("ERROR_INT_STAT_R		= %x\n", msc_readw(MSC_ERROR_INT_STAT_R));
	msc_debug("ADMA_ERR_STAT_R			= %x\n", msc_readw(MSC_ADMA_ERR_STAT_R));
	msc_debug("HOST_CTRL1_R			= %x\n", msc_readb(MSC_HOST_CTRL1_R));
	msc_debug("HOST_CTRL2_R			= %x\n", msc_readw(MSC_HOST_CTRL2_R));
}
#else
static void dump_error_status(void) {}
#endif


static void msc_reset(u8 mask)
{
	u32 timeout = 0xffff;

	msc_writeb(MSC_SW_RST_R, mask);
	while((msc_readb(MSC_SW_RST_R) & mask) && --timeout) {
		udelay(10);
	}

	if(!timeout)
		printf("mshc host reset=0x%x fail!\n", mask);
}

static void mmc_init_host(void)
{
	msc_reset(MSC_SW_RST_ALL_BIT);

	msc_writew(MSC_NORMAL_INT_STAT_EN_R, 0xffff);
	msc_writew(MSC_ERROR_INT_STAT_EN_R, 0xffff);

	msc_writeb(MSC_TOUT_CTRL_R, 0xe);
}

static void msc_clk_switch(int high_frq)
{
	uint32_t val;

#ifdef CONFIG_FPGA
#define CPM_MSC0_CLK_R			  (0xB0000068)
#define CPM_MSC1_CLK_R			  (0xB00000a4)
#define CPM_MSC2_CLK_R			  (0xB00000a8)
#define MSC_CLK_H_FREQ			  (0x1 << 20)
  #ifdef CONFIG_JZ_MMC_MSC0
	#define CPM_MSC_CLK_R   CPM_MSC0_CLK_R
  #endif
  #ifdef CONFIG_JZ_MMC_MSC1
	#define CPM_MSC_CLK_R   CPM_MSC1_CLK_R
  #endif
  #ifdef CONFIG_JZ_MMC_MSC2
	#define CPM_MSC_CLK_R   CPM_MSC2_CLK_R
  #endif
	/* set clk */
	val = readl(CPM_MSC_CLK_R);
	if(high_frq)
		val |= MSC_CLK_H_FREQ;
	else
		val &= ~MSC_CLK_H_FREQ;
	writel(val, CPM_MSC_CLK_R);
#else //CONFIG_FPGA
#define MSC_INIT_CLK    200000
#ifndef MSC_WORKING_CLK
#define MSC_WORKING_CLK 50000000
#endif
  #ifdef CONFIG_JZ_MMC_MSC0
	#define CPM_MSC MSC0
  #endif
  #ifdef CONFIG_JZ_MMC_MSC1
	#define CPM_MSC MSC1
  #endif
  #ifdef CONFIG_JZ_MMC_MSC2
	#define CPM_MSC MSC2
  #endif
	/* TODO: set clk */
	msc_writew(MSC_CLK_CTRL_R, MSC_SD_CLK_EN_BIT | MSC_INTERNAL_CLK_EN_BIT);
	/* set clk */
	if (!high_frq) {
		clk_set_rate(CPM_MSC, MSC_INIT_CLK);
	}
	else
		clk_set_rate(CPM_MSC, MSC_WORKING_CLK);

	printf("%s : clk_id[%d], set clk[%d], clk_get_rate=%d\n", __func__,
			CPM_MSC, high_frq ? MSC_WORKING_CLK : MSC_INIT_CLK, clk_get_rate(CPM_MSC));
#endif

}

#ifdef DEBUG_MSC
static u32 cmd_err_stat_check(void)
{
	if(msc_readw(MSC_ERROR_INT_STAT_R) \
			& (MSC_CMD_TOUT_ERR_STAT_BIT \
			| MSC_CMD_CRC_ERR_STAT_BIT \
			| MSC_CMD_END_BIT_ERR_STAT_BIT \
			| MSC_CMD_IDX_ERR_STAT_BIT)){
		msc_debug("[ERROR]:Command check Error!!!\n");
		dump_error_status();
		msc_writew(MSC_ERROR_INT_STAT_R, 0xffff);
		return -1;
	}
	return 0;
}

static u32 xfer_err_stat_check(void)
{
	if(msc_readw(MSC_ERROR_INT_STAT_R) \
			& (MSC_DATA_TOUT_ERR_STAT_BIT \
			| MSC_DATA_CRC_ERR_STAT_BIT)){
		msc_debug("[ERROR]:Data Xfer Error!!!\n");
		dump_error_status();
		msc_writew(MSC_ERROR_INT_STAT_R, 0xffff);
		return -1;
	}
	return 0;
}
#endif

static u32 wait_cmd_complete(void)
{
	u32 timeout = 100000;

	while(!(msc_readw(MSC_NORMAL_INT_STAT_R) \
				& MSC_CMD_COMPLETE_STAT_BIT) && --timeout){
		udelay(10);
	}

	if(!timeout) {
		printf("[ERROR]: MSC_ERROR_INT_STAT_R : %x, cmd timeout...\n", msc_readw(MSC_ERROR_INT_STAT_R));
		return -1;
	}

	msc_writew(MSC_NORMAL_INT_STAT_R, MSC_CMD_COMPLETE_STAT_BIT);

	return 0;
}

static u32 wait_xfer_complete(void)
{
	u32 timeout = 0xffff;

	while(!(msc_readw(MSC_NORMAL_INT_STAT_R) \
				& MSC_XFER_COMPLETE_STAT_BIT) && --timeout){
		udelay(10);
	}

	if(!timeout) {
		printf("[ERROR]: MSC_ERROR_INT_STAT_R : %x, xfer timeout...\n", msc_readw(MSC_ERROR_INT_STAT_R));
		return -1;
	}

	msc_writew(MSC_NORMAL_INT_STAT_R, MSC_XFER_COMPLETE_STAT_BIT);

	return 0;
}

static u32 wait_buf_rb(void)
{
	unsigned int timeout = 0xffff;

	while(!(msc_readw(MSC_NORMAL_INT_STAT_R) \
				& MSC_BUF_RD_READY_STAT_BIT) && --timeout){
		udelay(10);  //1 block read time
	}

	if(!timeout) {
		printf("[ERROR]: MSC_ERROR_INT_STAT_R: %x, buf read timeout err ...\n", msc_readw(MSC_ERROR_INT_STAT_R));
		dump_error_status();
		return -1;
	}

	msc_writew(MSC_NORMAL_INT_STAT_R, MSC_BUF_RD_READY_STAT_BIT);

	return 0;
}
static u16 package_cmd(u32 cmdidx, u32 cmdat, u32 rtype)
{
	u16 cmd = 0;

	cmd |= cmdidx << MSC_CMD_INDEX_LBIT;

	switch (rtype) {
		case MSC_CMDAT_RESPONSE_NONE:
			cmd &= ~MSC_CMD_IDX_CHK_ENABLE_BIT;
			cmd	&= ~MSC_CMD_CRC_CHK_ENABLE_BIT; //command index & crc check disable
			break;
		case MSC_CMDAT_RESPONSE_R1:
		case MSC_CMDAT_RESPONSE_R7:
			cmd |= MSC_CMD_IDX_CHK_ENABLE_BIT | MSC_CMD_CRC_CHK_ENABLE_BIT;
			cmd &= ~MSC_RESP_TYPE_SELECT_MASK;
			cmd |= MSC_RESP_TYPE_SELECT_RESP_LEN_48; //length 48
			break;
		case MSC_CMDAT_RESPONSE_R1b:
			cmd |= MSC_CMD_IDX_CHK_ENABLE_BIT | MSC_CMD_CRC_CHK_ENABLE_BIT;
			cmd &= ~MSC_RESP_TYPE_SELECT_MASK;
			cmd |= MSC_RESP_TYPE_SELECT_RESP_LEN_48B; //length 48B
			break;
		case MSC_CMDAT_RESPONSE_R2:
			cmd &= ~(MSC_CMD_IDX_CHK_ENABLE_BIT | MSC_CMD_CRC_CHK_ENABLE_BIT);
			cmd &= ~MSC_RESP_TYPE_SELECT_MASK;
			cmd |= MSC_RESP_TYPE_SELECT_RESP_LEN_136; //length 136
			break;
		case MSC_CMDAT_RESPONSE_R3:
		case MSC_CMDAT_RESPONSE_R6:
			cmd &= ~(MSC_CMD_IDX_CHK_ENABLE_BIT | MSC_CMD_CRC_CHK_ENABLE_BIT);
			cmd &= ~MSC_RESP_TYPE_SELECT_MASK;
			cmd |= MSC_RESP_TYPE_SELECT_RESP_LEN_48; //length 48
			break;
		default:
			break;
	}

	return cmd;
}

static  u8* msc_get_resp(void)
{
	static u8 resp[20] = {0};
	u32 RESP_ARRAY[4];
	u32 words, i, j;

	memset(RESP_ARRAY, 0, sizeof(RESP_ARRAY));
	RESP_ARRAY[3] = msc_readl(MSC_RESP01_R);
	RESP_ARRAY[2] = msc_readl(MSC_RESP23_R);
	RESP_ARRAY[1] = msc_readl(MSC_RESP45_R);
	RESP_ARRAY[0] = msc_readl(MSC_RESP67_R);

	words = 1; //low lost eight
	memset(resp, 0, sizeof(u8)*20);
	for(i = 4; i > 0; i--){
		for(j = 0; j < 4; j++){
			resp[words] = (RESP_ARRAY[i - 1] & (0xff << j * 8)) >> j * 8;
			words++;
		}
	}
#if 0
	printf("Response of CMD\n");
	for(i=0; i<=3; i++){
		printf("\tRESP%d%d=0x%08x\n",6-(i*2),7-(i*2), RESP_ARRAY[i]);
	}
#endif

	return resp;
}

static u32 msc_check_cmd_data_line(u32 cmdidx)
{
	unsigned int mask = 0;
	unsigned int timeout = 100;

	mask = MSC_CMD_INHIBIT_BIT | MSC_CMD_INHIBIT_DAT_BIT;

	/* We shouldn't wait for data inihibit for stop commands, even
	   though they might use busy signaling */
	if (cmdidx == MMC_CMD_STOP_TRANSMISSION)
		mask &= ~MSC_CMD_INHIBIT_DAT_BIT;

	while (msc_readl(MSC_PSTATE_REG) & mask) {
		if (timeout == 0) {
			printf("Controller never released inhibit bit(s).\n");
			return -1;
		}
		timeout--;
		udelay(1000);
	}

	return 0;
}

static u8* mmc_cmd(u32 cmdidx, u32 arg, u32 cmdat, u32 rtype)
{
	u16 cmd_args;
	u8 *resp = NULL;

	if(msc_check_cmd_data_line(cmdidx))
		goto exit;

	cmd_args = package_cmd(cmdidx, cmdat, rtype);
	cmd_args |= cmdat;

	msc_writel(MSC_ARGUMENT_R, arg);
	msc_writew(MSC_CMD_R, cmd_args);

	msc_debug("cmdidx=%d, cmd_args=0x%x, arg=0x%x, response=R%d\n", cmdidx, cmd_args, arg, rtype);

	if(wait_cmd_complete())
		goto exit;

#ifdef DEBUG_MSC
	if(cmd_err_stat_check())
		goto exit;
#endif
	resp = msc_get_resp();


exit:
	if(!(cmdat & MSC_DATA_PRESENT_SEL_BIT)) {
		msc_reset(MSC_SW_RST_CMD_BIT);
		msc_reset(MSC_SW_RST_DAT_BIT);
	}

	return resp;
}

static void msc_set_xfer_bus_width(unsigned int buswidth)
{
	u32 val;

	/* bus width */
	val = msc_readb(MSC_HOST_CTRL1_R);

	switch (buswidth) {
		case MSC_BUS_WIDTH_1:
			val &= ~MSC_DAT_XFER_WIDTH_BIT;  //1bit mode
			break;
		case MSC_BUS_WIDTH_4:
			val |= MSC_DAT_XFER_WIDTH_BIT;  //4bit mode
			break;
		case MSC_BUS_WIDTH_8:
			val |= MSC_EXT_DAT_XFER_BIT;  //8bit mode
			break;
		default:
			val &= ~MSC_DAT_XFER_WIDTH_BIT;  //1bit mode
			break;
	}

	msc_writeb(MSC_HOST_CTRL1_R, val);
}

static u32 mmc_block_read(u32 start, u32 blkcnt, u32 *dst)
{
	u32 cnt, nob;
	u32 cmd_args = 0;
	u32 xfer_data = 0;

	if(blkcnt <= 0)
		return -1;

	if(highcap == 0) {
		/* standard capacity : Bytes addressed */
		cmd_args = start * 512;
	} else {
		/* high capacity: sector addressed */
		cmd_args = start;
	}

	msc_debug("%s-->start: %d \n", __func__, start);
	msc_debug("%s-->blkcnt: %d \n", __func__, blkcnt);
	msc_debug("%s-->dst: 0x%x\n", __func__, dst);
	msc_debug("%s-->bus_width: %d\n", __func__, bus_width);

	nob = blkcnt;
	msc_writew(MSC_BLOCKSIZE_R, 0x200);
	msc_writew(MSC_BLOCKCOUNT_R, nob);

	msc_set_xfer_bus_width(bus_width);
	msc_debug("HOST_CTRL1_R			= %x\n", msc_readb(MSC_HOST_CTRL1_R));

	xfer_data = msc_readw(MSC_XFER_MODE_R);
	xfer_data |= (MSC_BLOCK_COUNT_ENABLE_BIT | MSC_DATA_XFER_DIR_BIT  \
				| MSC_AUTO_CMD12_ENABLE);
	if (1 != blkcnt)
		xfer_data |= MSC_MULTI_BLK_SEL_BIT;

	msc_writew(MSC_XFER_MODE_R, xfer_data);

	mmc_cmd(MMC_CMD_SET_BLOCKLEN, 0x200, 0, MSC_CMDAT_RESPONSE_R1);
	if(1 == blkcnt)
		mmc_cmd(MMC_CMD_READ_SINGLE_BLOCK, cmd_args, MSC_DATA_PRESENT_SEL_BIT, MSC_CMDAT_RESPONSE_R1);
	else
		mmc_cmd(MMC_CMD_READ_MULTIPLE_BLOCK, cmd_args, MSC_DATA_PRESENT_SEL_BIT, MSC_CMDAT_RESPONSE_R1);

	for(; nob > 0; nob--) {
		cnt = 512 / 4;
		if(wait_buf_rb())
			goto err;

		while(cnt--) {
			*dst = msc_readl(MSC_BUF_DATA_R);
			dst++;
		}
	}

	if(wait_xfer_complete())
		goto err;
#ifdef DEBUG_MSC
	if(xfer_err_stat_check())
		goto err;
#endif

err:
	msc_reset(MSC_SW_RST_CMD_BIT);
	msc_reset(MSC_SW_RST_DAT_BIT);
	return blkcnt - nob;
}

static void msc_sync_abort(void)
{
	msc_writeb(MSC_BGAP_CTRL_R, MSC_STOP_BG_REQ_BIT);

	wait_xfer_complete();

	mmc_cmd(12, 0, MSC_CMD_TYPE_ABORT_CMD, MSC_CMDAT_RESPONSE_R1b);

	msc_reset(MSC_SW_RST_CMD_BIT);
	msc_reset(MSC_SW_RST_DAT_BIT);
}


static int sd_found(void)
{
	u8 *resp;
	u32 cardaddr, timeout = 0xffff;
	int rca;

	msc_debug("sd_found\n");
	resp = mmc_cmd(55, 0, 0, MSC_CMDAT_RESPONSE_R1);

	resp = mmc_cmd(41, 0x40ff8000, 0, MSC_CMDAT_RESPONSE_R3);

	while (timeout-- && !(resp[4] & 0x80)) {
		mdelay(1);
		resp = mmc_cmd(55, 0, 0, MSC_CMDAT_RESPONSE_R1);
		resp = mmc_cmd(41, 0x40ff8000, 0, MSC_CMDAT_RESPONSE_R3);
	}

	if (!(resp[4] & 0x80)) {
		printf("sd init fail!\n");
		return -1;
	}

	if((resp[4] & 0x60 ) == 0x40)
		highcap = 1;
	else
		highcap =0;

	resp = mmc_cmd(2, 0, 0, MSC_CMDAT_RESPONSE_R2);
	resp = mmc_cmd(3, 0, 0, MSC_CMDAT_RESPONSE_R6);
	cardaddr = (resp[4] << 8) | resp[3];
	rca = cardaddr << 16;
	resp = mmc_cmd(9, rca, 0, MSC_CMDAT_RESPONSE_R2);

	resp = mmc_cmd(7, rca, 0, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(55, rca, 0, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(6, bus_width, 0, MSC_CMDAT_RESPONSE_R1);
	msc_clk_switch(1);
	return 0;
}

static int mmc_found(void)
{
	u8 *resp;
	u32 rca, status;
	u32 buswidth_arg, buswidth, timeout = 100;

	msc_debug("mmc_found\n");

	msc_sync_abort();

	resp = mmc_cmd(0, 0, 0, MSC_CMDAT_RESPONSE_NONE);
	resp = mmc_cmd(1, 0x40ff8000, 0, MSC_CMDAT_RESPONSE_R3);

	while (timeout-- && !(resp[4] & 0x80)) {
		mdelay(1);
		resp = mmc_cmd(1, 0x40ff8000, 0, MSC_CMDAT_RESPONSE_R3);
	}

	if (!timeout) {
		printf("emmc card init err ...\n");
		return -1;
	}

	if((resp[4] & 0x60 ) == 0x40)
		highcap = 1;
	else
		highcap =0;

	resp = mmc_cmd(2, 0, 0, MSC_CMDAT_RESPONSE_R2);

	rca = 0x10 << 16;
	resp = mmc_cmd(3, rca, 0, MSC_CMDAT_RESPONSE_R1);

	resp = mmc_cmd(7, rca, 0, MSC_CMDAT_RESPONSE_R1);

	buswidth = (bus_width - 1) < 0 ? 0 : (bus_width - 1);
	buswidth_arg = 0x3 << 24 | 183 << 16 | buswidth << 8 | 0x1;
	resp = mmc_cmd(6, buswidth_arg, 0, MSC_CMDAT_RESPONSE_R1b); /* set buswidth*/

	timeout = 1000;
	do{
		resp = mmc_cmd(13, rca, 0, MSC_CMDAT_RESPONSE_R1);
		status = resp[1] | (resp[2] << 8) | (resp[3] << 16) | (resp[4] << 24);
		if((status & (0xf << 9)) != (7 << 9))
			break;
		udelay(100);
	}while(--timeout);

	msc_clk_switch(1);

	return 0;
}

static int jzmmc_init(void)
{
	u8 *resp;
	u32 ret = 0;

	mmc_init_host();

	msc_clk_switch(0);

	msc_sync_abort();

	resp = mmc_cmd(0, 0, 0, MSC_CMDAT_RESPONSE_NONE);
	resp = mmc_cmd(8, 0x1aa, 0, MSC_CMDAT_RESPONSE_R1);

	resp = mmc_cmd(55, 0, 0, MSC_CMDAT_RESPONSE_R1);
	if (resp[1] & 1 << 5) {
		resp = mmc_cmd(41, 0x40ff8000, 0, MSC_CMDAT_RESPONSE_R3);
		if(*(u32*)resp)
			ret = sd_found();
		else
			ret = mmc_found();
	} else {
		ret = mmc_found();
	}

	return ret;
}

#ifdef DEBUG_DDR_CONTENT
static int dump_ddr_content(unsigned int *src, int len)
{
	int i;
	volatile unsigned int *p = src;
	for(i=0; i<len/4; i++) {
		msc_debug("%x:%x\n", &p[i], p[i]);
		src++;
	}
}
#endif

static int mmc_load_image_raw(unsigned long sector)
{
	int err = 0;
	u32 image_size_sectors;
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
					 sizeof(struct image_header));

	/* read image header to find the image size & load address */
	err = mmc_block_read(sector, 1, header);
	if (err < 0)
		goto end;

#ifdef DEBUG_DDR_CONTENT
	dump_ddr_content(header, 0x200);
#endif
	header->ih_name[IH_NMLEN - 1] = 0;
	spl_parse_image_header(header);

	/* convert size to sectors - round up */
	image_size_sectors = (spl_image.size + 0x200 - 1) / 0x200;

	/* Read the header too to avoid extra memcpy */
	err = mmc_block_read(sector, image_size_sectors,
			     (void *)spl_image.load_addr);

#ifdef DEBUG_DDR_CONTENT
	dump_ddr_content(spl_image.load_addr, 200);
#endif
	flush_cache_all();
end:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	if (err < 0)
		msc_debug("spl: mmc blk read err - %lu\n", err);
#endif
	return err;
}

#ifdef CONFIG_SPL_OS_BOOT

#ifndef CONFIG_GPT_CREATOR
#error "must define CONFIG_GPT_CREATOR"
#endif

static int mmc_load_img_from_partition(const char *name)
{
	unsigned int start_sector;
	int ret;

	ret = spl_get_built_in_gpt_partition(name, &start_sector, NULL);
	if (ret) {
		printf("mmc: failed to get partition: %s\n", name);
		return ret;
	}

	return mmc_load_image_raw(start_sector);
}
#endif

#ifdef CONFIG_SPL_OS_OTA_BOOT

static int mmc_ota_load_img_from_partition(const char *name)
{
	unsigned int start_sector;
	int ret;
	int is_kernel2 = 0;
	const char *kernel_name = name;

	ret = spl_get_built_in_gpt_partition(CONFIG_SPL_OTA_NAME, &start_sector, NULL);
	if (!ret) {
		const char *buf = (const char *)(CONFIG_SYS_TEXT_BASE);
		const char *kernel2 = "ota:"CONFIG_SPL_OS_NAME2;

		mmc_block_read(start_sector, 1, (u32 *)buf);
		if (!strncmp(kernel2, buf, strlen(kernel2))) {
			is_kernel2 = 1;
			kernel_name = CONFIG_SPL_OS_NAME2;
		}
	}

	ret = spl_get_built_in_gpt_partition(kernel_name, &start_sector, NULL);
	if (ret) {
		printf("kernel not found: "CONFIG_SPL_OS_NAME"\n");
		hang();
	}

	debug("kernel:%s %x\n", kernel_name, start_sector);

	mmc_load_image_raw(start_sector);

	if (is_kernel2)
		return CONFIG_SYS_SPL_ARGS_ADDR2;
	else
		return CONFIG_SYS_SPL_ARGS_ADDR;

}
#endif

#ifdef CONFIG_JZSD_OTA_VERSION20
static struct jzsd_ota_ops jzsd_ota_ops = {
	.jzsd_read = mmc_block_read,
	.jzsd_load_img_from_partition = mmc_load_img_from_partition,
};
#endif

#ifdef CONFIG_SPL_RTOS_BOOT

struct rtos_header rtos_header;

#ifdef CONFIG_RTOS_BOOT_ON_SECOND_CPU
#include <asm/arch/ccu.h>

unsigned char second_cpu_little_stack[128] __attribute__((aligned(8)));

__attribute__ ((noreturn)) void do_boot_second_cpu(void)
{
	rtos_raw_start(&rtos_header);
	while (1);
}

static void boot_second_cpu(void)
{
  asm volatile (
    "    .set    push                        \n"
    "    .set    reorder                     \n"
    "    .set    noat                        \n"
    "    la    $29, (second_cpu_little_stack+128)   \n"
    "    j do_boot_second_cpu   \n"
    "    nop                    \n"
    "    .set    pop                         \n"
    :
    :
    : "memory"
    );
}

static void start_second_cpu(void)
{
	unsigned long value;
	volatile unsigned long *rtos_start = (unsigned long *)rtos_header.img_start;

	value = *rtos_start;

	writel((unsigned long)boot_second_cpu, CCU_IO_BASE+CCU_RER);
	writel(0, CCU_IO_BASE+CCU_CSRR);

	if (rtos_header.version & (1 << 0)) {
		while(*rtos_start == value) {
			mdelay(1);
		}
		writel(1 << 1, CCU_IO_BASE+CCU_CSRR);
	}
}
#endif

static int mmc_rtos_load(struct rtos_header *rtos, unsigned int sector_offset)
{
	int err = 0;
	u32 rtos_size_sectors;

	err = mmc_block_read(sector_offset, 1, rtos);
	if (err == 0)
		goto end;

	if (rtos_check_header(rtos))
		return -1;

	rtos_size_sectors = (rtos->img_end - rtos->img_start + 512 - 1) / 512;

	/* load rtos */
	err = mmc_block_read(sector_offset, rtos_size_sectors, rtos->img_start);

	if (err == 0)
		goto end;

	return 0;
end:
	printf("spl: [rtos] mmc blk read err , %d\n", err);
	return -1;
}

static void mmc_load_rtos_boot(void)
{
	int ret;
	unsigned int rtos_offset = CONFIG_RTOS_OFFSET_SECTOR;

#ifdef CONFIG_SPL_OS_OTA_BOOT
	const char *rtos_name = CONFIG_SPL_RTOS_NAME;

	ret = spl_get_built_in_gpt_partition(CONFIG_SPL_OTA_NAME, &rtos_offset, NULL);
	if (!ret) {
		const char *buf = (const char *)(CONFIG_SYS_TEXT_BASE);
		const char *kernel2 = "ota:"CONFIG_SPL_OS_NAME2;

		mmc_block_read(rtos_offset, 1, (u32 *)buf);
		if (!strncmp(kernel2, buf, strlen(kernel2))) {
			rtos_name = CONFIG_SPL_RTOS_NAME2;
		}
	}

	ret = spl_get_built_in_gpt_partition(rtos_name, &rtos_offset, NULL);
	if (ret) {
		printf("rtos not found: "CONFIG_SPL_RTOS_NAME"\n");
		hang();
	}

	debug("rtos:%s %x\n", rtos_name, rtos_offset);
#else
	ret = spl_get_built_in_gpt_partition(CONFIG_SPL_RTOS_NAME, &rtos_offset, NULL);
	if (ret) {
		printf("rtos not found: "CONFIG_SPL_RTOS_NAME"\n");
		printf("use rtos default offset_addr:%d\n", CONFIG_RTOS_OFFSET_SECTOR);
		rtos_offset = CONFIG_RTOS_OFFSET_SECTOR;
	}
#endif

	if (mmc_rtos_load(&rtos_header, rtos_offset))
		hang();

	flush_cache_all();

#ifdef CONFIG_RTOS_BOOT_ON_SECOND_CPU
	start_second_cpu();
#else
/* NOTE: not return */
	rtos_raw_start(&rtos_header);
#endif
}
#endif

char *spl_mmc_load_image(void)
{
#ifdef CONFIG_JZ_MMC_MSC0
	io_base = MSC0_BASE;
#endif
#ifdef CONFIG_JZ_MMC_MSC1
	io_base = MSC1_BASE;
#endif
#ifdef CONFIG_JZ_MMC_MSC2
	io_base = MSC2_BASE;
#endif

	jzmmc_init();

#ifdef CONFIG_SPL_RTOS_BOOT
	mmc_load_rtos_boot();
#endif

#ifdef CONFIG_SPL_OS_OTA_BOOT
	return mmc_ota_load_img_from_partition(CONFIG_SPL_OS_NAME);
#elif defined(CONFIG_SPL_OS_BOOT)
#ifdef CONFIG_JZSD_OTA_VERSION20
	register_jzsd_ota_ops(&jzsd_ota_ops);
	return spl_jzsd_ota_load_image();
#else
	mmc_load_img_from_partition(CONFIG_SPL_OS_NAME);
#endif
#else
	mmc_load_image_raw(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#endif

	return NULL;
}
