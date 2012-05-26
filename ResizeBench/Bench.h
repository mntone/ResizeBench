#pragma once

#include "stdafx.h"

class Bench
{
protected:
	HWND hWnd_;
	HDC  hdc_, deskhdc_;
	
	std::unique_ptr<Rgb24Image> i1_, i2_, i3_;

public:
	Bench( HWND hWnd );
	~Bench( void );

	void Test( void );
	void Test2( void );
	void Test3( void );
};

