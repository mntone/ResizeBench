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

	void NearestNeighbor1( Rgb24Image *src );
	void Bilinear1( Rgb24Image *src );
	void Bicubic1( Rgb24Image *src );
};

