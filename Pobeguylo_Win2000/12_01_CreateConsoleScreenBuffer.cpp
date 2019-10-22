#include <windows.h>
#include <conio.h>

int main()
{
  HANDLE  hStdOutOld, hStdOutNew;  // дескрипторы буфера экрана
  DWORD  dwWritten;                // для количества выведенных символов

  // создаем буфер экрана
  hStdOutNew = CreateConsoleScreenBuffer(
    GENERIC_READ | GENERIC_WRITE,  // чтение и запись
    0,         // не разделяемый
    NULL,      // защита по умолчанию
    CONSOLE_TEXTMODE_BUFFER,       // текстовый режим
    NULL);     // не используется

  if (hStdOutNew == INVALID_HANDLE_VALUE)
  {
    _cputs("Create console screen buffer failed.\n");
    return GetLastError();
  }
  // сохраняем старый буфер экрана
  hStdOutOld = GetStdHandle(STD_OUTPUT_HANDLE);
    // ждем команду на переход к новому буферу экрана
  _cputs("Press any key to set new screen buffer active.\n");
  _getch();

  // делаем активным новый буфер экрана
  if (!SetConsoleActiveScreenBuffer(hStdOutNew))
  {
    _cputs("Set new console active screen buffer failed.\n");
    return GetLastError();
  }

  // выводим текст в новый буфер экрана
  char  text[] = "This is a new screen buffer.";
  if (!WriteConsole(
      hStdOutNew,    // дескриптор буфера экрана
      text,          // символы, которые выводим
      sizeof(text),  // длина текста
      &dwWritten,    // количество выведенных символов
      NULL))         // не используется
    _cputs("Write console output character failed.\n");

  // выводим сообщение о вводе символа
  char  str[] = "\nPress any key to set old screen buffer.";
  if (!WriteConsole(
      hStdOutNew,    // дескриптор буфера экрана
      str,           // символы, которые выводим
      sizeof(str),   // длина текста
      &dwWritten,    // количество выведенных символов
      NULL))         // не используется
    _cputs("Write console output character failed.\n");

  _getch();

  // восстанавливаем старый буфер экрана
  if (!SetConsoleActiveScreenBuffer(hStdOutOld))
  {
    _cputs("Set old console active screen buffer failed.\n");
    return GetLastError();
  }
  // пишем в старый буфер экрана
  _cputs("This is an old console screen buffer.\n");

  // закрываем новый буфер экрана
  CloseHandle(hStdOutNew);
  // ждем команду на завершение программы
  _cputs("Press any key to finish.\n");
  _getch();

  return 0;
}