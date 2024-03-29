// ResizeBench.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "ResizeBench.h"

const int TARGET_RESOLUTION = 1;

// グローバル変数:
HINSTANCE hInst;								// 現在のインターフェイス

TIMECAPS tc;
UINT     wTimerRes;
int      count;
bool     initialized;

// アプリケーション本体
CBench *bench;

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM				MyRegisterClass( HINSTANCE hInstance );
BOOL				InitInstance( HINSTANCE, int );
LRESULT CALLBACK	WndProc( HWND, UINT, WPARAM, LPARAM );

void SetTimer( HWND hWnd );
HRESULT ClearTimer( void );
#if X86-64
void CALLBACK TimerProc( UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR );
#else
void CALLBACK TimerProc( UINT, UINT, DWORD, DWORD, DWORD );
#endif

int APIENTRY _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

	// Direct2D の初期化
	if( FAILED( CoInitialize( NULL ) ) )
		return FALSE;

	MSG msg;
	HACCEL hAccelTable;

	// グローバル文字列を初期化しています。
	MyRegisterClass( hInstance );

	// アプリケーションの初期化を実行します:
	if( !InitInstance( hInstance, nCmdShow ) )
		return FALSE;

	hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_RESIZEBENCH ) );

	// メイン メッセージ ループ:
	while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		if( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	// D2D の解放
	CoUninitialize();

	return static_cast<int>( msg.wParam );
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
//  コメント:
//
//    この関数および使い方は、'RegisterClassEx' 関数が追加された
//    Windows 95 より前の Win32 システムと互換させる場合にのみ必要です。
//    アプリケーションが、関連付けられた
//    正しい形式の小さいアイコンを取得できるようにするには、
//    この関数を呼び出してください。
//
ATOM MyRegisterClass( HINSTANCE hInstance )
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof( WNDCLASSEX );

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon( hInstance, MAKEINTRESOURCE( IDI_RESIZEBENCH ) );
	wcex.hCursor		= LoadCursor( NULL, IDC_ARROW);
	wcex.hbrBackground	= reinterpret_cast<HBRUSH>( COLOR_WINDOW + 1 );
	wcex.lpszMenuName	= MAKEINTRESOURCE( IDC_RESIZEBENCH );
	wcex.lpszClassName	= L"ResizeBench";
	wcex.hIconSm		= LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_SMALL ) );

	return RegisterClassEx( &wcex );
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
	HWND hWnd;

	hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

	hWnd = CreateWindow( L"ResizeBench", L"Resize Bench", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 3*360+20, 60+360+20, NULL, NULL, hInstance, NULL );

	if( !hWnd )
		return FALSE;

	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );
	
	return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:  メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	int wmId, wmEvent;
	HDC hdc;
	int a, b, c;

	switch( message )
	{
	case WM_CREATE:
		bench = new CBench( hWnd );
		break;

	case WM_COMMAND:
		wmId    = LOWORD( wParam );
		wmEvent = HIWORD( wParam );
		// 選択されたメニューの解析:
		switch( wmId )
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
			swprintf_s( str, L"%.2f ms", a / 100.0 );
			TextOut( hdc, 0 + 280, 0, str, wcslen( str ) );
			swprintf_s( str, L"%.2f ms", b / 100.0 );
			TextOut( hdc, 360 + 280, 0, str, wcslen( str ) );
			swprintf_s( str, L"%.2f ms", c / 100.0 );
			TextOut( hdc, 720 + 280, 0, str, wcslen( str ) );
			ReleaseDC( hWnd, hdc );
			break;

		case IDM_EXIT:
			DestroyWindow( hWnd );
			break;

		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		break;

	case WM_DESTROY:
		delete bench;
		PostQuitMessage( 0 );
		break;

	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return 0;
}


void SetTimer( HWND hWnd )
{
	count = 0;

	if( !initialized )
	{
		// タイマーセット
		if( timeGetDevCaps( &tc, sizeof( TIMECAPS ) ) != TIMERR_NOERROR ) 
		{
			MessageBox( hWnd, L"Camera Studio カーネルでエラーが発生しました\n(Error Code: !TIMERR_NOERROR)", L"エラー", MB_OK );
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