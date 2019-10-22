#include <windows.h>
#include <iostream.h>
#include <string.h>

int main()
{
  char   machineName[80];
  char   pipeName[80];
  HANDLE   hNamedPipe;
  DWORD   dwBytesWritten;    // для количества записанных байтов
  DWORD   dwBytesRead;       // для количества прочитанных байтов
  char     pchMessage[80];   // для сообщения
  int    nMessageLength;     // длина сообщения
  
  // вводим имя машины в сети, на которой работает сервер
  cout << "Enter a name of the server machine: ";
  cin >> machineName;
  // подставляем имя машины в имя канала
  wsprintf(pipeName, "\\\\%s\\pipe\\demo_pipe", machineName);
  
  // связываемся с именованным каналом
  hNamedPipe = CreateFile(
    pipeName,          // имя канала
    GENERIC_READ | GENERIC_WRITE,       // читаем и записываем в канал
    FILE_SHARE_READ | FILE_SHARE_WRITE, // разрешаем чтение и запись 
    NULL,                  // безопасность по умолчанию
    OPEN_EXISTING,         // открываем существующий канал
    FILE_ATTRIBUTE_NORMAL, // атрибуты по умолчанию
    NULL);                 // дополнительных атрибутов нет
  
  // проверяем связь с каналом
  if (hNamedPipe==INVALID_HANDLE_VALUE)
  {
    cerr << "Connection with the named pipe failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to exit.";
    cin.get();

    return 0;
  }

  // вводим строку
  cin.get();
  cout << "Input a string: ";
  cin.getline(pchMessage, 80);
  // определяем длину строки
  nMessageLength = strlen(pchMessage) + 1;

  // пишем в именованный канал
  if (!WriteFile(
    hNamedPipe,      // дескриптор канала
    pchMessage,      // данные
    nMessageLength,  // размер данных
    &dwBytesWritten, // количество записанных байтов
    NULL))           // синхронная запись
  {
    // ошибка записи
    cerr << "Write file failed: " << endl
      << "The last error code: " << GetLastError() << endl;
    CloseHandle(hNamedPipe);
    cout << "Press any key to exit.";
    cin.get();

    return 0;
  }
  // выводим посланное сообщение на консоль
  cout << "The client sent the message to a server: "
    << endl << '\t' << pchMessage << endl;
  // читаем из именованного канала
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
  cout << "Press any key to exit.";
  cin.get();

  return 0;
}