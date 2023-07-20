#include "CommonHeader.h"
#include <Chipset/timer.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/LoadedImage.h>
#include <Resources/FbColor.h>
#include <Uefi.h>

typedef struct {
  const UINT8   index;
  const CHAR16 *name;
  void (*function)();
} MenuEntry;

void option1Function(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void option2Function(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void exitMenu(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void startTetris(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void prepareConsole(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout,
    OUT EFI_SIMPLE_TEXT_OUTPUT_MODE    *modeToStore);
void restoreInitialConsoleMode(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout,
    IN EFI_SIMPLE_TEXT_OUTPUT_MODE     *storedMode);
void HandleKeyInput(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);

EFI_STATUS StartAnotherApp(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable,
    IN EFI_GUID *AppGuid);

MenuEntry menuOptions[] = {
    {1, L"Option 1", &option1Function},
    {2, L"Option 2", &option2Function},
    {3, L"Play Tetris", &startTetris},
    {4, L"Exit", &exitMenu}
    };

UINTN menuOptionCount = sizeof(menuOptions) / sizeof(menuOptions[0]);

UINTN selectedIndex = 0;




void prepareConsole(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout,
    OUT EFI_SIMPLE_TEXT_OUTPUT_MODE    *modeToStore)
{
  EFI_STATUS status;
  CopyMem(modeToStore, cout->Mode, sizeof(EFI_SIMPLE_TEXT_OUTPUT_MODE));

  status = cout->EnableCursor(cout, FALSE);
  if (status != EFI_UNSUPPORTED) { // workaround
    ASSERT_EFI_ERROR(status);
  }

  status = cout->ClearScreen(cout);
  ASSERT_EFI_ERROR(status);
  status = cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
  ASSERT_EFI_ERROR(status);
  status = cout->SetCursorPosition(cout, 0, 0);
  ASSERT_EFI_ERROR(status);
}

void restoreInitialConsoleMode(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout,
    IN EFI_SIMPLE_TEXT_OUTPUT_MODE     *storedMode)
{
  EFI_STATUS status;

  status = cout->EnableCursor(cout, storedMode->CursorVisible);
  ASSERT_EFI_ERROR(status);
  status = cout->SetCursorPosition(
      cout, storedMode->CursorColumn, storedMode->CursorRow);
  ASSERT_EFI_ERROR(status);
  status = cout->SetAttribute(cout, storedMode->Attribute);
  ASSERT_EFI_ERROR(status);
  status = cout->ClearScreen(cout);
  ASSERT_EFI_ERROR(status);
}


EFI_STATUS EFIAPI
ShellAppMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_SIMPLE_TEXT_OUTPUT_MODE initialMode;

  prepareConsole(SystemTable->ConOut, &initialMode);

  // Print menu title
  gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_GREEN, EFI_BLUE));
  setCursorPos(15, 1);
  Print(L" HtcLeoRevivalProject EDK2 Main Menu ");

  // Print menu options
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *consoleOut = gST->ConOut;
  consoleOut->SetAttribute(consoleOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));

  // Loop that never exits
  while (TRUE) {
    UINTN i;
    for (i = 0; i < menuOptionCount; i++) {
      setCursorPos(25, 3 + i);
      if (i == selectedIndex) {
        consoleOut->SetAttribute(consoleOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
      }
      else {
        consoleOut->SetAttribute(
            consoleOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
      }
      Print(L"%d. %s ", menuOptions[i].index, menuOptions[i].name);
    }

    HandleKeyInput(ImageHandle, SystemTable);
  }

  // Restore initial console mode
  restoreInitialConsoleMode(SystemTable->ConOut, &initialMode);

  return EFI_SUCCESS;
}



void HandleKeyInput(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_INPUT_KEY key;
  EFI_STATUS    status;

  status = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);

  if (status != EFI_NOT_READY) {
    ASSERT_EFI_ERROR(status);

    switch (key.ScanCode) {
    case CHAR_BACKSPACE:
      break;
    case SCAN_HOME:
      // home button
      break;
    case CHAR_TAB:
      // windows button
      break;
    case SCAN_UP:
      // volume up button
      if (selectedIndex == 0) {
        selectedIndex = menuOptionCount - 1;
      }
      else {
        selectedIndex--;
      }
      break;
    case SCAN_DOWN:
      // volume down button
      if (selectedIndex == menuOptionCount - 1) {
        selectedIndex = 0;
      }
      else {
        selectedIndex++;
      }
      break;
    case SCAN_ESC:
      // power button
      break;
    case CHAR_CARRIAGE_RETURN:
      // dial button
      if (menuOptions[selectedIndex].function != NULL) {
        menuOptions[selectedIndex].function(ImageHandle, SystemTable);
      }
      break;
    }
  }
}

EFI_STATUS StartAnotherApp(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable,
    IN EFI_GUID *AppGuid)
{
  DEBUG((EFI_D_ERROR, "Start another app\n"));
  mdelay(3000);

  EFI_STATUS Status;

  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
  EFI_HANDLE                 AppImageHandle;

  // Get the LoadedImage protocol of the current application
  DEBUG((EFI_D_ERROR, "Assigning loaded image protocol guid\n"));
  mdelay(3000);
  Status = gBS->HandleProtocol(
      ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);
  if (EFI_ERROR(Status)) {

    return Status;
  }
  DEBUG((EFI_D_ERROR, "Starting application\n"));
  mdelay(3000);
  // Load the EFI application image by its GUID
  Status = gBS->LoadImage(TRUE, ImageHandle, AppGuid, NULL, 0, AppImageHandle);
  if (EFI_ERROR(Status)) {

    return Status;
  }

  // Start the EFI application
  Status = gBS->StartImage(AppImageHandle, NULL, NULL);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "You selected Option 4\n"));
    mdelay(3000);
    // Handle the error condition if needed
  }

  return Status;
}

void option1Function(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  DEBUG((EFI_D_ERROR, "You selected Option 1\n"));
}

void option2Function(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  DEBUG((EFI_D_ERROR, "You selected Option 2\n"));
}

void exitMenu(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
   SystemTable->BootServices->Exit(ImageHandle, EFI_SUCCESS, 0, NULL);
}

void startTetris(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{

  EFI_GUID TetrisAppGuid = {
      0xD7A59C0A,
      0x899E,
      0x4E31,
      {0xA1, 0xC3, 0x3D, 0x55, 0x1D, 0x7A, 0x53, 0x81}};

  // ...
  mdelay(1000);
  EFI_STATUS Status = StartAnotherApp(ImageHandle, SystemTable, &TetrisAppGuid);
}
