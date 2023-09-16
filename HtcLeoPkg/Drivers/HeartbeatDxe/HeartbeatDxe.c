/* Heartbeat: periodically flash the keypad LED to indicate firmware status */

#include <PiDxe.h>
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/gpio.h>

#define HTCLEO_GPIO_KP_LED	 			48

EFI_EVENT m_CallbackTimer = NULL;
EFI_EVENT m_ExitBootServicesEvent = NULL;

VOID EFIAPI LedHeartbeatCallback(
    IN EFI_EVENT Event, 
    IN VOID *Context
)
{
    // Turn on button backlight
    gpio_set(HTCLEO_GPIO_KP_LED, 1);
	MicroSecondDelay(500);
	gpio_set(HTCLEO_GPIO_KP_LED, 0);
}

EFI_STATUS
EFIAPI
HeartbeatDxeInitialize(
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS Status;

    Status = gBS->CreateEvent(
        EVT_NOTIFY_SIGNAL | EVT_TIMER,
        TPL_CALLBACK, LedHeartbeatCallback, NULL,
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