#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE  hMailslot;         // дескриптор почтового ящика
  DWORD  dwNextMessageSize;  // размер следующего сообщения
  DWORD  dwMessageCount;     // количество сообщений
  
  // создаем почтовый ящик
  hMailslot = CreateMailslot(
    "\\\\.\\mailslot\\demo_mailslot",    // имя почтового ящика
    0,       // длина сообщения произвольна
    MAILSLOT_WAIT_FOREVER,   // ждем сообщения произвольно долго
    NULL     // безопасность по умолчанию
    );
  // проверяем на успешное создание
  if (hMailslot == INVALID_HANDLE_VALUE)
  {
    cerr << "Create mailslot failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to finish server.";
    cin.get();
    
    return 0;
  }
  
  cout << "The mailslot is created." << endl;
  
  // ждем сообщений
  cout << "Press any key to read messages." << endl;
  cin.get();
  
  // получаем информацию о почтовом ящике
  if (!GetMailslotInfo(
    hMailslot,   // дескриптор почтового ящика
    NULL,       // максимальный размер сообщения не нужен
    &dwNextMessageSize,  // размер следующего сообщения
    &dwMessageCount,     // количество сообщений
    NULL))       // интервал ожидания не нужен
  {
    cerr << "Get mailslot info failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to finish server.";
    cin.get();
    
    return 0;
  }

  // читаем сообщения
  while (dwMessageCount != 0)
  {
    DWORD dwBytesRead;
    char*  pchMessage;
    
    // захватываем память для сообщения
    pchMessage = (char*) new char[dwNextMessageSize];
    
    // читаем одно сообщение 
    if (!ReadFile(
      hMailslot,       // дескриптор канала
      pchMessage,      // адрес буфера для ввода данных
      dwNextMessageSize,   // количество читаемых байтов
      &dwBytesRead,        // количество прочитанных байтов
      NULL             // передача данных синхронная
      ))
    {
      cerr << "Read file failed." << endl
        << "The last error code: " << GetLastError() << endl;
      CloseHandle(hMailslot);
      cout << "Press any key to finish server.";
      cin.get();
      
      return 0;
    }
    
    // выводим сообщение на консоль
    cout << "The message << " << pchMessage << " >> was read" << endl;

    // получаем информацию о следующем сообщении
    if (!GetMailslotInfo(
      hMailslot,   // дескриптор почтового ящика
      NULL,        // максимальный размер сообщения не нужен
      &dwNextMessageSize,  // размер следующего сообщения
      &dwMessageCount,     // количество сообщений
      NULL))       // интервал ожидания не нужен
    {
      cerr << "Get mailslot info failed." << endl
        << "The last error code: " << GetLastError() << endl;
      cout << "Press any key to finish server.";
      cin.get();
      
      return 0;
    }

    // освобождаем память для сообщения
    delete[] pchMessage;
  }
  
  // закрываем дескриптор почтового ящика 
  CloseHandle(hMailslot);
  
  // завершаем процесс
  cout << "Press any key to exit.";
  cin.get();
  
  return 0;
}