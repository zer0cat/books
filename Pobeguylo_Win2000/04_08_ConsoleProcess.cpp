#include <windows.h>
#include <conio.h>

int main(int argc, char *argv[])
{
  HANDLE  hThread;
  char  c;

  // преобразуем символьное представление дескриптора в число
  hThread = (HANDLE)atoi(argv[1]);
  // ждем команды о завершении потока
  while (true)
  {
    _cputs("Input 't' to terminate the thread: ");
    c = _getch();
    if (c == 't')
    {
      _cputs("t\n");
      break;
    }
  }
  // завершаем поток
  TerminateThread(hThread, 0);
  // закрываем дескриптор потока
  CloseHandle(hThread);

  _cputs("Press any key to exit.\n");
  _getch();

  return 0;
}