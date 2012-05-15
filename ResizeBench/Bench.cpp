#include "StdAfx.h"
#include "Bench.h"


Bench::Bench( HWND _hWnd ):
	hWnd( _hWnd )
{
	hdc = GetDC( hWnd );

	i1 = new Rgb24Image( hWnd, 640, 480 );
	i2 = new Rgb24Image( hWnd, 480, 360 );

	// アプリケーション本体が動くスレッドの優先度を上げる
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
}


Bench::~Bench(void)
{
}

void Bench::Test( void )
{
	HDC deskhdc = GetDC( 0 );
	BitBlt( i1->GetImageDC(), 0, 0, 640, 480, deskhdc, 0, 0, SRCCOPY );
	ReleaseDC( 0, deskhdc );

	for( int i = 0; i < 1024; ++i )
		i2->Bilinear1( i1 );

	BitBlt( hdc, 0, 0, 480, 360, i2->GetImageDC(), 0, 0, SRCCOPY );
	
	//delete i1;
	//delete i2;

	return;
}