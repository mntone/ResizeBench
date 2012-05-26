#include "StdAfx.h"
#include "Bench.h"


Bench::Bench( HWND hWnd_ ):
	hWnd( hWnd_ )
{
	// HDC 取得
	hdc = GetDC( hWnd );

	// サンプル画像用意
	i1.reset( new Rgb24Image( hWnd, 640, 480 ) );
	i2.reset( new Rgb24Image( hWnd, 480, 360 ) );
	i3.reset( new Rgb24Image( hWnd, 480, 360 ) );

	// アプリケーション本体が動くスレッドの優先度を上げる
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

	// デスクトップの HDC 取得
	deskhdc = GetDC( 0 );
}


Bench::~Bench( void )
{
	// デスクトップの HDC 解放
	ReleaseDC( 0, deskhdc );
	deskhdc = NULL;

	// HDC 解放
	ReleaseDC( hWnd, hdc );
	hdc = NULL;
	hWnd = NULL;
}

void Bench::Test( void )
{
	BitBlt( i1->GetImageDC(), 0, 0, 640, 480, deskhdc, 0, 0, SRCCOPY );

	i2->NearestNeighbor( i1.get() );
	i3->Mozaic( 3, i2.get() );

	BitBlt( hdc, 0, 20, 480, 360, i3->GetImageDC(), 0, 0, SRCCOPY );

	return;
}

void Bench::Test2( void )
{
	BitBlt( i1->GetImageDC(), 0, 0, 640, 480, deskhdc, 0, 0, SRCCOPY );
	
	i2->Bilinear1( i1.get() );

	BitBlt( hdc, 360, 20, 480, 360, i2->GetImageDC(), 0, 0, SRCCOPY );

	return;
}

void Bench::Test3( void )
{
	BitBlt( i1->GetImageDC(), 0, 0, 640, 480, deskhdc, 0, 0, SRCCOPY );

	i2->Bicubic1( i1.get() );

	BitBlt( hdc, 720, 20, 480, 360, i2->GetImageDC(), 0, 0, SRCCOPY );

	return;
}