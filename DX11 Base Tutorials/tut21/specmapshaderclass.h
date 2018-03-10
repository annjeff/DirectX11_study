#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

struct MatrixBufferType;
struct LightBufferType;
struct CameraBufferType;

class SpecMapShaderClass {
public:
	SpecMapShaderClass() {}

	SpecMapShaderClass(const SpecMapShaderClass&) = delete;

	~SpecMapShaderClass() {}
public:
	bool Initialize(HWND);

	void Shutdown();

	bool Render(int,
				const DirectX::XMMATRIX&,
				const DirectX::XMMATRIX&,
				const DirectX::XMMATRIX&,
				ID3D11ShaderResourceView**,
				const DirectX::XMFLOAT3&,
				const DirectX::XMFLOAT4&,
				const DirectX::XMFLOAT3&,
				const DirectX::XMFLOAT4&,
				float);
private:
	bool InitializeShader(HWND, WCHAR*, WCHAR*);

	void ShutdownShader();

	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(
		const DirectX::XMMATRIX&,
		const DirectX::XMMATRIX&,
		const DirectX::XMMATRIX&,
		ID3D11ShaderResourceView**,
		const DirectX::XMFLOAT3&,
		const DirectX::XMFLOAT4&,
		const DirectX::XMFLOAT3&,
		const DirectX::XMFLOAT4&,
		float);

	void RenderShader(int);
private:
	ID3D11VertexShader * vertex_shader_ = nullptr;

	ID3D11PixelShader* pixel_shader_ = nullptr;

	ID3D11InputLayout* layout_ = nullptr;

	ID3D11Buffer* matrix_buffer_ = nullptr, *light_buffer_ = nullptr, *camera_buffer_ = nullptr;

	ID3D11SamplerState* sample_state_ = nullptr;
};
