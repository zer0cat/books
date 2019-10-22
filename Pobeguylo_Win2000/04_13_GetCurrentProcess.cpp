#include <windows.h>
#include <iostream.h>

int main()
{
  HANDLE  hProcess;

  // получаем псевдодескриптор текущего процесса
  hProcess = GetCurrentProcess();
  // выводим псевдодескриптор на консоль
  cout << hProcess << endl;

  cin.get();

  return 0;
}