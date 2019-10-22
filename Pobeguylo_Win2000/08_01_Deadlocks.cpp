#include <windows.h>
#include <iostream.h>
#include <math.h>

const int size = 10;   // размерность массива
int    a[size];        // обрабатываемый массив
HANDLE  hDeadlock;     // сигнал о тупике
HANDLE  hAnswer[2];    // для обработки тупика

DWORD WINAPI marker(LPVOID)
{
  int    i;
  DWORD  dwValue; 

  for (;;)
  {
    // вычисляем случайный индекс
    i = abs(rand()) % size;
    // проверяем, занят ли элемент
    if (!a[i])
      // нет, заполняем элемент
      a[i] = 1;  
    else
    {
      // да, сигнализируем о тупике
      SetEvent(hDeadlock);
      // ждем ответа
      dwValue = WaitForMultipleObjects(
              2, hAnswer, FALSE, INFINITE);
      if (dwValue == WAIT_FAILED)
      {
        cerr << "Wait function failed." << endl;
        cerr << "Press any key to exit." << endl;
        cin.get();

        return GetLastError();
      }
      // вычисляем индекс сигнального объекта
      dwValue -= WAIT_OBJECT_0;
      switch (dwValue)
      {
      case 0:    // продолжаем работу
        continue;
      case 1:    // завершаем работу
        ExitThread(1);
        break;
      default:
        ExitThread(2);
        break;
      }
    }
  }
}

int main()
{
  HANDLE  hMarker;
  DWORD  idMarker;

  // создаем событие, оповещающее о тупике
  hDeadlock = CreateEvent(NULL, FALSE, FALSE, NULL);
  // создаем события для обработки тупика
  hAnswer[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
  hAnswer[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
  // запускаем поток marker
  hMarker = CreateThread(NULL, 0, marker,NULL, 0, &idMarker);
  if (hMarker == NULL)
    return GetLastError();
  for (;;)
  {
    char c;
    // ждем сигнал о тупике
    WaitForSingleObject(hDeadlock, INFINITE);
    // выводим на консоль текущее состояние массива
    cout << "Current state of the array: ";
    for (int i = 0; i < size; ++i)
      cout << a[i] << ' ';
    cout << endl;
    // завершать или нет поток marker?
    cout << "Input 'y' to continue: ";
    cin >> c;
    if (c == 'y')
      SetEvent(hAnswer[0]);  // продолжаем работу
    else
    {
      SetEvent(hAnswer[1]);  // завершаем работу
      break;
    }
  }

  WaitForSingleObject(hMarker, INFINITE);
  CloseHandle(hMarker);

  return 0;
}