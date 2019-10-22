#include <windows.h>
#include <iostream.h>

HANDLE  hStdIn, hStdOut; // для дескрипторов стандартного ввода и вывода
BOOL  bRead = TRUE;      // для цикла обработки событий 

// функция обработки сообщений от клавиатуры
VOID KeyEventProc(KEY_EVENT_RECORD kir)
{
  cout << "\tKey event record:" << endl;
    // просто выводим на консоль содержимое записи
  cout << "bKeyDown = " << hex << kir.bKeyDown << endl;
  cout << "wRepeatCount = " << dec << kir.wRepeatCount << endl;
  cout << "wVirtualKeyCode = " << hex << kir.wVirtualKeyCode << endl;
  cout << "wVirtualScanCode = " << kir.wVirtualScanCode << endl;
  cout << "uChar.AsciiChar = " << kir.uChar.AsciiChar << endl;
  cout << "dwControlKeyState = " << kir.dwControlKeyState << endl;

  // если ввели букву 'q', то выходим из цикла обработки событий
  if (kir.uChar.AsciiChar == 'q')
    bRead = FALSE;
}

// функция обработки сообщений от мыши
VOID MouseEventProc(MOUSE_EVENT_RECORD mer)
{
  cout << "\tMouse event record:" << endl << dec;
    // просто выводим на консоль содержимое записи
  cout << "dwMousePosition.X = " << mer.dwMousePosition.X << endl;
  cout << "dwMousePosition.Y = " << mer.dwMousePosition.Y << endl;
  cout << "dwButtonState = " << hex << mer.dwButtonState << endl;
  cout << "dwControlKeyState = " << mer.dwControlKeyState << endl;
  cout << "dwEventFlags = " << mer.dwEventFlags << endl;
}

// функция обработки сообщения об изменении размеров окна
VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr)
{
  // изменяем размеры буфера вывода
  SetConsoleScreenBufferSize(hStdOut, wbsr.dwSize);
}
  

int main() 
{
  INPUT_RECORD  ir;    // входная запись
  DWORD  cNumRead;     // для количества прочитанных записей

  // получить дескрипторы стандартного ввода и вывода
  hStdIn = GetStdHandle(STD_INPUT_HANDLE); 
  if (hStdIn == INVALID_HANDLE_VALUE)
  {
    cout << "Get standard input handle failed." << endl;
    return GetLastError();
  }
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE); 
  if (hStdOut == INVALID_HANDLE_VALUE)
  {
    cout << "Get standard output handle failed." << endl;
    return GetLastError();
  }
  // начинаем обработку событий ввода
  cout << "Begin input event queue processing." << endl;
  cout << "Input 'q' to quit."<< endl << endl;
  // цикл обработки событий ввода
  while (bRead) 
  {
    // ждем событие ввода
    WaitForSingleObject(hStdIn, INFINITE);

    // читаем запись ввода
    if (!ReadConsoleInput( 
        hStdIn,      // дескриптор ввода 
        &ir,         // буфер для записи 
        1,           // читаем одну запись 
        &cNumRead))  // количество прочитанных записей
    {
      cout << "Read console input failed." << endl;
      break;
    }

    // вызываем соответствующий обработчик
    switch(ir.EventType)
    {
      case KEY_EVENT:        // событие ввода с клавиатуры 
        KeyEventProc(ir.Event.KeyEvent); 
        break; 

      case MOUSE_EVENT:      // событие ввода с мыши
        MouseEventProc(ir.Event.MouseEvent);
        break;

      case WINDOW_BUFFER_SIZE_EVENT:   // изменения размеров окна
        ResizeEventProc(
          ir.Event.WindowBufferSizeEvent);
        break; 

      case FOCUS_EVENT:      // события фокуса ввода игнорируем
        break;

      case MENU_EVENT:       // события меню игнорируем
        break;

      default:               // неизвестное событие
        cout << "Unknown event type."; 
        break; 
    }
  }
  
  return 0; 
}