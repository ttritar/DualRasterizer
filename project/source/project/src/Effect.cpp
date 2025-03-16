#include "Effect.h"

namespace dae
{
	Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
		:m_pDevice(pDevice)
	{
		// Load the effect using the function defined to the right, store the resulting pointer in a member of type ID3DX11Effect
		m_pEffect = LoadEffect(pDevice, assetFile);

		if (!m_pEffect) std::cerr << "Effect loading has failed!\n";
		
		// VARIABLES
		//---------------

		// Matrices
		m_pMatWorldViewProjectionVariable = m_pEffect->GetVariableByName("gWorldViewProjection")->AsMatrix();	// WorldViewProjection
		if (!m_pMatWorldViewProjectionVariable->IsValid())
			std::wcout << L"m_pMatWorldViewProjectionVariable not valid!\n";

		m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();	// WorldViewProjection
		if (!m_pMatWorldVariable->IsValid())
			std::wcout << L"m_pMatWorldVariable not valid!\n";
		

		m_pCameraPositionVariable = m_pEffect->GetVariableByName("gCameraPosition")->AsVector();		// glossiness
		if (!m_pCameraPositionVariable->IsValid())
			std::wcout << L"m_pCameraPositionVariable not valid!\n";
		
	}

	Effect::~Effect() {
		// Release resources
		//----------------------

		if (m_pCameraPositionVariable)
			m_pCameraPositionVariable->Release();

		//Matricees
		if (m_pMatWorldViewProjectionVariable)
			m_pMatWorldViewProjectionVariable->Release();
		if (m_pMatWorldVariable)
			m_pMatWorldVariable->Release();
		
		//Effect
		if (m_pEffect)
			m_pEffect->Release();

		// Do NOT release m_pDevice here; it is managed at a higher level (e.g., Renderer).

	}



	ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif 

		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss{};
				for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); ++i)
					ss << pErrors[i];

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << std::endl;
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
				std::wcout << ss.str() << std::endl;
				return nullptr;
			}
		}

		return pEffect;
	}


	// SetVariables
	//--------------

	void Effect::SetWorldViewProjectionMatrix(const Matrix& matrix)
	{
		m_pMatWorldViewProjectionVariable->SetMatrix(reinterpret_cast<const float*> (&matrix));
	}
	void Effect::SetWorldMatrix(const Matrix& matrix)
	{
		m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*> (&matrix));
	}

	void Effect::SetCameraPosition(const Vector3& position)
	{
		m_pCameraPositionVariable->SetFloatVector(reinterpret_cast<const float*>( & position));
	}

}
