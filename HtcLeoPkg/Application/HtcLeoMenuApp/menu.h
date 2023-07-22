#ifndef _MAIN_MENU_H_
#define _MAIN_MENU_H_
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

typedef struct {
  UINT8   index;
  CHAR16 *name;
  BOOLEAN isActive;
  void (*function)();
} MenuEntry;

void option1Function(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void rebootMenu(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void option2Function(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
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
void drawMenu(IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *consoleOut);

void  nullfunction();
UINTN getActiveMenuEntryLength();
void returnToMainMenu(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void shutdownWrapper(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void rebootWrapper(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void enterBootMGR(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);

#endif