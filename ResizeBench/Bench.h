#pragma once

#include "stdafx.h"

class Bench
{
protected:
	HWND hWnd;
	HDC  hdc;
	
	Rgb24Image *i1, *i2;

public:
	Bench( HWND hWnd );
	~Bench( void );

	void Bench::Test( void );
	void NearestNeighbor1( Rgb24Image *src, Rgb24Image *dst );
};

