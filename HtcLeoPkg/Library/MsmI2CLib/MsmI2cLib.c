#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Chipset/gpio.h>
#include <Library/InterruptsLib.h>
#include <Chipset/msm_i2c.h>
#include <Chipset/clock.h>
#include <Chipset/irqs.h>
#include <Chipset/iomap.h>
#include <Library/pcom.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Device/Gpio.h>

#define MEMORY_REGION_ADDRESS 0xA9900000
#define MEMORY_REGION_LENGTH 0x00100000


void EFIAPI msm_set_i2c_mux(int mux_to_i2c) {
	if (mux_to_i2c) {
		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_CLK, 0, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_8MA), 0);
		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_DAT, 0, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_8MA), 0);
	} else {
		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_CLK, 1, MSM_GPIO_CFG_INPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_2MA), 0);
		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_DAT, 1, MSM_GPIO_CFG_INPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_2MA), 0);
	}
}

static struct msm_i2c_pdata i2c_pdata = {
	.i2c_clock = 400000,
	.clk_nr	= I2C_CLK,
	.irq_nr = INT_PWB_I2C,
	.scl_gpio = GPIO_I2C_CLK,
	.sda_gpio = GPIO_I2C_DAT,
	.set_mux_to_i2c = &msm_set_i2c_mux,
	.i2c_base = (void*)MSM_I2C_BASE,
};

RETURN_STATUS
EFIAPI
MsmI2cInitialize(VOID)
{
	msm_i2c_probe(&i2c_pdata);

  return EFI_SUCCESS;
}