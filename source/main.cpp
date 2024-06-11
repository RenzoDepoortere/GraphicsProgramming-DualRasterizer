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
		"DualRasterizer - Renzo Depoortere - 2DAE07",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);
	bool printFPS{ false };

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
				
				switch (e.key.keysym.sym)
				{
				case SDLK_F1:
					pRenderer->ToggleRenderer();
					break;

				case SDLK_F2:
					pRenderer->ToggleSpinning();
					break;

				case SDLK_F3:
					pRenderer->ToggleFireFX();
					break;

				case SDLK_F4:
					pRenderer->ToggleFilterMethods();
					break;

				case SDLK_F5:
					pRenderer->ToggleShadingMode();
					break;

				case SDLK_F6:
					pRenderer->ToggleNormalMap();
					break;

				case SDLK_F7:
					pRenderer->ToggleDepthBuffer();
					break;

				case SDLK_F8:
					pRenderer->ToggleBoundingBox();
					break;

				case SDLK_F9:
					pRenderer->ToggleCullingMode();
					break;

				case SDLK_F10:
					pRenderer->ToggleUniformClearColor();
					break;

				case SDLK_F11:
					printFPS = !printFPS;

					if (printFPS)
					{
						std::cout << "FPS printing enabled" << std::endl;
					}
					else
					{
						std::cout << "FPS printing disabled" << std::endl;
					}

					break;
				}

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

		if (printFPS)
		{
			printTimer += pTimer->GetElapsed();
			if (printTimer >= 1.f)
			{
				printTimer = 0.f;
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}