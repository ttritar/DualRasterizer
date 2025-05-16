#pragma once

//includes
#include "Math.h"

namespace dae{

	struct Camera {

		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle, float _aspectRatio) :
			origin{ _origin },
			fovAngle{ _fovAngle },
			aspectRatio{ _aspectRatio }
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		float aspectRatio{};
		float nearPlane{ 0.1f };
		float farPlane{ 100.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float initialFOV{}; //dont touch 
		float initialAspectRatio{}; //dont touch
		float totalPitch{};
		float totalYaw{};
		


		void Initialize(float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f }, float _aspectRatio = { 0.f })
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;

			aspectRatio = _aspectRatio;
		}

		Matrix GetViewMatrix() const
		{
			Matrix invViewMatrix{};
			Matrix viewMatrix{};

			// = CameraToWorld
			invViewMatrix = Matrix::CreateLookAtLH(origin, forward, up);

			// Calculate WorldToCamera / View Matrix
			viewMatrix = Matrix::Inverse(invViewMatrix);

			return viewMatrix;
		};
		Matrix GetProjectionMatrix() const
		{
			Matrix projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);

			return projectionMatrix;
		};

		void Update(const Timer* pTimer)
		{
			//Camera Update Logic
			const float deltaTime = pTimer->GetElapsed();
 
			float moveSpeed{ 30.f };
			const float rotationSpeed{ 30.f };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			if (pKeyboardState[SDL_SCANCODE_LSHIFT])	moveSpeed *= 2;		 // Give movementspeed boost 
			

			if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])	// Move (local) Forward (Arrow Up) and (‘W’)
			{
				origin += forward * deltaTime * moveSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])	// Move (local) Left (Arrow Left) and (‘A’)
			{
				origin += right * deltaTime * moveSpeed * -1;
			}
			if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])	// Move (local) Backward (Arrow Down) and (‘S’)
			{
				origin += forward * deltaTime * moveSpeed * -1;
			}
			if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])	// Move (local) Right (Arrow Right) and (‘D’)
			{
				origin += right * deltaTime * moveSpeed;
			}

			

			//Mouse Input

			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			Matrix finalRotation{};

			if (mouseState == 5) // Move (world) Up/Down (LMB + RMB + Mouse Move Y)
			{
				origin += up * float(mouseY);
			}
			else if (mouseState == 1)
			{
				totalYaw += float(mouseX) / 360 * float(M_PI);	// Rotate Yaw (LMB + Mouse Move X)
				origin -= forward * mouseY * 0.2f;	// Move (local) Forward/Backward (LMB + Mouse Move Y)

				finalRotation = Matrix::CreateRotation(totalPitch, totalYaw, 0);

				forward = finalRotation.TransformVector(Vector3::UnitZ);
				forward.Normalize();

				up = finalRotation.TransformVector(Vector3::UnitY);
				up.Normalize();

				right = finalRotation.TransformVector(Vector3::UnitX);
				right.Normalize();
			}
			else if (mouseState == 4)
			{
				totalYaw += float(mouseX) / 360 * float(M_PI);	// Rotate Yaw (LMB + Mouse Move X)
				totalPitch -= float(mouseY) / 360 * float(M_PI);	// Rotate Pitch (RMB + Mouse Move Y)

				finalRotation = Matrix::CreateRotation(totalPitch, totalYaw, 0);

				forward = finalRotation.TransformVector(Vector3::UnitZ);
				forward.Normalize();

				up = finalRotation.TransformVector(Vector3::UnitY);
				up.Normalize();

				right = finalRotation.TransformVector(Vector3::UnitX);
				right.Normalize();
			}
		}
	};

}