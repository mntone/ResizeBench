#include "StdAfx.h"
#include "Bench.h"


CBench::CBench( HWND hWnd ):
	hWnd_( hWnd )
{
	// HDC �擾
	hdc_ = GetDC( hWnd_ );

	// �T���v���摜�p��
	i1c_.reset( new CHdcImage( 800, 600 ) );
	i1r_.reset( new CProcessingImage( 480, 360 ) );
	buf.reset( new CHdcImage( 480, 360 ) );

	// �A�v���P�[�V�����{�̂������X���b�h�̗D��x���グ��
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

	// �f�X�N�g�b�v�� HDC �擾
	deskhdc_ = GetDC( 0 );
}


CBench::~CBench( void )
{
	// �f�X�N�g�b�v�� HDC ���
	ReleaseDC( 0, deskhdc_ );
	deskhdc_ = NULL;

	// HDC ���
	ReleaseDC( hWnd_, hdc_ );
	hdc_ = NULL;
	hWnd_ = NULL;
}

void CBench::Test( void )
{
	BitBlt( i1c_->GetImageDC(), 0, 0, 800, 600, deskhdc_, 0, 0, SRCCOPY );

	i1r_->NearestNeighbor( i1c_.get() );
	buf->Copy( i1r_.get() );

	BitBlt( hdc_, 0, 20, 480, 360, buf->GetImageDC(), 0, 0, SRCCOPY );

	return;
}

void CBench::Test2( void )
{
	BitBlt( i1c_->GetImageDC(), 0, 0, 800, 600, deskhdc_, 0, 0, SRCCOPY );

	i1r_->Bilinear( i1c_.get() );
	buf->Copy( i1r_.get() );

	BitBlt( hdc_, 360, 20, 480, 360, buf->GetImageDC(), 0, 0, SRCCOPY );

	return;
}

void CBench::Test3( void )
{
	BitBlt( i1c_->GetImageDC(), 0, 0, 800, 600, deskhdc_, 0, 0, SRCCOPY );

	i1r_->Bicubic( i1c_.get() );
	buf->Copy( i1r_.get() );

	BitBlt( hdc_, 720, 20, 480, 360, buf->GetImageDC(), 0, 0, SRCCOPY );

	return;
}