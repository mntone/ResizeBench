#pragma once

#include <Windows.h>

#ifndef _IImage
#define _IImage

class IImage
{
public:
	virtual ~IImage( void ) = 0;

	virtual const int& GetWidth( void ) const = 0;
	virtual const int& GetHeight( void ) const = 0;
	virtual const LPBYTE& GetPixel( void ) const = 0;
};

#endif
