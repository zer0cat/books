#include <windows.h>
#include <stdio.h>

extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow (); 

int main()
{
  HWND hWindow = NULL;     // дескриптор окна
  HDC hDeviceContext;      // контекст устройства
  HPEN hPen;               // дескриптор пера
  HGDIOBJ hObject;         // дескриптор GDI объекта
    
  // получаем дескриптор окна
  hWindow = GetConsoleWindow();

  if (hWindow == NULL)
  {
    printf("Get console window failed.\n");

    return 1;
  }
  else
    printf("Cet console window is done.\n");

  // получаем контекст устройства
  hDeviceContext = GetDC(hWindow);
  // создаем перо
  hPen = CreatePen(PS_SOLID, 10, RGB(0, 255, 0));
  // устанавливает перо
  hObject = SelectObject(hDeviceContext, hPen);

  // рисуем линию
  MoveToEx(hDeviceContext, 100, 100, NULL);
  LineTo(hDeviceContext, 500, 100);

  // востанавливает старый объект
  SelectObject(hDeviceContext, hObject);

  // освобождаем объекты
  DeleteObject(hPen);
  ReleaseDC(hWindow, hDeviceContext);

  return 0;
}