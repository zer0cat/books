#include <windows.h>
 
HANDLE hStdOut, hStdIn; 

int main(void) 
{
  LPSTR lpszPrompt1  = "Input 'y' and press Enter to exit.\n"; 
  LPSTR lpszPrompt2 = "Input string and press Enter:\n";
  CHAR  chBuffer[80]; 
  DWORD cRead, cWritten;
  DWORD dwOldMode, dwNewMode; 

  // читаем дескрипторы стандартного ввода и вывода 
  hStdIn = GetStdHandle(STD_INPUT_HANDLE); 
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE); 
  if (hStdIn == INVALID_HANDLE_VALUE || hStdOut == INVALID_HANDLE_VALUE)
  {
    MessageBox(NULL, "Get standard handle failed.", "Win32 API error",
      MB_OK | MB_ICONINFORMATION);
    return GetLastError();
  }
  // читаем режимы, установленные по умолчанию
  if (!GetConsoleMode(hStdIn, &dwOldMode))
  {
    MessageBox(NULL, "Get console mode failed.", "Win32 API error",
      MB_OK | MB_ICONINFORMATION);
    return GetLastError();
  }
  // отключаем режим ENABLE_ECHO_INPUT
  dwNewMode = dwOldMode & ~ENABLE_ECHO_INPUT; 
  // устанавливаем новый режим
  if (!SetConsoleMode(hStdIn, dwNewMode))
  {
    MessageBox(NULL, "Set console mode failed.", "Win32 API error",
      MB_OK | MB_ICONINFORMATION);
    return GetLastError();
  }    
  // выводим сообщение о том, как выйти из цикла чтения 
  if (!WriteConsole( 
      hStdOut,       // дескриптор стандартного вывода 
      lpszPrompt1,   // строка, которую выводим 
      lstrlen(lpszPrompt1),  // длина строки 
      &cWritten,     // количество записанных байтов
      NULL))         // не используется
  {    
    MessageBox(NULL, "Write file failed.", "Win32 API error",
      MB_OK | MB_ICONINFORMATION);
    return GetLastError();
  }
  // цикл чтения
  for (;;) 
  {
    // выводим сообщение о вводе строки
    if (!WriteConsole(
        hStdOut,       // дескриптор стандартного вывода 
        lpszPrompt2,   // строка, которую выводим 
        lstrlen(lpszPrompt2),  // длина строки 
        &cWritten,     // количество записанных байтов
        NULL))         // не используется
    {
      MessageBox(NULL, "Write file failed.", "Win32 API error",
        MB_OK | MB_ICONINFORMATION);
      return GetLastError();
    }
    // вводим строку с клавиатуры
    if (!ReadConsole( 
        hStdIn,    // дескриптор стандартного ввода 
        chBuffer,  // буфер для чтения 
        80,        // длина буфера 
        &cRead,    // количество прочитанных байт 
        NULL))     // не используется 
    {
      MessageBox(NULL, "Read file failed.", "Win32 API error",
        MB_OK | MB_ICONINFORMATION);
      return GetLastError();
    }
    // выход из программы
    if (chBuffer[0] == 'y')
      return 1;
    // дублируем строку на экране
    if (!WriteConsole( 
        hStdOut,   // дескриптор стандартного вывода 
        chBuffer,  // строка, которую выводим 
        cRead,     // длина строки 
        &cWritten, // количество записанных байтов
        NULL))     // не используется
    {
      MessageBox(NULL, "Write file failed.", "Win32 API error",
        MB_OK | MB_ICONINFORMATION);
      return GetLastError();
    }
  }

  return 0;
}