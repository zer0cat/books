#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE  hConsoleOutput;  // дескриптор буфера экрана
  COORD  coord;            // координаты окна консоли

  // получаем дескриптор окна консоли
  hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hConsoleOutput == INVALID_HANDLE_VALUE)
  {
    cout << "Get standard handle failed." << endl;
    return GetLastError();
  }
  // получаем максимальные размеры окна консоли
  coord = GetLargestConsoleWindowSize(hConsoleOutput);
  if (coord.X == 0 && coord.Y == 0)
  {
    cout << "Get largest console window size failed." << endl;
    return GetLastError();
  }

  cout << "Coordinate X = " << coord.X << endl;
  cout << "Coordinate Y = " << coord.Y << endl;

  cout << "Press any key to exit.";
  cin.get();

  return 0;
}