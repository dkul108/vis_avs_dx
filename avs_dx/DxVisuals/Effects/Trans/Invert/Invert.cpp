#pragma once
#include "stdafx.h"
#include <Effects/Common/EffectImpl.hpp>

struct InvertStructs
{
	struct AvsState
	{
		int enabled;
	};

	using StateData = EmptyStateData;

	static ID3D11VertexShader* vertexShader()
	{
		return StaticResources::fullScreenTriangle;
	}

	using PsData = Hlsl::Trans::Invert::InvertPS;
};

class Invert : public EffectBase1<InvertStructs>
{
public:
	Invert( AvsState* avs ) : EffectBase1( avs )
	{ }

	DECLARE_EFFECT()

	HRESULT render( bool isBeat, RenderTargets& rt ) override;
};

IMPLEMENT_EFFECT( Invert, C_InvertClass )

HRESULT Invert::render( bool isBeat, RenderTargets& rt )
{
	if( !avs->enabled )
		return S_FALSE;
	if( !renderer.bindShaders( isBeat ) )
		return S_FALSE;

	BoundPsResource bound;
	CHECK( rt.writeToNext( bound ) );

	omBlend( eBlend::None );
	drawFullscreenTriangle( false );
	return S_OK;
}