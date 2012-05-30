#pragma once

#include "ProcessingImage.h"

#ifndef _CHdcImage
#define _CHdcImage

class CHdcImage:
	public IImage
{
protected:
	const int width_;
	const int height_;
	LPBYTE lpPixel_;

	HDC hMemDC_;
	HBITMAP hBitmap_;
	LPBITMAPINFO bmpi_;

public:
	CHdcImage( int, int );
	~CHdcImage( void );

	virtual const int& GetWidth( void ) const{ return width_; };
	virtual const int& GetHeight( void ) const{ return height_; };
	virtual const LPBYTE& GetPixel( void ) const{ return lpPixel_; };

	const HDC& GetImageDC( void ) const{ return hMemDC_; };

	bool Copy( IImage *src );
};

#endif
