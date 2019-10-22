#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE  hStdOut;   // дескриптор стандартного вывода
  SMALL_RECT  sr;    // прямоугольник окна

  // читаем дескриптор стандартного вывода
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

  cout << "Set new window rectangle in characters."<<endl<<endl;
  // устанавливаем прямоугольник окна в символах
  cout << "Input left coordinate (0-79): ";
  cin >> sr.Left;
  cout << "Input top coordinate (0-24): ";
  cin >> sr.Top;
  cout << "Input right coordinate (" << (sr.Left - 79) << ',' 
       << sr.Left << "): ";
  cin >> sr.Right;
  cout << "Input bottom coordinate (" << (sr.Top - 24) << ','
    << sr.Top << "): ";
  cin >> sr.Bottom;
  // устанавливаем новое окно в относительных координатах
  if (!SetConsoleWindowInfo(hStdOut, FALSE, &sr))
    cout << "Set console window info failed." << endl;

  cin.get();
  cout << endl << "Press key to exit.";
  cin.get();

  return 0;
}