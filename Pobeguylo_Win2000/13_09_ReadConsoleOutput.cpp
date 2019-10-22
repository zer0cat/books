#include <windows.h>
#include <iostream.h>

int main() 
{
  HANDLE    hConsoleOutput;    // для дескриптора буфера экрана
  CHAR_INFO  lpBuffer[4];      // буфер для ввода
  COORD    dwBufferSize = {2, 2};    // размеры буфера
  COORD    dwBufferCoord = {0, 0};   // координаты первого элемента в буфере
  SMALL_RECT  ReadRegion = {0, 0, 1, 1};   // прямоугольник, который читаем

  // выводим символы, которые будем читать
  cout << 'a' << 'b' << endl << 'c' << 'd' << endl;
  // получаем дескриптор ввода
  hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hConsoleOutput == INVALID_HANDLE_VALUE)
  {
    cout << "Get standard handle failed." << endl;
    return GetLastError();
  }
  // читаем символы
  if (!ReadConsoleOutput(hConsoleOutput, lpBuffer, dwBufferSize, dwBufferCoord, &ReadRegion))
  {
    cout << "Read console input failed." << endl;
    return GetLastError();
  }
  // распечатываем прочитанные символы
  cout << "Read cells." << hex << endl;
  for (int i = 0; i < 4; ++i)
    cout << lpBuffer[i].Attributes << ' ' << lpBuffer[i].Char.AsciiChar << endl;

  return 0;
}