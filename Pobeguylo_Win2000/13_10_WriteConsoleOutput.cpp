#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE  hStdOut;       // дескриптор стандартного вывода
  CHAR_INFO  ci[80*25];  // прямоугольник, из которого будем выводить
  COORD    size;         // размеры этого прямоугольника
  // координаты левого угла прямоугольника, из которого выводим
  COORD    coord; 
  // координаты левого угла прямоугольника, в который пишем
  SMALL_RECT  sr; 

  // читаем стандартный дескриптор вывода
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  // заполняем прямоугольник, который будем выводить, пробелами
  for (int i = 0; i < 80*25; ++i)
  {
    ci[i].Char.AsciiChar = ' ';
    ci[i].Attributes = BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  }
  // устанавливаем левый угол многоугольника, из которого пишем
  coord.X = 0;
  coord.Y = 0;
  // устанавливаем размеры прямоугольника, который пишем
  size.X = 80;
  size.Y = 25;
  // вводим координаты левого верхнего угла многоугольника, 
  // в который пишем
  cout << "Input left coordinate to write: ";
  cin >> sr.Left;
  cout << "Input top coordinate to write: ";
  cin >> sr.Top;
  // вводим координаты правого нижнего угла прямоугольника, 
  // в который пишем
  cout << "Input right coordinate to write: ";
  cin >> sr.Right;
  cout << "Input down coordinate to write: ";
  cin >> sr.Bottom;

  // пишем прямоугольник в буфер экрана
  if (!WriteConsoleOutput(
      hStdOut,   // дескриптор буфера экрана
      ci,        // прямоугольник, из которого пишем
      size,      // размеры этого прямоугольника
      coord,     // и его левый угол
      &sr))      // прямоугольник, в который пишем
  {
    cout << "Write console output failed." << endl;
    return GetLastError();
  }

  return 0;
}