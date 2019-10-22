#include <windows.h>
#include <iostream.h>
#include <string.h>

int main()
{
  char    machineName[80];
  char    pipeName[80];
  DWORD   dwBytesRead;       // дл€ количества прочитанных байтов
  char    pchInBuffer[80];   // дл€ записи сообщени€
  char    pchOutBuffer[80];  // дл€ чтени€ сообщени€
  int     nMessageLength;    // длина сообщени€
  
  // вводим им€ машины в сети, на которой работает сервер
  cout << "Enter a name of the server machine: ";
  cin >> machineName;
  // подставл€ем им€ машины в им€ канала
  wsprintf(pipeName, "\\\\%s\\pipe\\demo_pipe", machineName);
  
  // вводим строку
  cin.get();
  cout << "Input a string: ";
  cin.getline(pchInBuffer, 80);
  // определ€ем длину строки
  nMessageLength = strlen(pchInBuffer) + 1;

  // св€зываемс€, пишем и читаем из именованного канала одной транзакцией
  if (!CallNamedPipe(
    pipeName,        // им€ именованного канала
    &pchInBuffer,    // адрес входного буфера канала
    nMessageLength,  // длина входного сообщени€
    &pchOutBuffer,   // адрес выходного буфера канала
    sizeof(pchOutBuffer),    // длина выходного буфера канала
    &dwBytesRead,    // количество прочитанных байтов
    NMPWAIT_WAIT_FOREVER))   // ждем бесконечно долго
  {
    // ошибка транзакции
    cerr << "Call named pipe failed: " << endl
      << "The last error code: " << GetLastError() << endl;
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
  // завершаем процесс
  cout << "Press any key to exit.";
  cin.get();

  return 0;
}