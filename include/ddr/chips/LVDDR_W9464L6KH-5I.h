#ifndef __MDDR_CONFIG_H
#define __MDDR_CONFIG_H


/* MDDR paramters */
#define DDR_ROW 12 /* ROW : 12 to 14 row address */
#define DDR_ROW1 12 /* ROW : 12 to 14 row address */
#define DDR_COL 8/* COL :  8 to 10 column address */
#define DDR_COL1 8/* COL :  8 to 10 column address */
#define DDR_BANK8 0 /* Banks each chip: 0-4bank, 1-8bank 0 for falcon fpga, 1 for develop board */
#define DDR_CL 3 /* CAS latency: 1 to 7 */


/*
 *  * MDDR controller timing1 register
 *   */
#define DDR_tRAS DDR__ns(40) /*t2 tRAS: ACTIVE to PRECHARGE command period to the same bank. */
#define DDR_tRP  DDR__ns(15) /*t3 tRP: PRECHARGE command period to the same bank */
#define DDR_tRCD DDR__ns(15) /*t2 ACTIVE to READ or WRITE command period to the same bank. */
#define DDR_tRC  (DDR_tRAS + DDR_tRP) /*t3 ACTIVE to ACTIVE command period to the same bank.*/
#define DDR_tRRD DDR__ns(10)   /*t3 ACTIVE bank A to ACTIVE bank B command period. */
#define DDR_tWR  DDR__ns(15) /*t1 WRITE Recovery Time defined by register MR of DDR2 memory */
#define DDR_tWTR DDR__tck(2) /*t1 WRITE to READ command delay. */

/*
 *  * MDDR controller timing2 register
 *   */
#define DDR_tRFC DDR__ns(70) /*t4 ns,  AUTO-REFRESH command period. */

/*#define DDR_tXP  DDR__tck(2) t4 EXIT-POWER-DOWN to next valid command period; note:get from JEDEC LPDDR*/
#define DDR_tXP  DDR__ns(25) /*t4 EXIT-POWER-DOWN to next valid command period; note:get from JEDEC LPDDR*/
/*#define DDR_tMRD DDR__ns(10) t4 unit: tCK Load-Mode-Register to next valid command period: 1 to 4 tCK */
#define DDR_tMRD DDR__tck(2) /*t4 unit: tCK Load-Mode-Register to next valid command period: 1 to 4 tCK */

/* new add */
#define DDR_BL  4   /* MDDR Burst length: 3 - 8 burst, 2 - 4 burst , 1 - 2 burst*/
#define DDR_RL  DDR__tck(DDR_CL)    /* MDDR: Read Latency = tAL + tCL */
#define DDR_WL  DDR__tck(1)     /* MDDR: must 1 */

#define DDR_tCKE DDR__tck(2)        /*t4 CKE minimum pulse width, tCK */
#define DDR_tXSR DDR__tck(200)   /*t6?*/
#define DDR_tDQSSMAX DDR__tck(1)

/*
 *  * MDDR controller refcnt register
 *   */
#define DDR_tREFI           DDR__ns(3900)   /* Refresh period: 4096 refresh cycles/64ms */


#endif /* __MDDR_CONFIG_H */


