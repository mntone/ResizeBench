#include "D2dImage.h"


CD2dImage::CD2dImage( int width, int height ):
	CHdcImage( width, height ),
	pD2DFactory_( NULL ),
    pDWriteFactory_( NULL ),
    pDCRT_( NULL ),
    pBlackBrush_( NULL )
{
	HRESULT hr = S_OK;

	// �t�@�N�g���[ �I�u�W�F�N�g���쐬
	hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory_ );
	if( FAILED( hr ) )
		throw L"";

	hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( pDWriteFactory_ ),
		reinterpret_cast<IUnknown **>( &pDWriteFactory_ ) );
	if( FAILED( hr ) )
		throw L"";

	// �����_���[�^�[�Q�b�g�̐ݒ�
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE ) );

	hr = pD2DFactory_->CreateDCRenderTarget( &props, &pDCRT_ );
	if( FAILED( hr ) )
		throw L"";

	// �y���̍쐬
	hr = pDCRT_->CreateSolidColorBrush( D2D1::ColorF( D2D1::ColorF::Black ), &pBlackBrush_ );
	if( FAILED( hr ) )
		throw L"";

	// DC ���Z�b�g����
	RECT r;
	r.top = r.left = 0;
	r.right = width_;
	r.bottom = height_;

	hr = pDCRT_->BindDC( hMemDC_, &r );
	if( FAILED( hr ) )
		throw L"";

}


CD2dImage::~CD2dImage( void )
{

}



bool CD2dImage::DrawText( const LPCWSTR str )
{
	D2D1_SIZE_F rtSize = pDCRT_->GetSize();

	pDCRT_->BeginDraw();

	/*hr = */pDWriteFactory_->CreateTextFormat(
	L"�l�r �o����",
	NULL,
	DWRITE_FONT_WEIGHT_NORMAL,
	DWRITE_FONT_STYLE_NORMAL,
	DWRITE_FONT_STRETCH_NORMAL,
	40,
	L"", //locale
	&pTextFormat_
	);

	pDCRT_->DrawText( str, wcslen( str ),
pTextFormat_,
D2D1::RectF(0, 0, rtSize.width, rtSize.height), pBlackBrush_ );

	HRESULT hr = pDCRT_->EndDraw();

	return FAILED( hr ) ? false : true;
}
