#include "StdAfx.h"
#include "Rgb24Image.h"

Rgb24Image::Rgb24Image( HWND _hWnd, int _width, int _height ):
	hWnd( _hWnd ),
	width( _width ),
	height( _height ),
	lpPixel( NULL )
{
	// DIB の情報を容易
	bmpi = new BITMAPINFO();
	bmpi->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bmpi->bmiHeader.biWidth = width;
	bmpi->bmiHeader.biHeight = height;
	bmpi->bmiHeader.biPlanes = 1;
	bmpi->bmiHeader.biBitCount = 24;
	bmpi->bmiHeader.biCompression = BI_RGB;

	// HDC 取得
	hdc = GetDC( hWnd );

	// DIBSection の作成
	hBitmap = CreateDIBSection( hdc, bmpi, DIB_RGB_COLORS, ( void ** )&lpPixel, NULL, 0 );
	hMemDC = CreateCompatibleDC( hdc );
	SelectObject( hMemDC, hBitmap );

	// HDC 解放
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


// ----------- 以下、画像処理関数群 ------------
//typedef struct { BYTE b, g, r; } RGB24;
//typedef RGB24 *LPRGB24;

// NearestNeighbor (素コード、準最適化)
// [todo] scaleh * h を掛け算して求めるのではなく、足し算で求めるほうがいい?
//        (e.g.) double hprop = scaleh * h; y(double) += hprop; y0 = ( int )( y + 0.5 );
void Rgb24Image::NearestNeighbor1( Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();	// src の LPBYTE のポインタ
	int sw = src->GetWidth();		// src の横幅
	int sl = 3 * sw;				// src の 1 列のビット長
	int dl = 3 * width;				// dst の 1 列のビット長

	double scalew = ( double )sw / width / 3.0;				// 拡大倍率 (1/3 倍)
	double scaleh = ( double )src->GetHeight() / height;		// 拡大倍率

	int w, h, hxdl, pl, x0, y0;		// ループ中で確保する一時変数
	for( h = 0; h < height; ++h )
	{
		hxdl = h * dl;				// (高さ) × (1 列のビット長)
		
		// src の基準点 (x0, y0) の y0 を求める
		y0 = ( int )( scaleh * h + 0.5 );

		for( w = 0; w < 3 * width; w += 3 )
		{
			// src の基準点 (x0, y0) の x0 を求める
			x0 = ( int )( scalew * w + 0.5 );

			// src の範囲外の点を範囲内に引き戻す
			if( x0 < 0 ) x0 = 0;
			else if( x0 > src->GetWidth() - 1 ) x0 = src->GetWidth() - 1;
			if( y0 < 0 ) y0 = 0;
			else if( y0 > src->GetHeight() - 1 ) y0 = src->GetHeight() - 1;

			// src の基準点までのビット長算出
			pl = y0 * sl + 3 * x0;

			// src の基準点 (x0, y0) を dst の点 (w, h) にコピー
			lpPixel[hxdl + w    ] = ( *sp )[pl    ];
			lpPixel[hxdl + w + 1] = ( *sp )[pl + 1];
			lpPixel[hxdl + w + 2] = ( *sp )[pl + 2];
		}
	}
}

// テスト段階。まだ動きません。バグるｗ
void Rgb24Image::Bilinear1( Rgb24Image *src )
{
	BYTE colorbuf[4];

	LPBYTE *sp = src->GetPixel();
	int sw = src->GetWidth();
	int sl = 3 * sw;
	int dl = 3 * width;

	double scalew = ( double )sw / width;
	double scaleh = ( double )src->GetHeight() / height;

	for( int h = 0; h < height; ++h )
		for( int w = 0; w < width; ++w )
		{
			double x = scalew * w;
			double y = scaleh * h;
			int x0 = ( int )( x + 0.5 );
			int y0 = ( int )( y + 0.5 );
			x -= x0;
			y -= y0;

			if( x0 < 0 ) x0 = 0;
			else if( x0 > src->GetWidth() - 2 ) x0 = src->GetWidth() - 2;
			if( y0 < 0 ) y0 = 0;
			else if( y0 > src->GetHeight() - 2 ) y0 = src->GetHeight() - 2;
       
            for( int i = 0; i < 3; ++i )
			{
				colorbuf[0] = ( *sp )[( y0     ) * sl + 3 * ( x0     ) + i];
				colorbuf[1] = ( *sp )[( y0     ) * sl + 3 * ( x0 + 1 ) + i];
				colorbuf[2] = ( *sp )[( y0 + 1 ) * sl + 3 * ( x0     ) + i];
				colorbuf[3] = ( *sp )[( y0 + 1 ) * sl + 3 * ( x0 + 1 ) + i];
                                
				lpPixel[h * dl + 3 * w + i] = ( int )( ( 1.0 - x ) * ( 1.0 - y ) * colorbuf[0] + ( 1.0 - x ) * y * colorbuf[1] + x * ( 1.0 - y ) * colorbuf[2] + x * y * colorbuf[3] + 0.5 );
            }		
		}
}