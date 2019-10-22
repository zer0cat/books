#include <windows.h>
#include <iostream.h>
#include <string.h>

int main()
{
  char  machineName[80];
  char  pipeName[80];
  HANDLE  hNamedPipe;

  DWORD  dwFlags = PIPE_CLIENT_END |
        PIPE_TYPE_MESSAGE;   // клиент канала и передача данных сообщени€ми
  DWORD  dwOutBufferSize;    // состо€ние канала
  DWORD  dwInBufferSize;     // количество экземпл€ров канала
  DWORD  dwMaxInstances;     // размер буфера клиента канала
  
  // вводим им€ машины в сети, на которой работает сервер
  cout << "Enter a name of the server machine: ";
  cin >> machineName;
  cin.get();
  // подставл€ем им€ машины в им€ канала
  wsprintf(pipeName, "\\\\%s\\pipe\\demo_pipe", machineName);
  
  // св€зываемс€ с именованным каналом
  hNamedPipe = CreateFile(
    pipeName,          // им€ канала
    GENERIC_READ | GENERIC_WRITE,    // читаем и записываем в канал
    FILE_SHARE_READ | FILE_SHARE_WRITE,  // разрешаем чтение и запись
    NULL,                  // безопасность по умолчанию
    OPEN_EXISTING,         // открываем существующий канал
    FILE_ATTRIBUTE_NORMAL, // атрибуты по умолчанию
    NULL);                 // дополнительных атрибутов нет
  
  // провер€ем св€зь с каналом
  if (hNamedPipe==INVALID_HANDLE_VALUE)
  {
    cerr << "Connection with the named pipe failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to exit.";
    cin.get();

    return 0;
  }

  // получаем информацию о канале
  if (!GetNamedPipeInfo(
    hNamedPipe,        // дескриптор именованного канала
    &dwFlags,          // тип канала
    &dwOutBufferSize,  // размер выходного буфера
    &dwInBufferSize,   // размер входного буфера 
    &dwMaxInstances))  // максимальна€ количество экземпл€ров канала
  {
    cerr << "Get named pipe info failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to exit.";
    cin.get();

    return 0;
  }

  // выводим информацию на консоль
  cout << "Out buffer size: " << dwOutBufferSize << endl
    << "In buffer size: " << dwInBufferSize << endl
    << "Max instances: " << dwMaxInstances << endl;

  // закрываем дескриптор канала
  CloseHandle(hNamedPipe);
  // завершаем процесс
  cout << "Press any key to exit.";
  cin.get();

  return 0;
}