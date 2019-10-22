#include <windows.h>

HANDLE hStdOut, hStdIn; 
 
int main(void) 
{
  LPSTR lpszPrompt1 = "Input 'q' and press Enter to exit.\n"; 
  LPSTR lpszPrompt2 = "Input string and press Enter:\n";
  CHAR  chBuffer[80]; 
  DWORD cRead, cWritten;

  // читаем дескрипторы стандартного ввода и вывода
  hStdIn = GetStdHandle(STD_INPUT_HANDLE); 
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE); 
  if (hStdIn == INVALID_HANDLE_VALUE || hStdOut == INVALID_HANDLE_VALUE)
  {
    MessageBox(NULL, "Get standard handle failed", "Win32 API error",
      MB_OK | MB_ICONINFORMATION);
    return GetLastError();
  }
  // по умолчанию установлены режимы ввода: ENABLE_LINE_INPUT,
  // ENABLE_ECHO_INPUT, ENABLE_PROCESSED_INPUT 

  // выводим сообщение о том, как выйти из цикла чтения
  if (!WriteFile( 
      hStdOut,       // дескриптор стандартного вывода 
      lpszPrompt1,   // строка, которую выводим 
      lstrlen(lpszPrompt1),  // длина строки 
      &cWritten,     // количество записанных байтов
      NULL))         // синхронный вывод
    {
      MessageBox(NULL, "Write file failed", "Win32 API error",
        MB_OK | MB_ICONINFORMATION);
      return GetLastError();
    }
  // цикл чтения
  for (;;) 
  {
    // выводим сообщение о вводе строки
    if (!WriteFile( 
        hStdOut,       // дескриптор стандартного вывода 
        lpszPrompt2,   // строка, которую выводим 
        lstrlen(lpszPrompt2),  // длина строки 
        &cWritten,     // количество записанных байтов
        NULL))         // синхронный вывод
      {
        MessageBox(NULL, "Write file failed", "Win32 API error",
          MB_OK | MB_ICONINFORMATION);
        return GetLastError();
      }
    // вводим строку с клавиатуры и дублируем её на экран
    if (!ReadFile( 
        hStdIn,    // дескриптор стандартного ввода
        chBuffer,  // буфер для чтения 
        80,        // длина буфера 
        &cRead,    // количество прочитанных байтов
        NULL))     // синхронный ввод 
      {
        MessageBox(NULL, "Write file failed", "Win32 API error",
          MB_OK | MB_ICONINFORMATION);
        return GetLastError();
      }
    // выход из программы
    if (chBuffer[0] == 'q')
      return 1;
  }
  
  return 0;
}