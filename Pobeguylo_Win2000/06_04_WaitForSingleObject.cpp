#include <windows.h>
#include <iostream.h>

void thread()
{
  int i;

  for (i = 0; i < 10 ; ++i)
  {
    cout << i << ' ' << flush;
    Sleep(100);
  }
  cout << endl;
}

int main()
{
  HANDLE   hThread;
  DWORD  dwThread;

  hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread, NULL, 
                         0, &dwThread);
  if (hThread == NULL)
    return GetLastError();

  // ждем завершения потока thread
  if(WaitForSingleObject(hThread, INFINITE) != WAIT_OBJECT_0)
  {
    cout << "Wait for single object failed." << endl;
    cout << "Press any key to exit." << endl;
    cin.get();
  }
  // закрываем дескриптор потока thread
  CloseHandle(hThread);

  return 0;
}