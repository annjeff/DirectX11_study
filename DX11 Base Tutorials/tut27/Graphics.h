#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class TextureShaderClass;
class RenderTextureClass;
class ReflectionShaderClass;

class GraphicsClass :public GraphicsBase {
public:
	GraphicsClass();

	GraphicsClass(const GraphicsClass& rhs) = delete;

	GraphicsClass& operator=(const GraphicsClass& rhs) = delete;

	virtual ~GraphicsClass();
public:
	virtual bool Initialize(int, int, HWND)override;

	virtual void Shutdown()override;

	virtual bool Frame()override;

	virtual bool Render()override;
private:
	bool RenderToTexture();

	bool RenderScene();
private:
	DirectX11Device *m_D3D = nullptr;

	Camera *m_Camera = nullptr;

	ModelClass *m_Model = nullptr;

	TextureShaderClass* m_TextureShader = nullptr;

	RenderTextureClass* m_RenderTexture = nullptr;

	ModelClass* m_FloorModel = nullptr;

	ReflectionShaderClass* m_ReflectionShader = nullptr;
};

#endif