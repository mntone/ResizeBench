#include "StdAfx.h"
#include "Rgb24Image.h"

Rgb24Image::Rgb24Image( HWND hWnd, int width, int height ):
	hWnd_( hWnd ),
	width_( width ),
	height_( height ),
	lpPixel_( NULL ),
	bmpi_( NULL )
{
	// DIB の情報を用意
	bmpi_ = new BITMAPINFO();
	bmpi_->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bmpi_->bmiHeader.biWidth = width;
	bmpi_->bmiHeader.biHeight = height;
	bmpi_->bmiHeader.biPlanes = 1;
	bmpi_->bmiHeader.biBitCount = 24;
	bmpi_->bmiHeader.biCompression = BI_RGB;

	// HDC 取得
	hdc_ = GetDC( hWnd_ );

	// DIBSection の作成
	hBitmap_ = CreateDIBSection( hdc_, bmpi_, DIB_RGB_COLORS, reinterpret_cast<void **>( &lpPixel_ ), NULL, 0 );
	hMemDC_ = CreateCompatibleDC( hdc_ );
	SelectObject( hMemDC_, hBitmap_ );

	// HDC 解放
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


// ----------- 以下、画像処理関数群 ------------

// Copy
bool Rgb24Image::Copy( Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sh = src->GetHeight();			// src の高さ

	// サイズが違うかチェック
	if( sw != width_ || sh != height_ )
		return false;

	// コピー
	memcpy( lpPixel_, *sp, 3 * width_ * height_ );

	return true;
}

// Trim
bool Rgb24Image::Trim( RECT rect, Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sl = 3 * sw;					// src の 1 列のビット長
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width_;				// dst の 1 列のビット長
	int ll = 3 * rect.left;				// 垂直方向の trim の始点までのビット長

	// サイズが違うかチェック
	if( sw < rect.right || sh < rect.bottom || rect.right != width_ + rect.left || rect.bottom != height_ + rect.top  )
		return false;

	for( int h = 0; h < height_; ++h )
		memcpy( lpPixel_ + h * dl, *sp + ( rect.top + h ) * sl + ll, dl );

	return true;
}

// NearestNeighbor
// [todo] scaleh * h を掛け算して求めるのではなく、足し算で求めるほうがいい?
//        (e.g.) double hprop = scaleh * h; y(double) += hprop; y0 = ( int )( y + 0.5 );
void Rgb24Image::NearestNeighbor( Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sl = 3 * sw;					// src の 1 列のビット長
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width_;				// dst の 1 列のビット長

	double scalew = static_cast<double>( sw ) / static_cast<double>( width_ ) / 3.0;	// 拡大倍率 (1/3 倍)
	double scaleh = static_cast<double>( sh ) / static_cast<double>( height_ );			// 拡大倍率

	int w, h, hxdl, y0xsl, x0, y0;		// ループ中で確保する一時変数
	for( h = 0; h < height_; ++h )
	{		
		// src の基準点 (x0, y0) の y0 を求める
		y0 = static_cast<int>( scaleh * h + 0.5 );

		// src 上の点 y0 を範囲外の点を範囲内に引き戻す
		if( y0 >= sh )
			y0 = sh - 1;

		hxdl = h * dl;					// (高さ) × (1 列のビット長)
		y0xsl = y0 * sl;

		for( w = 0; w < dl; w += 3 )
		{
			// src の基準点 (x0, y0) の x0 を求める
			x0 = static_cast<int>( scalew * w + 0.5 );
			
			// src 上の点 x0 を範囲外の点を範囲内に引き戻す
			if( x0 >= sw )
				x0 = sw - 1;
			
			// src の基準点 (x0, y0) を dst の点 (w, h) にコピー
			memcpy( lpPixel_ + hxdl + w, *sp + y0xsl + 3 * x0, 3 );
		}
	}
}


// Bilinear
//   1. s00, s01, s10, s11 と予めポインタの座標を計算した場合、
//      メモリアクセス回数が増えすぎて、余計遅くなるかも？
//      （レツノバッテリー駆動で 15 sec → 16 sec 程度）
//      メモリアクセスが遅い DDR2 世代だとさらに差が開くかも。
void Rgb24Image::Bilinear( Rgb24Image *src )
{
	BYTE colorbuf[4];					// カラーバッファ
	
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sl = 3 * sw;					// src の 1 列のビット長
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width_;				// dst の 1 列のビット長
	
	double scalew = static_cast<double>( sw ) / static_cast<double>( width_ ) / 3.0;	// 拡大倍率 (1/3 倍)
	double scaleh = static_cast<double>( sh ) / static_cast<double>( height_ );			// 拡大倍率

	int w, h, i, x0, y0;
	double x, y;
	for( h = 0; h < height_; ++h )
	{
		// src の基準場所 (x, y) の y を求める
		y = scaleh * h;

		// src の基準点 (x0, y0) の y0 を求める
		y0 = static_cast<int>( y );

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
			x0 = static_cast<int>( x );

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
                                
				lpPixel_[h * dl + w + i] = ( BYTE )( ( 1.0 - x ) * ( 1.0 - y ) * colorbuf[0] + x * ( 1.0 - y ) * colorbuf[1]
					+ ( 1.0 - x ) * y * colorbuf[2] + x * y * colorbuf[3] + 0.5 );
			}
		}
	}
}


// Bicubic
void Rgb24Image::Bicubic( Rgb24Image *src )
{
	double colorbuf[3];					// カラーバッファ
	
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sl = 3 * sw;					// src の 1 列のビット長
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width_;				// dst の 1 列のビット長
	
	double scalew = static_cast<double>( sw ) / static_cast<double>( width_ ) / 3.0;	// 拡大倍率 (1/3 倍)
	double scaleh = static_cast<double>( sh ) / static_cast<double>( height_ );			// 拡大倍率

	int w, h, i, j, x0, y0, mx, my;
	double x, y, wx, wy, dx, dy;
	for( h = 0; h < height_; ++h )
	{
		// src の基準場所 (x, y) の y を求める
		y = scaleh * h;

		// src の基準点 (x0, y0) の y0 を求める
		y0 = static_cast<int>( y );

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
			x0 = static_cast<int>( x );

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
				dy = static_cast<double>( my ) - y;

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
					dx = static_cast<double>( mx ) - x;
					
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
					colorbuf[0] += wx * static_cast<double>( ( *sp )[my * sl + 3 * mx    ] );
					colorbuf[1] += wx * static_cast<double>( ( *sp )[my * sl + 3 * mx + 1] );
					colorbuf[2] += wx * static_cast<double>( ( *sp )[my * sl + 3 * mx + 2] );
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
			lpPixel_[h * dl + w    ] = static_cast<int>( colorbuf[0] + 0.5 );
			lpPixel_[h * dl + w + 1] = static_cast<int>( colorbuf[1] + 0.5 );
			lpPixel_[h * dl + w + 2] = static_cast<int>( colorbuf[2] + 0.5 );
		}
	}
}


// FlipXY
// flipX = false AND flipY = false のとき Copy 実行
bool Rgb24Image::FilpXY( bool flipX, bool flipY, Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int dl = 3 * width_;				// dst の 1 列のビット長

	// サイズが違うかチェック
	if( src->GetWidth() != width_ || src->GetHeight() != height_ )
		return false;

	// 処理速度を高速化するために、ループ外で条件分岐
	int w, h;

	// 上下左右反転
	if( flipX && flipY )
		for( h = 0; h < height_; ++h )
			for( w = 0; w < dl; w += 3 )
				memcpy( lpPixel_ + h * dl + w, *sp + ( height_ - h - 1 ) * dl + ( dl - w - 3 ), 3 );

	// 左右反転
	else if( flipX )
		for( h = 0; h < height_; ++h )
			for( w = 0; w < dl; w += 3 )
				memcpy( lpPixel_ + h * dl + w, *sp + h * dl + ( dl - w - 3 ), 3 );

	// 上下反転
	else if( flipY )
		for( h = 0; h < height_; ++h )
			memcpy( lpPixel_ + h * dl, *sp + ( height_ - h - 1 ) * dl, dl );

	// どのフラグもついていなければ、コピー
	else
		return Copy( src );

	return true;
}


// Rotate90
bool Rgb24Image::Rotate90( Rgb24Image *src )
{
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sl = 3 * sw;					// src の 1 列のビット長
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width_;				// dst の 1 列のビット長

	// サイズが違うかチェック
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
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int dl = 3 * width_;				// dst の 1 列のビット長

	// サイズが違うかチェック
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
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width_;				// dst の 1 列のビット長
	int ll = 3 * level;					// level のビット長

	// サイズが違うかチェック
	if( sw != width_ || sh != height_ )
		return false;

	// レベルが 1 より小ならコピー
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
				// 範囲外チェック
				if( h + j > height_ )
					continue;

				for( i = 0; i < ll; i += 3 )
				{
					// 範囲外チェック
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
	LPBYTE *sp = src->GetPixel();		// src の LPBYTE のポインタ
	int sw = src->GetWidth();			// src の幅
	int sh = src->GetHeight();			// src の高さ
	int dl = 3 * width_;				// dst の 1 列のビット長
	int ll = 3 * level;					// level のビット長

	// サイズが違うかチェック
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
				// 範囲外チェック
				if( h + j < 0 || h + j > height_ )
					continue;

				for( i = -ll; i < ll; i += 3 )
				{
					// 範囲外チェック
					if( w + i < 0 || w + i > dl || abs( i * j / 3 ) >= level )
						continue;

					// 足し合わせる
					colorbuf[0] += ( *sp )[( h + j ) * dl + w + i    ];
					colorbuf[1] += ( *sp )[( h + j ) * dl + w + i + 1];
					colorbuf[2] += ( *sp )[( h + j ) * dl + w + i + 2];
					p += 1.0;
				}
			}
			
			// 結果を出力
			if( p != 0.0 )
			{
				lpPixel_[h * dl + w    ] = static_cast<BYTE>( colorbuf[0] / p );
				lpPixel_[h * dl + w + 1] = static_cast<BYTE>( colorbuf[1] / p );
				lpPixel_[h * dl + w + 2] = static_cast<BYTE>( colorbuf[2] / p );
			}
		}

	return true;
}

