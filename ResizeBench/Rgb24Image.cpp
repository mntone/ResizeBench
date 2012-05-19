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
	//hWnd = NULL;
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

	//delete[] lpPixel;
	//lpPixel = NULL;
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
//   1. s00, s01, s10, s11 �Ɨ\�߃|�C���^�̍��W���v�Z�����ꍇ�A
//      �������A�N�Z�X�񐔂����������āA�]�v�x���Ȃ邩���H
//      �i���c�m�o�b�e���[�쓮�� 15 sec �� 16 sec ���x�j
//      �������A�N�Z�X���x�� DDR2 ���ゾ�Ƃ���ɍ����J�������B
void Rgb24Image::Bilinear1( Rgb24Image *src )
{
	BYTE colorbuf[4];					// �J���[�o�b�t�@
	
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sl = 3 * sw;					// src �� 1 ��̃r�b�g��
	int sh = src->GetHeight();			// src �̍���
	int dl = 3 * width;					// dst �� 1 ��̃r�b�g��
	
	double scalew = ( double )sw / width / 3.0;		// �g��{�� (1/3 �{)
	double scaleh = ( double )sh / height;			// �g��{��

	int w, h, i, x0, y0;
	double x, y;
	for( h = 0; h < height; ++h )
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

		for( w = 0; w < dl; w += 3 )
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

			// �e�_���R�s�[
			for( i = 0; i < 3; ++i )
			{
				colorbuf[0] = ( *sp )[( y0     ) * sl + 3 * ( x0     ) + i];
				colorbuf[1] = ( *sp )[( y0     ) * sl + 3 * ( x0 + 1 ) + i];
				colorbuf[2] = ( *sp )[( y0 + 1 ) * sl + 3 * ( x0     ) + i];
				colorbuf[3] = ( *sp )[( y0 + 1 ) * sl + 3 * ( x0 + 1 ) + i];
                                
				lpPixel[h * dl + w + i] = ( BYTE )( ( 1.0 - x ) * ( 1.0 - y ) * colorbuf[0]	+ x * ( 1.0 - y ) * colorbuf[1]
					+ ( 1.0 - x ) * y * colorbuf[2] + x * y * colorbuf[3] + 0.5 );
			}
		}
	}
}


// Bicubic (�f�R�[�h�A�œK���Ȃ�)
void Rgb24Image::Bicubic1( Rgb24Image *src )
{
	double colorbuf[3];					// �J���[�o�b�t�@
	
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sl = 3 * sw;					// src �� 1 ��̃r�b�g��
	int sh = src->GetHeight();			// src �̍���
	int dl = 3 * width;					// dst �� 1 ��̃r�b�g��
	
	double scalew = ( double )sw / width / 3.0;		// �g��{�� (1/3 �{)
	double scaleh = ( double )sh / height;			// �g��{��

	int w, h, i, j, x0, y0, mx, my;
	double x, y, wx, wy, dx, dy;
	for( h = 0; h < height; ++h )
	{
		// src �̊�ꏊ (x, y) �� y �����߂�
		y = scaleh * h;

		// src �̊�_ (x0, y0) �� y0 �����߂�
		y0 = ( int )y;

		// src ��̓_ y0 ��͈͊O�̓_��͈͓��Ɉ����߂�
		if( y0 < 1 )
			y0 = 1;
		else if( y0 > sh - 3 )
			y0 = sh - 3;

		for( w = 0; w < dl; w += 3 )
		{
			// src �̊�ꏊ (x, y) �� x �����߂�
			x = scalew * w;

			// src �̊�_ (x0, y0) �� x0 �����߂�
			x0 = ( int )x;

			// src ��̓_ x0 ��͈͊O�̓_��͈͓��Ɉ����߂�
			if( x0 < 1 )
				x0 = 1;
			else if( x0 > sw - 3 )
				x0 = sw - 3;
			
			// �J���[�o�b�t�@���Z�b�g
			colorbuf[0] = colorbuf[1] = colorbuf[2] = 0.0;
			
			for( j = -1; j <= 2; ++j )
			{
				my = y0 + j;
				dy = ( double )my - y;

				if( dy < 0.0 )
					dy = -dy;

				if( dy <= 1.0 )
					wy = 1.0 - 2.0 * dy * dy + dy * dy * dy;
				else if( dy <= 2.0 )
					wy = 4.0 - 8.0 * dy + 5.0 * dy * dy - dy * dy * dy;
				else
					continue;

				for( i = -1; i <= 2; ++i )
				{
					mx = x0 + i;
					dx = ( double )mx - x;
					
					if( dx < 0.0 )
						dx = -dx;

					if( dx <= 1.0 )
						wx = wy * ( 1.0 - 2.0 * dx * dx + dx * dx * dx );
					else if( dx <= 2.0 )
						wx = wy * ( 4.0 - 8.0 * dx + 5.0 * dx * dx - dx * dx * dx );
					else
						continue;

					colorbuf[0] += wx * ( double )( ( *sp )[my * sl + 3 * mx    ] );
					colorbuf[1] += wx * ( double )( ( *sp )[my * sl + 3 * mx + 1] );
					colorbuf[2] += wx * ( double )( ( *sp )[my * sl + 3 * mx + 2] );

			//wchar_t str[256];
			//swprintf( str, L"weight: %.2f, wy: %.2f, x: %.2f, y: %.2f, dx: %.2f, dy: %.2f, c: %d", wx, wy, x, y, dx, dy, ( *sp )[my * sl + 3 * mx    ] );
			//MessageBox( hWnd, str, L"INFO", MB_OK );
				}
			}

			//wchar_t str[256];
			//swprintf( str, L"cb[0]: %.3f, cb[1]:  %.3f, cb[2]: %.3f", colorbuf[0], colorbuf[1], colorbuf[2] );
			//MessageBox( hWnd, str, L"INFO", MB_OK );
			
			if( colorbuf[0] < 0.0 )
				colorbuf[0] = 0.0;
			else if( colorbuf[0] > 255.0 )
				colorbuf[0] = 255.0;

			if( colorbuf[1] < 0.0 )
				colorbuf[1] = 0.0;
			else if( colorbuf[1] > 255.0 )
				colorbuf[1] = 255.0;
			
			if( colorbuf[2] < 0.0 )
				colorbuf[2] = 0.0;
			else if( colorbuf[2] > 255.0 )
				colorbuf[2] = 255.0;

			lpPixel[h * dl + w    ] = ( int )( colorbuf[0] + 0.5 );
			lpPixel[h * dl + w + 1] = ( int )( colorbuf[1] + 0.5 );
			lpPixel[h * dl + w + 2] = ( int )( colorbuf[2] + 0.5 );
		}
	}
}