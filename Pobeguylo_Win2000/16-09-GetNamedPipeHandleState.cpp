#include <windows.h>
#include <iostream.h>
#include <string.h>

int main()
{
  char  machineName[80];
  char  pipeName[80];
  HANDLE   hNamedPipe;

  DWORD  dwState;              // состояние канала
  DWORD  dwCurInstances;       // количество экземпляров канала
  DWORD  dwMaxCollectionCount; // размер буфера клиента канала
  DWORD  dwCollectDataTimeout; // задержка перед передачей данных
  TCHAR  chUserName[255];      // имя владельца именованного канала
  
  // вводим имя машины в сети, на которой работает сервер
  cout << "Enter a name of the server machine: ";
  cin >> machineName;
  cin.get();
  // подставляем имя машины в имя канала
  wsprintf(pipeName, "\\\\%s\\pipe\\demo_pipe", machineName);
  
  // связываемся с именованным каналом
  hNamedPipe = CreateFile(
    pipeName,                // имя канала
    GENERIC_READ | GENERIC_WRITE,        // читаем и записываем в канал
    FILE_SHARE_READ | FILE_SHARE_WRITE,  // разрешаем чтение и запись
                                         // в канал
    NULL,                    // безопасность по умолчанию
    OPEN_EXISTING,           // открываем существующий канал
    FILE_ATTRIBUTE_NORMAL,   // атрибуты по умолчанию
    NULL);                   // дополнительных атрибутов нет
  
  // проверяем связь с каналом
  if (hNamedPipe==INVALID_HANDLE_VALUE)
  {
    cerr << "Connection with the named pipe failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to exit.";
    cin.get();

    return 0;
  }

  // определяем состояние канала
  if (!GetNamedPipeHandleState(
    hNamedPipe,            // дескриптор именованного канала
    &dwState,              // состояние именованного канала
    &dwCurInstances,       // количество экземпляров канала
    &dwMaxCollectionCount, // размер буфера клиента канала 
    &dwCollectDataTimeout, // макс. задержка перед передачей данных
    chUserName,            // имя пользователя канала
    255))                  // максимальная длина имени
  {
    cerr << "Get named pipe handle state failed." << endl
      << "The last error code: " << GetLastError() << endl;
    cout << "Press any key to exit.";
    cin.get();

    return 0;
  }

  // выводим состояние канала на консоль
  cout << "State: ";
  switch (dwState)
  {
  case (PIPE_NOWAIT):
    cout << "PIPE_NOWAIT" << endl;
    break;
  case (PIPE_READMODE_MESSAGE):
    cout << "PIPE_READMODE_MESSAGE" << endl;
    break;
  case (PIPE_NOWAIT | PIPE_READMODE_MESSAGE):
    cout << "PIPE_NOWAIT and PIPE_READMODE_MESSAGE" << endl;
    break;
  default:
    cout << "Unknown state." << endl;
    break;
  }

  cout << "Current instances: " << dwCurInstances << endl
    << "Max collection count: " << dwMaxCollectionCount << endl
    << "Collection data timeout: " << dwCollectDataTimeout << endl
    << "User name: " << chUserName << endl;

  // закрываем дескриптор канала
  CloseHandle(hNamedPipe);
  // завершаем процесс
  cout << "Press any key to exit.";
  cin.get();

  return 0;
}