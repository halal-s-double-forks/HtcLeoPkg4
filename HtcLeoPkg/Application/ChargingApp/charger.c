/** @file
 * Charging App
 *
 * Copyright (c) 2023, J0SH1X <aljoshua.hell@gmail.com>
 * Copyright (C) 2011-2012 Shantanu Gupta <shans95g@gmail.com>
 * Copyright (C) 2010-2011 Alexander Tarasikov <alexander.tarasikov@gmail.com>
 * Copyright (C) 2007-2011 htc-linux.org and XDANDROID project
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

**/
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>

#include <Chipset/iomap.h>
#include <Chipset/irqs.h>
#include <Chipset/timer.h>
#include <Library/gpio.h>
#include <Library/DS2746.h>

#include <Device/Gpio.h>

#include <Protocol/GpioTlmm.h>

// USB
#define B_SESSION_VALID   (1 << 11)
#define USB_OTGSC         (MSM_USB_BASE + 0x01A4)
#define USB_STATUS        0xef20c

EFI_EVENT m_CallbackTimer = NULL;
EFI_EVENT EfiExitBootServicesEvent      = (EFI_EVENT)NULL;

// Cached copy of the Hardware Gpio protocol instance
TLMM_GPIO *gGpio = NULL;

enum PSY_CHARGER_STATE {
	CHG_OFF,
	CHG_AC,
	CHG_USB_LOW,
	CHG_USB_HIGH,
	CHG_OFF_FULL_BAT,
};

//toDo
//be able to get battery voltage
//enable charging, use LED as indicator
static void EFIAPI SetCharger(enum PSY_CHARGER_STATE state)
{
	switch (state) {
		case CHG_USB_LOW:
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, 0);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, GPIO_OUTPUT);
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, 0);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, GPIO_OUTPUT);
			break;
		case CHG_AC: case CHG_USB_HIGH:
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, 1);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, GPIO_OUTPUT);
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, 0);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, GPIO_OUTPUT);
			break;
		case CHG_OFF_FULL_BAT: // zzz
		case CHG_OFF:
		default:	
			// 0 enable; 1 disable;
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, 1);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, GPIO_OUTPUT);
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, 1);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, GPIO_OUTPUT);
			/*gGpio->Set(HTCLEO_GPIO_POWER_USB, 0);
			gGpio->Config(HTCLEO_GPIO_POWER_USB, GPIO_OUTPUT);*/
			break;
	}
}

BOOLEAN  
CheckUsbStatus()
{
  return !!MmioRead32(MSM_SHARED_BASE + USB_STATUS);
}

/*
 * koko: ..._chg_voltage_threshold is used along ds2746 driver
 *		 to detect when the battery is FULL.
 *		 Consider setting it to a value lower than DS2746_HIGH_VOLTAGE.
 *		 Avoiding full charge has benefits, and some manufacturers set
 *		 the charge threshold lower on purpose to prolong battery life.
 * ######################################################################
 * #			Typical charge characteristics of lithium-ion			#
 * ######################################################################
 * # Charge V/cell #   Capacity at	 # Charge time # Capacity with full #
 * #			   # cut-off voltage #			   #	 saturation     #
 * ######################################################################
 * #	 3.80	   #		60%		 #	 120 min   #		65%			#
 * #	 3.90	   #		70%		 #	 135 min   #		76%			#
 * #	 4.00	   #		75%		 #	 150 min   #		82%			#
 * #	 4.10	   #		80%		 #	 165 min   #		87%			#
 * #	 4.20	   #		85%		 #	 180 min   #	   100%			#
 * ######################################################################
 */
static UINT32 default_chg_voltage_threshold[] =
{
	3700,
	3800,
	3900,
	4000,
	4100,
	4200,//DS2746_HIGH_VOLTAGE
};

VOID EFIAPI WantsCharging(
    IN EFI_EVENT Event, 
    IN VOID *Context)
{
  BOOLEAN usbStatus = CheckUsbStatus();
  UINT32 voltage = 0;
  //get battery volate here and calc to percent
  
  if (usbStatus){ //todo add battery percentage check somelike battery < 80 % && usbStatus should ensure we wont overcharge
    voltage = ds2746_voltage(DS2746_I2C_SLAVE_ADDR);
    DEBUG((EFI_D_ERROR, "Battery Volatage is: %d\n", voltage));

    if(voltage < default_chg_voltage_threshold[1]) {
      DEBUG((EFI_D_ERROR, "Voltage < default_threshold"));
      /*
      // If battery needs charging, set new charger state
      if (htcleo_ac_online()) {
        if (htcleo_charger_state() != CHG_AC ) {
          writel(0x00080000, USB_USBCMD);
          ulpi_write(0x48, 0x04);
          htcleo_set_charger(CHG_AC);
        }
      } else {
        if (htcleo_charger_state() != CHG_USB_LOW ) {
          writel(0x00080001, USB_USBCMD);
          mdelay(10);
          htcleo_set_charger(CHG_USB_LOW);
        }
      }
      */
    }
    else {
      // Battery is full
      DEBUG((EFI_D_ERROR, "Voltage >= default_threshold"));
      // Set charger state to CHG_OFF_FULL_BAT
      /*if (htcleo_charger_state() != CHG_OFF_FULL_BAT ) {
        writel(0x00080001, USB_USBCMD);
        mdelay(10);
        htcleo_set_charger(CHG_OFF_FULL_BAT);
      }*/
      // and turn off led
    }
  }
  else {
    DEBUG((EFI_D_ERROR, "USB not connected!"));
    // Set charger state to CHG_OFF
    /*if (htcleo_charger_state() != CHG_OFF ) {
      writel(0x00080001, USB_USBCMD);
      mdelay(10);
      SetCharger(CHG_OFF);
    }*/
    // and turn off led
  }
}

VOID
EFIAPI
ExitBootServicesEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  // Make sure charging is disabled
  SetCharger(CHG_OFF);
}

EFI_STATUS 
EFIAPI 
ChargingDxeInit(
  IN EFI_HANDLE ImageHandle, 
  IN EFI_SYSTEM_TABLE *SystemTable) 
{
  EFI_STATUS Status = EFI_SUCCESS;

  DEBUG((EFI_D_ERROR, "ChargingApp: Init()"));

  // Find the gpio controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gTlmmGpioProtocolGuid, NULL, (VOID **)&gGpio);
  ASSERT_EFI_ERROR (Status);

  do {
    WantsCharging(NULL, NULL);
  }while(1);

  // Install a timer to check for want charging every 3 seconds
  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL | EVT_TIMER,
      TPL_CALLBACK, WantsCharging, NULL,
      &m_CallbackTimer
  );
  ASSERT_EFI_ERROR(Status);

  Status = gBS->SetTimer(
      m_CallbackTimer, TimerPeriodic,
      EFI_TIMER_PERIOD_MILLISECONDS(3000)
  );
  ASSERT_EFI_ERROR(Status);

  // Register for an ExitBootServicesEvent
  Status = gBS->CreateEvent(EVT_SIGNAL_EXIT_BOOT_SERVICES, TPL_NOTIFY, ExitBootServicesEvent, NULL, &EfiExitBootServicesEvent);
  ASSERT_EFI_ERROR(Status);

  return Status;
}