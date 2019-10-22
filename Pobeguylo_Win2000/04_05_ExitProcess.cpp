#include <windows.h>
#include <iostream.h>

volatile UINT count;

void thread()
{
  for (;;)
  {
    count++;
    Sleep(100);
  }
}

int main()
{
  char c;
  HANDLE   hThread;
  DWORD  IDThread;

  hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread, NULL, 
                          0, &IDThread);
  if (hThread == NULL)
    return GetLastError();

  for (;;)
  {    
    cout << "Input 'y' to display the count or any char to exit: ";
    cin >> (char)c;
    if (c == 'y')
      cout << "count = " << count << endl;
    else
      ExitProcess(1);
  }
}