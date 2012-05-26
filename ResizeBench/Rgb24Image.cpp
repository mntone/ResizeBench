#include "StdAfx.h"
#include "Rgb24Image.h"

Rgb24Image::Rgb24Image( HWND hWnd, int width, int height ):
	hWnd_( hWnd ),
	width_( width ),
	height_( height ),
	lpPixel_( NULL ),
	bmpi_( NULL )
{
	// DIB �̏���p��
	bmpi_ = new BITMAPINFO();
	bmpi_->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bmpi_->bmiHeader.biWidth = width;
	bmpi_->bmiHeader.biHeight = height;
	bmpi_->bmiHeader.biPlanes = 1;
	bmpi_->bmiHeader.biBitCount = 24;
	bmpi_->bmiHeader.biCompression = BI_RGB;

	// HDC �擾
	hdc_ = GetDC( hWnd_ );

	// DIBSection �̍쐬
	hBitmap_ = CreateDIBSection( hdc_, bmpi_, DIB_RGB_COLORS, reinterpret_cast<void **>( &lpPixel_ ), NULL, 0 );
	hMemDC_ = CreateCompatibleDC( hdc_ );
	SelectObject( hMemDC_, hBitmap_ );

	// HDC ���
	ReleaseDC( hWnd, hdc_ );
	hdc_ = NULL;
	hWnd_ = NULL;
}


Rgb24Image::~Rgb24Image( void )
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


// ----------- �ȉ��A�摜�����֐��Q ------------

// Copy
bool Rgb24Image::Copy( Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sh = src->GetHeight();			// src �̍���

	// �T�C�Y���Ⴄ���`�F�b�N
	if( sw != width_ || sh != height_ )
		return false;

	// �R�s�[
	memcpy( lpPixel_, *sp, 3 * width_ * height_ );

	return true;
}

// Trim
bool Rgb24Image::Trim( RECT rect, Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sl = 3 * sw;					// src �� 1 ��̃r�b�g��
	int sh = src->GetHeight();			// src �̍���
	int dl = 3 * width_;				// dst �� 1 ��̃r�b�g��
	int ll = 3 * rect.left;				// ���������� trim �̎n�_�܂ł̃r�b�g��

	// �T�C�Y���Ⴄ���`�F�b�N
	if( sw < rect.right || sh < rect.bottom || rect.right != width_ + rect.left || rect.bottom != height_ + rect.top  )
		return false;

	for( int h = 0; h < height_; ++h )
		memcpy( lpPixel_ + h * dl, *sp + ( rect.top + h ) * sl + ll, dl );

	return true;
}

// NearestNeighbor
// [todo] scaleh * h ���|���Z���ċ��߂�̂ł͂Ȃ��A�����Z�ŋ��߂�ق�������?
//        (e.g.) double hprop = scaleh * h; y(double) += hprop; y0 = ( int )( y + 0.5 );
void Rgb24Image::NearestNeighbor( Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sl = 3 * sw;					// src �� 1 ��̃r�b�g��
	int sh = src->GetHeight();			// src �̍���
	int dl = 3 * width_;				// dst �� 1 ��̃r�b�g��

	double scalew = static_cast<double>( sw ) / static_cast<double>( width_ ) / 3.0;	// �g��{�� (1/3 �{)
	double scaleh = static_cast<double>( sh ) / static_cast<double>( height_ );			// �g��{��

	int w, h, hxdl, y0xsl, x0, y0;		// ���[�v���Ŋm�ۂ���ꎞ�ϐ�
	for( h = 0; h < height_; ++h )
	{		
		// src �̊�_ (x0, y0) �� y0 �����߂�
		y0 = static_cast<int>( scaleh * h + 0.5 );

		// src ��̓_ y0 ��͈͊O�̓_��͈͓��Ɉ����߂�
		if( y0 >= sh )
			y0 = sh - 1;

		hxdl = h * dl;					// (����) �~ (1 ��̃r�b�g��)
		y0xsl = y0 * sl;

		for( w = 0; w < dl; w += 3 )
		{
			// src �̊�_ (x0, y0) �� x0 �����߂�
			x0 = static_cast<int>( scalew * w + 0.5 );
			
			// src ��̓_ x0 ��͈͊O�̓_��͈͓��Ɉ����߂�
			if( x0 >= sw )
				x0 = sw - 1;
			
			// src �̊�_ (x0, y0) �� dst �̓_ (w, h) �ɃR�s�[
			memcpy( lpPixel_ + hxdl + w, *sp + y0xsl + 3 * x0, 3 );
		}
	}
}


// Bilinear
//   1. s00, s01, s10, s11 �Ɨ\�߃|�C���^�̍��W���v�Z�����ꍇ�A
//      �������A�N�Z�X�񐔂����������āA�]�v�x���Ȃ邩���H
//      �i���c�m�o�b�e���[�쓮�� 15 sec �� 16 sec ���x�j
//      �������A�N�Z�X���x�� DDR2 ���ゾ�Ƃ���ɍ����J�������B
void Rgb24Image::Bilinear( Rgb24Image *src )
{
	BYTE colorbuf[4];					// �J���[�o�b�t�@
	
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sl = 3 * sw;					// src �� 1 ��̃r�b�g��
	int sh = src->GetHeight();			// src �̍���
	int dl = 3 * width_;				// dst �� 1 ��̃r�b�g��
	
	double scalew = static_cast<double>( sw ) / static_cast<double>( width_ ) / 3.0;	// �g��{�� (1/3 �{)
	double scaleh = static_cast<double>( sh ) / static_cast<double>( height_ );			// �g��{��

	int w, h, i, x0, y0;
	double x, y;
	for( h = 0; h < height_; ++h )
	{
		// src �̊�ꏊ (x, y) �� y �����߂�
		y = scaleh * h;

		// src �̊�_ (x0, y0) �� y0 �����߂�
		y0 = static_cast<int>( y );

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
			x0 = static_cast<int>( x );

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
                                
				lpPixel_[h * dl + w + i] = ( BYTE )( ( 1.0 - x ) * ( 1.0 - y ) * colorbuf[0] + x * ( 1.0 - y ) * colorbuf[1]
					+ ( 1.0 - x ) * y * colorbuf[2] + x * y * colorbuf[3] + 0.5 );
			}
		}
	}
}


// Bicubic
void Rgb24Image::Bicubic( Rgb24Image *src )
{
	double colorbuf[3];					// �J���[�o�b�t�@
	
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sl = 3 * sw;					// src �� 1 ��̃r�b�g��
	int sh = src->GetHeight();			// src �̍���
	int dl = 3 * width_;				// dst �� 1 ��̃r�b�g��
	
	double scalew = static_cast<double>( sw ) / static_cast<double>( width_ ) / 3.0;	// �g��{�� (1/3 �{)
	double scaleh = static_cast<double>( sh ) / static_cast<double>( height_ );			// �g��{��

	int w, h, i, j, x0, y0, mx, my;
	double x, y, wx, wy, dx, dy;
	for( h = 0; h < height_; ++h )
	{
		// src �̊�ꏊ (x, y) �� y �����߂�
		y = scaleh * h;

		// src �̊�_ (x0, y0) �� y0 �����߂�
		y0 = static_cast<int>( y );

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
			x0 = static_cast<int>( x );

			// src ��̓_ x0 ��͈͊O�̓_��͈͓��Ɉ����߂�
			if( x0 < 1 )
				x0 = 1;
			else if( x0 > sw - 3 )
				x0 = sw - 3;
			
			// �J���[�o�b�t�@���Z�b�g
			colorbuf[0] = colorbuf[1] = colorbuf[2] = 0.0;
			
			// ��_���璼�߂� 16 �_�ŏd�݌v�Z
			for( j = -1; j <= 2; ++j )
			{
				// ��舵���_ (mx, my) �� my ���v�Z
				// ��ꏊ����̋����� ��y �����߂�
				my = y0 + j;
				dy = static_cast<double>( my ) - y;

				// ��Βl
				if( dy < 0.0 )
					dy = -dy;

				// y �����̏d�݌v�Z
				if( dy <= 1.0 )
					wy = 1.0 - 2.0 * dy * dy + dy * dy * dy;
				else if( dy <= 2.0 )
					wy = 4.0 - 8.0 * dy + 5.0 * dy * dy - dy * dy * dy;
				else
					continue;

				for( i = -1; i <= 2; ++i )
				{
					// ��舵���_ (mx, my) �� mx ���v�Z
					// ��ꏊ����̋����� ��x �����߂�
					mx = x0 + i;
					dx = static_cast<double>( mx ) - x;
					
					// ��Βl
					if( dx < 0.0 )
						dx = -dx;

					// x �����̏d�݌v�Z
					if( dx <= 1.0 )
						wx = wy * ( 1.0 - 2.0 * dx * dx + dx * dx * dx );
					else if( dx <= 2.0 )
						wx = wy * ( 4.0 - 8.0 * dx + 5.0 * dx * dx - dx * dx * dx );
					else
						continue;

					// �J���[�o�b�t�@�[�ɑ���
					colorbuf[0] += wx * static_cast<double>( ( *sp )[my * sl + 3 * mx    ] );
					colorbuf[1] += wx * static_cast<double>( ( *sp )[my * sl + 3 * mx + 1] );
					colorbuf[2] += wx * static_cast<double>( ( *sp )[my * sl + 3 * mx + 2] );
				}
			}
			
			// �I�[�o�[�������̂�߂� (�l�̌ܓ��l���ς�)
			if( colorbuf[0] < 0.0 )
				colorbuf[0] = 0.0;
			else if( colorbuf[0] > 254.9 )
				colorbuf[0] = 254.9;

			if( colorbuf[1] < 0.0 )
				colorbuf[1] = 0.0;
			else if( colorbuf[1] > 254.9 )
				colorbuf[1] = 254.9;
			
			if( colorbuf[2] < 0.0 )
				colorbuf[2] = 0.0;
			else if( colorbuf[2] > 254.9 )
				colorbuf[2] = 254.9;

			// �i�[
			lpPixel_[h * dl + w    ] = static_cast<int>( colorbuf[0] + 0.5 );
			lpPixel_[h * dl + w + 1] = static_cast<int>( colorbuf[1] + 0.5 );
			lpPixel_[h * dl + w + 2] = static_cast<int>( colorbuf[2] + 0.5 );
		}
	}
}


// FlipXY
// flipX = false AND flipY = false �̂Ƃ� Copy ���s
bool Rgb24Image::FilpXY( bool flipX, bool flipY, Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int dl = 3 * width_;				// dst �� 1 ��̃r�b�g��

	// �T�C�Y���Ⴄ���`�F�b�N
	if( src->GetWidth() != width_ || src->GetHeight() != height_ )
		return false;

	// �������x�����������邽�߂ɁA���[�v�O�ŏ�������
	int w, h;

	// �㉺���E���]
	if( flipX && flipY )
		for( h = 0; h < height_; ++h )
			for( w = 0; w < dl; w += 3 )
				memcpy( lpPixel_ + h * dl + w, *sp + ( height_ - h - 1 ) * dl + ( dl - w - 3 ), 3 );

	// ���E���]
	else if( flipX )
		for( h = 0; h < height_; ++h )
			for( w = 0; w < dl; w += 3 )
				memcpy( lpPixel_ + h * dl + w, *sp + h * dl + ( dl - w - 3 ), 3 );

	// �㉺���]
	else if( flipY )
		for( h = 0; h < height_; ++h )
			memcpy( lpPixel_ + h * dl, *sp + ( height_ - h - 1 ) * dl, dl );

	// �ǂ̃t���O�����Ă��Ȃ���΁A�R�s�[
	else
		return Copy( src );

	return true;
}


// Rotate90
bool Rgb24Image::Rotate90( Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sl = 3 * sw;					// src �� 1 ��̃r�b�g��
	int sh = src->GetHeight();			// src �̍���
	int dl = 3 * width_;				// dst �� 1 ��̃r�b�g��

	// �T�C�Y���Ⴄ���`�F�b�N
	if( sw != height_ || sh != width_ )
		return false;

	int w, h;
	for( h = 0; h < height_; ++h )
		for( w = 0; w < width_; ++w )
			memcpy( lpPixel_ + h * dl + 3 * w, *sp + w * sl + 3 * ( height_ - h - 1 ), 3 );

	return true;
}


// Inverse Negative-Positive
bool Rgb24Image::InvNegaPosi( Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int dl = 3 * width_;				// dst �� 1 ��̃r�b�g��

	// �T�C�Y���Ⴄ���`�F�b�N
	if( src->GetWidth() != width_ || src->GetHeight() != height_ )
		return false;

	int w, h;
	for( h = 0; h < height_; ++h )
		for( w = 0; w < dl; w += 3 )
		{
			lpPixel_[h * dl + w    ] = ~( *sp )[h * dl + w    ];
			lpPixel_[h * dl + w + 1] = ~( *sp )[h * dl + w + 1];
			lpPixel_[h * dl + w + 2] = ~( *sp )[h * dl + w + 2];
		}

	return true;
}


// Mozaic
bool Rgb24Image::Mozaic( int level, Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sh = src->GetHeight();			// src �̍���
	int dl = 3 * width_;				// dst �� 1 ��̃r�b�g��
	int ll = 3 * level;					// level �̃r�b�g��

	// �T�C�Y���Ⴄ���`�F�b�N
	if( sw != width_ || sh != height_ )
		return false;

	// ���x���� 1 ��菬�Ȃ�R�s�[
	if( level <= 1 )
		return Copy( src );

	BYTE colorbuf[2];
	int w, h, i, j;
	for( h = 0; h < height_; h += level )
		for( w = 0; w < dl; w += level * 3 )
		{
			colorbuf[0] = ( *sp )[h * dl + w    ];
			colorbuf[1] = ( *sp )[h * dl + w + 1];
			colorbuf[2] = ( *sp )[h * dl + w + 2];

			for( j = 0; j < level; ++j )
			{
				// �͈͊O�`�F�b�N
				if( h + j > height_ )
					continue;

				for( i = 0; i < ll; i += 3 )
				{
					// �͈͊O�`�F�b�N
					if( w + i > dl )
						continue;

					lpPixel_[( h + j ) * dl + w + i    ] = colorbuf[0];
					lpPixel_[( h + j ) * dl + w + i + 1] = colorbuf[1];
					lpPixel_[( h + j ) * dl + w + i + 2] = colorbuf[2];
				}
			}
		}

	return true;
}


// Blur
bool Rgb24Image::Blur( int level, Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src �� LPBYTE �̃|�C���^
	int sw = src->GetWidth();			// src �̕�
	int sh = src->GetHeight();			// src �̍���
	int dl = 3 * width_;				// dst �� 1 ��̃r�b�g��
	int ll = 3 * level;					// level �̃r�b�g��

	// �T�C�Y���Ⴄ���`�F�b�N
	if( sw != width_ || sh != height_ || level < 1 )
		return false;

	int w, h, i, j;
	double colorbuf[2], p;
	for( h = 0; h < height_; ++h )
		for( w = 0; w < dl; w += 3 )
		{
			p = 0.0;
			colorbuf[0] = colorbuf[1] = colorbuf[2] = 0.0;
			for( j = -level; j < level; ++j )
			{
				// �͈͊O�`�F�b�N
				if( h + j < 0 || h + j > height_ )
					continue;

				for( i = -ll; i < ll; i += 3 )
				{
					// �͈͊O�`�F�b�N
					if( w + i < 0 || w + i > dl || abs( i * j / 3 ) >= level )
						continue;

					// �������킹��
					colorbuf[0] += ( *sp )[( h + j ) * dl + w + i    ];
					colorbuf[1] += ( *sp )[( h + j ) * dl + w + i + 1];
					colorbuf[2] += ( *sp )[( h + j ) * dl + w + i + 2];
					p += 1.0;
				}
			}
			
			// ���ʂ��o��
			if( p != 0.0 )
			{
				lpPixel_[h * dl + w    ] = static_cast<BYTE>( colorbuf[0] / p );
				lpPixel_[h * dl + w + 1] = static_cast<BYTE>( colorbuf[1] / p );
				lpPixel_[h * dl + w + 2] = static_cast<BYTE>( colorbuf[2] / p );
			}
		}

	return true;
}

