#include <windows.h>
#include <iostream.h>

int main() 
{
  DWORD  dwNumber;     // для количества кнопок у мыши

  // подсчитываем количество кнопок у мыши
  if (!GetNumberOfConsoleMouseButtons(&dwNumber))
  {
    cout << "Get number of console mouse buttons failed." << endl;
    return GetLastError();
  }
  // выводим количество кнопок у мыши
  cout << "Number of console mouse buttons = " << dwNumber << endl;

  return 0; 
}