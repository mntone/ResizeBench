#include "StdAfx.h"
#include "Bench.h"


Bench::Bench( HWND _hWnd ):
	hWnd( _hWnd )
{
	// HDC 取得
	hdc = GetDC( hWnd );

	// サンプル画像用意
	i1.reset( new Rgb24Image( hWnd, 640, 480 ) );
	i2.reset( new Rgb24Image( hWnd, 480, 360 ) );

	// アプリケーション本体が動くスレッドの優先度を上げる
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
}


Bench::~Bench(void)
{
	// HDC 解放
	ReleaseDC( hWnd, hdc );
	hdc = NULL;
	hWnd = NULL;
}

void Bench::Test( void )
{
	HDC deskhdc = GetDC( 0 );
	BitBlt( i1->GetImageDC(), 0, 0, 640, 480, deskhdc, 0, 0, SRCCOPY );
	ReleaseDC( 0, deskhdc );

	i2->NearestNeighbor1( i1.get() );

	BitBlt( hdc, 0, 20, 480, 360, i2->GetImageDC(), 0, 0, SRCCOPY );

	return;
}

void Bench::Test2( void )
{
	HDC deskhdc = GetDC( 0 );
	BitBlt( i1->GetImageDC(), 0, 0, 640, 480, deskhdc, 0, 0, SRCCOPY );
	ReleaseDC( 0, deskhdc );

	i2->Bilinear1( i1.get() );

	BitBlt( hdc, 360, 20, 480, 360, i2->GetImageDC(), 0, 0, SRCCOPY );

	return;
}

void Bench::Test3( void )
{
	HDC deskhdc = GetDC( 0 );
	BitBlt( i1->GetImageDC(), 0, 0, 640, 480, deskhdc, 0, 0, SRCCOPY );
	ReleaseDC( 0, deskhdc );

	i2->Bicubic1( i1.get() );

	BitBlt( hdc, 720, 20, 480, 360, i2->GetImageDC(), 0, 0, SRCCOPY );

	return;
}