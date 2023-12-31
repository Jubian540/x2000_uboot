#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define GD_DEVICES_NUM          10
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		20
#define	TSHSL_W		20

#define TRD		80
#define TRD_4G		120
#define TPP		700
#define TBE		5

#define TRD_Q5	        50
#define TPP_Q5	        600

static struct jz_sfcnand_device *gd_nand;

static struct jz_sfcnand_base_param gd_param[GD_DEVICES_NUM] = {

	[0] = {
		/*GD5F1GQ4UB*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[1] = {
		 /*GD5F2GQ4UB*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[2] = {
		/*GD5F4GQ4UB*/
		.pagesize = 4 * 1024,
		.blocksize = 4 * 1024 * 64,
		.oobsize = 256,
		.flashsize = 4 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD_4G,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[3] = {
		/*GD5F1GQ4UC*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[4] = {
		/*GD5F2GQ4UC*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[5] = {
		/*GD5F4GQ4UC*/
		.pagesize = 4 * 1024,
		.blocksize = 4 * 1024 * 64,
		.oobsize = 256,
		.flashsize = 4 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD_4G,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},

	[6] = {
		/*GD5F1GQ4RF9IG*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},

	[7] = {
		/*GD5F4GQ6UE*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 4096,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[8] = {
		/*GD5F2GQ5UE*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD_Q5,
		.tPP = TPP_Q5,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[9] = {
		/*GD5F1GQ5UE*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD_Q5,
		.tPP = TPP_Q5,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x4,
		.need_quad = 1,
	},

};

static struct device_id_struct device_id[GD_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0xD1, "GD5F1GQ4UB",&gd_param[0]),
	DEVICE_ID_STRUCT(0xD2, "GD5F2GQ4UB",&gd_param[1]),
	DEVICE_ID_STRUCT(0xD4, "GD5F4GQ4UB",&gd_param[2]),
	DEVICE_ID_STRUCT(0xB1, "GD5F1GQ4UC",&gd_param[3]),
	DEVICE_ID_STRUCT(0xB2, "GD5F2GQ4UC",&gd_param[4]),
	DEVICE_ID_STRUCT(0xB4, "GD5F4GQ4UC",&gd_param[5]),
	DEVICE_ID_STRUCT(0xA1, "GD5F1GQ4RF",&gd_param[6]),
	DEVICE_ID_STRUCT(0x55, "GD5F4GQ6UE",&gd_param[7]),
	DEVICE_ID_STRUCT(0x52, "GD5F2GQ5UE",&gd_param[8]),
	DEVICE_ID_STRUCT(0x51, "GD5F1GQ5UE",&gd_param[9]),
};


static cdt_params_t *gd_get_cdt_params(struct sfc_flash *flash, uint8_t device_id)
{
	CDT_PARAMS_INIT(gd_nand->cdt_params);

	switch(device_id) {
	    case 0xB1 ... 0xB4:
	    case 0xA1:
		    gd_nand->cdt_params.standard_r.addr_nbyte = 3;
		    gd_nand->cdt_params.quad_r.addr_nbyte = 3;
		    break;
	    case 0xD1 ... 0xD4:
	    case 0x51 ... 0x55:
		    break;
	    default:
		    pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
		    return NULL;
	}

	return &gd_nand->cdt_params;
}


static int32_t gd_get_f0_register_value(struct sfc_flash *flash)
{
	struct sfc_cdt_xfer xfer;
	uint32_t buf = 0;

	memset(&xfer, 0, sizeof(xfer));

	/*set index*/
	xfer.cmd_index = NAND_GET_FEATURE;

	/* set addr */
	xfer.staaddr0 = 0xf0;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = 1;
	xfer.config.data_dir = GLB_TRAN_DIR_READ;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = (uint8_t *)&buf;

	if(sfc_sync_cdt(flash->sfc, &xfer)){
	        pr_err("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}
	return buf;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint8_t device_id, uint8_t ecc_status)
{
	int ret = 0;

	switch(device_id) {
		case 0xA1:
		case 0xB1 ... 0xB4:
			switch((ecc_status >> 4) & 0x7) {
				case 0x7:
					ret = -EBADMSG;
					break;
				case 0x6:
					ret = 0x8;
					break;
				case 0x5:
					ret = 0x7;
					break;
				default:
					ret = 0;
			}
			break;
		case 0xD1 ... 0xD4:
			switch((ecc_status >> 4) & 0x3) {
				case 0x3:
					ret = 0x8;
					break;
				case 0x2:
					ret = -EBADMSG;
					break;
				case 0x1:
					if((ret = gd_get_f0_register_value(flash)) < 0)
						return ret;
					if(((ret >> 4) & 0x3) == 0x3)
						ret = 0x7;
					break;
				default:
					ret = 0;
					break;
			}
			break;
		case 0x51:
		case 0x55:
			switch((ecc_status >> 4) & 0x3) {
				case 0x3:
				case 0x0:
					ret = 0x0;
					break;
				case 0x1:
					ret = 0x4;
					break;
				default:
					ret = -EBADMSG;
			}
			break;
		case 0x52:
			switch((ecc_status >> 4) & 0x3) {
				case 0x1:
					if((ret = gd_get_f0_register_value(flash)) < 0)
						return ret;
					switch((ecc_status >> 4) & 0x3) {
						case 0x0:
							ret = 0x1;
							break;
						case 0x1:
							ret = 0x2;
							break;
						case 0x2:
							ret = 0x3;
							break;
						case 0x3:
							ret = 0x4;
							break;
						default:
							break;
					}
					break;
				case 0x2:
					ret = -EBADMSG;
					break;
				default:
					ret = 0x0;
					break;
			}
			break;
		default:
			pr_err("device_id err,it maybe don`t support	\
					this device, please check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;   //notice!!!
	}
	return ret;
}


static int gd_nand_init(void) {

	gd_nand = kzalloc(sizeof(*gd_nand), GFP_KERNEL);
	if(!gd_nand) {
		pr_err("alloc gd_nand struct fail\n");
		return -ENOMEM;
	}

	gd_nand->id_manufactory = 0xC8;
	gd_nand->id_device_list = device_id;
	gd_nand->id_device_count = GD_DEVICES_NUM;

	gd_nand->ops.get_cdt_params = gd_get_cdt_params;
	gd_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	gd_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(gd_nand);
}

SPINAND_MOUDLE_INIT(gd_nand_init);
