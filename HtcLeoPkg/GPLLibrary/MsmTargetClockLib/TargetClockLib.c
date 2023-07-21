#include <Uefi.h>
#include <Chipset/clock.h>

RETURN_STATUS
EFIAPI
MsmTargetClockLibInitialize(VOID)
{
  EFI_STATUS         Status = EFI_SUCCESS;

  msm_clock_init();

  return Status;
}