#pragma once

class Camera final
{
public:

	// Constructor and Destructor
	explicit Camera(float aspectRatio, const dae::Vector3& origin = { 0,0,0 }, float fovAngle = 90.f);
	~Camera() = default;

	// Rule of Five
	Camera(const Camera&) = delete;
	Camera(Camera&&) noexcept = delete;
	Camera& operator=(const Camera&) = delete;
	Camera& operator=(Camera&&) noexcept = delete;


	// Public Functions
	void Update(const dae::Timer* pTimer);

	dae::Matrix GetInvViewMatrix() const;
	dae::Matrix GetProjectionMatrix() const;

	dae::Vector3 GetOrigin() const;

private:

	// Member Variables
	// ==================

	float m_AspectRatio{};
	dae::Vector3 m_Origin{};
	float m_FovAngle{};
	float m_Fov{};

	float m_TotalPitch{};
	float m_TotalYaw{};

	dae::Vector3 m_ForwardVector{ dae::Vector3::UnitZ };
	dae::Vector3 m_UpVector{ dae::Vector3::UnitY };
	dae::Vector3 m_RightVector{ dae::Vector3::UnitX };

	dae::Matrix m_ViewMatrix{};
	dae::Matrix m_InvViewMatrix{};

	dae::Matrix m_ProjectionMatrix{};

	// Member Functions
	// ================

	void CalculateViewMatrix();
	void CalculateProjectionMatrix();

	void FOVInput(const uint8_t* pKeyboardState, float deltaTime);
	void MovementInput(const uint8_t* pKeyboardState, float deltaTime);
	void MouseInput(const uint8_t* pKeyboardState, const uint32_t mouseState, int mouseX, int mouseY, float deltaTime);
};

