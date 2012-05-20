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


// ----------- 以下、画像処理関数群 ------------
//typedef struct { BYTE b, g, r; } RGB24;
//typedef RGB24 *LPRGB24;

bool Rgb24Image::Copy( Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sh = src->GetHeight();			// src の高さ

	// サイズが違うかチェック
	if( sw != width || sh != height )
		return false;

	// コピー
	memcpy( lpPixel, *sp, 3 * width * height );

	return true;
}

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
//   1. s00, s01, s10, s11 と予めポインタの座標を計算した場合、
//      メモリアクセス回数が増えすぎて、余計遅くなるかも？
//      （レツノバッテリー駆動で 15 sec → 16 sec 程度）
//      メモリアクセスが遅い DDR2 世代だとさらに差が開くかも。
void Rgb24Image::Bilinear1( Rgb24Image *src )
{
	BYTE colorbuf[4];					// カラーバッファ
	
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sl = 3 * sw;					// src の 1 列のビット長
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width;					// dst の 1 列のビット長
	
	double scalew = ( double )sw / width / 3.0;		// 拡大倍率 (1/3 倍)
	double scaleh = ( double )sh / height;			// 拡大倍率

	int w, h, i, x0, y0;
	double x, y;
	for( h = 0; h < height; ++h )
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

		for( w = 0; w < dl; w += 3 )
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

			// 各点をコピー
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


// Bicubic (素コード、最適化なし)
void Rgb24Image::Bicubic1( Rgb24Image *src )
{
	double colorbuf[3];					// カラーバッファ
	
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sl = 3 * sw;					// src の 1 列のビット長
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width;					// dst の 1 列のビット長
	
	double scalew = ( double )sw / width / 3.0;		// 拡大倍率 (1/3 倍)
	double scaleh = ( double )sh / height;			// 拡大倍率

	int w, h, i, j, x0, y0, mx, my;
	double x, y, wx, wy, dx, dy;
	for( h = 0; h < height; ++h )
	{
		// src の基準場所 (x, y) の y を求める
		y = scaleh * h;

		// src の基準点 (x0, y0) の y0 を求める
		y0 = ( int )y;

		// src 上の点 y0 を範囲外の点を範囲内に引き戻す
		if( y0 < 1 )
			y0 = 1;
		else if( y0 > sh - 3 )
			y0 = sh - 3;

		for( w = 0; w < dl; w += 3 )
		{
			// src の基準場所 (x, y) の x を求める
			x = scalew * w;

			// src の基準点 (x0, y0) の x0 を求める
			x0 = ( int )x;

			// src 上の点 x0 を範囲外の点を範囲内に引き戻す
			if( x0 < 1 )
				x0 = 1;
			else if( x0 > sw - 3 )
				x0 = sw - 3;
			
			// カラーバッファリセット
			colorbuf[0] = colorbuf[1] = colorbuf[2] = 0.0;
			
			// 基準点から直近の 16 点で重み計算
			for( j = -1; j <= 2; ++j )
			{
				// 取り扱う点 (mx, my) の my を計算
				// 基準場所からの距離差 Δy を求める
				my = y0 + j;
				dy = ( double )my - y;

				// 絶対値
				if( dy < 0.0 )
					dy = -dy;

				// y 方向の重み計算
				if( dy <= 1.0 )
					wy = 1.0 - 2.0 * dy * dy + dy * dy * dy;
				else if( dy <= 2.0 )
					wy = 4.0 - 8.0 * dy + 5.0 * dy * dy - dy * dy * dy;
				else
					continue;

				for( i = -1; i <= 2; ++i )
				{
					// 取り扱う点 (mx, my) の mx を計算
					// 基準場所からの距離差 Δx を求める
					mx = x0 + i;
					dx = ( double )mx - x;
					
					// 絶対値
					if( dx < 0.0 )
						dx = -dx;

					// x 方向の重み計算
					if( dx <= 1.0 )
						wx = wy * ( 1.0 - 2.0 * dx * dx + dx * dx * dx );
					else if( dx <= 2.0 )
						wx = wy * ( 4.0 - 8.0 * dx + 5.0 * dx * dx - dx * dx * dx );
					else
						continue;

					// カラーバッファーに足す
					colorbuf[0] += wx * ( double )( ( *sp )[my * sl + 3 * mx    ] );
					colorbuf[1] += wx * ( double )( ( *sp )[my * sl + 3 * mx + 1] );
					colorbuf[2] += wx * ( double )( ( *sp )[my * sl + 3 * mx + 2] );
				}
			}
			
			// オーバーしたものを戻す (四捨五入考慮済み)
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

			// 格納
			lpPixel[h * dl + w    ] = ( int )( colorbuf[0] + 0.5 );
			lpPixel[h * dl + w + 1] = ( int )( colorbuf[1] + 0.5 );
			lpPixel[h * dl + w + 2] = ( int )( colorbuf[2] + 0.5 );
		}
	}
}


// FlipXY (素コード、最適化なし)
// flipX = false AND flipY = false のとき Copy 実行
bool Rgb24Image::FilpXY1( bool flipX, bool flipY, Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sl = 3 * sw;					// src の 1 列のビット長
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width;					// dst の 1 列のビット長

	// サイズが違うかチェック
	if( sw != width || sh != height )
		return false;

	// 処理速度を高速化するために、ループ外で判断
	int w, h;

	// 上下左右反転
	if( flipX && flipY )
		for( h = 0; h < height; ++h )
			for( w = 0; w < dl; w += 3 )
			{
				lpPixel[h * dl + w    ] = ( *sp )[( height - h - 1 ) * dl + ( dl - w - 3 )    ];
				lpPixel[h * dl + w + 1] = ( *sp )[( height - h - 1 ) * dl + ( dl - w - 3 ) + 1];
				lpPixel[h * dl + w + 2] = ( *sp )[( height - h - 1 ) * dl + ( dl - w - 3 ) + 2];
			}

	// 上下反転
	else if( flipX )
		for( h = 0; h < height; ++h )
			for( w = 0; w < dl; w += 3 )
			{
				lpPixel[h * dl + w    ] = ( *sp )[( height - h - 1 ) * dl + w    ];
				lpPixel[h * dl + w + 1] = ( *sp )[( height - h - 1 ) * dl + w + 1];
				lpPixel[h * dl + w + 2] = ( *sp )[( height - h - 1 ) * dl + w + 2];
			}

	// 左右反転
	else if( flipY )
		for( h = 0; h < height; ++h )
			for( w = 0; w < dl; w += 3 )
			{
				lpPixel[h * dl + w    ] = ( *sp )[h * dl + ( dl - w - 3 )    ];
				lpPixel[h * dl + w + 1] = ( *sp )[h * dl + ( dl - w - 3 ) + 1];
				lpPixel[h * dl + w + 2] = ( *sp )[h * dl + ( dl - w - 3 ) + 2];
			}
	else
		Copy( src );
		

	return true;
}