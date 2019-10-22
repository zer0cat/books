#include <windows.h>
#include <iostream.h>
#include <string.h>

int main()
{
  HANDLE  hMailslot;
  char  mailslotName[] = "\\\\.\\mailslot\\demo_mailslot";
  
  // связываемся с почтовым ящиком
  hMailslot = CreateFile(
    mailslotName,      // имя почтового ящика
    GENERIC_WRITE,     // записываем в ящик
    FILE_SHARE_READ,   // разрешаем одновременное чтение из ящика
    NULL,              // защита по умолчанию
    OPEN_EXISTING,     // открываем существующий канал
    0,                 // атрибуты по умолчанию
    NULL               // дополнительных атрибутов нет
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
  
  // вводим количество передаваемых сообщений
  int  n;
  cout << "Input a number of messages: ";
  cin >> n;
  cin.get();
  
  // пишем сообщения в почтовый ящик
  for (int i = 0; i < n; ++i)
  {
    DWORD  dwBytesWritten;
    char  pchMessage[256];
    int    nMessageSize;
    
    cout << "Input message: ";
    // читаем сообщение
    cin.getline(pchMessage, 256);
    // определем длину сообщения
    nMessageSize = strlen(pchMessage) + 1;
    
    // пишем сообщение
    if (!WriteFile(
      hMailslot,       // дескриптор почтового ящика
      pchMessage,      // данные
      nMessageSize,    // размер данных
      &dwBytesWritten, // количество записанных байтов
      NULL             // синхронная запись
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
  }
  
  // закрываем дескриптор канала
  CloseHandle(hMailslot);
  // завершаем процесс
  cout << "The messages are written by the client." << endl
    << "Press any key to exit." << endl;
  cin.get();
  
  return 0;
}