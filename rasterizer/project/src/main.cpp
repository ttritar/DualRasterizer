#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"Dual Rasterizer - Thalia Tritar",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	// Keybinds std::cout
	{

		// Shared Key Bindings
		std::cout << STRINGCOLOR_SHARED_CYAN << "[Key Bindings - SHARED]" << std::endl;
		std::cout << STRINGCOLOR_SHARED_CYAN << "   [F1]  Toggle Rasterizer Mode (HARDWARE/SOFTWARE)" << std::endl;
		std::cout << STRINGCOLOR_SHARED_CYAN << "   [F2]  Toggle Vehicle Rotation (ON/OFF)" << std::endl;
		std::cout << STRINGCOLOR_SHARED_CYAN << "   [F9]  Cycle CullMode (BACK/FRONT/NONE)" << std::endl;
		std::cout << STRINGCOLOR_SHARED_CYAN << "   [F10] Toggle Uniform ClearColor (ON/OFF)" << std::endl;
		std::cout << STRINGCOLOR_SHARED_CYAN << "   [F11] Toggle Print FPS (ON/OFF)" << std::endl;

		// Hardware Key Bindings
		std::cout << STRINGCOLOR_HARDWARE_YELLOW << "\n[Key Bindings - HARDWARE]" << std::endl;
		std::cout << STRINGCOLOR_HARDWARE_YELLOW << "   [F3] Toggle FireFX (ON/OFF)" << std::endl;
		std::cout << STRINGCOLOR_HARDWARE_YELLOW << "   [F4] Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)" << std::endl;

		// Software Key Bindings
		std::cout <<STRINGCOLOR_SOFTWARE_GREEN << "\n[Key Bindings - SOFTWARE]" << std::endl;
		std::cout <<STRINGCOLOR_SOFTWARE_GREEN << "   [F5] Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)" << std::endl;
		std::cout <<STRINGCOLOR_SOFTWARE_GREEN << "   [F6] Toggle NormalMap (ON/OFF)" << std::endl;
		std::cout <<STRINGCOLOR_SOFTWARE_GREEN << "   [F7] Toggle DepthBuffer Visualization (ON/OFF)" << std::endl;
		std::cout <<STRINGCOLOR_SOFTWARE_GREEN << "   [F8] Toggle BoundingBox Visualization (ON/OFF)\n\n" << STRINGCOLOR_RESET << std::endl;
	}

	// FPS COLOR
	const std::string STRINGCOLOR_RESET = "\033[0m";
	const std::string STRINGCOLOR_GREY = "\033[38;5;238m";

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//SHARED
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)	// Switch RasterizerMode (‘F1’)
					pRenderer->SwitchRasterizerMode();
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)	// Toggle Rotation (Rotate/Idle) (‘F2’)
					pRenderer->ToggleRotation();
				if (e.key.keysym.scancode == SDL_SCANCODE_F9)	// Switch CullMode (‘F9’)
					pRenderer->SwitchCullMode();
				if (e.key.keysym.scancode == SDL_SCANCODE_F10)	// Toggle Uniform Color (‘F10’)
					pRenderer->ToggleUniformColor();
				if (e.key.keysym.scancode == SDL_SCANCODE_F11)	// Toggle FPS Printing (‘F11’)
					pRenderer->TogglePrintFPS();

				//HARDWARE
				if (e.key.keysym.scancode == SDL_SCANCODE_F3)	// Toggle fire mesh (‘F3’)
					pRenderer->ToggleFireMesh();
				if (e.key.keysym.scancode == SDL_SCANCODE_F4)	// Switch filtering mode (‘F4’)
					pRenderer->SwitchFilterMode();

				//SOFTWARE
				if (e.key.keysym.scancode == SDL_SCANCODE_F5)	// Switch ShadingMode (‘F5’)
					pRenderer->SwitchShadingMode();
				if (e.key.keysym.scancode == SDL_SCANCODE_F6)	// Toggle NormalMap (‘F6’)
					pRenderer->ToggleNormalMap();
				if (e.key.keysym.scancode == SDL_SCANCODE_F7)	// Switch DepthBuffer Visualization (‘F7’)
					pRenderer->ToggleDepthVisualization();
				if (e.key.keysym.scancode == SDL_SCANCODE_F8)	// Switch AABB Visualization (‘F8’)
					pRenderer->ToggleAABBVisualization();

				
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			if(pRenderer->m_IsPrintingFPS)
			std::cout<< STRINGCOLOR_GREY << "dFPS: " << pTimer->GetdFPS() <<STRINGCOLOR_RESET<< std::endl;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}