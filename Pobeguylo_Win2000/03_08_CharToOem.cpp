#include <windows.h>
#include <iostream.h>

int main()
{

  char big[] = "ÀÁÂÃÄÅ¨ÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞ‗";
  char sml[] = "אבגדהו¸זחטיךכלםמןנסעףפץצקרשתûü‎‏ÿ";

  CharToOem(big,big);
  CharToOem(sml,sml);

  cout << big << endl;
  cout << sml << endl;

  return 0;
}