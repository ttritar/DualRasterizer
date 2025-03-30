
//includes
#include "Texture.h"

namespace dae {

	Texture::Texture(ID3D11Device* pDevice,  SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{

		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = pSurface->w;
		desc.Height = pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

		HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);
		if (FAILED(hr) || m_pResource == nullptr) // Check for failure or null resource
		{
			std::cerr << "Failed to create texture2D. HRESULT: " << hr << std::endl;
			SDL_FreeSurface(pSurface); // Free the SDL_Surface since the constructor failed
			pSurface = nullptr;
			return; // Early return on failure
		}


		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);
		if (FAILED(hr) || m_pSRV == nullptr) // Check for failure or null SRV
		{
			std::cerr << "Failed to create shader resource view. HRESULT: " << hr << std::endl;
			m_pResource->Release(); // Clean up the texture resource if the SRV creation fails
			m_pResource = nullptr;

			SDL_FreeSurface(pSurface);
			pSurface = nullptr;
			return; // Early return on failure
		}
	}

	Texture::~Texture()
	{
		if (m_pSRV)
			m_pSRV->Release();

		if (m_pResource)
			m_pResource->Release();

		if (m_pSurface) {
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}




	Texture* Texture::LoadFromFile(const std::string& path, ID3D11Device* pDevice)
	{
		//Load SDL_Surface using IMG_LOAD
		auto SDL_Surf = IMG_Load(path.c_str());
		Texture* pTex = new Texture( pDevice, SDL_Surf );

		return pTex;
	}


	ID3D11ShaderResourceView* Texture::GetSRV()
	{
		return m_pSRV;
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		if (!m_pSurface)
		{
			std::cerr << "Surface was not valid!\n";
			return { 0.f, 0.f, 0.f }; // Return a default color
		}

		int X = static_cast<int>(uv.x * m_pSurface->w);
		int Y = static_cast<int>(uv.y * m_pSurface->h);

		// Clamp X and Y to ensure they are within bounds
		X = Clamp(X, 0, m_pSurface->w - 1);
		Y = Clamp(Y, 0, m_pSurface->h - 1);

		// Fetch pixel data
		Uint8 R, G, B;
		SDL_PixelFormat* formatPtr = m_pSurface->format;
		if (!m_pSurfacePixels) 
		{
			std::cerr << "SurfacePixels was not valid!\n";
			return { 0.f, 0.f, 0.f }; // Return a default color
		}
		SDL_GetRGB(m_pSurfacePixels[X + (Y * m_pSurface->w)], formatPtr, &R, &G, &B);

		// Normalize the colors
		return { R / 255.f,G / 255.f,B / 255.f };
	}
}