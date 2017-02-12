#pragma once

#ifndef GRAPHICS_HEADER_TEXTURE_SHADER_H

#define GRAPHICS_HEADER_TEXTURE_SHADER_H

#include "common.h"
#include "rendeable.h"

class TextureShader :public Renderable {
private:
	typedef struct _MatrixBufferType {
		DirectX::XMMATRIX world;

		DirectX::XMMATRIX view;

		DirectX::XMMATRIX projection;
	} MatrixBufferType;

public:
	TextureShader();

	TextureShader(const TextureShader& copy);

	virtual ~TextureShader();

public:
	bool Initialize(HWND hwnd);

	bool InitializeGraphicsPipeline(
		DirectX::XMMATRIX world,
		DirectX::XMMATRIX view,
		DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView *srv_texture
		);

	void DrawIndexed(const int index)override;

private:
	bool InitializeShadersAndInputLayout(HWND hwnd, WCHAR *vs_filename, WCHAR *ps_filename);

	bool InitializeGraphicsPipelineParameters(
		DirectX::XMMATRIX world, 
		DirectX::XMMATRIX view, 
		DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView *srv_texture
		);

	bool InitializeConstantBuffers();

	bool InitializeSampler();

	void SetShadersAndInputLayout();

private:
	int vertex_shader_ = -1;
	
	int pixel_shader_ = -1;

	int input_layout_ = -1;

	int matrix_buffer_ = -1;

	int sampler_state_ = -1;
};

#endif // !GRAPHICS_HEADER_TEXTURE_SHADER_H
