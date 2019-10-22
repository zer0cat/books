#include <windows.h>
#include <iostream.h>

int main()
{
  char c;
  HANDLE  hStdOut;   // дескриптор стандартного вывода
  DWORD  dwLength;   // количество заполняемых клеток
  DWORD  dwWritten;  // для колическтва заполенных клеток
  COORD  coord;      // координаты первой клетки
  CONSOLE_SCREEN_BUFFER_INFO csbi;   // для параметров буфера экрана

  // читаем дескриптор стандартного вывода
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
  // устанавливаем координаты первой клетки
  coord.X = 0;
  coord.Y = 0; 
  // вводим символ-заполнитель
  cout << "Input any char to fill screen buffer: ";
  cin >> c;
  // заполняем буфер экрана символом-заполнителем
  if (!FillConsoleOutputCharacter(
      hStdOut,     // стандартный дескриптор вывода
      c,           // символ заполнения 
      dwLength,    // длина буфера в символах
      coord,       // индекс первой клетки
      &dwWritten)) // количество заполненных клеток
  {
    cout << "Fill console output character failed." << endl;
    return GetLastError();
  }
  // ждем команды на очищение буфера экрана
  cout << "In order to clear screen buffer, press any char: ";
  cin >> c;
  // очищаем буфер экрана пробелами
  if (!FillConsoleOutputCharacter(
      hStdOut,     // стандартный дескриптор вывода
      ' ',         // символ заполнения 
      dwLength,    // длина буфера в символах
      coord,       // индекс первой клетки
      &dwWritten)) // количество заполненных клеток
  {
    cout << "Fill console output character failed." << endl;
    return GetLastError();
  }
  
  return 0;
}