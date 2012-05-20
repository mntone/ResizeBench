#pragma once

#include "stdafx.h"

class Bench
{
protected:
	HWND hWnd;
	HDC  hdc;
	
	std::unique_ptr<Rgb24Image> i1, i2, i3;

public:
	Bench( HWND hWnd );
	~Bench( void );

	void Test( void );
	void Test2( void );
	void Test3( void );
};

