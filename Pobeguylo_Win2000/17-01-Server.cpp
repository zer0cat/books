#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE  hMailslot;       // дескриптор почтового ящика
  
  // создаем почтовый ящик
  hMailslot = CreateMailslot(
    "\\\\.\\mailslot\\demo_mailslot",  // имя почтового ящика
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
  
  cout << "The mailslot is waiting a message." << endl;
  // читаем одно целое число из почтового ящика 
  int nData;
  DWORD dwBytesRead;
  if (!ReadFile(
    hMailslot,       // дескриптор почтового ящика
    &nData,          // адрес буфера для ввода данных
    sizeof(nData),   // количество читаемых байтов
    &dwBytesRead,    // количество прочитанных байтов
    (LPOVERLAPPED)NULL   // передача данных синхронная
    ))
  {
    cerr << "Read file failed." << endl
      << "The last error code: " << GetLastError() << endl;
    CloseHandle(hMailslot);
    cout << "Press any key to finish server.";
    cin.get();
    
    return 0;
  }
  
  // выводим число на консоль
  cout << "The number " << nData << " was read by the server" << endl;

  // закрываем дескриптор почтового ящика 
  CloseHandle(hMailslot);

  // завершаем процесс
  cout << "Press any key to exit.";
  cin.get();

  return 0;
}