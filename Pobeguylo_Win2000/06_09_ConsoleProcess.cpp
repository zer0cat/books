#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE  hMutex;
  int    i,j;

  // открываем мьютекс
  hMutex = OpenMutex(SYNCHRONIZE, FALSE, "DemoMutex");
  if (hMutex == NULL)
  {
    cout << "Open mutex failed." << endl;
    cout << "Press any key to exit." << endl;
    cin.get();

    return GetLastError();
  }

  for (j = 10; j < 20; j++)
  {
    // захватываем мьютекс
    WaitForSingleObject(hMutex, INFINITE);
    for (i = 0; i < 10; i++)
    {
      cout << j << ' ' << flush;
      Sleep(5);
    }
    cout << endl;
    // освобождаем мьютекс
    ReleaseMutex(hMutex);
  }
  // закрываем дескриптор объекта
  CloseHandle(hMutex);
    
  return 0;
}
