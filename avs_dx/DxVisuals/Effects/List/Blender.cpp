#include "stdafx.h"
#include "Blender.h"
#include "../../Resources/staticResources.h"

bool Blender::modeUsesShader( eBlendMode mode )
{
	switch( mode )
	{
	case eBlendMode::Ignore:
	case eBlendMode::Replace:
	case eBlendMode::Buffer:
		return false;
	}
	return true;
}

bool Blender::updateBindings( Binder& binder )
{
	BoolHr hr = blendShader.update( binder, nullptr, nullptr );
	return hr.value();
}

HRESULT Blender::blend( RenderTargets& source, RenderTargets& dest, eBlendMode mode, float blendVal )
{
	if( eBlendMode::Ignore == mode )
		return S_FALSE;

	RenderTarget& src = source.lastWritten();

	if( eBlendMode::Replace == mode )
	{
		RenderTarget& dst = dest.lastWritten();
		if( !dst )
			CHECK( dst.create() );

		if( src )
		{
			src.copyTo( dst );
			return S_OK;
		}
		dst.clear();
		return S_OK;
	}

	if( (uint8_t)mode > (uint8_t)eBlendMode::Minimum )
		return E_INVALIDARG;
	if( mode == eBlendMode::Buffer )
		return E_NOTIMPL;

	CHECK( ensureShader( mode, blendVal ) );
	if( !blendShader.bind() )
		return S_FALSE;

	const UINT bindSource = blendShader.data().source;
	BoundPsResource boundDest;
	CHECK( dest.writeToNext( boundDest ) );

	auto bound = boundResource<eStage::Pixel>( bindSource, src ? src.srv() : StaticResources::blackTexture.operator ->() );

	omBlend( eBlend::None );
	drawFullscreenTriangle( true );
	return S_OK;
}

HRESULT Blender::ensureShader( eBlendMode mode, float blendVal )
{
	if( blendShader.data().blend == (uint8_t)mode && blendShader.data().blendVal == blendVal )
	{
		// The parameters have not changed
		const eShaderState ss = blendShader.getState();
		switch( ss )
		{
		case eShaderState::Good:
			return S_OK;
		case eShaderState::Failed:
			return E_FAIL;
		case eShaderState::Pending:
			return S_FALSE;
		}
	}
	else
	{
		blendShader.data().blend = (uint8_t)mode;
		blendShader.data().blendVal = blendVal;
	}

	return blendShader.compile( 0 );
}