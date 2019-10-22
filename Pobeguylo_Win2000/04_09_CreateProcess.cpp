#include <windows.h>
#include <conio.h>

volatile int count;

void thread()
{
  for (;;)
  {
    count++;
    Sleep(500);
    _cprintf ("count = %d\n", count);
  }
}

int main()
{
  char lpszComLine[80];  // для командной строки

  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  SECURITY_ATTRIBUTES sa;

  HANDLE  hThread;
  DWORD  IDThread;

  _cputs("Press any key to start the count-thread.\n");
  _getch();

  // устанавливает атрибуты защиты потока
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = NULL;    // защита по умолчанию
  sa.bInheritHandle = TRUE;          // дескриптор потока наследуемый
  
  // запускаем поток-счетчик
  hThread = CreateThread(&sa, 0, (LPTHREAD_START_ROUTINE)thread, NULL, 0, 
                          &IDThread);
  if (hThread == NULL)
    return GetLastError();

  // устанавливаем атрибуты нового процесса
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb=sizeof(STARTUPINFO);
  // формируем командную строку
  wsprintf(lpszComLine, "C:\\ConsoleProcess.exe %d", (int)hThread); 
  // запускаем новый консольный процесс
  if (!CreateProcess(
      NULL,    // имя процесса
      lpszComLine,  // адрес командной строки
      NULL,    // атрибуты защиты процесса по умолчанию
      NULL,    // атрибуты защиты первичного потока по умолчанию
      TRUE,    // наследуемые дескрипторы текущего процесса
               // наследуются новым процессом
      CREATE_NEW_CONSOLE,  // новая консоль  
      NULL,    // используем среду окружения процесса предка
      NULL,    // текущий диск и каталог, как и в процессе предке
      &si,     // вид главного окна - по умолчанию
      &pi      // здесь будут дескрипторы и идентификаторы
               // нового процесса и его первичного потока
      )
    )
  {
    _cputs("The new process is not created.\n");
    _cputs("Press any key to finish.\n");
    _getch();
    return GetLastError();
  }
  // закрываем дескрипторы нового процесса
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  // ждем закрытия потока-счетчика
  WaitForSingleObject(hThread, INFINITE);
  _cputs("Press any key to exit.\n");
  _getch();
  // закрываем дескриптор потока
  CloseHandle(hThread);

  return 0;
}