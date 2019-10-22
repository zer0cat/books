#include <windows.h>
#include <iostream.h>
#include <string.h>

int main()
{
  char    machineName[80];
  char    pipeName[80];
  HANDLE   hNamedPipe;
  DWORD   dwBytesRead;         // для количества прочитанных байтов
  DWORD  dwTotalBytesAvail;    // количество байтов в канале
  DWORD  dwBytesLeftThisMessage;   // количество непрочитанных байтов
  char     pchMessage[80];     // для сообщения
  
  // вводим имя машины в сети, на которой работает сервер
  cout << "Enter a name of the server machine: ";
  cin >> machineName;
  cin.get();
  // подставляем имя машины в имя канала
  wsprintf(pipeName, "\\\\%s\\pipe\\demo_pipe", machineName);
  
  // связываемся с именованным каналом
  hNamedPipe = CreateFile(
    pipeName,          // имя канала
    GENERIC_READ | GENERIC_WRITE,    // читаем и записываем в канал
    FILE_SHARE_READ | FILE_SHARE_WRITE,  // разрешаем чтение и запись 
    NULL,            // безопасность по умолчанию
    OPEN_EXISTING,   // открываем существующий канал
    FILE_ATTRIBUTE_NORMAL,   // атрибуты по умолчанию
    NULL);           // дополнительных атрибутов нет
  
  // проверяем связь с каналом
  if (hNamedPipe==INVALID_HANDLE_VALUE)
  {
    cerr << "Connection with the named pipe failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to exit.";
    cin.get();

    return 0;
  }

  // ждем команду на копирование сообщения из канала
  cout << "Press any key to peek a message." << endl;
  cin.get();

  // копируем информацию из именованного канала
  if (!PeekNamedPipe(
    hNamedPipe,            // дескриптор канала
    pchMessage,            // данные
    sizeof(pchMessage),    // размер данных
    &dwBytesRead,          // количество записанных байтов
    &dwTotalBytesAvail,    // количество байтов в канале
    &dwBytesLeftThisMessage  // количество непрочитанных байтов
    ))
  {
    // ошибка чтения сообщения
    cerr << "Peek named pipe failed: " << endl
      << "The last error code: " << GetLastError() << endl;
    CloseHandle(hNamedPipe);
    cout << "Press any key to exit.";
    cin.get();

    return 0;
  }
  // выводим полученное сообщение на консоль
  if (dwTotalBytesAvail)
    cout << "The peeked message: "
    << endl << '\t' << pchMessage << endl;
  else
    cout << "There is no mesage." << endl;

  // теперь читаем сообщение из именованного канала
  if (!ReadFile(
    hNamedPipe,      // дескриптор канала
    pchMessage,      // данные
    sizeof(pchMessage),  // размер данных
    &dwBytesRead,    // количество записанных байтов
    NULL))           // синхронное чтение
  {
    // ошибка чтения
    cerr << "Read file failed: " << endl
      << "The last error code: " << GetLastError() << endl;
    CloseHandle(hNamedPipe);
    cout << "Press any key to exit.";
    cin.get();

    return 0;
  }
  // выводим полученное сообщение на консоль
  cout << "The client received the message from a server: "
    << endl << '\t' << pchMessage << endl;
  // закрываем дескриптор канала
  CloseHandle(hNamedPipe);
  // завершаем процесс
  cout << "Press any key to exit." << endl;
  cin.get();

  return 0;
}