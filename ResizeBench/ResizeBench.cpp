// ResizeBench.cpp : �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "ResizeBench.h"

#define MAX_LOADSTRING 100
#define TARGET_RESOLUTION 1

// �O���[�o���ϐ�:
HINSTANCE hInst;								// ���݂̃C���^�[�t�F�C�X
TCHAR szTitle[MAX_LOADSTRING];					// �^�C�g�� �o�[�̃e�L�X�g
TCHAR szWindowClass[MAX_LOADSTRING];			// ���C�� �E�B���h�E �N���X��

TIMECAPS tc;
UINT     wTimerRes;
int      count;
bool     initialized;

// �A�v���P�[�V�����{��
Bench *bench;

// ���̃R�[�h ���W���[���Ɋ܂܂��֐��̐錾��]�����܂�:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void SetTimer( HWND hWnd );
HRESULT ClearTimer( void );
#if X86-64
void CALLBACK TimerProc( UINT uId, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2 );
#else
void CALLBACK TimerProc( UINT uId, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 );
#endif

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: �����ɃR�[�h��}�����Ă��������B
	MSG msg;
	HACCEL hAccelTable;

	// �O���[�o������������������Ă��܂��B
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_RESIZEBENCH, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// �A�v���P�[�V�����̏����������s���܂�:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RESIZEBENCH));

	// ���C�� ���b�Z�[�W ���[�v:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  �֐�: MyRegisterClass()
//
//  �ړI: �E�B���h�E �N���X��o�^���܂��B
//
//  �R�����g:
//
//    ���̊֐�����юg�����́A'RegisterClassEx' �֐����ǉ����ꂽ
//    Windows 95 ���O�� Win32 �V�X�e���ƌ݊�������ꍇ�ɂ̂ݕK�v�ł��B
//    �A�v���P�[�V�������A�֘A�t����ꂽ
//    �������`���̏������A�C�R�����擾�ł���悤�ɂ���ɂ́A
//    ���̊֐����Ăяo���Ă��������B
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RESIZEBENCH));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_RESIZEBENCH);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �֐�: InitInstance(HINSTANCE, int)
//
//   �ړI: �C���X�^���X �n���h����ۑ����āA���C�� �E�B���h�E���쐬���܂��B
//
//   �R�����g:
//
//        ���̊֐��ŁA�O���[�o���ϐ��ŃC���X�^���X �n���h����ۑ����A
//        ���C�� �v���O���� �E�B���h�E���쐬����ѕ\�����܂��B
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // �O���[�o���ϐ��ɃC���X�^���X�������i�[���܂��B

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 3*360+20, 60+360+20, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  �֐�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  �ړI:  ���C�� �E�B���h�E�̃��b�Z�[�W���������܂��B
//
//  WM_COMMAND	- �A�v���P�[�V���� ���j���[�̏���
//  WM_PAINT	- ���C�� �E�B���h�E�̕`��
//  WM_DESTROY	- ���~���b�Z�[�W��\�����Ė߂�
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	int a, b, c;

	switch (message)
	{
	case WM_CREATE:
		bench = new Bench( hWnd );
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �I�����ꂽ���j���[�̉��:
		switch (wmId)
		{
		case ID_BENCH_BEGIN:
			hdc = GetDC( hWnd );
			TextOut( hdc, 0, 0, L"NearestNeighbor", 15 );
			TextOut( hdc, 360, 0, L"Bilinear", 8 );
			TextOut( hdc, 720, 0, L"Bicubic", 7 );

			a = 0;
			for( int i = 0; i < 100; ++i )
			{
				SetTimer( hWnd );
				bench->Test();
				ClearTimer();
				a += count;
			}
			b = 0;
			for( int i = 0; i < 100; ++i )
			{
				SetTimer( hWnd );
				bench->Test2();
				ClearTimer();
				b += count;
			}
			c = 0;
			for( int i = 0; i < 100; ++i )
			{
				SetTimer( hWnd );
				bench->Test3();
				ClearTimer();
				c += count;
			}

			wchar_t str[100];
			swprintf( str, L"%.2f ms", a / 100.0 );
			TextOut( hdc, 0 + 280, 0, str, wcslen( str ) );
			swprintf( str, L"%.2f ms", b / 100.0 );
			TextOut( hdc, 360 + 280, 0, str, wcslen( str ) );
			swprintf( str, L"%.2f ms", c / 100.0 );
			TextOut( hdc, 720 + 280, 0, str, wcslen( str ) );
			ReleaseDC( hWnd, hdc );
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: �`��R�[�h�������ɒǉ����Ă�������...
		EndPaint(hWnd, &ps);
		break;

	case WM_CLOSE:
		ClearTimer();
		DestroyWindow( hWnd );
		break;

	case WM_DESTROY:
		delete bench;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// �o�[�W�������{�b�N�X�̃��b�Z�[�W �n���h���[�ł��B
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void SetTimer( HWND hWnd )
{
	count = 0;

	if( !initialized )
	{
		// �^�C�}�[�Z�b�g
		if( timeGetDevCaps( &tc, sizeof( TIMECAPS ) ) != TIMERR_NOERROR ) 
		{
			MessageBox( hWnd, L"Camera Studio �J�[�l���ŃG���[���������܂���\n(Error Code: !TIMERR_NOERROR)", L"�G���[", MB_OK );
			PostMessage( hWnd, WM_CLOSE, 0, 0 );
		}

		wTimerRes = min( max( tc.wPeriodMin, TARGET_RESOLUTION ), tc.wPeriodMax );

		HRESULT hr = timeSetEvent( 1, wTimerRes, TimerProc, 0, TIME_PERIODIC );

		initialized = true;
	}
	timeBeginPeriod( wTimerRes );
}


HRESULT ClearTimer( void )
{
	return timeEndPeriod( tc.wPeriodMin );
}

#if X86-64
void CALLBACK TimerProc( UINT uId, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2 )
#else
void CALLBACK TimerProc( UINT uId, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 )
#endif
{
	count++;
}