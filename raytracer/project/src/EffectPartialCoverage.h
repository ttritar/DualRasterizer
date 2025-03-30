#pragma once
#include "Effect.h"

namespace dae
{
	class EffectPartialCoverage : public Effect
	{
	public:
		// CTOR + DTOR
		// ------
		EffectPartialCoverage(ID3D11Device* pDevice, const std::wstring& assetFile)
			: Effect(pDevice,assetFile)
		{
			m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
			if (!m_pTechnique->IsValid())
				std::wcout << L"m_pTechnique not valid\n";



			// VARIABLES
			//---------------
		
			// Initialize only resources specific to EffectPartialCoverage
			m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
			if (!m_pDiffuseMapVariable->IsValid())
				std::wcout << L"m_pDiffuseMapVariable not valid!\n";
		}
		~EffectPartialCoverage()
		{
			if (m_pTechnique)
				m_pTechnique->Release();

			if (m_pDiffuseMapVariable)
				m_pDiffuseMapVariable->Release();

		}

		// Rule of 5
		// ------
		EffectPartialCoverage(const EffectPartialCoverage&) = delete;
		EffectPartialCoverage(EffectPartialCoverage&&) noexcept = delete;
		EffectPartialCoverage& operator=(const EffectPartialCoverage&) = delete;
		EffectPartialCoverage& operator=(EffectPartialCoverage&&) noexcept = delete;

		// MemberFunctiuons
		// ------
		void SetDiffuseMap(Texture* pDiffuseTexture)
		{
			if (m_pDiffuseMapVariable)
				m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
		}

		virtual ID3DX11EffectTechnique* GetTechnique(const FilteringMethod& filteringMethod) const override
		{
			return m_pTechnique;
		};

	private:
		//Textures
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;

		ID3DX11EffectTechnique* m_pTechnique;
	};
}