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
  // имя нового процесса с пробелом
  char lpszComLine[80]="C:\\ConsoleProcess.exe ";
  // для символьного представления дескриптора
  char lpszHandle[20];

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  HANDLE  hThread;
  DWORD  IDThread;
  
  _cputs("Press any key to start the count-thread.\n");
  _cputs("After terminating the thread press any key to exit.\n");
  _getch();

  // запускаем поток-счетчик
  hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread, NULL, 
                          0, &IDThread);
  if (hThread == NULL)
    return GetLastError();

  // делаем дескриптор потока наследуемым
  if(!SetHandleInformation(
      hThread,               // дескриптор потока
      HANDLE_FLAG_INHERIT,   // изменяем наследование дескриптора
      HANDLE_FLAG_INHERIT))  // делаем дескриптор наследуемым
  {
    _cputs("The inheritance is not changed.\n");
    _cputs("Press any char to finish.\n");
    _getch();
    return GetLastError();
  }

  // устанавливаем атрибуты нового процесса
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb=sizeof(STARTUPINFO);
  // преобразуем дескриптор в символьную строку
  _itoa((int)hThread,lpszHandle,10);
  // создаем командную строку
  strcat(lpszComLine,lpszHandle);
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

  _getch();

  // закрываем дескриптор потока
  CloseHandle(hThread);

  return 0;
}