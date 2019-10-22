#include <windows.h>
#include <iostream.h>

int main()
{
  char c;
  HANDLE  hStdOut;     // дескриптор стандартного вывода
  WORD    wAttribute;  // цвет фона и текста
  DWORD  dwLength;     // количество заполняемых клеток
  DWORD  dwWritten;    // для колическтва заполенных клеток
  COORD    coord;      // координаты первой клетки
  CONSOLE_SCREEN_BUFFER_INFO csbi;   // для параметров буфера экрана
  
  cout << "In order to fill console attributes, input any char: ";
  cin >> c;
  // читаем стандартный дескриптор вывода
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hStdOut == INVALID_HANDLE_VALUE)
  {
    cout << "Get standard handle failed." << endl;
    return GetLastError();
  }
  // читаем параметры выходного буфера
  if (!GetConsoleScreenBufferInfo(hStdOut, &csbi))
  {
    cout << "Console screen buffer info failed." << endl;
    return GetLastError();
  }
  
  // вычисляем размер буфера экрана в символах
  dwLength = csbi.dwSize.X * csbi.dwSize.Y;
    // начинаем заполнять буфер с первой клетки
  coord.X = 0;
  coord.Y = 0;
  // устанавливаем цвет фона голубым, а цвет символов желтым
  wAttribute = BACKGROUND_BLUE | BACKGROUND_INTENSITY |
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  // заполняем буфер атрибутами
  if (!FillConsoleOutputAttribute(
    hStdOut,     // стандартный дескриптор вывода
    wAttribute,  // цвет фона и текста 
    dwLength,    // длина буфера в символах
    coord,       // индекс первой клетки
    &dwWritten)) // количество заполненных клеток
  {
    cout << "Fill console output attribute failed." << endl;
    return GetLastError();
  }
  
  cout << "The fill attributes was changed." << endl;
  
  return 0;
}