#include <windows.h>
#include <iostream.h>

int main()
{
  int  i,j;

  for (j = 10; j < 20; ++j)
  {
    for (i = 0; i < 10; ++i)
    {
      cout << j << ' ' << flush;
      Sleep(5);
    }
    cout << endl;
  }
    
  return 0;
}
