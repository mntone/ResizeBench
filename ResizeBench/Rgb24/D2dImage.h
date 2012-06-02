#pragma once

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include "HdcImage.h"

class CD2dImage:
	public CHdcImage
{
protected:
	ID2D1Factory *pD2DFactory_;
    IDWriteFactory *pDWriteFactory_;
    ID2D1DCRenderTarget *pDCRT_;
	IDWriteTextFormat *pTextFormat_;
    ID2D1SolidColorBrush *pSolidColorBrush_;

public:
	CD2dImage( int, int );
	~CD2dImage( void );

	bool SetColor( UINT32 rgb );
	bool DrawText( const LPCWSTR );
};

