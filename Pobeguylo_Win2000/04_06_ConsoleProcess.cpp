#include <windows.h>
#include <iostream.h>

int count;

void main()
{
  for ( ; ; )
  {
    count++;
    Sleep(1000);
    cout << "count = " << count << endl;
  }
}