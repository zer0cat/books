#include <windows.h>
#include <iostream.h>

DWORD WINAPI thread(LPVOID)
{
  int i,j;
  
  for (j = 0; j < 10; ++j)
  {
    // выводим строку чисел j
    for (i = 0; i < 10; ++i)
    {
      cout << j << ' ' << flush;
      Sleep(17);
    }
    cout << endl;
  }
    
  return 0;
}

int main()
{
  int i,j;
  HANDLE  hThread;
  DWORD  IDThread;

  hThread=CreateThread(NULL, 0, thread, NULL, 0, &IDThread);
  if (hThread == NULL)
    return GetLastError();

for (j = 10; j < 20; ++j)
  {
    for (i = 0; i < 10; ++i)
    {
      // выводим строку чисел j
      cout << j << ' ' << flush;
      Sleep(17);
    }
    cout << endl;
  }
  // ждем, пока поток thread закончит свою работу
  WaitForSingleObject(hThread, INFINITE);

  return 0;
}