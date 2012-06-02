#pragma once

#include "stdafx.h"
#include "Rgb24/ProcessingImage.h"
#include "Rgb24/HdcImage.h"
#include "Rgb24/D2dImage.h"

#ifndef _CBench
#define _CBench

class CBench
{
protected:
	HWND hWnd_;
	HDC  hdc_, deskhdc_;
	
	std::unique_ptr<CD2dImage> i1c_, buf;
	std::unique_ptr<CProcessingImage> i1r_;

public:
	CBench( HWND hWnd );
	~CBench( void );

	void Test( void );
	void Test2( void );
	void Test3( void );
};

#endif
