#include <windows.h>
#include <iostream.h>
#include <string.h>

int main()
{
  char    machineName[80];
  char    pipeName[80];
  HANDLE  hNamedPipe;
  DWORD   dwBytesRead;       // дл€ количества прочитанных байтов
  char    pchInBuffer[80];   // дл€ записи сообщени€
  char    pchOutBuffer[80];  // дл€ чтени€ сообщени€
  int     nMessageLength;    // длина сообщени€
  
  // вводим им€ машины в сети, на которой работает сервер
  cout << "Enter a name of the server machine: ";
  cin >> machineName;
  // подставл€ем им€ машины в им€ канала
  wsprintf(pipeName, "\\\\%s\\pipe\\demo_pipe", machineName);
  
  // св€зываемс€ с именованным каналом
  hNamedPipe = CreateFile(
    pipeName,          // им€ канала
    GENERIC_READ | GENERIC_WRITE,    // читаем и записываем в канал
    FILE_SHARE_READ | FILE_SHARE_WRITE,  // разрешаем чтение и запись 
    NULL,            // безопасность по умолчанию
    OPEN_EXISTING,   // открываем существующий канал
    FILE_ATTRIBUTE_NORMAL,   // атрибуты по умолчанию
    NULL);           // дополнительных атрибутов нет
  
  // провер€ем св€зь с каналом
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
  cin.getline(pchInBuffer, 80);
  // определ€ем длину строки
  nMessageLength = strlen(pchInBuffer) + 1;

  // пишем и читаем из именованного канала одной транзакцией
  if (!TransactNamedPipe(
    hNamedPipe,      // дескриптор канала
    &pchInBuffer,    // адрес входного буфера канала
    nMessageLength,  // длина входного сообщени€
    &pchOutBuffer,   // адрес выходного буфера канала
    sizeof(pchOutBuffer),    // длина выходного буфера канала
    &dwBytesRead,    // количество прочитанных байтов
    NULL))           // передача транзакции синхронна€
  {
    // ошибка транзакции
    cerr << "Transact named pipe failed: " << endl
      << "The last error code: " << GetLastError() << endl;
    CloseHandle(hNamedPipe);
    cout << "Press any key to exit.";
    cin.get();

    return 0;
  }
  // выводим посланное сообщение на консоль
  cout << "The sent message: "
    << endl << '\t' << pchInBuffer << endl;
  // выводим полученное сообщение на консоль
  cout << "The received message: "
    << endl << '\t' << pchOutBuffer << endl;
  // закрываем дескриптор канала
  CloseHandle(hNamedPipe);
  // завершаем процесс
  cout << "Press any key to exit.";
  cin.get();

  return 0;
}