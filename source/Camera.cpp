#include "pch.h"
#include "Camera.h"

Camera::Camera(float aspectRatio, const dae::Vector3& origin, float fovAngle)
	: m_AspectRatio(aspectRatio)
	, m_Origin{origin}
	, m_FovAngle(fovAngle)
{
	m_Fov = tanf((fovAngle * dae::TO_RADIANS) / 2.f);

	CalculateProjectionMatrix();
}

void Camera::Update(const dae::Timer* pTimer)
{
	const float deltaTime = pTimer->GetElapsed();

	const float originalFOV{ m_Fov };
	const float originalAspectRatio{ m_AspectRatio };

	// Camera Update Logic
	//Keyboard Input
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	//Mouse Input
	int mouseX{}, mouseY{};
	const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

	//UpdatesMovement
	//FOVInput(pKeyboardState, deltaTime);
	MovementInput(pKeyboardState, deltaTime);

	// Handles mouseInput, if any input
	if (mouseX != 0 || mouseY != 0) MouseInput(pKeyboardState, mouseState, mouseX, mouseY, deltaTime);


	//Update Matrices
	CalculateViewMatrix();

	if ((originalFOV != m_Fov) || (originalAspectRatio != m_AspectRatio))
	{
		CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
	}
}

dae::Matrix Camera::GetInvViewMatrix() const
{
	return m_InvViewMatrix;
}

dae::Matrix Camera::GetProjectionMatrix() const
{
	return m_ProjectionMatrix;
}

dae::Vector3 Camera::GetOrigin() const
{
	return m_Origin;
}

void Camera::CalculateViewMatrix()
{
	m_RightVector = dae::Vector3::Cross(dae::Vector3::UnitY, m_ForwardVector).Normalized();
	m_UpVector = dae::Vector3::Cross(m_ForwardVector, m_RightVector).Normalized();

	m_ViewMatrix = dae::Matrix::CreateLookAtLH(m_Origin, m_ForwardVector, m_UpVector, m_RightVector);
	m_InvViewMatrix = dae::Matrix::Inverse(m_ViewMatrix);
}

void Camera::CalculateProjectionMatrix()
{
	const float nearPlane{ 0.1f };
	const float farPlane{ 100.0f };

	m_ProjectionMatrix = dae::Matrix::CreatePerspectiveFovLH(m_Fov, m_AspectRatio, nearPlane, farPlane);
}


void Camera::FOVInput(const uint8_t* pKeyboardState, float deltaTime)
{
	// FOV Change
	float angleSpeed{ 10.f };
	const float maxAngle{ 180 }, minAngle{ 0 };

	if (pKeyboardState[SDL_SCANCODE_UP])
	{
		// Ensurance to not overstretch
		if (m_FovAngle == maxAngle - 1)
		{
			return;
		}

		m_FovAngle += angleSpeed * deltaTime;
	}
	else if (pKeyboardState[SDL_SCANCODE_DOWN])
	{
		// Ensurance to not overstretch
		if (m_FovAngle == minAngle + 1)
		{
			return;
		}

		m_FovAngle -= angleSpeed * deltaTime;
	}
}

void Camera::MovementInput(const uint8_t* pKeyboardState, float deltaTime)
{
	float movementSpeed{ 5.f };
	dae::Vector3 rightVector{ dae::Vector3::Cross(dae::Vector3::UnitY,m_ForwardVector).Normalized() };

	// Boost
	if (pKeyboardState[SDL_SCANCODE_LSHIFT])
	{
		movementSpeed *= 2.f;
	}

	// Z-axis
	if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
	{
		m_Origin += m_ForwardVector * movementSpeed * deltaTime;
	}
	else if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
	{
		m_Origin -= m_ForwardVector * movementSpeed * deltaTime;
	}

	// Y-axis
	if (pKeyboardState[SDL_SCANCODE_Q])
	{
		m_Origin -= m_UpVector * movementSpeed * deltaTime;
	}
	else if (pKeyboardState[SDL_SCANCODE_E])
	{
		m_Origin += m_UpVector * movementSpeed * deltaTime;
	}

	// X-axis
	if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
	{
		m_Origin -= rightVector * movementSpeed * deltaTime;
	}
	else if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
	{
		m_Origin += rightVector * movementSpeed * deltaTime;
	}
}

void Camera::MouseInput(const uint8_t* pKeyboardState, const uint32_t mouseState, int mouseX, int mouseY, float deltaTime)
{
	float movementSpeed{ 3.f };

	// Boost
	if (pKeyboardState[SDL_SCANCODE_LCTRL])
	{
		movementSpeed *= 50.f;
	}

	if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) && mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
	{
		// Movement
		m_Origin += m_UpVector * float(mouseY) * movementSpeed * deltaTime;
	}
	else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
	{
		// Rotation
		m_TotalPitch -= float(mouseY) * movementSpeed * deltaTime;
		m_TotalYaw += float(mouseX) * movementSpeed * deltaTime;
	}
	else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		// Rotation
		m_TotalYaw += float(mouseX) * movementSpeed * deltaTime;

		// Movement
		m_Origin += m_ForwardVector * float(mouseY) * movementSpeed * deltaTime;
	}

	const dae::Matrix rotationMatrix{ dae::Matrix::CreateRotation(m_TotalPitch * dae::TO_RADIANS,m_TotalYaw * dae::TO_RADIANS,0) };
	m_ForwardVector = rotationMatrix.TransformVector(dae::Vector3::UnitZ);
	m_ForwardVector.Normalize();
}