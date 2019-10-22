#include <windows.h>
#include <iostream.h>

volatile int a[10];
HANDLE hSemaphore;

DWORD WINAPI thread(LPVOID)
{
  for (int i = 0; i < 10; i++)
  {
    a[i] = i + 1;
    // отмечаем, что один элемент готов
    ReleaseSemaphore(hSemaphore,1,NULL);
    Sleep(500);
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
  // создаем семафор    
  hSemaphore=CreateSemaphore(NULL, 0, 10, NULL);
  if (hSemaphore == NULL)
    return GetLastError();
    
  // создаем поток, который готовит элементы массива
  hThread = CreateThread(NULL, 0, thread, NULL, 0, &IDThread);
  if (hThread == NULL)
    return GetLastError();

  // поток main выводит элементы массива 
  // только после их подготовки потоком thread
  cout << "A final state of the array: ";
  for (i = 0; i < 10; i++)
  {
    WaitForSingleObject(hSemaphore, INFINITE);
    cout << a[i] << " \a" << flush;
  }
  cout << endl;

  CloseHandle(hSemaphore);
  CloseHandle(hThread);

  return 0;
}