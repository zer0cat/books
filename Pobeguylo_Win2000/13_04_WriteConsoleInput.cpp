#include <windows.h>
#include <iostream.h>

int main() 
{
  HANDLE  hStdIn;      // для дескриптора стандартного ввода
  INPUT_RECORD  ir;    // входная запись
  DWORD  dwNumberWritten;  // количество записанных записей
  DWORD  dwNumber;     // для количества записей в буфере ввода

  // получить дескриптор стандартного ввода
  hStdIn = GetStdHandle(STD_INPUT_HANDLE);
  if (hStdIn == INVALID_HANDLE_VALUE)
  {
    cout << "Get standard input handle failed." << endl;
    return GetLastError();
  }
  // подсчитываем записи в буфере ввода
  if (!GetNumberOfConsoleInputEvents(hStdIn, &dwNumber))
  {
    cout << "Get number of console input events failed." << endl;
    return GetLastError();
  }
  // печатаем количество событий ввода
  cout << "Number of console input events = " << dwNumber << endl;
  // инициализируем запись события ввода
  ir.EventType = KEY_EVENT;
  ir.Event.KeyEvent.bKeyDown = 0x1;
  ir.Event.KeyEvent.wRepeatCount = 1;
  ir.Event.KeyEvent.wVirtualKeyCode = 0x43;
  ir.Event.KeyEvent.wVirtualScanCode = 0x2e;
  ir.Event.KeyEvent.uChar.AsciiChar = 'c';
  ir.Event.KeyEvent.dwControlKeyState =0x20;
  // записываем запись в буфер ввода
  if (!WriteConsoleInput(hStdIn, &ir, 1, &dwNumberWritten))
  {
    cout << "Write console input failed." << endl;
    return GetLastError();
  }
  cout << "Write one record into the input buffer." << endl;
  // подсчитываем записи в буфере ввода
  if (!GetNumberOfConsoleInputEvents(hStdIn, &dwNumber))
  {
    cout << "Get number of console input events failed." << endl;
    return GetLastError();
  }
  // печатаем количество событий ввода
  cout << "Number of console input events = " << dwNumber << endl;
  // очищаем входной буфер
  cout << "Flush console input buffer." << endl;
  if (!FlushConsoleInputBuffer(hStdIn))
  {
    cout << "Flush console input buffer failed." << endl;
    return GetLastError();
  }
  // подсчитываем записи в буфере ввода
  if (!GetNumberOfConsoleInputEvents(hStdIn, &dwNumber))
  {
    cout << "Get number of console input events failed." << endl;
    return GetLastError();
  }
  // печатаем количество событий ввода
  cout << "Number of console input events = " << dwNumber << endl;

  return 0;
}