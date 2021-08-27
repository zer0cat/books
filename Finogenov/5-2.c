#include <time.h>
#include <windows.h>
#include <windowsx.h>

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
void OnPaint(HWND);
void OnDestroy(HWND);
BOOL OnCreate(HWND,LPCREATESTRUCT);

HPEN hRedPen,hBluePen,hGreenPen;
HBRUSH hRedBrush,hBlueBrush,hGreenBrush;


int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hprev,LPSTR lpCmdLine,int nShowCmd)
{
	char szClassName[] = "MainWindow";
	char szTitle[] = "Program 5-2";
	MSG Msg;
	WNDCLASSA wc;
	HWND hwnd;

	ZeroMemory(&wc,sizeof(wc));
	
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL,IDI_APPLICATION);	
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(200,200,100));
	wc.lpszClassName = szClassName;
	RegisterClassA(&wc);
	
	hwnd = CreateWindow(szClassName,szTitle,WS_OVERLAPPEDWINDOW,20,20,500,190,HWND_DESKTOP,NULL,hInstance,NULL);
	ShowWindow(hwnd,SW_SHOW);
	
	while(GetMessage(&Msg,NULL,0,0))
		{
		DispatchMessage(&Msg);	
		}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
	HANDLE_MSG(hwnd,WM_PAINT,OnPaint);
	HANDLE_MSG(hwnd,WM_DESTROY,OnDestroy);
	HANDLE_MSG(hwnd,WM_CREATE,OnCreate);	
	default:
		return (DefWindowProc(hwnd,msg,wParam,lParam));
	}
return 0;
}

void OnDestroy(HWND hwnd)
{
PostQuitMessage(0);
}

BOOL OnCreate(HWND hwnd,LPCREATESTRUCT lpS)
{
hRedPen = CreatePen(PS_SOLID,4,RGB(150,0,0));
hGreenPen = CreatePen(PS_SOLID,10,RGB(0,150,0));
hBluePen = CreatePen(PS_SOLID,20,RGB(0,0,150));

hRedBrush = CreateSolidBrush(RGB(255,120,120));
hGreenBrush = CreateSolidBrush(RGB(120,255,120));
hBlueBrush = CreateSolidBrush(RGB(120,120,255));

return TRUE;
}

void OnPaint(HWND hwnd)
{
PAINTSTRUCT ps;
char szText1[] = "Rectangle( )";
char szText2[] = "Ellipse( )";
char szText3[] = "Pie( )";
char szText4[] = "Chord( )";
HDC hdc = BeginPaint(hwnd, &ps);
 
SetBkMode(hdc,TRANSPARENT);
SetTextColor(hdc,RGB(0,0,200));

TextOut(hdc,25,5,szText1,lstrlenA(szText1));
TextOut(hdc,155,5,szText2,lstrlenA(szText2));
TextOut(hdc,280,5,szText3,lstrlenA(szText3));
TextOut(hdc,395,5,szText4,lstrlenA(szText4));

Rectangle(hdc,10,40,110,140);

SelectPen(hdc,hRedPen);
SelectBrush(hdc,hRedBrush);

Ellipse(hdc,130,40,230,140);

SelectPen(hdc,hGreenPen);
SelectBrush(hdc,hGreenBrush);

Pie(hdc,250,40,350,140,350,140,250,140);

SelectPen(hdc,hBluePen);
SelectBrush(hdc,hBlueBrush);

Chord(hdc,370,40,470,140,470,125,370,125);

EndPaint(hwnd, &ps); 
}