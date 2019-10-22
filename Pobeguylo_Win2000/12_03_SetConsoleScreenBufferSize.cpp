#include <windows.h>
#include <iostream.h>

int main()
{
  COORD  coord;      // для размера буфера экрана
  HANDLE  hStdOut;   // дескриптор стандартного вывода

  // читаем дескриптор стандартного вывода
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

  // вводим новый размер буфера экрана
  cout << "Enter new screen buffer size." << endl;
  cout << "A number of columns: ";
  cin >> coord.X;
  cout << "A number of rows: ";
  cin >> coord.Y;
  // устанавливаем новый размер буфера экрана
  if (!SetConsoleScreenBufferSize(hStdOut, coord))
  {
    cout << "Set console screen buffer size failed." << endl;
    return GetLastError();
  }

  return 0;
}