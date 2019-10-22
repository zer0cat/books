#include <windows.h>
#include <iostream.h>

volatile int a[10];

DWORD WINAPI thread(LPVOID)
{
  for (int i = 0; i < 10; i++)
  {
    a[i] = i + 1;
    Sleep(7);
  }

  return 0;
}

int main()
{
  int i;
  HANDLE  hThread;
  DWORD  IDThread;
 
  cout << "An initial state of the array: ";
  for (i = 0; i < 10; i++)
    cout << a[i] <<' ';
  cout << endl;
    
  // создаем поток, который готовит элементы массива
  hThread = CreateThread(NULL, 0, thread, NULL, 0, &IDThread);
  if (hThread == NULL)
    return GetLastError();

  // поток main выводит элементы массива 
  cout << "A modified state of the array: ";
  for (i = 0; i < 10; i++)
  {
    cout << a[i] << ' ' << flush;
    Sleep(11);
  }
  cout << endl;

  CloseHandle(hThread);

  return 0;
}