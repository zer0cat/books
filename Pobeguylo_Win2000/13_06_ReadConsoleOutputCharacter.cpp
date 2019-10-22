#include <windows.h>
#include <iostream.h>

int main() 
{
  HANDLE  hConsoleOutput;    // для дескриптора буфера экрана
  CHAR    lpBuffer[80];    // буфер для ввода
  COORD    dwReadCoord = {0, 0};  // координаты первого элемента в буфере
  DWORD  nNumberOfCharsRead;  // количество прочитанных символов

  // получаем дескриптор буфера экрана
  hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hConsoleOutput == INVALID_HANDLE_VALUE)
  {
    cout << "Get standard handle failed." << endl;
    return GetLastError();
  }
  // выводим те символы, которые будем читать
  cout << 'a' << 'b' << endl;
  // читаем эти символы в буфер
  if (!ReadConsoleOutputCharacter(
      hConsoleOutput,  // дескриптор буфера экрана 
      lpBuffer,        // буфер для ввода символов
      2,               // количество читаемых символов
      dwReadCoord,     // координата первого символа
      &nNumberOfCharsRead))  // количество прочитанных символов
  {
    cout << "Read consoleoutput character failed." << endl;
    return GetLastError();
  }
    // выводим количество прочитанных символов и сами символы
  cout << "Number of chars read: " << nNumberOfCharsRead << endl;
  cout << "Read chars: " << lpBuffer[0] << lpBuffer[1] << endl;

  return 0; 
}