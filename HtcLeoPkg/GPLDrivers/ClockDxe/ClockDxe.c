/*
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2007 QUALCOMM Incorporated
 * Copyright (c) 2010 Cotulla
 * Copyright (c) 2012 Shantanu Gupta <shans95g@gmail.com>
 * Copyright (c) 2023 Dominik Kobinski <dominikkobinski314@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/pcom.h>
#include <Library/acpuclock.h>
#include <Library/reg.h>

#include <Chipset/iomap.h>
#include <Chipset/clock.h>

#include <Protocol/EmbeddedClock.h>

#define TCX0               19200000
#define GLBL_CLK_ENA_2     ((UINT32)MSM_CLK_CTL_BASE + 0x220)
#define PLLn_BASE(n)       (MSM_CLK_CTL_BASE + 0x300 + 32 * (n))
#define PLL_FREQ(l, m, n)  (TCX0 * (l) + TCX0 * (m) / (n))

#define MSM_CLOCK_REG(frequency,M,N,D,PRE,a5,SRC,MNE,pll_frequency) { \
	.freq = (frequency), \
	.md = ((0xffff & (M)) << 16) | (0xffff & ~((D) << 1)), \
	.ns = ((0xffff & ~((N) - (M))) << 16) \
	    | ((0xff & (0xa | (MNE))) << 8) \
	    | ((0x7 & (a5)) << 5) \
	    | ((0x3 & (PRE)) << 3) \
	    | (0x7 & (SRC)), \
	.pll_freq = (pll_frequency), \
	.calc_freq = 1000*((pll_frequency/1000)*M/((PRE+1)*N)), \
}

static UINTN ClocksLookup[NR_CLKS] = {-1};

struct mdns_clock_params
{
	unsigned freq;
	UINT32 calc_freq;
	UINT32 md;
	UINT32 ns;
	UINT32 pll_freq;
	UINT32 clk_id;
};

struct msm_clock_params
{
	unsigned clk_id;
	UINT32   glbl;
	unsigned idx;
	unsigned offset;
	unsigned ns_only;
	char	 *name;
};

VOID
FillClocksLookup()
{
  // Fill used clocks
  ClocksLookup[ICODEC_RX_CLK] = 50;
  ClocksLookup[ICODEC_TX_CLK] = 52;
  ClocksLookup[ECODEC_CLK] = 42;
  ClocksLookup[SDAC_MCLK] = 64;
  ClocksLookup[IMEM_CLK] = 55;
  ClocksLookup[GRP_CLK] = 56;
  ClocksLookup[ADM_CLK] = 19;
  ClocksLookup[UART1DM_CLK] = 78;
  ClocksLookup[UART2DM_CLK] = 80;
  ClocksLookup[VFE_AXI_CLK] = 24;
  ClocksLookup[VFE_MDC_CLK] = 40;
  ClocksLookup[VFE_CLK] = 41;
  ClocksLookup[MDC_CLK] = 53;
  ClocksLookup[SPI_CLK] = 95;
  ClocksLookup[MDP_CLK] = 9;
  ClocksLookup[SDC1_CLK] = 66;
  ClocksLookup[SDC2_CLK] = 67;
  ClocksLookup[SDC1_PCLK] = 17;
  ClocksLookup[SDC2_PCLK] = 16;
}

// Cotullaz "new" clock functions
UINTN
CotullaClkSetRate(UINT32 Id, UINTN Rate)
{
	// TODO?
    UINTN Speed = 0;
    switch (Id) {
		case ICODEC_RX_CLK:
			if (Rate > 11289600)     Speed = 9;
			else if (Rate > 8192000) Speed = 8;
			else if (Rate > 6144000) Speed = 7;
			else if (Rate > 5644800) Speed = 6;
			else if (Rate > 4096000) Speed = 5;
			else if (Rate > 3072000) Speed = 4;
			else if (Rate > 2822400) Speed = 3;
			else if (Rate > 2048000) Speed = 2;
			else Speed = 1;
			break;
		case ICODEC_TX_CLK:
			if (Rate > 11289600) 	 Speed = 9;
			else if (Rate > 8192000) Speed = 8;
			else if (Rate > 6144000) Speed = 7;
			else if (Rate > 5644800) Speed = 6;
			else if (Rate > 4096000) Speed = 5;
			else if (Rate > 3072000) Speed = 4;
			else if (Rate > 2822400) Speed = 3;
			else if (Rate > 2048000) Speed = 2;
			else Speed = 1;
			break;
		case SDAC_MCLK:
			if (Rate > 1411200) 	Speed = 9;
			else if (Rate > 1024000)Speed = 8;
			else if (Rate > 768000) Speed = 7;
			else if (Rate > 705600) Speed = 6;
			else if (Rate > 512000) Speed = 5;
			else if (Rate > 384000) Speed = 4;
			else if (Rate > 352800) Speed = 3;
			else if (Rate > 256000) Speed = 2;
			else Speed = 1;
			break;
		case UART1DM_CLK:
			if (Rate > 61440000) 	  Speed = 15;
			else if (Rate > 58982400) Speed = 14;
			else if (Rate > 56000000) Speed = 13;
			else if (Rate > 51200000) Speed = 12;
			else if (Rate > 48000000) Speed = 11;
			else if (Rate > 40000000) Speed = 10;
			else if (Rate > 32000000) Speed = 9;
			else if (Rate > 24000000) Speed = 8;
			else if (Rate > 16000000) Speed = 7;
			else if (Rate > 15360000) Speed = 6;
			else if (Rate > 14745600) Speed = 5;
			else if (Rate >  7680000) Speed = 4;
			else if (Rate >  7372800) Speed = 3;
			else if (Rate >  3840000) Speed = 2;
			else Speed = 1;
			break;
		case UART2DM_CLK:
			if (Rate > 61440000) 	  Speed = 15;
			else if (Rate > 58982400) Speed = 14;
			else if (Rate > 56000000) Speed = 13;
			else if (Rate > 51200000) Speed = 12;
			else if (Rate > 48000000) Speed = 11;
			else if (Rate > 40000000) Speed = 10;
			else if (Rate > 32000000) Speed = 9;
			else if (Rate > 24000000) Speed = 8;
			else if (Rate > 16000000) Speed = 7;
			else if (Rate > 15360000) Speed = 6;
			else if (Rate > 14745600) Speed = 5;
			else if (Rate >  7680000) Speed = 4;
			else if (Rate >  7372800) Speed = 3;
			else if (Rate >  3840000) Speed = 2;
			else Speed = 1;
			break;
		case ECODEC_CLK:
			if (Rate > 2048000) 	Speed = 3;
			else if (Rate > 128000) Speed = 2;
			else Speed = 1;
			break;
		case VFE_MDC_CLK:
			if (Rate == 96000000) 		Speed = 37;
			else if (Rate == 48000000)	Speed = 32;
			else if (Rate == 24000000) 	Speed = 22;
			else if (Rate == 12000000) 	Speed = 14;
			else if (Rate ==  6000000) 	Speed = 6;
			else if (Rate ==  3000000) 	Speed = 1;
			else return 0;
			break;
		case VFE_CLK:
			if (Rate == 36000000) 		Speed = 1;
			else if (Rate == 48000000) 	Speed = 2;
			else if (Rate == 64000000) 	Speed = 3;
			else if (Rate == 78000000) 	Speed = 4;
			else if (Rate == 96000000) 	Speed = 5;
			else return 0;
			break;
		case SPI_CLK:
			if (Rate > 15360000) 		Speed = 5;
			else if (Rate > 9600000) 	Speed = 4;
			else if (Rate > 4800000) 	Speed = 3;
			else if (Rate >  960000) 	Speed = 2;
			else Speed = 1;
			break;
		case SDC1_CLK:
			if (Rate > 50000000) 		Speed = 14;
			else if (Rate > 49152000) 	Speed = 13;
			else if (Rate > 45000000) 	Speed = 12;
			else if (Rate > 40000000) 	Speed = 11;
			else if (Rate > 35000000) 	Speed = 10;
			else if (Rate > 30000000) 	Speed = 9;
			else if (Rate > 25000000) 	Speed = 8;
			else if (Rate > 20000000) 	Speed = 7;
			else if (Rate > 15000000) 	Speed = 6;
			else if (Rate > 10000000) 	Speed = 5;
			else if (Rate > 5000000)  	Speed = 4;
			else if (Rate > 400000)		Speed = 3;
			else if (Rate > 144000)		Speed = 2;
			else Speed = 1;
			break;
		case SDC2_CLK:
			if (Rate > 50000000) 		Speed = 14;
			else if (Rate > 49152000) 	Speed = 13;
			else if (Rate > 45000000) 	Speed = 12;
			else if (Rate > 40000000) 	Speed = 11;
			else if (Rate > 35000000) 	Speed = 10;
			else if (Rate > 30000000) 	Speed = 9;
			else if (Rate > 25000000) 	Speed = 8;
			else if (Rate > 20000000) 	Speed = 7;
			else if (Rate > 15000000) 	Speed = 6;
			else if (Rate > 10000000) 	Speed = 5;
			else if (Rate > 5000000)  	Speed = 4;
			else if (Rate > 400000)		Speed = 3;
			else if (Rate > 144000)		Speed = 2;
			else Speed = 1;
			break;
		case SDC1_PCLK:
		case SDC2_PCLK:
			return 0;
			break;
		default:
			return -1;  
    }
    msm_proc_comm(PCOM_CLK_REGIME_SEC_SEL_SPEED, &ClocksLookup[Id], &Speed);
	
    return 0;
}

static struct msm_clock_params msm_clock_parameters[] = {
	/* clk_id 					 glbl 					 idx 		 offset 			 ns_only 			 name  			*/
	{ .clk_id = SDC1_CLK,		.glbl = GLBL_CLK_ENA, 	.idx =  7,	.offset = 0x0a4,						.name="SDC1_CLK"	},
	{ .clk_id = SDC2_CLK,		.glbl = GLBL_CLK_ENA, 	.idx =  8, 	.offset = 0x0ac, 						.name="SDC2_CLK"	},
	{ .clk_id = SDC3_CLK,		.glbl = GLBL_CLK_ENA, 	.idx = 27, 	.offset = 0x3d8,						.name="SDC3_CLK"	},
	{ .clk_id = SDC4_CLK,		.glbl = GLBL_CLK_ENA, 	.idx = 28, 	.offset = 0x3e0,						.name="SDC4_CLK"	},
	{ .clk_id = UART1DM_CLK,	.glbl = GLBL_CLK_ENA, 	.idx = 17, 	.offset = 0x124,						.name="UART1DM_CLK"	},
	{ .clk_id = UART2DM_CLK,	.glbl = GLBL_CLK_ENA, 	.idx = 26, 	.offset = 0x12c,						.name="UART2DM_CLK"	},
	{ .clk_id = USB_HS_CLK,		.glbl = GLBL_CLK_ENA_2,	.idx =  7, 	.offset = 0x3e8,	.ns_only = 0xb41, 	.name="USB_HS_CLK"	},
	{ .clk_id = GRP_CLK,		.glbl = GLBL_CLK_ENA, 	.idx =  3, 	.offset = 0x084, 	.ns_only = 0xa80, 	.name="GRP_CLK"		}, 
	{ .clk_id = IMEM_CLK,		.glbl = GLBL_CLK_ENA, 	.idx =  3, 	.offset = 0x084, 	.ns_only = 0xa80, 	.name="IMEM_CLK"	},
	{ .clk_id = VFE_CLK,		.glbl = GLBL_CLK_ENA, 	.idx =  2, 	.offset = 0x040, 						.name="VFE_CLK"		},
	{ .clk_id = MDP_CLK,		.glbl = GLBL_CLK_ENA, 	.idx =  9, 											.name="MDP_CLK"		},
	{ .clk_id = GP_CLK,			.glbl = GLBL_CLK_ENA, 				.offset = 0x058, 	.ns_only = 0x800, 	.name="GP_CLK"		},
	{ .clk_id = PMDH_CLK,		.glbl = GLBL_CLK_ENA_2, .idx =  4, 	.offset = 0x08c, 	.ns_only = 0x00c, 	.name="PMDH_CLK"	},
	{ .clk_id = I2C_CLK,											.offset = 0x064, 	.ns_only = 0xa00, 	.name="I2C_CLK"		},
	{ .clk_id = SPI_CLK,		.glbl = GLBL_CLK_ENA_2, .idx = 13, 	.offset = 0x14c, 	.ns_only = 0xa08, 	.name="SPI_CLK"		}
};

struct mdns_clock_params msm_clock_freq_parameters[] = {
	MSM_CLOCK_REG(  144000,   3, 0x64, 0x32, 3, 3, 0, 1, 19200000),  /* SD, 144kHz */
	MSM_CLOCK_REG(  400000,   1, 0x30, 0x15, 0, 3, 0, 1, 19200000),  /* SD, 400kHz */
	MSM_CLOCK_REG( 7372800,   3, 0x64, 0x32, 0, 2, 4, 1, 245760000), /* 460800*16, will be divided by 4 for 115200 */
	MSM_CLOCK_REG(12000000,   1, 0x20, 0x10, 1, 3, 1, 1, 768000000), /* SD, 12MHz */
	MSM_CLOCK_REG(14745600,   3, 0x32, 0x19, 0, 2, 4, 1, 245760000), /* BT, 921600 (*16)*/
	MSM_CLOCK_REG(19200000,   1, 0x0a, 0x05, 3, 3, 1, 1, 768000000), /* SD, 19.2MHz */
	MSM_CLOCK_REG(24000000,   1, 0x10, 0x08, 1, 3, 1, 1, 768000000), /* SD, 24MHz */
	MSM_CLOCK_REG(24576000,   1, 0x0a, 0x05, 0, 2, 4, 1, 245760000), /* SD, 24,576000MHz */
	MSM_CLOCK_REG(25000000,  14, 0xd7, 0x6b, 1, 3, 1, 1, 768000000), /* SD, 25MHz */
	MSM_CLOCK_REG(32000000,   1, 0x0c, 0x06, 1, 3, 1, 1, 768000000), /* SD, 32MHz */
	MSM_CLOCK_REG(48000000,   1, 0x08, 0x04, 1, 3, 1, 1, 768000000), /* SD, 48MHz */
	MSM_CLOCK_REG(50000000,  25, 0xc0, 0x60, 1, 3, 1, 1, 768000000), /* SD, 50MHz */
	MSM_CLOCK_REG(58982400,   6, 0x19, 0x0c, 0, 2, 4, 1, 245760000), /* BT, 3686400 (*16) */
	MSM_CLOCK_REG(64000000,0x19, 0x60, 0x30, 0, 2, 4, 1, 245760000), /* BT, 4000000 (*16) */
};

static inline struct msm_clock_params msm_clk_get_params(UINT32 id)
{
	struct msm_clock_params empty = { .clk_id = 0xdeadbeef, .glbl = 0, .idx = 0, .offset = 0, .ns_only = 0, .name="deafbeef"};
	for (UINT32 i = 0; i < ARRAY_SIZE(msm_clock_parameters); i++) {
		if (id == msm_clock_parameters[i].clk_id) {
			return msm_clock_parameters[i];
		}
	}
	
	return empty;
}

static inline unsigned msm_clk_reg_offset(UINT32 id)
{
	struct msm_clock_params params;
	params = msm_clk_get_params(id);
	
	return params.offset;
}

static unsigned long get_mdns_host_clock(UINT32 id)
{
	UINT32 n;
	UINT32 offset;
	UINT32 mdreg;
	UINT32 nsreg;
	unsigned long freq = 0;

	offset = msm_clk_reg_offset(id);
	if (offset == 0)
		return -1;

	mdreg = MmioRead32(MSM_CLK_CTL_BASE + offset - 4);
	nsreg = MmioRead32(MSM_CLK_CTL_BASE + offset);

	for (n = 0; n < ARRAY_SIZE(msm_clock_freq_parameters); n++) {
		if (msm_clock_freq_parameters[n].md == mdreg &&
			msm_clock_freq_parameters[n].ns == nsreg) {
			freq = msm_clock_freq_parameters[n].freq;
			break;
		}
	}

	return freq;
}

/* note: used in lcdc */
EFI_STATUS
ClkSetRate(UINTN Id, UINTN Freq)
{
	UINT32 Retval = 0;
  	Retval = CotullaClkSetRate(Id, Freq);

	return Retval;
}

UINTN
ClkGetRate(UINT32 Id)
{
	UINTN Rate = 0;

	if (ClocksLookup[Id] != -1) {
		// Cotullas function
		msm_proc_comm(PCOM_CLK_REGIME_SEC_MSM_GET_CLK_FREQ_KHZ, &ClocksLookup[Id], &Rate);
	}
	if (Rate == 0)
	{
		switch(Id) {
			case SDC1_CLK:
			case SDC2_CLK:
			case SDC3_CLK:
			case SDC4_CLK:
			case UART1DM_CLK:
			case UART2DM_CLK:
			case USB_HS_CLK:
			case SDAC_CLK:
			case TV_DAC_CLK:
			case TV_ENC_CLK:
			case USB_OTG_CLK:
				Rate = get_mdns_host_clock(Id);
				break;
			case SDC1_PCLK:
			case SDC2_PCLK:
			case SDC3_PCLK:
			case SDC4_PCLK:
				Rate = 64000000;
				break;
			default:
				Rate = 0;
		}
	}
	return Rate;
}

UINTN
ClkEnable(UINTN Id)
{
	if (ClocksLookup[Id] != -1) {
    	msm_proc_comm(PCOM_CLK_REGIME_SEC_ENABLE, &ClocksLookup[Id], 0);
    	return ClocksLookup[Id];
  	}
	return -1;
}

VOID
ClkDisable(UINTN Id)
{
	if (ClocksLookup[Id] != -1) {
		msm_proc_comm(PCOM_CLK_REGIME_SEC_DISABLE, &ClocksLookup[Id], 0);
	}
}

int msm_pll_request(unsigned id, unsigned on)
{
	on = !!on;
	return msm_proc_comm(PCOM_CLKCTL_RPC_PLL_REQUEST, &id, &on);
}

/**
 Protocol variable definition
 **/
EMBEDDED_CLOCK_PROTOCOL  gClock = {
  ClkEnable,
  ClkDisable,
  ClkSetRate,
};

EFI_STATUS
EFIAPI
ClockDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS  Status = EFI_SUCCESS;
  	EFI_HANDLE       Handle;
	
	//
	// Make sure the clock protocol has not been installed in the system yet.
	//
	ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEmbeddedClockProtocolGuid);

	FillClocksLookup();

	// Setup Scorpion PLL
	msm_acpu_clock_init(11+6);

	// Install the Embedded Clock Protocol onto a new handle
	Handle = NULL;
	Status = gBS->InstallMultipleProtocolInterfaces (
					&Handle,
					&gEmbeddedClockProtocolGuid,
					&gClock,
					NULL
					);

	if (EFI_ERROR (Status)) {
		Status = EFI_OUT_OF_RESOURCES;
	}

	return EFI_SUCCESS;
}