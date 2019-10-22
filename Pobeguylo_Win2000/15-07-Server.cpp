#include <windows.h>
#include <conio.h>

int main()
{
  char lpszComLine1[80] = "C:\\Client1.exe";   // имя первого клиента
  char lpszComLine2[80] = "C:\\Client2.exe";   // имя второго клиента

  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  HANDLE hWritePipe, hReadPipe;
  SECURITY_ATTRIBUTES sa;

  // устанавливает атрибуты защиты канала
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = NULL;  // защита по умолчанию
  sa.bInheritHandle = TRUE;        // дескрипторы наследуемые

  // создаем анонимный канал
  if(!CreatePipe(
      &hReadPipe,  // дескриптор для чтения
      &hWritePipe, // дескриптор для записи
      &sa,   // атрибуты защиты по умолчанию, дескрипторы наследуемые
      0))    // размер буфера по умолчанию
      
  {
    _cputs("Create pipe failed.\n");
    _cputs("Press any key to finish.\n");
    _getch();
    return GetLastError();
  }
  // устанавливаем атрибуты нового процесса
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  // использовать стандартные дескрипторы
  si.dwFlags = STARTF_USESTDHANDLES;
  // устанавливаем стандартные дескрипторы
  si.hStdInput = hReadPipe;
  si.hStdOutput = hWritePipe;
  si.hStdError = hWritePipe;
  // запускаем первого клиента
  if (!CreateProcess(
      NULL,    // имя процесса
      lpszComLine1,  // командная строка
      NULL,    // атрибуты защиты процесса по умолчанию
      NULL,    // атрибуты защиты первичного потока по умолчанию
      TRUE,    // наследуемые дескрипторы текущего процесса
               // наследуются новым процессом
      CREATE_NEW_CONSOLE,  // создаем новую консоль
      NULL,    // используем среду окружения процесса предка
      NULL,    // текущий диск и каталог, как и в процессе предке
      &si,     // вид главного окна - по умолчанию
      &pi      // здесь будут дескрипторы и идентификаторы
               // нового процесса и его первичного потока
      )
    )
  {
    _cputs("Create process failed.\n");
    _cputs("Press any key to finish.\n");
    _getch();
    return GetLastError();
  }
  // закрываем дескрипторы первого клиента
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  // запускаем второго клиента
  if (!CreateProcess(
      NULL,    // имя процесса
      lpszComLine2,  // командная строка
      NULL,    // атрибуты защиты процесса по умолчанию
      NULL,    // атрибуты защиты первичного потока по умолчанию
      TRUE,    // наследуемые дескрипторы текущего процесса
          // наследуются новым процессом
      CREATE_NEW_CONSOLE,  // создаем новую консоль
      NULL,    // используем среду окружения процесса предка
      NULL,    // текущий диск и каталог, как и в процессе предке
      &si,     // вид главного окна - по умолчанию
      &pi      // здесь будут дескрипторы и идентификаторы
               // нового процесса и его первичного потока
      )
    )
  {
    _cputs("Create process failed.\n");
    _cputs("Press any key to finish.\n");
    _getch();
    return GetLastError();
  }
  // закрываем дескрипторы второго клиента
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  // закрываем дескрипторы канала
  CloseHandle(hReadPipe);
  CloseHandle(hWritePipe);

  _cputs("The clients are created.\n");
  _cputs("Press any key to exit.\n");
  _getch();

  return 0;
}