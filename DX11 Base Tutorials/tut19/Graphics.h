#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "../CommonFramework/GraphicsBase.h"
#include "alphamapshaderclass.h"

class DirectX11Device;
class Camera;
class ModelClass;
class AlphaMapShaderClass;

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
	DirectX11Device *m_D3D = nullptr;

	Camera *m_Camera = nullptr;

	ModelClass *m_Model = nullptr;

	AlphaMapShaderClass* m_AlphaMapShader = nullptr;
};

#endif