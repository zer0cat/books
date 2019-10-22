#include <windows.h>
#include <conio.h>

int main()
{
  // распределяем консоль
  if (!AllocConsole())
  {
    MessageBox(NULL,
      "Console allocation failed", "Ошибка Win32 API",
      MB_OK | MB_ICONINFORMATION
    );
    return 0;
  }

  _cputs("I am created.\n");
  _cputs("Press any char to exit.\n");
  _getch();

  // освобождаем консоль
  if (!FreeConsole())
  {
    _cputs("Free console failed.\n");
    _cputs("Press any key to exit.\n");
    _getch();
  }

  return 0;
}