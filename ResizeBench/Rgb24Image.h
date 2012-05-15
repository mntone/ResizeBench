#pragma once

#include "stdafx.h"

class Rgb24Image sealed
{
protected:
	HWND hWnd;
	HDC hdc, hMemDC;
	HBITMAP hBitmap;

	int width;
	int height;
	LPBITMAPINFO bmpi;
	LPBYTE lpPixel;

public:
	Rgb24Image( HWND _hwnd, int _width, int _height );
	~Rgb24Image( void );

	int GetWidth( void );
	int GetHeight( void );

	HDC GetImageDC( void );
	LPBYTE *GetPixel( void );

	void Rgb24Image::NearestNeighbor1( Rgb24Image *src );
	void Rgb24Image::Bilinear1( Rgb24Image *src );
};

