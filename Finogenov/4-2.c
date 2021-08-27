#include <windows.h>
#include <windowsx.h>

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
void OnDestroy(HWND);

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hprev,LPSTR lpCmdLine,int nShowCmd)
{
	char szClassName[] = "MainWindow";
	char szTitle[] = "Program 4-2";
	MSG Msg;
	WNDCLASSA wc;
	HWND hwnd;

	ZeroMemory(&wc,sizeof(wc));
	
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL,IDI_ASTERISK);	
	wc.hCursor = LoadCursor(NULL,IDC_CROSS);
	wc.hbrBackground = GetStockBrush(LTGRAY_BRUSH);
	wc.lpszClassName = szClassName;
	RegisterClassA(&wc);
	
	hwnd = CreateWindow(szClassName,szTitle,WS_OVERLAPPEDWINDOW,10,10,300,100,HWND_DESKTOP,NULL,hInstance,NULL);
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
	HANDLE_MSG(hwnd,WM_DESTROY,OnDestroy);
		
	default:
		return (DefWindowProc(hwnd,msg,wParam,lParam));
	}
return 0;
}

void OnDestroy(HWND hwnd)
{
PostQuitMessage(0);
}