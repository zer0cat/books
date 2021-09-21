#include <windows.h>
#include <windowsx.h>

#define XMAX 600
#define YMAX 400

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
void OnCreate(HWND,LPCREATESTRUCT);
void OnPaint(HWND);
void OnDestroy(HWND);

HFONT hFont1,hFont2,hFont3,hFont4;
HBRUSH hBrush;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hprev,LPSTR lpCmdLine,int nShowCmd)
{
	char szClassName[] = "MainWindow";
	char szTitle[] = "Program 5-3";
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
	
	hwnd = CreateWindow(szClassName,szTitle,WS_OVERLAPPEDWINDOW,0,0,XMAX,YMAX,HWND_DESKTOP,NULL,hInstance,NULL);
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

BOOL OnCreate(HWND hwnd,LPCREATESTRUCT lpS)
{
char lpszFace1[] = "Time New Roman Cyr";
char lpszFace2[] = "Arial Cyr";
LOGFONT lf;
ZeroMemory(&lf,sizeof(lf));

/* font1 - caption*/
lf.lfHeight = 60;
lstrcpyA(lp.lpFaceName,lpszFace1);
hFont1 = CreateFontIndirect(&lf);

/* font1 - caption*/
lf.lfHeight = 18;
lf.lfItalic = 1;
lstrcpyA(lp.lpFaceName,lpszFace2);
hFont2 = CreateFontIndirect(&lf);

/* font1 - caption*/
lf.lfHeight = 18;
lf.lfItalic = 0;
lf.lfEscapement = 900;
hFont3 = CreateFontIndirect(&lf);

lf.lfHeight = 16;
lf.lfEscapement = 0;
hFont4 = CreateFontIndirect(&lf);

hBrush = CreateSolidBrush(RGB(0,127,127));

return TRUE;
}

void OnPaint(HWND hwnd)
{
RECT r;
PAINTSTRUCT ps;
TEXTMETRIC tm;
char szMainTitle[] = "Koledg \"Rubikon\""; 
char szSubTitle[] = "Vypusk slushateley po godam";
char szYAxes[] = "Kolichestvo vypusknikov"; 
char *szYears[10] = {"1993","1994","1995","1996","1997","1998","1999","2000","2001","2002"}
char *szScale[3] = {"100"," 50","  0"};
int nData[10] = {20,35,35,30,70,75,85,90,90,95};
HFONT hOldFont = NULL;
unsigned int i;

HDC hdc = BeginPaint(hwnd, &ps);

Rectangle(hdc,90,120,550,320); 
hOldFont = SelectFont(hdc,hFont1);

/* draw caption */
SetTextColor(hdc,RGB(128,0,0));
GetTextMetrics(hdc,&tm);
r.left = 0;
r.top = 10;
r.right = XMAX;
r.bottom = r.top + tm.tmHeight;
DrawText(hdc,szMainTitle,lstrlenA(szMainTitle),&r,DT_CENTER);

/* draw Subcaption */
SetTextColor(hdc,RGB(0,0,128));
SelectFont(hdc,hFont2);
GetTextMetrics(hdc,&tm);
r.left = 0;
r.top = r.bottom + 5;
r.right = XMAX;
r.bottom += tm.tmHeight;
DrawText(hdc,szSubTitle,lstrlenA(szSubTitle),&r,DT_CENTER);
 
/* years */
for (i = 0; i< 10; i++)
	{ TextOut(hdc,100+i*45,330,szYears[i],lstrlenA(szYears[i]));}

/* draw vertical */
SelectFont(hdc,hFont3);
SetTextColor(hdc,RGB(0,0,0)); 
TextOut(hdc,30,320,szYAxes,lstrlenA(szYAxes));
SelectFont(hdc,hFont4); 
for (i = 2; i >= 0; i--)
	{ TextOut(hdc,60,110+i*100,szScale[i],lstrlenA(szScale[i]));} 
SelectFont(hdc,hOldFont);

/* draw diagram */
SelectBrush(hdc,hBrush);
for (i = 0; i< 10; i++)
	{ Rectangle(hdc,100+i*45,320-nData[i]*2,100+i*45+30,320); }
 
EndPaint(hwnd, &ps);

}


void OnDestroy(HWND hwnd)
{
PostQuitMessage(0);
}

