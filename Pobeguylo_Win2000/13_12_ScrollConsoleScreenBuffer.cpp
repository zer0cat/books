#include <windows.h>
 
HANDLE hStdOut, hStdIn;

// функция перехода на новую строку в буфере экрана
int GoToNewLine(void)
{
  CONSOLE_SCREEN_BUFFER_INFO csbi;   // информация о буфере экрана
  SMALL_RECT    srScroll;      // перемещаемый прямоугольник
  SMALL_RECT    srClip;        // рассматриваемая область
  COORD      coord;            // новое положение
  CHAR_INFO    ci;             // символ заполнитель

  // читаем информацию о буфере экрана
  if (!GetConsoleScreenBufferInfo(hStdOut, &csbi))
  {
    MessageBox(NULL, "Get console screen buffer info failed.", "Win32 API error",
      MB_OK | MB_ICONINFORMATION);
    return 0;
  }    
  // переходим на первый столбец
  csbi.dwCursorPosition.X = 0; 
  // если это не последняя строка,
  if ((csbi.dwCursorPosition.Y+1) < csbi.dwSize.Y)
    // то переводим курсор на следующую строку 
    csbi.dwCursorPosition.Y += 1;
  // иначе, прокручиваем буфер экрана
  else
  {
    // координаты прямоугольника, который прокручиваем
    srScroll.Left = 0;
    srScroll.Top = 1;
    srScroll.Right = csbi.dwSize.X;
    srScroll.Bottom = csbi.dwSize.Y;
    // координаты прямоугольника буфера экрана 
    srClip.Left = 0;
    srClip.Top = 0;
    srClip.Right = csbi.dwSize.X;
    srClip.Bottom = csbi.dwSize.Y;
    // устанавливаем новые координаты левого угла прямоугольника srScroll
    coord.X = 0;
    coord.Y = 0;
    // устанавливаем атрибуты и символ заполнитель для последней строки
    ci.Attributes = csbi.wAttributes;
    ci.Char.AsciiChar = ' ';
    // прокручиваем прямоугольник srScroll
    if (!ScrollConsoleScreenBuffer(
        hStdOut,     // дескриптор стандартного вывода
        &srScroll,   // прокручиваемый прямоугольник
        &srClip,     // буфер экрана
        coord,       // начало буфера экрана
        &ci))        // атрибуты и символ заполнитель
    {
      MessageBox(NULL, "Set console window info failed.", "Win32 API error",
        MB_OK | MB_ICONINFORMATION);
      return 0;
    }
  }
  // теперь устанавливаем курсор  
  if (!SetConsoleCursorPosition(hStdOut, csbi.dwCursorPosition))
  {
    MessageBox(NULL, "Set console cursor position failed.", "Win32 API error",
      MB_OK | MB_ICONINFORMATION);
    return 0;
  }

  return 0;
}

int main(void) 
{
  LPSTR  lpszPrompt = "Press 'y' to exit.\n"; 
  CHAR  c; 
  DWORD  cRead, cWritten;
  DWORD  dwOldMode, dwNewMode; 

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
  // отключаем режиы ENABLE_LINE_INPUT и ENABLE_ECHO_INPUT
  dwNewMode=dwOldMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT); 
  // устанавливаем новый режим
  if (!SetConsoleMode(hStdIn, dwNewMode))
  {
    MessageBox(NULL, "Set console mode failed.", "Win32 API error",
      MB_OK | MB_ICONINFORMATION);
    return GetLastError();
  }    
  // выводим сообщение о том, как выйти из цикла чтения 
  if (!WriteFile( 
      hStdOut,      // дескриптор стандартного вывода 
      lpszPrompt,    // строка, которую выводим 
      lstrlen(lpszPrompt),  // длина строки 
      &cWritten,    // количество записанных байт 
      NULL))      // синхроный вывод
  {
    MessageBox(NULL, "Write file failed.", "Win32 API error",
      MB_OK | MB_ICONINFORMATION);
    return GetLastError();
  }
  // цикл чтения
  for ( ; ; )
  {
    // читаем следующий символ
    if (!ReadFile(hStdIn, &c, 1, &cRead, NULL))
    {
      MessageBox(NULL, "Write file failed.", "Win32 API error",
        MB_OK | MB_ICONINFORMATION);
      return GetLastError();
    }
    // выбор действия
    switch (c)
    {
    // переход на новую строку
    case '\r':
      if (!GoToNewLine())
        {
          MessageBox(NULL, "Go to a new line failed.", "Win32 API error",
            MB_OK | MB_ICONINFORMATION);
          return GetLastError();
        }
      break;
    // выход из программы
    case 'y':
      return 1;
    // распечатываем введенный символ
    default:
      if (!WriteFile(hStdOut, &c, cRead, &cWritten, NULL))
      {
        MessageBox(NULL, "Write file failed.", "Win32 API error",
          MB_OK | MB_ICONINFORMATION);
        return GetLastError();
      }
    }
  } 

  return 0;
}