#include <windows.h>
#include <conio.h>

int main()
{
  if (!AllocConsole())
  {
    MessageBox(NULL,
      "Console allocation failed",
      "Ошибка Win32 API",
      MB_OK | MB_ICONINFORMATION
    );
    return 0;
  }

  _cputs("I am created.");
  _cputs("\nPress any char to exit.\n");
  _getch();

  return 0;
}