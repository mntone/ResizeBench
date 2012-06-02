#include "D2dImage.h"


CD2dImage::CD2dImage( int width, int height ):
	CHdcImage( width, height ),
	pD2DFactory_( NULL ),
    pDWriteFactory_( NULL ),
    pDCRT_( NULL ),
	pTextFormat_( NULL ),
    pSolidColorBrush_( NULL )
{
	HRESULT hr = S_OK;

	// ファクトリー オブジェクトを作成
	hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory_ );
	if( FAILED( hr ) )
		throw L"";

	hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( pDWriteFactory_ ),
		reinterpret_cast<IUnknown **>( &pDWriteFactory_ ) );
	if( FAILED( hr ) )
		throw L"";

	// レンダラーターゲットの設定
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE ) );

	hr = pD2DFactory_->CreateDCRenderTarget( &props, &pDCRT_ );
	if( FAILED( hr ) )
		throw L"";

	// DC をセットする
	RECT r;
	r.top = r.left = 0;
	r.right = width_;
	r.bottom = height_;

	hr = pDCRT_->BindDC( hMemDC_, &r );
	if( FAILED( hr ) )
		throw L"";

	// ペンをセットしておく
	SetColor( 0 );
}


CD2dImage::~CD2dImage( void )
{
    if( pSolidColorBrush_ != NULL )
	{
		pSolidColorBrush_->Release();
		pSolidColorBrush_ = NULL;
	}

    if( pTextFormat_ != NULL )
	{
		pTextFormat_->Release();
		pTextFormat_ = NULL;
	}

    if( pDCRT_ != NULL )
	{
		pDCRT_->Release();
		pDCRT_ = NULL;
	}

    if( pDWriteFactory_ != NULL )
	{
		pDWriteFactory_->Release();
		pDWriteFactory_ = NULL;
	}

	if( pD2DFactory_ != NULL )
	{
		pD2DFactory_->Release();
		pD2DFactory_ = NULL;
	}
}

bool CD2dImage::SetColor( UINT32 rgb )
{
	HRESULT hr = pDCRT_->CreateSolidColorBrush( D2D1::ColorF( rgb ), &pSolidColorBrush_ );

	return FAILED( hr ) ? false : true;
}


bool CD2dImage::DrawText( const LPCWSTR str )
{
	D2D1_SIZE_F rtSize = pDCRT_->GetSize();

	pDCRT_->BeginDraw();

	/*hr = */pDWriteFactory_->CreateTextFormat(
	L"ＭＳ Ｐ明朝",
	NULL,
	DWRITE_FONT_WEIGHT_NORMAL,
	DWRITE_FONT_STYLE_NORMAL,
	DWRITE_FONT_STRETCH_NORMAL,
	40,
	L"", //locale
	&pTextFormat_
	);

	
    /*hr = */pTextFormat_->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );
    /*hr = */pTextFormat_->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );

	pDCRT_->DrawText( str, wcslen( str ), pTextFormat_,
		D2D1::RectF( 0, 0, rtSize.width, rtSize.height ), pSolidColorBrush_ );

	HRESULT hr = pDCRT_->EndDraw();

	return FAILED( hr ) ? false : true;
}
