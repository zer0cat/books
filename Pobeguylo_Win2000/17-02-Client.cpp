#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE  hMailslot;
  char mailslotName[] = "\\\\.\\mailslot\\demo_mailslot";
  
  // связываемся с почтовым ящиком
  hMailslot = CreateFile(
    mailslotName,      // имя почтового ящика
    GENERIC_WRITE,     // записываем в ящик
    FILE_SHARE_READ,   // разрешаем одновременное чтение из ящика
    NULL,              // защита по умолчанию
    OPEN_EXISTING,     // открываем существующий канал
    0,         // атрибуты по умолчанию
    NULL       // дополнительных атрибутов нет
  );
  
  // проверяем связь с почтовым ящиком
  if (hMailslot == INVALID_HANDLE_VALUE)
  {
    cerr << "Create file failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to finish the client.";
    cin.get();

    return 0;
  }

  // вводим целое число
  int n;
  cout << "Input an integer: ";
  cin >> n;

  // пишем число в почтовый ящик
  DWORD dwBytesWritten;    
  if (!WriteFile(
      hMailslot,   // дескриптор почтового ящика
      &n,          // данные
      sizeof(n),   // размер данных
      &dwBytesWritten,   // количество записанных байтов
      NULL         // синхронная запись
    ))
  {
    // ошибка записи
    cerr << "Write file failed: " << endl
    << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to finish the client.";
    cin.get();

    CloseHandle(hMailslot);
    return 0;
  }

  // закрываем дескриптор канала
  CloseHandle(hMailslot);
  // завершаем процесс
  cout << "The number is written by the client." << endl
    << "Press any key to exit." << endl;
  cin.get();

  return 0;
}