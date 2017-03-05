#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "lightclass.h"
#include "projectionshaderclass.h"
#include "textureclass.h"
#include "viewpointclass.h"

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
		// Set the initial position and rotation of the camera.
		m_Camera->SetPosition(0.0f, 7.0f, -10.0f);
		m_Camera->SetRotation(35.0f, 0.0f, 0.0f);
	}

	{
		// Create the ground model object.
		m_GroundModel = new ModelClass();
		if (!m_GroundModel) {
			return false;
		}

		// Initialize the ground model object.
		result = m_GroundModel->Initialize(m_D3D->GetDevice(), "../../tut43/data/floor.txt", L"../../tut43/data/stone.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the ground model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		// Create the cube model object.
		m_CubeModel = new ModelClass();
		if (!m_CubeModel) {
			return false;
		}

		// Initialize the cube model object.
		result = m_CubeModel->Initialize(m_D3D->GetDevice(), "../../tut43/data/cube.txt", L"../../tut43/data/seafloor.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the cube model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		// Create the light object.
		m_Light = (LightClass*)_aligned_malloc(sizeof(LightClass), 16);
		new (m_Light)LightClass();
		if (!m_Light) {
			return false;
		}

		// Initialize the light object.
		m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
		m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light->SetDirection(0.0f, -0.75f, 0.5f);
	}

	{
		// Create the projection shader object.
		m_ProjectionShader = (ProjectionShaderClass*)_aligned_malloc(sizeof(ProjectionShaderClass), 16);
		new (m_ProjectionShader)ProjectionShaderClass();
		if (!m_ProjectionShader) {
			return false;
		}

		// Initialize the projection shader object.
		result = m_ProjectionShader->Initialize(m_D3D->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the projection shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		// Create the projection texture object.
		m_ProjectionTexture = (TextureClass*)_aligned_malloc(sizeof(TextureClass), 16);
		new (m_ProjectionTexture)TextureClass();
		if (!m_ProjectionTexture) {
			return false;
		}

		// Initialize the projection texture object.
		result = m_ProjectionTexture->Initialize(m_D3D->GetDevice(), L"../../tut43/data/dx11.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the projection texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		// Create the view point object.
		m_ViewPoint = (ViewPointClass*)_aligned_malloc(sizeof(ViewPointClass), 16);
		new (m_ViewPoint)ViewPointClass();
		if (!m_ViewPoint) {
			return false;
		}

		// Initialize the view point object.
		m_ViewPoint->SetPosition(2.0f, 5.0f, -2.0f);
		m_ViewPoint->SetLookAt(0.0f, 0.0f, 0.0f);
		m_ViewPoint->SetProjectionParameters((float)(XM_PI / 2.0f), 1.0f, 0.1f, 100.0f);
		m_ViewPoint->GenerateViewMatrix();
		m_ViewPoint->GenerateProjectionMatrix();
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the view point object.
	if (m_ViewPoint) {
		m_ViewPoint->~ViewPointClass();
		_aligned_free(m_ViewPoint);
		m_ViewPoint = 0;
	}

	// Release the projection texture object.
	if (m_ProjectionTexture) {
		m_ProjectionTexture->Shutdown();
		m_ProjectionTexture->~TextureClass();
		_aligned_free(m_ProjectionTexture);
		m_ProjectionTexture = 0;
	}

	// Release the projection shader object.
	if (m_ProjectionShader) {
		m_ProjectionShader->Shutdown();
		m_ProjectionShader->~ProjectionShaderClass();
		_aligned_free(m_ProjectionShader);
		m_ProjectionShader = 0;
	}

	// Release the light object.
	if (m_Light) {
		m_Light->~LightClass();
		_aligned_free(m_Light);
		m_Light = 0;
	}

	// Release the cube model object.
	if (m_CubeModel) {
		m_CubeModel->Shutdown();
		delete m_CubeModel;
		m_CubeModel = 0;
	}

	// Release the ground model object.
	if (m_GroundModel) {
		m_GroundModel->Shutdown();
		delete m_GroundModel;
		m_GroundModel = 0;
	}

	if (m_Camera) {
		m_Camera->~Camera();
		_aligned_free(m_Camera);
		m_Camera = 0;
	}

	if (m_D3D) {
		m_D3D->Shutdown();
		delete m_D3D;
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
	XMMATRIX viewMatrix2, projectionMatrix2;
	bool result;

	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	m_ViewPoint->GetViewMatrix(viewMatrix2);
	m_ViewPoint->GetProjectionMatrix(projectionMatrix2);

	worldMatrix = XMMatrixTranslation(0.0f, 1.0f, 0.0f);

	m_GroundModel->Render(m_D3D->GetDeviceContext());
	result = m_ProjectionShader->Render(m_D3D->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_GroundModel->GetTexture(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetDirection(),
		viewMatrix2, projectionMatrix2, m_ProjectionTexture->GetTexture());
	if (!result) {
		return false;
	}

	m_D3D->GetWorldMatrix(worldMatrix);
	worldMatrix = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

	m_CubeModel->Render(m_D3D->GetDeviceContext());
	result = m_ProjectionShader->Render(m_D3D->GetDeviceContext(), m_CubeModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_CubeModel->GetTexture(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetDirection(),
		viewMatrix2, projectionMatrix2, m_ProjectionTexture->GetTexture());
	if (!result) {
		return false;
	}

	m_D3D->EndScene();

	return true;
}