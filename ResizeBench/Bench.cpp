#include "StdAfx.h"
#include "Bench.h"


Bench::Bench( HWND hWnd ):
	hWnd_( hWnd )
{
	// HDC �擾
	hdc_ = GetDC( hWnd_ );

	// �T���v���摜�p��
	i1_.reset( new Rgb24Image( hWnd_, 640, 480 ) );
	i2_.reset( new Rgb24Image( hWnd_, 480, 360 ) );
	i3_.reset( new Rgb24Image( hWnd_, 480, 360 ) );

	// �A�v���P�[�V�����{�̂������X���b�h�̗D��x���グ��
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

	// �f�X�N�g�b�v�� HDC �擾
	deskhdc_ = GetDC( 0 );
}


Bench::~Bench( void )
{
	// �f�X�N�g�b�v�� HDC ���
	ReleaseDC( 0, deskhdc_ );
	deskhdc_ = NULL;

	// HDC ���
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