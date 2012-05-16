#include "StdAfx.h"
#include "Rgb24Image.h"

Rgb24Image::Rgb24Image( HWND _hWnd, int _width, int _height ):
	hWnd( _hWnd ),
	width( _width ),
	height( _height ),
	lpPixel( NULL )
{
	// DIB の情報を用意
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
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sl = 3 * sw;					// src の 1 列のビット長
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width;					// dst の 1 列のビット長

	double scalew = ( double )sw / width / 3.0;		// 拡大倍率 (1/3 倍)
	double scaleh = ( double )sh / height;			// 拡大倍率

	int w, h, hxdl, y0xsl, pl, x0, y0;	// ループ中で確保する一時変数
	for( h = 0; h < height; ++h )
	{		
		// src の基準点 (x0, y0) の y0 を求める
		y0 = ( int )( scaleh * h + 0.5 );

		// src 上の点 y0 を範囲外の点を範囲内に引き戻す
		if( y0 >= sh )
			y0 = sh - 1;

		hxdl = h * dl;					// (高さ) × (1 列のビット長)
		y0xsl = y0 * sl;

		for( w = 0; w < dl; w += 3 )
		{
			// src の基準点 (x0, y0) の x0 を求める
			x0 = ( int )( scalew * w + 0.5 );
			
			// src 上の点 x0 を範囲外の点を範囲内に引き戻す
			if( x0 >= sw )
				x0 = sw - 1;

			// src の基準点までのビット長算出
			pl = y0xsl + 3 * x0;

			// src の基準点 (x0, y0) を dst の点 (w, h) にコピー
			lpPixel[hxdl + w    ] = ( *sp )[pl    ];
			lpPixel[hxdl + w + 1] = ( *sp )[pl + 1];
			lpPixel[hxdl + w + 2] = ( *sp )[pl + 2];
		}
	}
}

// Bilinear (素コード、準最適化)
void Rgb24Image::Bilinear1( Rgb24Image *src )
{
	BYTE colorbuf[4];					// カラーバッファ
	
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sl = 3 * sw;					// src の 1 列のビット長
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width;					// dst の 1 列のビット長

	double scalew = ( double )sw / width;
	double scaleh = ( double )sh / height;

	int x0, y0;
	double x, y;
	for( int h = 0; h < height; ++h )
	{
		// src の基準場所 (x, y) の y を求める
		y = scaleh * h;

		// src の基準点 (x0, y0) の y0 を求める
		y0 = ( int )y;

		// src 上の点 y0 を範囲外の点を範囲内に引き戻す
		if( y0 > sh - 2 )
			y0 = sh - 2;

		// src の基準点からの基準場所 (Δx, Δy) の Δy を求める
		y -= y0;

		for( int w = 0; w < width; ++w )
		{
			// src の基準場所 (x, y) の x を求める
			x = scalew * w;

			// src の基準点 (x0, y0) の x0 を求める
			x0 = ( int )x;

			// src 上の点 x0 を範囲外の点を範囲内に引き戻す
			if( x0 > sw - 2 )
				x0 = sw - 2;
			
			// src の基準点からの基準場所 (Δx, Δy) の Δx を求める
			x -= x0;

			// TODO: propTL, propTR, propBL, propBR 変数で保存しておく
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