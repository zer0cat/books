#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE hNamedPipe;
  char pipeName[] = "\\\\.\\pipe\\demo_pipe";
  
  // связываемся с именованным каналом
  hNamedPipe = CreateFile(
    pipeName,        // имя канала
    GENERIC_WRITE,   // записываем в канал
    FILE_SHARE_READ, // разрешаем одновременное чтение из канала
    NULL,            // защита по умолчанию
    OPEN_EXISTING,   // открываем существующий канал
    0,     // атрибуты по умолчанию
    NULL   // дополнительных атрибутов нет
    );
  
  // проверяем связь с каналом
  if (hNamedPipe == INVALID_HANDLE_VALUE)
  {
    cerr << "Connection with the named pipe failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to exit.";

    return 0;
  }

  // пишем в именованный канал
  for (int i = 0; i < 10; i++)
  {
    DWORD dwBytesWritten;    
    if (!WriteFile(
      hNamedPipe,  // дескриптор канала
      &i,          // данные
      sizeof(i),   // размер данных
      &dwBytesWritten,   // количество записанных байтов
      NULL         // синхронная запись
      ))
    {
      // ошибка записи
      cerr << "Writing to the named pipe failed: " << endl
        << "The last error code: " << GetLastError() << endl;
      CloseHandle(hNamedPipe);
      cout << "Press any key to exit.";
      cin.get();

      return 0;
    }
    // выводим число на консоль
    cout << "The number " << i << " is written to the named pipe."
         << endl;
    Sleep(1000);
  }
  // закрываем дескриптор канала
  CloseHandle(hNamedPipe);
  // завершаем процесс
  cout << "The data are written by the client." << endl
    << "Press any key to exit.";
  cin.get();

  return 0;
}