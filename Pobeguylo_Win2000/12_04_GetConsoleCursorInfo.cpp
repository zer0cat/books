#include <windows.h>
#include <iostream.h>

int main()
{
  char  c;
  HANDLE  hStdOut;           // дескриптор стандартного вывода
  CONSOLE_CURSOR_INFO cci;   // информация о курсоре

  // читаем дескриптор стандартного вывода
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  // читаем информацию о курсоре
  if (!GetConsoleCursorInfo(hStdOut, &cci))
    cout << "Get console cursor info failed." << endl;
  // выводим информацию о курсоре
  cout << "Size of cursor in procents of char= " << cci.dwSize << endl;
  cout << "Visibility of cursor = " << cci.bVisible << endl;

  // читаем новый размер курсора
  cout << "Input a new size of cursor (1-100): ";
  cin >> cci.dwSize;
  // устанавливаем новый размер курсора
  if (!SetConsoleCursorInfo(hStdOut, &cci))
    cout << "Set console cursor info failed." << endl;

  cout << "Input any char to make the cursor invisible: ";
  cin >> c;
  // делаем курсор невидимым
  cci.bVisible = FALSE;
  // устанавливаем невидимый курсор
  if (!SetConsoleCursorInfo(hStdOut, &cci))
    cout << "Set console cursor info failed." << endl;

  cout << "Input any char to make the cursor visible: ";
  cin >> c;
  // делаем курсор невидимым
  cci.bVisible = TRUE;
  // устанавливаем видимый курсор
  if (!SetConsoleCursorInfo(hStdOut, &cci))
    cout << "Set console cursor info failed." << endl;

  return 0;
}