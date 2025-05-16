#pragma once
#include "Mesh.h"
#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	enum CullMode {
		None,Back,Front
	};
	enum ShadingMode
	{
		ObservedArea = 0,	// Lambert Cosine Law
		Diffuse = 1,		// Diffuse lambert
		Specular = 2,		// phong
		Combined = 3		// ObservedArea*Radiance*BRDF
	};

	// ANSI color codes
	const std::string STRINGCOLOR_RESET = "\033[0m";
	const std::string STRINGCOLOR_SHARED_CYAN = "\033[36m";
	const std::string STRINGCOLOR_HARDWARE_YELLOW = "\033[33m";
	const std::string STRINGCOLOR_SOFTWARE_GREEN = "\033[32m";


	class Renderer final
	{
	public:
		// Ctor and Dtor
		//==============
		Renderer(SDL_Window* pWindow);
		~Renderer();

		// Rule Of 5
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;


		void Update(const Timer* pTimer);
		void Render() ;


		// SOFTWARE SPECIFIC RENDERING
		//=============================

		bool RenderCheckPixel(const Vector2& pixel,
			const Vertex_Out& vertex0, const Vertex_Out& vertex1, const Vertex_Out& vertex2,
			Vertex_Out& interpolatedVertex, const Texture* pNormaltexture);
		void VertexTransformationFunction(const std::vector<Vertex_In>& vertices_in, std::vector<Vertex_Out>& vertices_out, const Matrix& worldMatrix) const;
		void VertexNDCToScreen(Vector4& vertexPosition) const;
		ColorRGB PixelShading(const Vertex_Out& vertex,
			const Texture* pDiffuseTexture, const Texture* pSpecularTexture, const Texture* pGlossinessTexture);


		// TOGGLES
		//=========
		
		//shared functions
		void SwitchRasterizerMode() {
			std::cout << STRINGCOLOR_SHARED_CYAN << " **(SHARED) ";
			m_IsSoftwareRasterizer = !m_IsSoftwareRasterizer;

			if (m_IsSoftwareRasterizer)	std::cout << "Rasterizer Mode = SOFTWARE\n" << STRINGCOLOR_RESET;
			else std::cout << "Rasterizer Mode = HARDWARE\n" << STRINGCOLOR_RESET;
		};  //F1
		void ToggleRotation() {
			std::cout << STRINGCOLOR_SHARED_CYAN << " **(SHARED) ";
			m_IsRotating = !m_IsRotating;
			if (m_IsRotating)	std::cout << "Vehicle Rotation ON\n"<<STRINGCOLOR_RESET;
			else 	std::cout << "Vehicle Rotation OFF\n"<<STRINGCOLOR_RESET;
		};
		void SwitchCullMode()
		{
			std::cout << STRINGCOLOR_SHARED_CYAN << " **(SHARED) ";

			// Delete current rasterizer state
			if (m_pRasterizerState)
				m_pRasterizerState->Release();
			D3D11_RASTERIZER_DESC rasterizerDesc = {};
			HRESULT result{};

			switch (m_CurrentCullMode)
			{
			case CullMode::None:
				// Create new rasterizerstate
				rasterizerDesc.CullMode = D3D11_CULL_BACK;
				rasterizerDesc.FillMode = D3D11_FILL_SOLID;

				result = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerState);

				if (FAILED(result))//check if failed
				{
					std::cout << "Switching cullmode failed!! :(\n";
					return;
				}

				std::cout << "CullMode = BACK\n"<<STRINGCOLOR_RESET;
				m_CurrentCullMode = CullMode::Back;	//update enum
				break;
			case CullMode::Back:
				rasterizerDesc.CullMode = D3D11_CULL_FRONT;
				rasterizerDesc.FillMode = D3D11_FILL_SOLID;

				result = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerState);
				if (FAILED(result))
				{
					std::cout << "Switching cullmode failed!! :(\n";
					return;
				}

				std::cout << "CullMode = FRONT\n"<<STRINGCOLOR_RESET;
				m_CurrentCullMode = CullMode::Front;
				break;
			case CullMode::Front:
				rasterizerDesc.CullMode = D3D11_CULL_NONE;
				rasterizerDesc.FillMode = D3D11_FILL_SOLID;

				result = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerState);
				if (FAILED(result))
				{
					std::cout << "Switching cullmode failed!! :(\n";
					return;
				}

				std::cout << "CullMode = NONE\n"<<STRINGCOLOR_RESET;
				m_CurrentCullMode = CullMode::None;
				break;
			}
		}
		void ToggleUniformColor() {
			std::cout << STRINGCOLOR_SHARED_CYAN << " **(SHARED) ";
			m_IsUniformColor = !m_IsUniformColor;

			if (m_IsUniformColor)	std::cout << "Uniform ClearColor ON\n"<<STRINGCOLOR_RESET;
			else 	std::cout << "Uniform ClearColor OFF\n"<<STRINGCOLOR_RESET;
		};
		void TogglePrintFPS() {
			std::cout << STRINGCOLOR_SHARED_CYAN << " **(SHARED) ";
			m_IsPrintingFPS = !m_IsPrintingFPS;

			if (m_IsPrintingFPS)	std::cout << "Print FPS ON\n"<<STRINGCOLOR_RESET;
			else std::cout << "Print FPS OFF\n"<<STRINGCOLOR_RESET;
		};
	
		//hardware-only
		void ToggleFireMesh() {
			if (!m_IsSoftwareRasterizer)
			{
				std::cout << STRINGCOLOR_HARDWARE_YELLOW << " **(HARDWARE) ";
				m_ShowFireMesh = !m_ShowFireMesh;

				if (m_ShowFireMesh)	std::cout << "FireFX ON\n" << STRINGCOLOR_RESET;
				else std::cout << "FireFX OFF\n" << STRINGCOLOR_RESET;
			}
		};
		void SwitchFilterMode()
		{
			if (!m_IsSoftwareRasterizer)
			{
				std::cout << STRINGCOLOR_HARDWARE_YELLOW << " **(HARDWARE) ";
				switch (m_FilteringMethod)
				{
				case dae::FilteringMethod::Point:
					m_FilteringMethod = FilteringMethod::Linear;
					std::cout << "Sampler Filter = LINEAR\n" << STRINGCOLOR_RESET;
					break;
				case dae::FilteringMethod::Linear:
					m_FilteringMethod = FilteringMethod::Anisotropic;
					std::cout << "Sampler Filter = ANISOTRPIC\n" << STRINGCOLOR_RESET;
					break;
				case dae::FilteringMethod::Anisotropic:
					m_FilteringMethod = FilteringMethod::Point;
					std::cout << "Sampler Filter = POINT\n" << STRINGCOLOR_RESET;
					break;
				default:
					m_FilteringMethod = FilteringMethod::Point;
					std::cout << "Sampler Filter = POINT\n" << STRINGCOLOR_RESET;
					break;
				}
			}
		};
		
		//software-only
		void SwitchShadingMode()	//  change lighting mode
		{
			if (m_IsSoftwareRasterizer)
			{
				std::cout << STRINGCOLOR_SOFTWARE_GREEN << " **(SOFTWARE) ";
				switch (m_CurrentShadingMode)
				{
				case dae::ObservedArea:
					m_CurrentShadingMode = Diffuse;
					std::cout << "Shading Mode = DIFFUSE\n" << STRINGCOLOR_RESET;
					break;
				case dae::Diffuse:
					m_CurrentShadingMode = Specular;
					std::cout << "Shading Mode = SPECULAR\n" << STRINGCOLOR_RESET;
					break;
				case dae::Specular:
					m_CurrentShadingMode = Combined;
					std::cout << "Shading Mode = COMBINED\n" << STRINGCOLOR_RESET;
					break;
				case dae::Combined:
					m_CurrentShadingMode = ObservedArea;
					std::cout << "Shading Mode = OBSERVED_AREA\n"<<STRINGCOLOR_RESET;
					break;
				default:
					m_CurrentShadingMode = Combined;
					std::cout << "Shading Mode = COMBINED\n" << STRINGCOLOR_RESET;
					break;
				}
			}
		};
		void ToggleNormalMap() {
			if (m_IsSoftwareRasterizer)
			{
				std::cout << STRINGCOLOR_SOFTWARE_GREEN << " **(SOFTWARE) ";
				m_IsNormalMapVisible = !m_IsNormalMapVisible;

				if (m_IsNormalMapVisible) std::cout << "NormalMap ON\n" << STRINGCOLOR_RESET;
				else std::cout << "NormalMap OFF\n" << STRINGCOLOR_RESET;
			}
		};
		void ToggleDepthVisualization() {
			if (m_IsSoftwareRasterizer)
			{
				std::cout << STRINGCOLOR_SOFTWARE_GREEN << " **(SOFTWARE) ";
				m_ShowDepthVisualization = !m_ShowDepthVisualization;

				if (m_ShowDepthVisualization) std::cout << "DepthBuffer Visualization ON\n" << STRINGCOLOR_RESET;
				else std::cout << "DepthBuffer Visualization OFF\n" << STRINGCOLOR_RESET;
			}
		};
		void ToggleAABBVisualization() {
			if (m_IsSoftwareRasterizer)
			{
				std::cout << STRINGCOLOR_SOFTWARE_GREEN << " **(SOFTWARE) ";
				m_ShowAABBVisualization = !m_ShowAABBVisualization;

				if (m_ShowAABBVisualization) std::cout << "BoundingBox Visualization ON\n" << STRINGCOLOR_RESET;
				else std::cout << "BoundingBox Visualization OFF\n" << STRINGCOLOR_RESET;
			}
		};

		bool m_IsPrintingFPS{ 0 };

	private:
		bool m_IsSoftwareRasterizer{};
		bool m_IsRotating{1};	
		bool m_ShowFireMesh{ 1 };	
		FilteringMethod m_FilteringMethod{}; 
		ShadingMode m_CurrentShadingMode{ ShadingMode::Combined };
		bool m_IsNormalMapVisible{ 1 };
		bool m_ShowDepthVisualization{ 0 };
		bool m_ShowAABBVisualization{ 0 };
		CullMode m_CurrentCullMode{None};	//F9 -shared
		bool m_IsUniformColor{0};
		
		


		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		std::vector<Mesh*> m_pMeshesVector;
		std::vector<MeshStruct> m_MeshesStructVector;

		const Matrix m_WorldMatrix{ {1,0,0,0},{0,1,0,0},{0,0,1,0} ,{0,0,0,1} };;
		Matrix m_MeshTransformMatrix= m_WorldMatrix;
		float m_Rotation{};

		Camera m_Camera{};
	

		// SOFTWARE
		//===========	

		// buffers
		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		std::vector <float> m_DepthBuffer;

		Matrix m_WorldViewProjectionMatrix;


		// HARDWARE
		//===========	






		//DIRECTX
		//======
		HRESULT InitializeDirectX();

		//Device & DeviceContent
		ID3D11Device* m_pDevice = nullptr;
		ID3D11DeviceContext* m_pDeviceContext = nullptr;

		//SwapChain
		IDXGISwapChain* m_pSwapChain = nullptr;
		
		//DepthStencil (DS) & DepthStencilView (DSV)
		ID3D11Texture2D* m_pDepthStencilBuffer = nullptr;
		ID3D11DepthStencilView* m_pDepthStencilView = nullptr;

		//RenderTarget (RT) & RenderTargetView (RTV)
		ID3D11Resource* m_pRenderTargetBuffer = nullptr;
		ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

		//RasterizerState
		ID3D11RasterizerState* m_pRasterizerState;	//F9 -shared




			
	};
}
