
// Filename: fireshaderclass.cpp

#include "fireshaderclass.h"


FireShaderClass::FireShaderClass()
{
	vertex_shader_ = 0;
	pixel_shader_ = 0;
	layout_ = 0;
	matrix_buffer_ = 0;
	m_noiseBuffer = 0;
	sample_state_ = 0;
	m_sampleState2 = 0;
	m_distortionBuffer = 0;
}


FireShaderClass::FireShaderClass(const FireShaderClass& other)
{
}


FireShaderClass::~FireShaderClass()
{
}


bool FireShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	
	result = InitializeShader(device, hwnd, L"../../tut33/fire.vs", L"../../tut33/fire.ps");
	if(!result)
	{
		return false;
	}

	return true;
}


void FireShaderClass::Shutdown()
{

	ShutdownShader();

	
}


bool FireShaderClass::Render(ID3D11DeviceContext* device_context, int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
							 const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* fireTexture, 
							 ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* alphaTexture, float frameTime,
							 const XMFLOAT3& scrollSpeeds, const XMFLOAT3& scales, const XMFLOAT2& distortion1, const XMFLOAT2& distortion2,
							 const XMFLOAT2& distortion3, float distortionScale, float distortionBias )
{
	bool result;



	result = SetShaderParameters(device_context, worldMatrix, viewMatrix, projectionMatrix, fireTexture, noiseTexture, alphaTexture, 
								 frameTime, scrollSpeeds, scales, distortion1, distortion2, distortion3, distortionScale, 
								 distortionBias);
	if(!result)
	{
		return false;
	}


	RenderShader(device_context, indexCount);

	return true;
}


bool FireShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC noiseBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_SAMPLER_DESC samplerDesc2;
	D3D11_BUFFER_DESC distortionBufferDesc;


	
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "FireVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
								   &vertexShaderBuffer, &errorMessage );
	if(FAILED(result))
	{
		
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    
	result = D3DCompileFromFile(psFilename, NULL, NULL, "FirePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
								   &pixelShaderBuffer, &errorMessage );
	if(FAILED(result))
	{
		
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}


    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, 
										&vertex_shader_);
	if(FAILED(result))
	{
		return false;
	}


    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, 
									   &pixel_shader_);
	if(FAILED(result))
	{
		return false;
	}

	
	
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), 
									   vertexShaderBuffer->GetBufferSize(), &layout_);
	if(FAILED(result))
	{
		return false;
	}

	
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

    
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the matrix buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
	if(FAILED(result))
	{
		return false;
	}

    // Setup the description of the dynamic noise constant buffer that is in the vertex shader.
    noiseBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	noiseBufferDesc.ByteWidth = sizeof(NoiseBufferType);
    noiseBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    noiseBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    noiseBufferDesc.MiscFlags = 0;
	noiseBufferDesc.StructureByteStride = 0;

	// Create the noise buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&noiseBufferDesc, NULL, &m_noiseBuffer);
	if(FAILED(result))
	{
		return false;
	}

	
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	
    result = device->CreateSamplerState(&samplerDesc, &sample_state_);
	if(FAILED(result))
	{
		return false;
	}

	// Create a second texture sampler state description for a Clamp sampler.
    samplerDesc2.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc2.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc2.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc2.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc2.MipLODBias = 0.0f;
    samplerDesc2.MaxAnisotropy = 1;
    samplerDesc2.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc2.BorderColor[0] = 0;
	samplerDesc2.BorderColor[1] = 0;
	samplerDesc2.BorderColor[2] = 0;
	samplerDesc2.BorderColor[3] = 0;
    samplerDesc2.MinLOD = 0;
    samplerDesc2.MaxLOD = D3D11_FLOAT32_MAX;

	
    result = device->CreateSamplerState(&samplerDesc2, &m_sampleState2);
	if(FAILED(result))
	{
		return false;
	}

    // Setup the description of the dynamic distortion constant buffer that is in the pixel shader.
    distortionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	distortionBufferDesc.ByteWidth = sizeof(DistortionBufferType);
    distortionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    distortionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    distortionBufferDesc.MiscFlags = 0;
	distortionBufferDesc.StructureByteStride = 0;

	// Create the distortion buffer pointer so we can access the pixel shader constant buffer from within this class.
	result = device->CreateBuffer(&distortionBufferDesc, NULL, &m_distortionBuffer);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void FireShaderClass::ShutdownShader()
{
	// Release the distortion constant buffer.
	if(m_distortionBuffer)
	{
		m_distortionBuffer->Release();
		m_distortionBuffer = 0;
	}

	// Release the second sampler state.
	if(m_sampleState2)
	{
		m_sampleState2->Release();
		m_sampleState2 = 0;
	}


	if(sample_state_)
	{
		sample_state_->Release();
		sample_state_ = 0;
	}

	// Release the noise constant buffer.
	if(m_noiseBuffer)
	{
		m_noiseBuffer->Release();
		m_noiseBuffer = 0;
	}


	if(matrix_buffer_)
	{
		matrix_buffer_->Release();
		matrix_buffer_ = 0;
	}

	
	if(layout_)
	{
		layout_->Release();
		layout_ = 0;
	}

	
	if(pixel_shader_)
	{
		pixel_shader_->Release();
		pixel_shader_ = 0;
	}

	
	if(vertex_shader_)
	{
		vertex_shader_->Release();
		vertex_shader_ = 0;
	}

	
}


void FireShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	SIZE_T bufferSize, i;
	ofstream fout;


	
	compileErrors = (char*)(errorMessage->GetBufferPointer());


	bufferSize = errorMessage->GetBufferSize();

	
	fout.open("shader-error.txt");


	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	
	fout.close();

	
	errorMessage->Release();
	errorMessage = 0;

	
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	
}


bool FireShaderClass::SetShaderParameters(ID3D11DeviceContext* device_context, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
										  const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* fireTexture, 
										  ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* alphaTexture, 
										  float frameTime, const XMFLOAT3& scrollSpeeds, const XMFLOAT3& scales, const XMFLOAT2& distortion1,
										  const XMFLOAT2& distortion2, const XMFLOAT2& distortion3, float distortionScale,
										  float distortionBias)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	NoiseBufferType* dataPtr2;
	DistortionBufferType* dataPtr3;
	unsigned int buffer_number;

	XMMATRIX worldMatrixCopy = worldMatrix;
	XMMATRIX viewMatrixCopy = viewMatrix;
	XMMATRIX projectionMatrixCopy = projectionMatrix;


	worldMatrixCopy = XMMatrixTranspose( worldMatrix );
	viewMatrixCopy = XMMatrixTranspose( viewMatrix );
	projectionMatrixCopy = XMMatrixTranspose( projectionMatrix );


	result = device_context->Map(matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the matrix constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the matrix constant buffer.
	dataPtr->world = worldMatrixCopy;
	dataPtr->view = viewMatrixCopy;
	dataPtr->projection = projectionMatrixCopy;

	// Unlock the matrix constant buffer.
    device_context->Unmap(matrix_buffer_, 0);

	// Set the position of the matrix constant buffer in the vertex shader.
	buffer_number = 0;

	// Now set the matrix constant buffer in the vertex shader with the updated values.
    device_context->VSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);

	// Lock the noise constant buffer so it can be written to.
	result = device_context->Map(m_noiseBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the noise constant buffer.
	dataPtr2 = (NoiseBufferType*)mappedResource.pData;

	// Copy the data into the noise constant buffer.
	dataPtr2->frameTime = frameTime;
	dataPtr2->scrollSpeeds = scrollSpeeds;
	dataPtr2->scales = scales;
	dataPtr2->padding = 0.0f;

	// Unlock the noise constant buffer.
    device_context->Unmap(m_noiseBuffer, 0);

	// Set the position of the noise constant buffer in the vertex shader.
	buffer_number = 1;

	// Now set the noise constant buffer in the vertex shader with the updated values.
    device_context->VSSetConstantBuffers(buffer_number, 1, &m_noiseBuffer);

	// Set the three shader texture resources in the pixel shader.
	device_context->PSSetShaderResources(0, 1, &fireTexture);
	device_context->PSSetShaderResources(1, 1, &noiseTexture);
	device_context->PSSetShaderResources(2, 1, &alphaTexture);

	// Lock the distortion constant buffer so it can be written to.
	result = device_context->Map(m_distortionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the distortion constant buffer.
	dataPtr3 = (DistortionBufferType*)mappedResource.pData;

	// Copy the data into the distortion constant buffer.
	dataPtr3->distortion1 = distortion1;
	dataPtr3->distortion2 = distortion2;
	dataPtr3->distortion3 = distortion3;
	dataPtr3->distortionScale = distortionScale;
	dataPtr3->distortionBias = distortionBias;

	// Unlock the distortion constant buffer.
    device_context->Unmap(m_distortionBuffer, 0);

	// Set the position of the distortion constant buffer in the pixel shader.
	buffer_number = 0;

	// Now set the distortion constant buffer in the pixel shader with the updated values.
    device_context->PSSetConstantBuffers(buffer_number, 1, &m_distortionBuffer);

	return true;
}


void FireShaderClass::RenderShader(ID3D11DeviceContext* device_context, int indexCount)
{

	device_context->IASetInputLayout(layout_);

 
    device_context->VSSetShader(vertex_shader_, NULL, 0);
    device_context->PSSetShader(pixel_shader_, NULL, 0);

	// Set the sampler states in the pixel shader.
	device_context->PSSetSamplers(0, 1, &sample_state_);
	device_context->PSSetSamplers(1, 1, &m_sampleState2);

	
	device_context->DrawIndexed(indexCount, 0, 0);

	
}