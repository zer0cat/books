#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE  hThread;

  // получаем псевдодескриптор текущего потока
  hThread = GetCurrentThread();
  // выводим псевдодескриптор на консоль
  cout << hThread << endl;

  cin.get();

  return 0;
}