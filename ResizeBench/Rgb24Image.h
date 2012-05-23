#pragma once

#include "stdafx.h"

class Rgb24Image sealed
{
protected:
	HWND hWnd;
	HDC hdc, hMemDC;
	HBITMAP hBitmap;

	const int width;
	const int height;
	LPBITMAPINFO bmpi;
	LPBYTE lpPixel;

public:
	Rgb24Image( HWND, int, int );
	~Rgb24Image( void );

	int GetWidth( void ){ return width; };
	int GetHeight( void ){ return height; };

	HDC GetImageDC( void ){ return hMemDC; };
	LPBYTE *GetPixel( void ){ return &lpPixel; };

	bool Copy( Rgb24Image * );
	bool Trim( RECT rect, Rgb24Image * );
	void NearestNeighbor( Rgb24Image * );
	void Bilinear1( Rgb24Image * );
	void Bicubic1( Rgb24Image * );
	bool FilpXY( bool, bool, Rgb24Image * );
	bool Rotate90( Rgb24Image * );
};

