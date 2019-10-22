#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE  hMailslot;     // дескриптор почтового ящика
  DWORD  dwReadTimeout;  // интервал для ожидания сообщения
  
  // создаем почтовый ящик
  hMailslot = CreateMailslot(
    "\\\\.\\mailslot\\demo_mailslot",  // имя почтового ящика
    0,     // длина сообщения произвольна
    0,     // интервал ожидания равен нулю
    NULL   // защита по умолчанию
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
  
  // получаем информацию о почтовом ящике
  if (!GetMailslotInfo(
    hMailslot,  // дескриптор почтового ящика
    NULL,      // максимальный размер сообщения не нужен
    NULL,      // размер следующего сообщения не нужен
    NULL,      // количество сообщений не нужно
    &dwReadTimeout))   // интервал ожидания сообщения
  {
    cerr << "Get mailslot info failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to finish server.";
    cin.get();
    
    return 0;
  }

  cout << "Read timeout: " << dwReadTimeout << endl;

  if (!SetMailslotInfo(
    hMailslot,   // дескриптор почтового ящика
    3000))       // изменяем интервал ожидания
  {
    cerr << "Set mailslot info failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to finish server.";
    cin.get();
    
    return 0;
  }

  // получаем информацию о почтовом ящике
  if (!GetMailslotInfo(
    hMailslot,   // дескриптор почтового ящика
    NULL,       // максимальный размер сообщения не нужен
    NULL,       // размер следующего сообщения не нужен
    NULL,       // количество сообщений не нужно
    &dwReadTimeout))   // интервал ожидания сообщения
  {
    cerr << "Get mailslot info failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to finish server.";
    cin.get();
    
    return 0;
  }

  cout << "Read timeout: " << dwReadTimeout << endl;
  
  // закрываем дескриптор почтового ящика 
  CloseHandle(hMailslot);
  
  // завершаем процесс
  cout << "Press any key to exit.";
  cin.get();
  
  return 0;
}