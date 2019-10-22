#include <windows.h>
#include <iostream.h>

int main()
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  // заполняем значения структуры STARTUPINFO по умолчанию
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);

  // запускаем процесс Notepad
  if (!CreateProcess(
      NULL,    // имя не задаем
      "Notepad.exe",  // имя программы
      NULL,    // атрибуты защиты процесса устанавливаем по умолчанию
      NULL,    // атрибуты защиты первичного потока по умолчанию
      FALSE,   // дескрипторы текущего процесса не наследуются
      0,       // по умолчанию NORMAL_PRIORITY_CLASS  
      NULL,    // используем среду окружения вызывающего процесса
      NULL,    // текущий диск и каталог, как и в вызывающем процессе
      &si,     // вид главного окна - по умолчанию
      &pi      // информация о новом процессе
)
    )
  {
    cout << "The mew process is not created." << endl
      << "Check a name of the process." << endl;
    return 0;
  }

  Sleep(1000);  // немного подождем и закончим свою работу
  // закроем дескрипторы запущенного процесса в текущем процессе
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  return 0;
}