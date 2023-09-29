/** @file
  Template for Timer Architecture Protocol driver of the ARM flavor

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Chipset/iomap.h>
#include <Chipset/irqs.h>
#include <Chipset/timer.h>
#include <Library/gpio.h>
#include <Library/DS2746.h>
#include <Library/LKEnvLib.h>

// USB
#define B_SESSION_VALID   (1 << 11)
#define MSM_USB_BASE          0xA0800000
#define USB_OTGSC            (MSM_USB_BASE + 0x01A4)

/* Battery */
#define HTCLEO_GPIO_BATTERY_CHARGER_ENABLE	22
#define HTCLEO_GPIO_BATTERY_CHARGER_CURRENT	16
#define HTCLEO_GPIO_BATTERY_OVER_CHG		147
#define HTCLEO_GPIO_POWER_USB     			109
#define HTCLEO_GPIO_USBPHY_3V3_ENABLE		104
#define DS2746_I2C_SLAVE_ADDR		0x26

EFI_EVENT m_CallbackTimer = NULL;
EFI_EVENT m_ExitBootServicesEvent = NULL;
BOOLEAN usbConnected = FALSE;

enum PSY_CHARGER_STATE {
	CHG_OFF,
	CHG_AC,
	CHG_USB_LOW,
	CHG_USB_HIGH,
	CHG_OFF_FULL_BAT,
};


    //toDo
    //get usb status, done read the value and check what to do with it
    //be able to get battery voltage
    //enable charging maybe use keypad leds as a charging indicator since we cant use i2c

static void EFIAPI SetCharger(enum PSY_CHARGER_STATE state)
{
	enum PSY_CHARGER_STATE chgr_state = state;
	switch (state) {
		case CHG_USB_LOW:
			gpio_set(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, 0);
			gpio_config(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, GPIO_OUTPUT);
			gpio_set(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, 0);
			gpio_config(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, GPIO_OUTPUT);
			break;
		case CHG_AC: case CHG_USB_HIGH:
			gpio_set(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, 1);
			gpio_config(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, GPIO_OUTPUT);
			gpio_set(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, 0);
			gpio_config(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, GPIO_OUTPUT);
			break;
		case CHG_OFF_FULL_BAT: // zzz
		case CHG_OFF:
		default:	
			// 0 enable; 1 disable;
			gpio_set(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, 1);
			gpio_config(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, GPIO_OUTPUT);
			gpio_set(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, 1);
			gpio_config(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, GPIO_OUTPUT);
			/*gpio_set(HTCLEO_GPIO_POWER_USB, 0);
			gpio_config(HTCLEO_GPIO_POWER_USB, GPIO_OUTPUT);*/
			break;
	}
}

BOOLEAN EFIAPI CheckUsbStatus(
    IN EFI_EVENT Event, 
    IN VOID *Context
)
{
  // DEBUG((EFI_D_ERROR, "Check Usb status\n"));
	// /*Verify B Session Valid Bit to verify vbus status */
  
	// if (B_SESSION_VALID & readl(USB_OTGSC)) {
	// 	usbConnected = true;
  //   DEBUG((EFI_D_ERROR, "Usb is connected\n"));
	// } else {
	// 	usbConnected = false;
  //   DEBUG((EFI_D_ERROR, "Usb is removed\n"));
	// }

//UINT32 value = MmioRead32(MSM_SHARED_BASE + 0xef20c);
// !!readl(MSM_SHARED_BASE + 0xef20c);
BOOLEAN usbStatus = TRUE;
return usbStatus;


}

VOID EFIAPI GetBatteryVoltage(
    IN EFI_EVENT Event, 
    IN VOID *Context
)
{
  
}

VOID EFIAPI ChangeChargingStatus(
    IN EFI_EVENT Event, 
    IN VOID *Context,
    BOOLEAN shouldCharge
)
{
  if (shouldCharge){

  }else {

  }
  
}


VOID EFIAPI WantsCharging(
    IN EFI_EVENT Event, 
    IN VOID *Context)
{
  BOOLEAN usbStatus = CheckUsbStatus(NULL,NULL);
 
  //get battery volate here and calc to percent


   UINT32 voltage = ds2746_voltage(DS2746_I2C_SLAVE_ADDR);
  DEBUG((EFI_D_ERROR, "Battery Volatage is: %d\n", voltage));
  if (usbStatus){ //todo add battery percentage check somelike battery < 80 % && usbStatus should ensure we wont overcharge
  
  }

  
}

EFI_STATUS EFIAPI ChargingDxeInit(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {


    EFI_STATUS Status;
  //install a timer to check for want charging every like 5 seconds
    Status = gBS->CreateEvent(
        EVT_NOTIFY_SIGNAL | EVT_TIMER,
        TPL_CALLBACK, WantsCharging, NULL,
        &m_CallbackTimer
    );

    ASSERT_EFI_ERROR(Status);

    Status = gBS->SetTimer(
        m_CallbackTimer, TimerPeriodic,
        EFI_TIMER_PERIOD_MILLISECONDS(1)
    );

    ASSERT_EFI_ERROR(Status);
    return Status;
  }