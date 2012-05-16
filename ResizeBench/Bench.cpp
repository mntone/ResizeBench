#include "StdAfx.h"
#include "Bench.h"


Bench::Bench( HWND _hWnd ):
	hWnd( _hWnd )
{
	// HDC �擾
	hdc = GetDC( hWnd );

	// �T���v���摜�p��
	i1.reset( new Rgb24Image( hWnd, 640, 480 ) );
	i2.reset( new Rgb24Image( hWnd, 480, 360 ) );

	// �A�v���P�[�V�����{�̂������X���b�h�̗D��x���グ��
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
}


Bench::~Bench(void)
{
	// HDC ���
	ReleaseDC( hWnd, hdc );
}

void Bench::Test( void )
{
	HDC deskhdc = GetDC( 0 );
	BitBlt( i1->GetImageDC(), 0, 0, 640, 480, deskhdc, 0, 0, SRCCOPY );
	ReleaseDC( 0, deskhdc );

	for( int i = 0; i < 1024; ++i )
		i2->Bilinear1( i1.get() );

	BitBlt( hdc, 0, 0, 480, 360, i2->GetImageDC(), 0, 0, SRCCOPY );

	return;
}