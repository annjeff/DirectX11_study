#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "rendertextureclass.h"
#include "textureshaderclass.h"
#include "glassshaderclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

	bool result;
	XMMATRIX baseViewMatrix;

	{
		directx_device_ = new DirectX11Device;
		if (!directx_device_) {
			return false;
		}
		result = directx_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
			return false;
		}
	}

	{
		camera_ = (Camera*)_aligned_malloc(sizeof(Camera), 16);
		new (camera_)Camera();
		if (!camera_) {
			return false;
		}
		camera_->SetPosition(0.0f, 0.0f, -10.0f);
		camera_->Render();
		camera_->GetViewMatrix(baseViewMatrix);
	}

	{
		model_ = new ModelClass();
		if (!model_) {
			return false;
		}
		result = model_->Initialize(directx_device_->GetDevice(), "../../tut32/data/cube.txt", L"../../tut32/data/seafloor.dds", L"../../tut32/data/bump03.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}

		m_WindowModel = new ModelClass();
		if (!m_WindowModel) {
			return false;
		}
		result = m_WindowModel->Initialize(directx_device_->GetDevice(), "../../tut32/data/square.txt", L"../../tut32/data/glass01.dds", L"../../tut32/data/bump03.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the window model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_RenderTexture = new RenderTextureClass();
		if (!m_RenderTexture) {
			return false;
		}
		result = m_RenderTexture->Initialize(directx_device_->GetDevice(), screenWidth, screenHeight);
		if (!result) {
			return false;
		}
	}

	{
		m_TextureShader = (TextureShaderClass*)_aligned_malloc(sizeof(TextureShaderClass), 16);
		new (m_TextureShader)TextureShaderClass();
		if (!m_TextureShader) {
			return false;
		}
		result = m_TextureShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_GlassShader = (GlassShaderClass*)_aligned_malloc(sizeof(GlassShaderClass), 16);
		new (m_GlassShader)GlassShaderClass();
		if (!m_GlassShader) {
			return false;
		}
		result = m_GlassShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the glass shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the glass shader object.
	if (m_GlassShader)
	{
		m_GlassShader->Shutdown();
		m_GlassShader->~GlassShaderClass();
		_aligned_free(m_GlassShader);
		m_GlassShader = 0;
	}

	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		m_TextureShader->~TextureShaderClass();
		_aligned_free(m_TextureShader);
		m_TextureShader = 0;
	}

	// Release the render to texture object.
	if (m_RenderTexture)
	{
		m_RenderTexture->Shutdown();
		delete m_RenderTexture;
		m_RenderTexture = 0;
	}

	// Release the window model object.
	if (m_WindowModel)
	{
		m_WindowModel->Shutdown();
		delete m_WindowModel;
		m_WindowModel = 0;
	}

	// Release the model object.
	if (model_)
	{
		model_->Shutdown();
		delete model_;
		model_ = 0;
	}

	if (camera_) {
		camera_->~Camera();
		_aligned_free(camera_);
		camera_ = 0;
	}

	if (directx_device_) {
		directx_device_->Shutdown();
		delete directx_device_;
		directx_device_ = 0;
	}
}

bool GraphicsClass::Frame() {

	static float rotation = 0.0f;
	bool result;


	// Update the rotation variable each frame.
	rotation += (float)XM_PI * 0.005f;
	if (rotation > 360.0f) {
		rotation -= 360.0f;
	}
	rotation_ = rotation;

	// Set the position of the camera.
	camera_->SetPosition(0.0f, 0.0f, -10.0f);

	// Render the scene to texture first.
	result = RenderToTexture(rotation);
	if (!result) {
		return false;
	}

	// Render the scene.
	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::RenderToTexture(float rotation) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	m_RenderTexture->SetRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView());

	m_RenderTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation);

	model_->Render(directx_device_->GetDeviceContext());

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	float refractionScale;
	bool result;

	refractionScale = 0.01f;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation_);

	model_->Render(directx_device_->GetDeviceContext());

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 0.0f, -1.5f);

	m_WindowModel->Render(directx_device_->GetDeviceContext());

	result = m_GlassShader->Render(directx_device_->GetDeviceContext(), m_WindowModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_WindowModel->GetTexture(), m_WindowModel->GetNormalMap(), m_RenderTexture->GetShaderResourceView(),
		refractionScale);
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}