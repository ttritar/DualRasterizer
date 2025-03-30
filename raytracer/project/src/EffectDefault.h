#pragma once
#include "Effect.h"

namespace dae
{
	class EffectDefault : public Effect
	{
	public:
		// CTOR + DTOR
		// ------
		EffectDefault(ID3D11Device* pDevice, const std::wstring& assetFile)
			:Effect(pDevice,assetFile)
		{
			// Get the technique and store it in a datamember
			m_pTechniquePoint = m_pEffect->GetTechniqueByName("PointTechnique");
			m_pTechniqueLinear = m_pEffect->GetTechniqueByName("LinearTechnique");
			m_pTechniqueAnisotropic = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
			if (!m_pTechniquePoint->IsValid())
				std::wcout << L"m_pTechniquePoint not valid\n";
			if (!m_pTechniqueLinear->IsValid())
				std::wcout << L"m_pTechniqueLinear not valid\n";
			if (!m_pTechniqueAnisotropic->IsValid())
				std::wcout << L"m_pTechniqueAnisotropic not valid\n";


			// Textures
			m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();	// diffuse
			if (!m_pDiffuseMapVariable->IsValid())
				std::wcout << L"m_pDiffuseMapVariable not valid!\n";
			m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();		// normal
			if (!m_pNormalMapVariable->IsValid())
				std::wcout << L"m_pNormalMapVariable not valid!\n";
			m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();		// specular
			if (!m_pSpecularMapVariable->IsValid())
				std::wcout << L"m_pNormalMapVariable not valid!\n";
			m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();		// glossiness
			if (!m_pGlossinessMapVariable->IsValid())
				std::wcout << L"m_pGlossinessMapVariable not valid!\n";
		}
		~EffectDefault()
		{

			//Textures
			if (m_pDiffuseMapVariable)
				m_pDiffuseMapVariable->Release();
			if (m_pNormalMapVariable)
				m_pNormalMapVariable->Release();
			if (m_pSpecularMapVariable)
				m_pSpecularMapVariable->Release();
			if (m_pGlossinessMapVariable)
				m_pGlossinessMapVariable->Release();


			//Techniques
			if (m_pTechniqueAnisotropic)
				m_pTechniqueAnisotropic->Release();
			if (m_pTechniqueLinear)
				m_pTechniqueLinear->Release();
			if (m_pTechniquePoint)
				m_pTechniquePoint->Release();

		}

		// Rule of 5
		// ------
		EffectDefault(const EffectDefault&) = delete;
		EffectDefault(EffectDefault&&) noexcept = delete;
		EffectDefault& operator=(const EffectDefault&) = delete;
		EffectDefault& operator=(EffectDefault&&) noexcept = delete;

		// Member Functions
		// ------
		void SetDiffuseMap(Texture* pDiffuseTexture)
		{
			if (m_pDiffuseMapVariable)
				m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
		}
		void SetNormalMap(Texture* pNormalTexture) {
			if (m_pNormalMapVariable)
				m_pNormalMapVariable->SetResource(pNormalTexture->GetSRV());
		}
		void SetSpecularMap(Texture* pSpecularTexture) {
			if (m_pSpecularMapVariable)
				m_pSpecularMapVariable->SetResource(pSpecularTexture->GetSRV());
		}
		void SetGlossinessMap(Texture* pGlossinessTexture) {
			if (m_pGlossinessMapVariable)
				m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetSRV());
		}

		virtual ID3DX11EffectTechnique* GetTechnique(const FilteringMethod& filteringMethod) const override
		{
			switch (filteringMethod)
			{
			case FilteringMethod::Point:
				return m_pTechniquePoint;
				break;
			case FilteringMethod::Linear:
				return m_pTechniqueLinear;
				break;
			case FilteringMethod::Anisotropic:
				return m_pTechniqueAnisotropic;
				break;
			default:
				return m_pTechniquePoint;
			}
		};

	private:
		//Technique
		ID3DX11EffectTechnique* m_pTechniquePoint;
		ID3DX11EffectTechnique* m_pTechniqueLinear;
		ID3DX11EffectTechnique* m_pTechniqueAnisotropic;

		//Textures
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;

	};
}