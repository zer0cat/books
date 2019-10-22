#include <windows.h>
#include <conio.h>
#include <iostream.h>

int main()
{
  // события для синхронизации обмена данными
  HANDLE hReadFloat, hReadText;
  char lpszReadFloat[] = "ReadFloat";
  char lpszReadText[] = "ReadText";

  // открываем события
  hReadFloat = CreateEvent(NULL, FALSE, FALSE, lpszReadFloat);
  hReadText = CreateEvent(NULL, FALSE, FALSE, lpszReadText);

  // читаем целые числа из анонимного канала
  for (int i = 0; i < 5; ++i)
  {
    int nData;
    cin >> nData;
    _cprintf("The number %d is read from the pipe.\n", nData);
  }

  // разрешаем читать плавающие числа из анонимного канала
  SetEvent(hReadFloat);
  // пишем плавающие числа в анонимный канал
  for (int j = 0; j < 5; ++j)
  {
    Sleep(500);
    cout << (j*0.1) << endl;
  }

  // ждем разрешения на чтение текста
  WaitForSingleObject(hReadText, INFINITE);
  _cputs("The process read the text: ");
  // теперь читаем текст
  char lpszInput[80];
  do
  {
    Sleep(500);
    cin >> lpszInput;
    _cputs(lpszInput);
    _cputs(" ");
  }
  while (*lpszInput != '\0');

  _cputs("\nThe process finished transmission of data.\n");
  _cputs("Press any key to exit.\n");
  _getch();

  CloseHandle(hReadFloat);
  CloseHandle(hReadText);

  return 0;
}