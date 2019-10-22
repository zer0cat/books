#include <windows.h>
#include <iostream.h>

int main() 
{
  HANDLE  hConsoleOutput;        // для дескриптора буфера экрана
  CHAR   lpBuffer[] = "abcd";    // буфер с символами для вывода
  COORD  dwWriteCoord = {10, 10};// координаты первого элемента в буфере
  DWORD  nNumberOfCharsWritten;  // количество записанных символов

  // получаем дескриптор буфера экрана
  hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hConsoleOutput == INVALID_HANDLE_VALUE)
  {
    cout << "Get standard handle failed." << endl;
    return GetLastError();
  }
  // записываем символы в буфер экрана
  if (!WriteConsoleOutputCharacter(
      hConsoleOutput,      // дескриптор буфера экрана 
      lpBuffer,      // буфер для ввода символов
      sizeof(lpBuffer),      // количество записываемых символов
      dwWriteCoord,      // координата первого символа
      &nNumberOfCharsWritten))  // количество записанных символов
  {
    cout << "Read console output character failed." << endl;
    return GetLastError();
  }
  // выводим количество записанных символов
  cout << "Number of chars written: " << nNumberOfCharsWritten << endl;

  return 0; 
}