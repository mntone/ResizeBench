#include "HdcImage.h"

CHdcImage::CHdcImage( int width, int height ):
	width_( width ),
	height_( height ),
	lpPixel_( NULL ),
	hMemDC_( NULL ),
	hBitmap_( NULL ),
	bmpi_( NULL )
{
	// ���N���X�Ŋm�ۂ����������̉��
	delete lpPixel_;
	lpPixel_ = NULL;

	// DIB �̏���p��
	bmpi_ = new BITMAPINFO();
	bmpi_->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bmpi_->bmiHeader.biWidth = width;
	bmpi_->bmiHeader.biHeight = height;
	bmpi_->bmiHeader.biPlanes = 1;
	bmpi_->bmiHeader.biBitCount = 24;
	bmpi_->bmiHeader.biCompression = BI_RGB;

	// HDC �擾
	HDC hdc = GetDC( 0 );

	// DIBSection �̍쐬
	hBitmap_ = CreateDIBSection( hdc, bmpi_, DIB_RGB_COLORS, reinterpret_cast<void **>( &lpPixel_ ), NULL, 0 );
	hMemDC_ = CreateCompatibleDC( hdc );
	SelectObject( hMemDC_, hBitmap_ );

	// HDC ���
	ReleaseDC( 0, hdc );
}


CHdcImage::~CHdcImage( void )
{
	if( hMemDC_ )
	{
		DeleteDC( hMemDC_ );
		hMemDC_ = NULL;
	}

	if( hBitmap_ )
	{
		DeleteObject( hBitmap_ );
		hBitmap_ = NULL;
	}

	if( bmpi_ )
	{
		delete bmpi_;
		bmpi_ = NULL;
	}
}


// Copy
bool CHdcImage::Copy( IImage *src )
{
	const LPBYTE sp = src->GetPixel();	// src �� LPBYTE �̃|�C���^

	// �T�C�Y���Ⴄ���`�F�b�N
	if( src->GetWidth() != width_ || src->GetHeight() != height_ )
		return false;

	// �R�s�[
	memcpy( lpPixel_, sp, 3 * width_ * height_ );

	return true;
}
