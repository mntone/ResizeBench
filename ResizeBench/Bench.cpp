#include "StdAfx.h"
#include "Bench.h"


Bench::Bench( HWND hWnd ):
	hWnd_( hWnd )
{
	// HDC 取得
	hdc_ = GetDC( hWnd_ );

	// サンプル画像用意
	i1_.reset( new Rgb24Image( hWnd_, 640, 480 ) );
	i2_.reset( new Rgb24Image( hWnd_, 480, 360 ) );
	i3_.reset( new Rgb24Image( hWnd_, 480, 360 ) );

	// アプリケーション本体が動くスレッドの優先度を上げる
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

	// デスクトップの HDC 取得
	deskhdc_ = GetDC( 0 );
}


Bench::~Bench( void )
{
	// デスクトップの HDC 解放
	ReleaseDC( 0, deskhdc_ );
	deskhdc_ = NULL;

	// HDC 解放
	ReleaseDC( hWnd_, hdc_ );
	hdc_ = NULL;
	hWnd_ = NULL;
}

void Bench::Test( void )
{
	BitBlt( i1_->GetImageDC(), 0, 0, 640, 480, deskhdc_, 0, 0, SRCCOPY );

	i2_->NearestNeighbor( i1_.get() );

	BitBlt( hdc_, 0, 20, 480, 360, i2_->GetImageDC(), 0, 0, SRCCOPY );

	return;
}

void Bench::Test2( void )
{
	BitBlt( i1_->GetImageDC(), 0, 0, 640, 480, deskhdc_, 0, 0, SRCCOPY );
	
	i2_->Bilinear( i1_.get() );

	BitBlt( hdc_, 360, 20, 480, 360, i2_->GetImageDC(), 0, 0, SRCCOPY );

	return;
}

void Bench::Test3( void )
{
	BitBlt( i1_->GetImageDC(), 0, 0, 640, 480, deskhdc_, 0, 0, SRCCOPY );

	i2_->Bicubic( i1_.get() );

	BitBlt( hdc_, 720, 20, 480, 360, i2_->GetImageDC(), 0, 0, SRCCOPY );

	return;
}