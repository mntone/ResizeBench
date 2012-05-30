#pragma once

#include "RawImage.h"

#ifndef _CProcessingImage
#define _CProcessingImage

class CProcessingImage:
	public CRawImage
{
public:
	CProcessingImage( int, int );
	~CProcessingImage( void );

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
