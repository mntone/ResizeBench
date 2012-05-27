#pragma once

#include "stdafx.h"

class Rgb24Image sealed
{
protected:
	HWND hWnd_;
	HDC hdc_, hMemDC_;
	HBITMAP hBitmap_;

	const int width_;
	const int height_;
	LPBITMAPINFO bmpi_;
	LPBYTE lpPixel_;

public:
	Rgb24Image( HWND, int, int );
	~Rgb24Image( void );

	const int& GetWidth( void ) const{ return width_; };
	const int& GetHeight( void ) const{ return height_; };

	const HDC& GetImageDC( void ) const{ return hMemDC_; };
	const LPBYTE& GetPixel( void ) const{ return lpPixel_; };

	bool Copy( Rgb24Image * );
	bool Trim( RECT rect, Rgb24Image * );
	void NearestNeighbor( Rgb24Image * );
	void Bilinear( Rgb24Image * );
	void Bicubic( Rgb24Image * );
	bool FilpXY( bool, bool, Rgb24Image * );
	bool Rotate90( Rgb24Image * );
	bool InvNegaPosi( Rgb24Image * );
	bool Mozaic( const int, Rgb24Image *src );
	bool Blur( const int, Rgb24Image *src );
};

