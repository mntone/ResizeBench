#pragma once

#include "Image.h"

#ifndef _CProcessingImage
#define _CProcessingImage

class CProcessingImage:
	public IImage
{
protected:
	const int width_;
	const int height_;
	LPBYTE lpPixel_;

public:
	CProcessingImage( int, int );
	~CProcessingImage( void );
	
	virtual const int& GetWidth( void ) const{ return width_; };
	virtual const int& GetHeight( void ) const{ return height_; };
	virtual const LPBYTE& GetPixel( void ) const{ return lpPixel_; };

	bool Copy( IImage * );
	bool Trim( RECT rect, IImage * );
	void NearestNeighbor( IImage * );
	void Bilinear( IImage * );
	void Bicubic( IImage * );
	bool FilpXY( bool, bool, IImage * );
	bool Rotate90( IImage * );
	bool InvNegaPosi( IImage * );
	bool Mozaic( const int, IImage *src );
	bool Blur( const int, IImage *src );
};

#endif
