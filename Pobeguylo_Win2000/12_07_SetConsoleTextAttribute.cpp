#include <windows.h>
#include <iostream.h>

int main()
{
  char c;
  HANDLE  hStdout;   // дескриптор стандартного вывода
  WORD  wAttribute;  // цвет фона и текста

  cout << "In order to set text attributes, input any char: ";
  cin >> c;

  // читаем стандартный дескриптор вывода
  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

  // задаем цвет фона зеленым, а цвет символов красным
  wAttribute = BACKGROUND_GREEN | BACKGROUND_INTENSITY |
    FOREGROUND_RED | FOREGROUND_INTENSITY;
  // устанавливаем новые атрибуты
  if (!SetConsoleTextAttribute(hStdout,wAttribute))
  {
    cout << "Set console text attribute failed." << endl;
    return GetLastError();
  }

  cout << "The text attributes was changed." << endl;

  return 0;
}