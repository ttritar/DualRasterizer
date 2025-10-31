#include "Mesh.h"

namespace dae {

	Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex_In>& vertices, const std::vector<uint32_t>& indices, bool isPartialCoverage)
		:m_IsPartialCoverage{ isPartialCoverage }
	{
		// Create an instance of the effect class you just created
		if(isPartialCoverage)
			m_pEffect = new EffectPartialCoverage(pDevice, L"../../../../../resources/PosCol3D_PartialCoverage.fx");
		else 
			m_pEffect = new EffectDefault(pDevice, L"../../../../../resources/PosCol3D.fx");

		m_pTechnique = m_pEffect->GetTechnique(m_FilteringMethod);

		if (isPartialCoverage)
		{
			m_pDiffuseTexture = Texture::LoadFromFile("resources/fireFX_diffuse.png", pDevice);

			m_pNormalTexture =		nullptr;
			m_pSpecularTexture =	nullptr;
			m_pGlossinessTexture =	nullptr;
		}
		else
		{
			m_pDiffuseTexture = Texture::LoadFromFile("resources/Kart_diffuse.png", pDevice);
			m_pNormalTexture = Texture::LoadFromFile("resources/Kart_normal.png", pDevice);
			m_pSpecularTexture = Texture::LoadFromFile("resources/Kart_specular.png", pDevice);
			m_pGlossinessTexture = Texture::LoadFromFile("resources/Kart_gloss.png", pDevice);
		}
			


		// Create the vertex layout
		static constexpr uint32_t numElements{ 4 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "TEXCOORD";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "NORMAL";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = 20;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "TANGENT";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[3].AlignedByteOffset = 32;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		// Create the input layout
		D3DX11_PASS_DESC passDesc{};
		m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		/*const*/ HRESULT result = pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pInputLayout);

		if (FAILED(result))
			assert(false); //or return

		// Create vertex buffer
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t> (vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = vertices.data();


		/*HRESULT*/ result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
		if (FAILED(result))
			result;

		// Create index buffer
		m_NumIndices = static_cast<uint32_t>(indices.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		initData.pSysMem = indices.data();
		result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
		if (FAILED(result))
			return;

	}

	Mesh::~Mesh()
	{
		// Release resources - oposite order of constr

		if (m_pIndexBuffer)
			m_pIndexBuffer->Release();

		if (m_pVertexBuffer)
			m_pVertexBuffer->Release();

		if (m_pInputLayout)
			m_pInputLayout->Release();

		if (m_pTechnique)
			m_pTechnique->Release();

		delete m_pDiffuseTexture; 
		delete m_pNormalTexture;
		delete m_pSpecularTexture;
		delete m_pGlossinessTexture;

		delete m_pEffect;
		
	}


	void Mesh::Render(ID3D11DeviceContext* pDeviceContext, const Matrix& worldMatrix, const Matrix& worldViewProjectionMatrix,const Vector3& cameraPos, const FilteringMethod& filteringMethod,  ID3D11RasterizerState* pRasterizerState)

	{
		if (pRasterizerState)
			pDeviceContext->RSSetState(pRasterizerState);

		//1. Set Primitive Topology
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//2. Set Input Layout
		pDeviceContext->IASetInputLayout(m_pInputLayout);

		//3. Set VertexBuffer
		constexpr UINT stride = sizeof(Vertex);
		constexpr UINT offset = 0;
		pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

		//4. Set Matrices + Pos
		m_pEffect->SetWorldViewProjectionMatrix(worldViewProjectionMatrix);
		m_pEffect->SetWorldMatrix(worldMatrix);

		m_pEffect->SetCameraPosition(cameraPos);

		//5. Set IndexBuffer
		pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		if (m_IsPartialCoverage)
		{
			static_cast<EffectPartialCoverage*>(m_pEffect)->SetDiffuseMap(m_pDiffuseTexture);
		}
		else
		{
			static_cast<EffectDefault*>(m_pEffect)->SetDiffuseMap(m_pDiffuseTexture);
			static_cast<EffectDefault*>(m_pEffect)->SetNormalMap(m_pNormalTexture);
			static_cast<EffectDefault*>(m_pEffect)->SetSpecularMap(m_pSpecularTexture);
			static_cast<EffectDefault*>(m_pEffect)->SetGlossinessMap(m_pGlossinessTexture);
		}
		


		//6. Draw
		D3DX11_TECHNIQUE_DESC techDesc{};
		m_FilteringMethod = filteringMethod;
		m_pEffect->GetTechnique(m_FilteringMethod)->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			m_pEffect->GetTechnique(m_FilteringMethod)->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}

	}



};