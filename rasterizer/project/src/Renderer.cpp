#include "pch.h"
#include "Renderer.h"
#include "Utils.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);


		// HARDWARE
		//===========	

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			//std::cout << "DirectX is initialized and ready!\n";
		}
		else std::cout << "DirectX initialization failed!\n";


		// SOFTWARE
		//===========	

		// Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
		for (int index{}; index < m_Width * m_Height; ++index)  // depth-buffer
			m_DepthBuffer.push_back(FLT_MAX);




		// Initialise Camera
		m_Camera.Initialize(45.f, { 0.f,0.f,0.f }, m_Width / static_cast<float>(m_Height));
	
		// Initialise Mesh	
		Mesh* pMesh;
		MeshStruct meshStruct;
		std::vector<Vertex_In> vertices;	// are cleared on theyre own when parsing in utils
		std::vector<uint32_t> indices;

		bool i=Utils::ParseOBJ("resources/Kart.obj",vertices,indices);	// tuktuk parsing
		pMesh = new Mesh(m_pDevice, vertices, indices,false);

		meshStruct = MeshStruct{ vertices,indices, PrimitiveTopology::TriangleList	,{}, m_WorldMatrix };
		for (const auto& vertex : meshStruct.vertices)
			meshStruct.vertices_out.push_back({});

		m_MeshesStructVector.push_back(meshStruct);
		m_pMeshesVector.push_back(pMesh);

		Utils::ParseOBJ("resources/fireFX.obj", vertices, indices);	// Fire parsing

		pMesh = new Mesh(m_pDevice, vertices, indices,true);
		meshStruct = MeshStruct{ vertices,indices, PrimitiveTopology::TriangleList	,{}, m_WorldMatrix };	// make a struct
		for (const auto& vertex : meshStruct.vertices)
			meshStruct.vertices_out.push_back({});

		m_MeshesStructVector.push_back(meshStruct);
		m_pMeshesVector.push_back(pMesh);


		// Transform meshesStruct
		for (auto& mesh : m_MeshesStructVector)
			mesh.worldMatrix *= Matrix::CreateTranslation(0.f, 0.f, 50.f);	// move the objects

		// Transform m_WorldMatrix to move the meshes (only "okay" to do like this if they should have the same world matrix)
		m_MeshTransformMatrix *= Matrix::CreateTranslation(0.f, 0.f, 50.f);	// move the objects

	}

	Renderer::~Renderer()
	{
		// Release state
		if (m_pRasterizerState)
			m_pRasterizerState->Release();

		// Release Render Target View
		if (m_pRenderTargetView) {
			m_pRenderTargetView->Release();
			m_pRenderTargetView = nullptr;
		}

		// Release Render Target Buffer
		if (m_pRenderTargetBuffer) {
			m_pRenderTargetBuffer->Release();
			m_pRenderTargetBuffer = nullptr;
		}

		// Release Depth Stencil View
		if (m_pDepthStencilView) {
			m_pDepthStencilView->Release();
			m_pDepthStencilView = nullptr;
		}

		// Release Depth Stencil Buffer
		if (m_pDepthStencilBuffer) {
			m_pDepthStencilBuffer->Release();
			m_pDepthStencilBuffer = nullptr;
		}

		// Release Swap Chain
		if (m_pSwapChain) {
			m_pSwapChain->Release();
			m_pSwapChain = nullptr;
		}

		// Release Device Context
		if (m_pDeviceContext) {
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
			m_pDeviceContext = nullptr;
		}

		// Release Device
		if (m_pDevice) {
			m_pDevice->Release();
			m_pDevice = nullptr;
		}

		// Delete all elements in m_pMeshesVector
		std::for_each(m_pMeshesVector.begin(), m_pMeshesVector.end(), [](Mesh* pMesh) {delete pMesh; });
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);

		// Update rotation
		if (m_IsRotating)
		{
			m_Rotation += M_PI/4* pTimer->GetElapsed();
			for (auto& mesh : m_MeshesStructVector)
				mesh.worldMatrix = Matrix::CreateRotationY(m_Rotation) * Matrix::CreateTranslation(0.f, 0.f, 50.f); ;	// rotate the object 

			m_MeshTransformMatrix = Matrix::CreateRotationY(m_Rotation) * Matrix::CreateTranslation(0.f, 0.f, 50.f); ;	// rotate the object 
		}
		
	}


	void Renderer::Render() 
	{
		// 1. CLEAR RTV & DSV
		constexpr float uniformColor[4] = { .1f,.1f,.1f , 1.f };	// UNIFORM - DarkGrey
		constexpr float softwareColor[4] = { .39f, .39f,.39f, 1.f };	//SOFTWARE - LightGrey
		constexpr float hardwareColor[4] = { .39f,.59f,.93f , 1.f };	//HARDWARE - CornflowerBlue

		if (m_IsUniformColor) m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, uniformColor);
		else
		{
			if (!m_IsSoftwareRasterizer) m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, hardwareColor);
		}

		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);



		if (m_IsSoftwareRasterizer)
		{
			//set color
			ColorRGB color;
			if (m_IsUniformColor) color = { uniformColor[0], uniformColor[2] ,uniformColor[2] };
			else  color = { softwareColor[0], softwareColor[2] ,softwareColor[2] };

			//@START
			//Lock BackBuffer
			SDL_LockSurface(m_pBackBuffer);

			// clear buffers
			SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, color.r*255, color.g*255, color.b*255));
			std::fill(m_DepthBuffer.begin(), m_DepthBuffer.end(), FLT_MAX);


			for (int idx{}; idx<m_MeshesStructVector.size();idx++)
			{
				if (!m_pMeshesVector[idx]->GetIsPartialCoverage())
				{
					auto& mesh = m_MeshesStructVector[idx];

					m_WorldViewProjectionMatrix = mesh.worldMatrix * m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix();
					VertexTransformationFunction(mesh.vertices, mesh.vertices_out, mesh.worldMatrix);

					for (int indicesIdx{};
						mesh.primitiveTopology == PrimitiveTopology::TriangleList ? indicesIdx < mesh.indices.size() / 3	// if it is a triangleList use this loop
						: indicesIdx < m_MeshesStructVector[indicesIdx].indices.size() - 2;	// else (if triangleStrip) use this
						++indicesIdx)	//for each triangle
					{
						// Set vertices
						Vertex_Out v0, v1, v2;
						if (mesh.primitiveTopology == PrimitiveTopology::TriangleList)
						{
							// Set Vertices
							v0 = mesh.vertices_out[mesh.indices[indicesIdx * 3]];
							v1 = mesh.vertices_out[mesh.indices[indicesIdx * 3 + 1]];
							v2 = mesh.vertices_out[mesh.indices[indicesIdx * 3 + 2]];
						}
						else if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip)
						{
							// set vertices depending on even or uneven
							if (indicesIdx % 2 == 0)
							{
								v0 = mesh.vertices_out[mesh.indices[indicesIdx]];
								v1 = mesh.vertices_out[mesh.indices[indicesIdx + 1]];
								v2 = mesh.vertices_out[mesh.indices[indicesIdx + 2]];
							}
							else
							{
								v0 = mesh.vertices_out[mesh.indices[indicesIdx]];
								v1 = mesh.vertices_out[mesh.indices[indicesIdx + 2]];
								v2 = mesh.vertices_out[mesh.indices[indicesIdx + 1]];
							}
						}


						// BOUNDING BOX
						int minX = std::min(v0.position.x, v1.position.x);
						minX = std::min(minX, (int)v2.position.x) - 1;
						if (minX < 0) minX = 0;
						if (minX > (m_Width)) minX = m_Width;

						int maxX = std::max(v0.position.x, v1.position.x);
						maxX = std::max(maxX, (int)v2.position.x) + 1;
						if (maxX < 0) maxX = 0;
						if (maxX > (m_Width)) maxX = m_Width;

						int minY = std::min(v0.position.y, v1.position.y);
						minY = std::min(minY, (int)v2.position.y) - 1;
						if (minY < 0) minY = 0;
						if (minY > (m_Height)) minY = m_Height;

						int maxY = std::max(v0.position.y, v1.position.y);
						maxY = std::max(maxY, (int)v2.position.y) + 1;
						if (maxY < 0) maxY = 0;
						if (maxY > (m_Height)) maxY = m_Height;


						// RENDERING
						for (int px{ minX }; px < maxX; ++px)
						{
							for (int py{ minY }; py < maxY; ++py)
							{
								if (m_ShowAABBVisualization)
								{
									//Update Color in Buffer
									m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
										static_cast<uint8_t>(255),
										static_cast<uint8_t>(255),
										static_cast<uint8_t>(255));
								}
								else 
								{
									Vector2 pixel = { px + 0.5f, py + 0.5f };	// point in middle of pixel (not top left)
									Vertex_Out interpolatedVertex{};

									if (RenderCheckPixel(pixel, v0, v1, v2, interpolatedVertex, m_pMeshesVector[idx]->GetNormalTexture()))
									{

										ColorRGB finalColor;

										// Shade
										if (m_ShowDepthVisualization)
											finalColor = colors::White * Remap(m_DepthBuffer[px + (py * m_Width)], 0.998f, 1.f);
										else
											finalColor = PixelShading(interpolatedVertex,
												m_pMeshesVector[idx]->GetDiffuseTexture(), m_pMeshesVector[idx]->GetSpecularTexture(), m_pMeshesVector[idx]->GetGlossinessTexture());

										finalColor.MaxToOne();
										//Update Color in Buffer
										m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
											static_cast<uint8_t>(finalColor.r * 255),
											static_cast<uint8_t>(finalColor.g * 255),
											static_cast<uint8_t>(finalColor.b * 255));
									}
								}
							}

						}
					}
				}
			}

			//@END
			//Update SDL Surface
			SDL_UnlockSurface(m_pBackBuffer);
			SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
			SDL_UpdateWindowSurface(m_pWindow);

		}
		else
		{
			if (!m_IsInitialized)
				return;

			// 2. SET PIPELINE + INVOKE DRAW CALLS (=RENDER)
			Matrix worldViewProjectionMatrix = m_MeshTransformMatrix * m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix();
			for (auto& mesh : m_pMeshesVector)
			{
				if (m_ShowFireMesh)
					mesh->Render(m_pDeviceContext, m_MeshTransformMatrix, worldViewProjectionMatrix, m_Camera.origin, m_FilteringMethod, m_pRasterizerState);
				else
				{
					if(!mesh->GetIsPartialCoverage())
						mesh->Render(m_pDeviceContext, m_MeshTransformMatrix, worldViewProjectionMatrix, m_Camera.origin, m_FilteringMethod, m_pRasterizerState);
				}
			}


			// 3. PRESENT BACKBUFFER (SWAP)
			m_pSwapChain->Present(0, 0);
		}
	}



	//	HARDWARE RASTERIZING FUNCTIONS
	//==================================

	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device & DeviceContent
		//=====
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG)|| defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
			1,D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

		if (FAILED(result))
			return result;

		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
			return result;



		//2. Create Swapchain
		//====
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_GetVersion(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create Swapchain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;



		//3. Create DepthStencil (DS) & DepthStencilView (DSV)
		//====
		
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;
		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;



		//4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//====
		
		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer,nullptr, &m_pRenderTargetView);
		if (FAILED(result))
			return result;



		//5. Bind RTV & DSV to Output Merger Stage
		//====
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);



		//6. Set Viewport
		//=====
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		
		//7. Set RasterizerState	(cullmode)
		//=====
		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;

		result = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerState);

		if (FAILED(result))	return result;


		//Release DXGIFactory
		pDxgiFactory->Release();



		return S_OK;
	}



	//	SOFTWARE RASTERIZING FUNCTIONS
	//==================================

	// PROJECTION STAGE (+ Rasterization stage)
	void Renderer::VertexTransformationFunction(const std::vector<Vertex_In>& vertices_in, std::vector<Vertex_Out>& vertices_out, const Matrix& worldMatrix) const
	{
		// PROJECTION STAGE
		//========================
		

		// Resize vertex_out vector if size isnt equal
		if (vertices_in.size() != vertices_out.size())
			vertices_out.resize(vertices_in.size());

		// TRANSFORMING VERTICES
		for (int index{}; index < vertices_in.size(); ++index)
		{
			// Transform vertices (store depth in w value)
			vertices_out[index].position = m_WorldViewProjectionMatrix.TransformPoint({ vertices_in[index].position.x,vertices_in[index].position.y,
																								vertices_in[index].position.z, 1.f });
			vertices_out[index].normal = worldMatrix.TransformVector(vertices_in[index].normal).Normalized();
			vertices_out[index].tangent = worldMatrix.TransformVector(vertices_in[index].tangent).Normalized();

			if (m_CurrentShadingMode == ShadingMode::Combined || m_CurrentShadingMode == ShadingMode::Specular)
				vertices_out[index].viewDirection = (worldMatrix.TransformPoint(vertices_in[index].position) - m_Camera.origin).Normalized();	// use the WORLD pos of the vertices

			// Carry over the other values into the out vertices
			//vertices_out[index].color = vertices_in[index].color;
			vertices_out[index].uv = vertices_in[index].uv;



			// Perspective divide
			if (vertices_out[index].position.w > 0.f)
			{
				vertices_out[index].position.x /= vertices_out[index].position.w;
				vertices_out[index].position.y /= vertices_out[index].position.w;
				vertices_out[index].position.z /= vertices_out[index].position.w;
			}



			// RASTERIZATION STAGE
			//========================
		
			// NDC to SCREEN ( RASTERIZATION)
			VertexNDCToScreen(vertices_out[index].position);
		}
	}
	
	// RASTERIZATION STAGE (NDC to raster space)
	void Renderer::VertexNDCToScreen(Vector4& vertexPosition) const
	{
		vertexPosition.x = ((vertexPosition.x + 1) / 2) * m_Width;
		vertexPosition.y = ((1 - vertexPosition.y) / 2) * m_Height;
	}
	// RASTERIZATION STAGE (Is pixel in triangle)
	bool Renderer::RenderCheckPixel(const Vector2& pixel,
		const Vertex_Out& vertex0, const Vertex_Out& vertex1, const Vertex_Out& vertex2,
		Vertex_Out& interpolatedVertex, const Texture* pNormaltexture)
	{

		Vector4 v0 = vertex0.position;
		Vector4 v1 = vertex1.position;
		Vector4 v2 = vertex2.position;

		////////////////////
		// Weights and Depth
		////////////////////

		Vector2 v0ToPixel = pixel - Vector2{ v0.x, v0.y };
		Vector2 v1ToPixel = pixel - Vector2{ v1.x, v1.y };
		Vector2 v2ToPixel = pixel - Vector2{ v2.x, v2.y };

		// Calculate CROSS
		float v0Weight = Vector2::Cross(Vector2{ v2.x - v1.x,v2.y - v1.y }, v1ToPixel);
		float v1Weight = Vector2::Cross(Vector2{ v0.x - v2.x,v0.y - v2.y }, v2ToPixel);
		float v2Weight = Vector2::Cross(Vector2{ v1.x - v0.x,v1.y - v0.y }, v0ToPixel);
		//if (v0Weight > 0) return false;
		//if (v1Weight > 0) return false;
		//if (v2Weight > 0) return false;

		// Face culling
		switch (m_CurrentCullMode)
		{
		case dae::None:
			if (v0Weight > 0)
			{
				if (v1Weight < 0) return false;
				if (v2Weight < 0) return false;
			}
			else
			{
				if (v0Weight > 0) return false;
				if (v1Weight > 0) return false;
				if (v2Weight > 0) return false;
			}
			break;
		case dae::Back:
			if (v0Weight < 0) return false;
			if (v1Weight < 0) return false;
			if (v2Weight < 0) return false;
			break;
		case dae::Front:
			if (v0Weight > 0) return false;
			if (v1Weight > 0) return false;
			if (v2Weight > 0) return false;
			break;
		default:
			break;
		}

		// Calculate WEIGHTS
		float totalArea = Vector2::Cross(Vector2{ v1.x - v0.x, v1.y - v0.y }, Vector2{ v2.x - v0.x, v2.y - v0.y });

		v0Weight /= totalArea;
		v1Weight /= totalArea;
		v2Weight /= totalArea;

		// Perspective-correct weights
		float denomDepth = v0Weight / v0.z + v1Weight / v1.z + v2Weight / v2.z;
		float denomDepthW = v0Weight / v0.w + v1Weight / v1.w + v2Weight / v2.w;
		float w0 = (v0Weight / vertex0.position.w) / denomDepthW;
		float w1 = (v1Weight / vertex1.position.w) / denomDepthW;
		float w2 = (v2Weight / vertex2.position.w) / denomDepthW;

		// Interpolate depth
		float interpolatedDepth = 1.0f / denomDepth;

		// Depth Check
		if (interpolatedDepth >= 0 and
			interpolatedDepth <= 1)
		{
			if (m_DepthBuffer[(int)pixel.x + (int)pixel.y * m_Width] >= interpolatedDepth)
			{
				m_DepthBuffer[(int)pixel.x + (int)pixel.y * m_Width] = interpolatedDepth;
			}
			else return false; // if not in front, dont render
		}
		else return false; // if not in 0,1 range, dont render


		// Interpolate vertices 
		interpolatedVertex.normal = (vertex0.normal * w0 + vertex1.normal * w1 + vertex2.normal * w2).Normalized();
		interpolatedVertex.uv = vertex0.uv * w0 + vertex1.uv * w1 + vertex2.uv * w2; // Interpolate UV using perspective-correct weights
		//interpolatedVertex.color = vertex0.color * w0 + vertex1.color * w1 + vertex2.color * w2;
		if (m_CurrentShadingMode == ShadingMode::Combined || m_CurrentShadingMode == ShadingMode::Specular)
			interpolatedVertex.viewDirection = (vertex0.viewDirection * w0 + vertex1.viewDirection * w1 + vertex2.viewDirection * w2).Normalized();
		//interpolatedVertex.position = Vector4{w0,w1,w2, interpolatedDepth };	//for debug



		if (m_IsNormalMapVisible)
		{
			interpolatedVertex.tangent = (vertex0.tangent * w0 + vertex1.tangent * w1 + vertex2.tangent * w2).Normalized();

			Vector3 binormal = dae::Vector3::Cross(interpolatedVertex.normal, interpolatedVertex.tangent).Normalized();

			dae::Matrix tangentSpaceAxis = { interpolatedVertex.tangent, binormal, interpolatedVertex.normal, Vector3::Zero };

			Vector3 sampledNormal = { pNormaltexture->Sample(interpolatedVertex.uv).r, pNormaltexture->Sample(interpolatedVertex.uv).g, pNormaltexture->Sample(interpolatedVertex.uv).b };

			//	sampledNormal /= 255;		//-> not necessary, texture sample output is already 0-1
			sampledNormal = 2.f * sampledNormal - Vector3{ 1.f, 1.f, 1.f };
			sampledNormal = tangentSpaceAxis.TransformVector(sampledNormal).Normalized();

			interpolatedVertex.normal = sampledNormal;

		}

		return true;
	}

	// Shading
	ColorRGB Renderer::PixelShading(const Vertex_Out& vertex,
		const Texture* pDiffuseTexture, const Texture* pSpecularTexture, const Texture* pGlossinessTexture)
	{
		ColorRGB lightContribution;

		// Hardcoded directional light
		Vector3 lightDirection = -Vector3{ .577f, -.577f, .577f };


		// Lambert cos
		float lambertsCos = Vector3::Dot(vertex.normal, lightDirection);
		Clamp(lambertsCos, 0.f, 1.f);

		// Lambert diffuse
		const float lightIntesity = 7.f;

		// Specular
		float specularReflectionCoefficient;
		const float shininess = 25.f; // Multiply our sampled exponent with this;
		float phongExponent;
		Vector3 reflectVector;
		float cosAlpha;

		// Ambient
		const ColorRGB ambient = { .025f,.025f,.025f };
			
		if (lambertsCos > 0)
		{
			switch (m_CurrentShadingMode)
			{
			case dae::ShadingMode::ObservedArea: // Lambert Cosine
				lightContribution = { lambertsCos, lambertsCos, lambertsCos };
				break;

			case dae::ShadingMode::Diffuse: // Lambert diffuse
				lightContribution = (lambertsCos * lightIntesity * pDiffuseTexture->Sample(vertex.uv)) / M_PI;
				break;

			case dae::ShadingMode::Specular: // Phong specular
				specularReflectionCoefficient = pSpecularTexture->Sample(vertex.uv).r;
				phongExponent = pGlossinessTexture->Sample(vertex.uv).r * shininess;	// Since these textures store their values in grey scale, we can use the red channel only (optimized)

				if (specularReflectionCoefficient > 0.0f)
				{
					reflectVector = (lightDirection - (2 * Vector3::Dot(vertex.normal, lightDirection) * vertex.normal)).Normalized();
					cosAlpha = Vector3::Dot(reflectVector, vertex.viewDirection);
					cosAlpha = std::max(cosAlpha, 0.f);

					lightContribution = specularReflectionCoefficient * (pow(cosAlpha, phongExponent)) * ColorRGB { 1.f, 1.f, 1.f };
				}
				else lightContribution = colors::Black;
				break;

			case dae::ShadingMode::Combined:
				// Lambert diffuse
				lightContribution = (lambertsCos * lightIntesity * pDiffuseTexture->Sample(vertex.uv)) / M_PI;

				//Phong
				specularReflectionCoefficient = pSpecularTexture->Sample(vertex.uv).r;
				phongExponent = pGlossinessTexture->Sample(vertex.uv).r * shininess;	// Since these textures store their values in grey scale, we can use the red channel only (optimized)

				if (specularReflectionCoefficient > 0.0f)
				{
					reflectVector = (lightDirection - (2 * Vector3::Dot(vertex.normal, lightDirection) * vertex.normal)).Normalized();
					cosAlpha = Vector3::Dot(reflectVector, vertex.viewDirection);
					cosAlpha = std::max(cosAlpha, 0.f);

					lightContribution += specularReflectionCoefficient * (pow(cosAlpha, phongExponent)) * ColorRGB { 1.f, 1.f, 1.f };
				}

				// Ambient
				lightContribution += ambient;
				break;
			}
		}
		return lightContribution;
	}

}
