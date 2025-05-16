#pragma once
#include <iostream>
#include "pch.h"
#include "Texture.h"

namespace dae
{
	enum class FilteringMethod
	{
		Point = 0,
		Linear = 1,
		Anisotropic = 2
	};

	class Effect
	{
	public:
		// Constructor + Destructor
		// ------
		Effect(ID3D11Device* pDevice, const std::wstring& assetFile);		
		virtual ~Effect();

		// Rule of 5
		// ------
		Effect(const Effect&) = delete;
		Effect(Effect&&) noexcept = delete;
		Effect& operator=(const Effect&) = delete;
		Effect& operator=(Effect&&) noexcept = delete;


		// Member Functions
		// ------
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		void SetWorldViewProjectionMatrix(const Matrix& matrix);
		void SetWorldMatrix(const Matrix& matrix);

		void SetCameraPosition(const Vector3& position);

		// Getter functions
		ID3DX11Effect* GetEffect()const {
			if(m_pEffect)
				return m_pEffect;
		};
		virtual ID3DX11EffectTechnique* GetTechnique(const FilteringMethod& filteringMethod) const
		{
			return nullptr;
		};


	protected:
		ID3DX11Effect* m_pEffect;
		ID3D11Device* m_pDevice;
		
		//Matrices
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjectionVariable;
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable;

		ID3DX11EffectVectorVariable* m_pCameraPositionVariable;
	};


}