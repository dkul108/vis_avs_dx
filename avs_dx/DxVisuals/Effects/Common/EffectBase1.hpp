#pragma once
#include "EffectBase.h"
#include "../Render/EffectRenderer.hpp"

// A base class for effects that implements a few required virtual methods by delegating the work to the EffectRenderer template class.
// This is optional, if you don't need what it offers you can inherit from EffectBase directly.
template<class TStruct, class TBase = EffectBase>
class EffectBase1 : public TStruct, public TBase
{
public:
	using tBase = EffectBase1<TStruct>;
	using AvsState = typename TStruct::AvsState;

protected:

	typename TStruct::AvsState* const avs;
	EffectBase1( typename TStruct::AvsState* ass ) : avs( ass ), stateData( *ass ) { }

	EffectRenderer<TStruct> renderer;

	typename TStruct::StateData stateData;

	HRESULT shouldRebuildState() override
	{
		__if_exists( TStruct::StateData::update )
		{
			return stateData.update( *avs );
		}
		__if_not_exists( TStruct::StateData::update )
		{
			const TStruct::StateData newState{ *avs };
			if( stateData == newState )
				return S_FALSE;
			stateData = newState;
			return S_OK;
		}
		return E_FAIL;
	}

protected:

	HRESULT buildState( EffectStateShader& ess ) override
	{
		ess.shaderTemplate = stateData.shaderTemplate();
		ess.stateSize = stateData.stateSize();
		return stateData.defines( ess.values );
	}

protected:

	HRESULT updateParameters( Binder& binder ) override
	{
		BoolHr hr = renderer.update( binder, *avs, stateData );
		if( hr.failed() || !hr.value() )
			return hr;

		const CAtlMap<CStringA, CStringA>* pIncludes = &::Hlsl::includes();
		__if_exists( TStruct::effectIncludes )
		{
			pIncludes = &TStruct::effectIncludes();
		}

		SILENT_CHECK( renderer.compileShaders( *pIncludes, TBase::stateOffset() ) );
		return S_OK;
	}
};

struct EmptyStateData
{
	EmptyStateData() = default;
	template<class A>
	EmptyStateData( const A& ) { }
	const StateShaderTemplate* shaderTemplate() { return nullptr; }
	UINT stateSize() { return 0; }
	HRESULT defines( Hlsl::Defines& def ) const { return S_FALSE; }
	bool operator==( const EmptyStateData& ) { return true; }
};