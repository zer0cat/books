#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE  hStdOut, hStdIn;     // дескрипторы консоли
  DWORD  dwWritten, dwRead;    // для количества символов
  char  buffer[80];            // для ввода символов
  char  str[] = "Input any string:";
  char  c;

  // читаем дескрипторы консоли
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  hStdIn = GetStdHandle(STD_INPUT_HANDLE);
  if (hStdOut == INVALID_HANDLE_VALUE || hStdIn == INVALID_HANDLE_VALUE)
  {
    cout << "Get standard handle failed." << endl;
    return GetLastError();
  }
  // выводим сообщения о вводе строки
  if(!WriteConsole(hStdOut, &str, sizeof(str), &dwWritten, NULL))
  {
    cout << "Write console failed." << endl;
    return GetLastError();
  }
  // вводим строку
  if(!ReadConsole(hStdIn, &buffer, sizeof(buffer), &dwRead, NULL))
  {
    cout << "Read console failed." << endl;
    return GetLastError();
  }
  // ждем команду на завершение работы
  cout << "Input any char to exit: ";
  cin >> c;

  return 0;
}