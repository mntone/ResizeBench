#include "StdAfx.h"
#include "Rgb24Image.h"

Rgb24Image::Rgb24Image( HWND _hWnd, int _width, int _height ):
	hWnd( _hWnd ),
	width( _width ),
	height( _height ),
	lpPixel( NULL )
{
	// DIB �̏���p��
	bmpi = new BITMAPINFO();
	bmpi->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bmpi->bmiHeader.biWidth = width;
	bmpi->bmiHeader.biHeight = height;
	bmpi->bmiHeader.biPlanes = 1;
	bmpi->bmiHeader.biBitCount = 24;
	bmpi->bmiHeader.biCompression = BI_RGB;

	// HDC �擾
	hdc = GetDC( hWnd );

	// DIBSection �̍쐬
	hBitmap = CreateDIBSection( hdc, bmpi, DIB_RGB_COLORS, ( void ** )&lpPixel, NULL, 0 );
	hMemDC = CreateCompatibleDC( hdc );
	SelectObject( hMemDC, hBitmap );

	// HDC ���
	ReleaseDC( hWnd, hdc );
	hdc = NULL;
	hWnd = NULL;
}


Rgb24Image::~Rgb24Image( void )
{
	if( hMemDC )
	{
		DeleteDC( hMemDC );
		hMemDC = NULL;
	}

	if( hBitmap )
	{
		DeleteObject( hBitmap );
		hBitmap = NULL;
	}

	delete bmpi;

	delete[] lpPixel;
	lpPixel = NULL;
}


int Rgb24Image::GetWidth( void )
{
	return width;
}


int Rgb24Image::GetHeight( void )
{
	return height;
}


HDC Rgb24Image::GetImageDC( void )
{
	return hMemDC;
}


LPBYTE *Rgb24Image::GetPixel( void )
{
	return &lpPixel;
}


// ----------- �ȉ��A�摜�����֐��Q ------------
//typedef struct { BYTE b, g, r; } RGB24;
//typedef RGB24 *LPRGB24;

// NearestNeighbor (�f�R�[�h�A���œK��)
// [todo] scaleh * h ���|���Z���ċ��߂�̂ł͂Ȃ��A�����Z�ŋ��߂�ق�������?
//        (e.g.) double hprop = scaleh * h; y(double) += hprop; y0 = ( int )( y + 0.5 );
void Rgb24Image::NearestNeighbor1( Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sl = 3 * sw;					// src �� 1 ��̃r�b�g��
	int sh = src->GetHeight();			// src �̍���
	int dl = 3 * width;					// dst �� 1 ��̃r�b�g��

	double scalew = ( double )sw / width / 3.0;		// �g��{�� (1/3 �{)
	double scaleh = ( double )sh / height;			// �g��{��

	int w, h, hxdl, y0xsl, pl, x0, y0;	// ���[�v���Ŋm�ۂ���ꎞ�ϐ�
	for( h = 0; h < height; ++h )
	{		
		// src �̊�_ (x0, y0) �� y0 �����߂�
		y0 = ( int )( scaleh * h + 0.5 );

		// src ��̓_ y0 ��͈͊O�̓_��͈͓��Ɉ����߂�
		if( y0 >= sh )
			y0 = sh - 1;

		hxdl = h * dl;					// (����) �~ (1 ��̃r�b�g��)
		y0xsl = y0 * sl;

		for( w = 0; w < dl; w += 3 )
		{
			// src �̊�_ (x0, y0) �� x0 �����߂�
			x0 = ( int )( scalew * w + 0.5 );
			
			// src ��̓_ x0 ��͈͊O�̓_��͈͓��Ɉ����߂�
			if( x0 >= sw )
				x0 = sw - 1;

			// src �̊�_�܂ł̃r�b�g���Z�o
			pl = y0xsl + 3 * x0;

			// src �̊�_ (x0, y0) �� dst �̓_ (w, h) �ɃR�s�[
			lpPixel[hxdl + w    ] = ( *sp )[pl    ];
			lpPixel[hxdl + w + 1] = ( *sp )[pl + 1];
			lpPixel[hxdl + w + 2] = ( *sp )[pl + 2];
		}
	}
}

// Bilinear (�f�R�[�h�A���œK��)
void Rgb24Image::Bilinear1( Rgb24Image *src )
{
	BYTE colorbuf[4];					// �J���[�o�b�t�@
	
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sl = 3 * sw;					// src �� 1 ��̃r�b�g��
	int sh = src->GetHeight();			// src �̍���
	int dl = 3 * width;					// dst �� 1 ��̃r�b�g��

	double scalew = ( double )sw / width;
	double scaleh = ( double )sh / height;

	int x0, y0;
	double x, y;
	for( int h = 0; h < height; ++h )
	{
		// src �̊�ꏊ (x, y) �� y �����߂�
		y = scaleh * h;

		// src �̊�_ (x0, y0) �� y0 �����߂�
		y0 = ( int )y;

		// src ��̓_ y0 ��͈͊O�̓_��͈͓��Ɉ����߂�
		if( y0 > sh - 2 )
			y0 = sh - 2;

		// src �̊�_����̊�ꏊ (��x, ��y) �� ��y �����߂�
		y -= y0;

		for( int w = 0; w < width; ++w )
		{
			// src �̊�ꏊ (x, y) �� x �����߂�
			x = scalew * w;

			// src �̊�_ (x0, y0) �� x0 �����߂�
			x0 = ( int )x;

			// src ��̓_ x0 ��͈͊O�̓_��͈͓��Ɉ����߂�
			if( x0 > sw - 2 )
				x0 = sw - 2;
			
			// src �̊�_����̊�ꏊ (��x, ��y) �� ��x �����߂�
			x -= x0;

			// TODO: propTL, propTR, propBL, propBR �ϐ��ŕۑ����Ă���
            for( int i = 0; i < 3; ++i )
			{
				colorbuf[0] = ( *sp )[( y0     ) * sl + 3 * ( x0     ) + i];
				colorbuf[1] = ( *sp )[( y0     ) * sl + 3 * ( x0 + 1 ) + i];
				colorbuf[2] = ( *sp )[( y0 + 1 ) * sl + 3 * ( x0     ) + i];
				colorbuf[3] = ( *sp )[( y0 + 1 ) * sl + 3 * ( x0 + 1 ) + i];
                                
				lpPixel[h * dl + 3 * w + i] = ( BYTE )( ( 1.0 - x ) * ( 1.0 - y ) * colorbuf[0]	+ x * ( 1.0 - y ) * colorbuf[1]
					+ ( 1.0 - x ) * y * colorbuf[2] + x * y * colorbuf[3] + 0.5 );
			}		
		}
	}
}