#pragma once

#include "Image.h"

#ifndef _CRawImage
#define _CRawImage

class CRawImage:
	public IImage
{
protected:
	const int width_;
	const int height_;
	LPBYTE lpPixel_;

public:
	CRawImage( int, int );
	~CRawImage( void );

	virtual const int& GetWidth( void ) const{ return width_; };
	virtual const int& GetHeight( void ) const{ return height_; };
	virtual const LPBYTE& GetPixel( void ) const{ return lpPixel_; };
};

#endif
