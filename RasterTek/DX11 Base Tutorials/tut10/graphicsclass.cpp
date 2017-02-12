#include "graphicsclass.h"
#include <new>

Graphics::Graphics(){
}

Graphics::Graphics(const Graphics& other){
}

Graphics::~Graphics(){
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	// Create the Direct3D object.
	d3d12_device_ = D3D12Device::GetD3d12DeviceInstance();
	if(!d3d12_device_)
	{
		return false;
	}
	// Initialize the Direct3D object.
	if(CHECK(d3d12_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))){
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	camera_ = std::make_shared<Camera>();
	if (!camera_) {
		return false;
	}
	camera_->SetPosition(0.0f, 0.0f, -5.0f);

	light_ = std::make_shared<Light>();
	if (!light_) {
		return false;
	}
	light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	light_->SetDirection(0.0f, 0.0f, 1.0f);
	light_->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	light_->SetSpecularPower(32.0f);

	light_shader_ = std::make_shared<LightShader>();
	if (!light_shader_) {
		return false;
	}
	if (CHECK(light_shader_->Initialize(L"../../tut10/light.vs", L"../../tut10/light.ps"))) {
		MessageBox(hwnd, L"Could not initialize Light Shader.", L"Error", MB_OK);
		return false;
	}

	model_ = std::make_shared<Model>();
	if (!model_) {
		return false;
	}

	model_->SetVertexShader(CD3DX12_SHADER_BYTECODE(light_shader_->GetVertexShaderBlob().Get()));
	model_->SetPixelShader(CD3DX12_SHADER_BYTECODE(light_shader_->GetPixelSHaderBlob().Get()));

	if (CHECK(model_->Initialize(L"../../tut10/data/cube.txt",L"../../tut10/data/seafloor.dds"))) {
		MessageBox(hwnd, L"Could not initialize Model.", L"Error", MB_OK);
		return false;
	}

	return true;
}


void Graphics::Shutdown(){
	// Release the D3D object.
	if(d3d12_device_){
		delete d3d12_device_;
		d3d12_device_ = nullptr;
	}
}

bool Graphics::Frame(){
	// Render the graphics scene.
	if (FAILED(Render())) {
		return false;
	}

	return true;
}


bool Graphics::Render() {

	static float rotation = 0.0f;
	rotation += (DirectX::XM_PI * 0.01f);
	if (rotation > 360.0f)
	{
		rotation -= 360.0f;
	}

	camera_->Update();

	DirectX::XMMATRIX world = {};
	d3d12_device_->GetWorldMatrix(world);
	world = DirectX::XMMatrixRotationY(rotation);
	world = DirectX::XMMatrixTranspose(world);

	DirectX::XMMATRIX projection = {};
	d3d12_device_->GetProjectionMatrix(projection);
	projection = DirectX::XMMatrixTranspose(projection);

	DirectX::XMMATRIX view = {};
	camera_->GetViewMatrix(view);
	view = DirectX::XMMatrixTranspose(view);

	if (CHECK(model_->UpdateCameraConstant(camera_->GetPosition()))) {
		return false;
	}

	if (CHECK(model_->UpdateMatrixConstant(world, view, projection))) {
		return false;
	}

	if (CHECK(model_->UpdateLightConstant(
		light_->GetAmbientColor(),
		light_->GetDiffuseColor(), 
		light_->GetDirection(),
		light_->GetSpecularColor(),
		light_->GetSpecularPower()
		))) {
		return false;
	}

	d3d12_device_->SetVertexBuffer(model_->GetVertexBufferView());
	d3d12_device_->SetIndexBuffer(model_->GetIndexBufferView(), model_->GetIndexCount());

	auto root_signature = model_->GetRootSignature();
	auto pso = model_->GetPipelineStateObject();
	auto matrix_constant = model_->GetMatrixConstantBuffer();
	auto light_constant = model_->GetLightConstantBuffer();
	auto camera_constant = model_->GetCameraConstantBuffer();
	
	ID3D12DescriptorHeap *heap[] = { model_->GetShaderRescourceView().Get() };

	if (CHECK(d3d12_device_->render(root_signature, pso, heap, 1, matrix_constant, light_constant, camera_constant))) {
		return false;
	}

	return true;
}
