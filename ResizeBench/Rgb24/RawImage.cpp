#include "RawImage.h"


CRawImage::CRawImage( int width, int height ):
	width_( width ),
	height_( height ),
	lpPixel_( new BYTE[3 * width_ * height_]() )
{
}


CRawImage::~CRawImage( void )
{
	if( lpPixel_ != NULL )
	{
		delete lpPixel_;
		lpPixel_ = NULL;
	}
}
