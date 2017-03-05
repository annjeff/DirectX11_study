#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "textureshaderclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {
	bool result;

	{
		m_D3D = new DirectX11Device;
		if (!m_D3D) {
			return false;
		}
		result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Camera = (Camera*)_aligned_malloc(sizeof(Camera), 16);
		new (m_Camera)Camera();
		if (!m_Camera) {
			return false;
		}
		m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
		m_Camera->Render();
	}

	{
		m_Model = new ModelClass();
		if (!m_Model) {
			return false;
		}
		result = m_Model->Initialize(m_D3D->GetDevice(), L"../../tut37/data/seafloor.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_TextureShader = (TextureShaderClass*)_aligned_malloc(sizeof(TextureShaderClass), 16);
		new (m_TextureShader)TextureShaderClass();
		if (!m_TextureShader) {
			return false;
		}
		result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		m_TextureShader->~TextureShaderClass();
		_aligned_free(m_TextureShader);
		m_TextureShader = 0;
	}

	// Release the model object.
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	if (m_Camera) {
		m_Camera->~Camera();
		_aligned_free(m_Camera);
		m_Camera = 0;
	}

	if (m_D3D) {
		m_D3D->Shutdown();
		m_D3D->~DirectX11Device();
		_aligned_free(m_D3D);
		m_D3D = 0;
	}
}

bool GraphicsClass::Frame() {

	bool result;

	// Render the graphics scene.
	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	m_Model->Render(m_D3D->GetDeviceContext());

	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model->GetVertexCount(), m_Model->GetInstanceCount(), worldMatrix, viewMatrix,
		projectionMatrix, m_Model->GetTexture());
	if (!result) {
		return false;
	}

	m_D3D->EndScene();

	return true;
}
