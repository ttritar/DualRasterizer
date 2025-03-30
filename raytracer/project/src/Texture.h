#pragma once

//includes
#include "pch.h"

namespace dae {
	class Texture
	{
	public:
		// Constructor + Destructor
		// ------
		~Texture();

		// Rule of 5
		// ------
		Texture(const Texture&) = delete;
		Texture(Texture&&) noexcept = delete;
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) noexcept = delete;


		// Member Functions
		// ------

		static Texture* LoadFromFile(const std::string& path, ID3D11Device* pDevice);
		ColorRGB Sample(const Vector2& uv) const;

		// Getter func
		ID3D11ShaderResourceView* GetSRV();

	private:
		Texture(ID3D11Device* pDevice, SDL_Surface* pSurface);

		ID3D11Texture2D* m_pResource = nullptr;
		ID3D11ShaderResourceView* m_pSRV = nullptr;

		//software
		SDL_Surface* m_pSurface=nullptr ;
		uint32_t* m_pSurfacePixels=nullptr ;
	};
}