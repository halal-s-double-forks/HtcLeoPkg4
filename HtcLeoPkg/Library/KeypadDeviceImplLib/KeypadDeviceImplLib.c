#include <PiDxe.h>
#include <Uefi.h>

#include <Library/LKEnvLib.h>

#include <Library/KeypadDeviceHelperLib.h>
#include <Library/KeypadDeviceImplLib.h>
#include <Library/UefiLib.h>
#include <Protocol/KeypadDevice.h>

#define HTCLEO_GPIO_KP_LED 48

typedef enum {
  KEY_DEVICE_TYPE_UNKNOWN,
  KEY_DEVICE_TYPE_LEGACY,
  KEY_DEVICE_TYPE_KEYMATRIX,
  KEY_DEVICE_TYPE_TLMM,
  KEY_DEVICE_TYPE_PM8X41,
  KEY_DEVICE_TYPE_PM8X41_PON,
} KEY_DEVICE_TYPE;

typedef struct {
  KEY_CONTEXT     EfiKeyContext;
  BOOLEAN         IsValid;
  KEY_DEVICE_TYPE DeviceType;

  // gpio
  UINT8   Gpio;
  BOOLEAN ActiveLow;

  // gpio keymatrix

  UINT8 GpioIn;
  UINT8 GpioOut;

  // pon
  BOOLEAN IsVolumeKey;
} KEY_CONTEXT_PRIVATE;

STATIC KEY_CONTEXT_PRIVATE KeyContextPower;
STATIC KEY_CONTEXT_PRIVATE KeyContextVolumeUp;
STATIC KEY_CONTEXT_PRIVATE KeyContextVolumeDown;
STATIC KEY_CONTEXT_PRIVATE KeyContextBack;
STATIC KEY_CONTEXT_PRIVATE KeyContextWindows;
STATIC KEY_CONTEXT_PRIVATE KeyContextHome;
STATIC KEY_CONTEXT_PRIVATE KeyContextDial;

STATIC KEY_CONTEXT_PRIVATE *KeyList[] = {
    &KeyContextPower, &KeyContextVolumeUp, &KeyContextVolumeDown,
    &KeyContextBack,  &KeyContextWindows,  &KeyContextHome,
    &KeyContextDial};

STATIC
VOID KeypadInitializeKeyContextPrivate(KEY_CONTEXT_PRIVATE *Context)
{
  Context->IsValid     = FALSE;
  Context->Gpio        = 0;
  Context->GpioOut     = 0;
  Context->GpioIn      = 0;
  Context->DeviceType  = KEY_DEVICE_TYPE_UNKNOWN;
  Context->ActiveLow   = FALSE;
  Context->IsVolumeKey = FALSE;
}

STATIC
KEY_CONTEXT_PRIVATE *KeypadKeyCodeToKeyContext(UINT32 KeyCode)
{
  if (KeyCode == 114)
    return &KeyContextVolumeDown;
  else if (KeyCode == 115)
    return &KeyContextVolumeUp;
  else if (KeyCode == 116)
    return &KeyContextPower;
  else if (KeyCode == 117)
    return &KeyContextBack;
  else if (KeyCode == 118)
    return &KeyContextWindows;
  else if (KeyCode == 119)
    return &KeyContextHome;
  else if (KeyCode == 120)
    return &KeyContextDial;
  else
    return NULL;
}

RETURN_STATUS
EFIAPI
KeypadDeviceImplConstructor(VOID)
{
  UINTN                Index;
  KEY_CONTEXT_PRIVATE *StaticContext;

  // Reset all keys
  for (Index = 0; Index < (sizeof(KeyList) / sizeof(KeyList[0])); Index++) {
    KeypadInitializeKeyContextPrivate(KeyList[Index]);
  }

  // Configure keys

  // power button
  StaticContext             = KeypadKeyCodeToKeyContext(116);
  StaticContext->DeviceType = KEY_DEVICE_TYPE_LEGACY;
  StaticContext->Gpio       = 94;
  StaticContext->ActiveLow  = 0x1 & 0x1;
  StaticContext->IsValid    = TRUE;

  // volume up button
  StaticContext              = KeypadKeyCodeToKeyContext(115);
  StaticContext->DeviceType  = KEY_DEVICE_TYPE_KEYMATRIX;
  StaticContext->GpioOut     = 33;
  StaticContext->GpioIn      = 42;
  StaticContext->ActiveLow   = 0x1 & 0x1;
  StaticContext->IsVolumeKey = TRUE;
  StaticContext->IsValid     = TRUE;

  // volume down button
  StaticContext              = KeypadKeyCodeToKeyContext(114);
  StaticContext->DeviceType  = KEY_DEVICE_TYPE_KEYMATRIX;
  StaticContext->GpioOut     = 33;
  StaticContext->GpioIn      = 41;
  StaticContext->ActiveLow   = 0x1 & 0x1;
  StaticContext->IsVolumeKey = TRUE;
  StaticContext->IsValid     = TRUE;

  // back button
  StaticContext             = KeypadKeyCodeToKeyContext(117);
  StaticContext->DeviceType = KEY_DEVICE_TYPE_KEYMATRIX;
  StaticContext->GpioOut    = 31;
  StaticContext->GpioIn     = 42;
  StaticContext->ActiveLow  = 0x1 & 0x1;
  StaticContext->IsValid    = TRUE;

  // windows button
  StaticContext             = KeypadKeyCodeToKeyContext(118);
  StaticContext->DeviceType = KEY_DEVICE_TYPE_KEYMATRIX;
  StaticContext->GpioOut    = 32;
  StaticContext->GpioIn     = 42;
  StaticContext->ActiveLow  = 0x1 & 0x1;
  StaticContext->IsValid    = TRUE;

  // home button
  StaticContext             = KeypadKeyCodeToKeyContext(119);
  StaticContext->DeviceType = KEY_DEVICE_TYPE_KEYMATRIX;
  StaticContext->GpioOut    = 31;
  StaticContext->GpioIn     = 41;
  StaticContext->ActiveLow  = 0x1 & 0x1;
  StaticContext->IsValid    = TRUE;

  // dial button
  StaticContext             = KeypadKeyCodeToKeyContext(120);
  StaticContext->DeviceType = KEY_DEVICE_TYPE_KEYMATRIX;
  StaticContext->GpioOut    = 32;
  StaticContext->GpioIn     = 41;
  StaticContext->ActiveLow  = 0x1 & 0x1;
  StaticContext->IsValid    = TRUE;

  return RETURN_SUCCESS;
}

EFI_STATUS EFIAPI KeypadDeviceImplReset(KEYPAD_DEVICE_PROTOCOL *This)
{
  LibKeyInitializeKeyContext(&KeyContextPower.EfiKeyContext);
  KeyContextPower.EfiKeyContext.KeyData.Key.ScanCode = SCAN_ESC;

  LibKeyInitializeKeyContext(&KeyContextVolumeUp.EfiKeyContext);
  KeyContextVolumeUp.EfiKeyContext.KeyData.Key.ScanCode = SCAN_UP;

  LibKeyInitializeKeyContext(&KeyContextVolumeDown.EfiKeyContext);
  KeyContextVolumeDown.EfiKeyContext.KeyData.Key.ScanCode = SCAN_DOWN;

  LibKeyInitializeKeyContext(&KeyContextBack.EfiKeyContext);
  KeyContextBack.EfiKeyContext.KeyData.Key.UnicodeChar = CHAR_BACKSPACE;

  LibKeyInitializeKeyContext(&KeyContextWindows.EfiKeyContext);
  KeyContextWindows.EfiKeyContext.KeyData.Key.UnicodeChar = CHAR_TAB;

  LibKeyInitializeKeyContext(&KeyContextHome.EfiKeyContext);
  KeyContextHome.EfiKeyContext.KeyData.Key.ScanCode = SCAN_HOME;

  LibKeyInitializeKeyContext(&KeyContextDial.EfiKeyContext);
  KeyContextDial.EfiKeyContext.KeyData.Key.UnicodeChar = CHAR_CARRIAGE_RETURN;

  return EFI_SUCCESS;
}

extern void gpio_set(unsigned n, unsigned on);
extern int  gpio_get(unsigned n);

EFI_EVENT m_CallbackTimer         = NULL;
EFI_EVENT m_ExitBootServicesEvent = NULL;
BOOLEAN timerRunning = FALSE;

// Callback function to disable the GPIO after a certain time
VOID EFIAPI DisableKeyPadLed(IN EFI_EVENT Event, IN VOID *Context)
{
  // Disable the GPIO
  gpio_set(HTCLEO_GPIO_KP_LED, 0);
  timerRunning = FALSE;
}

// Function to enable the GPIO and schedule the callback
VOID EnableKeypadLedWithTimer(VOID)
{
  timerRunning = TRUE;
  gpio_set(HTCLEO_GPIO_KP_LED, 1);
  EFI_STATUS Status;

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL | EVT_TIMER, TPL_CALLBACK, DisableKeyPadLed, NULL,
      &m_CallbackTimer);

  ASSERT_EFI_ERROR(Status);

  Status = gBS->SetTimer(
      m_CallbackTimer, TimerPeriodic, EFI_TIMER_PERIOD_MILLISECONDS(5000));

  ASSERT_EFI_ERROR(Status);
}

EFI_STATUS KeypadDeviceImplGetKeys(
    KEYPAD_DEVICE_PROTOCOL *This, KEYPAD_RETURN_API *KeypadReturnApi,
    UINT64 Delta)
{
  UINT8   GpioStatus;
  BOOLEAN IsPressed;
  UINTN   Index;
  // DEBUG((EFI_D_ERROR, "KeypadDeviceImplGetKeys!\n"));

  for (Index = 0; Index < (sizeof(KeyList) / sizeof(KeyList[0])); Index++) {
    KEY_CONTEXT_PRIVATE *Context = KeyList[Index];

    // check if this is a valid key
    if (Context->IsValid == FALSE)
      continue;

    // get status
    if (Context->DeviceType == KEY_DEVICE_TYPE_LEGACY) {
      // impliement hd2 gpio shit here
      GpioStatus = gpio_get(Context->Gpio);
    }
    else if (Context->DeviceType == KEY_DEVICE_TYPE_KEYMATRIX) {
      gpio_set(Context->GpioOut, 0);
      GpioStatus = gpio_get(Context->GpioIn);
    }
    else {
      continue;
    }

    // update key status
    // 0000 ^0001 = 0001 = decimal 1
    IsPressed = (GpioStatus ? 1 : 0) ^ Context->ActiveLow;

    if (IsPressed && !Context->IsVolumeKey && !timerRunning) {
      EnableKeypadLedWithTimer();
    }

    if (Context->DeviceType == KEY_DEVICE_TYPE_KEYMATRIX) {
      gpio_set(Context->GpioOut, 1);
    }

    LibKeyUpdateKeyStatus(
        &Context->EfiKeyContext, KeypadReturnApi, IsPressed, Delta);
  }

  return EFI_SUCCESS;
}
