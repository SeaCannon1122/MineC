#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#include <dxgidebug.h>

#include <cwindow/cwindow.h>

#include <wrl.h>
#include <stdio.h>

#define DX_CHECK_CALL(call)                                                  \
{                                                                            \
    HRESULT hr = (call);                                                     \
    if (FAILED(hr)) {                                                        \
        printf("DirectX call failed:\n  %s\n  HRESULT: 0x%08X\n", #call, hr); \
        DebugBreak();                                                        \
    }                                                                        \
}

#define DX_CHECK_NULL_PTR(ptr)                                               \
{                                                                            \
    if ((ptr) == 0) {                                                  \
        printf("DirectX pointer is NULL:\n  %s\n", #ptr);                    \
        DebugBreak();                                                        \
    }                                                                        \
}

const uint8_t shader_src[] =
"struct VSInput {\n"
"    float2 Position : POSITION;\n"
"    float4 Color : COLOR;\n"
"};\n"
"\n"
"struct VSOutput {\n"
"    float4 Position : SV_POSITION;\n"
"    float4 Color : COLOR;\n"
"};\n"
"\n"
"VSOutput VSMain(VSInput input) {\n"
"    VSOutput output;\n"
"    output.Position = float4(input.Position, 0.0, 1.0);\n"
"    output.Color = input.Color;\n"
"    return output;\n"
"}\n"
"\n"
"float4 PSMain(VSOutput input) : SV_TARGET {\n"
"    return input.Color;\n"
"}\n";

#define BUFFER_COUNT 3

ID3D12Debug* d3d12debug;
IDXGIDebug1* dxgidebug;

IDXGIFactory2* factory;

ID3D12Device4* device;
ID3D12CommandQueue* command_queue;

ID3D12Fence* fence;
UINT64 fence_value = 0;
HANDLE fence_event = NULL;

ID3D12CommandAllocator* command_allocator;
ID3D12GraphicsCommandList* command_list;

IDXGISwapChain3* swapchain;
ID3D12Resource* swapchain_buffers[BUFFER_COUNT];
D3D12_CPU_DESCRIPTOR_HANDLE rtv_handles[BUFFER_COUNT];

ID3D12DescriptorHeap* rtv_descriptor_heap;

ID3D12RootSignature* root_signature;

ID3D12PipelineState* pipeline_state;

ID3D12Resource* upload_buffer;
ID3D12Resource* vertex_buffer;

void signal_and_wait_fence()
{
	DX_CHECK_CALL(command_queue->Signal(fence, ++fence_value));

	DX_CHECK_CALL(fence->SetEventOnCompletion(fence_value, fence_event));
	DX_CHECK_NULL_PTR(WaitForSingleObject(fence_event, 20000) == WAIT_OBJECT_0);
}

void get_swapchain_buffers()
{
	for (uint32_t i = 0; i < BUFFER_COUNT; i++)
	{
		DX_CHECK_CALL(swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchain_buffers[i])));

		D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
		rtv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtv_desc.Texture2D.MipSlice = 0;
		rtv_desc.Texture2D.PlaneSlice = 0;

		device->CreateRenderTargetView(swapchain_buffers[i], &rtv_desc, rtv_handles[i]);
	}
}

void release_swapchain_buffers()
{
	for (uint32_t i = 0; i < BUFFER_COUNT; i++)
	{
		swapchain_buffers[i]->Release();
	}	
}

void flush()
{
	for (uint32_t i = 0; i < BUFFER_COUNT; i++) signal_and_wait_fence();
}

int main(int argc, char* argv[]) {

	cwindow_context* window_context = cwindow_context_create((const uint8_t* )"context");
	cwindow* window = cwindow_create(window_context, 100, 100, 200, 200, (uint8_t*)"window for test", true);

	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;

	cwindow_get_dimensions(window, &width, &height, &x, &y);

	HWND cwindow = cwindow_impl_windows_get_hwnd(window);

	DX_CHECK_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12debug)));
	d3d12debug->EnableDebugLayer();
	DX_CHECK_CALL(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgidebug)));
	dxgidebug->EnableLeakTrackingForThread();

	DX_CHECK_CALL(CreateDXGIFactory(IID_PPV_ARGS(&factory)));

	DX_CHECK_CALL(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

	D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
	command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	command_queue_desc.NodeMask = 0;
	command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	DX_CHECK_CALL(device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&command_queue)));

	DX_CHECK_CALL(device->CreateFence(fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	DX_CHECK_NULL_PTR(fence_event = CreateEventA(NULL, false, false, NULL));

	DX_CHECK_CALL(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)));
	DX_CHECK_CALL(device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&command_list)));

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Width = width;
	swap_chain_desc.Height = height;
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo = false;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = BUFFER_COUNT;
	swap_chain_desc.Scaling = DXGI_SCALING_NONE;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swap_chain_fullscreen_desc = {};
	swap_chain_fullscreen_desc.Windowed = true;

	IDXGISwapChain1* temp_swapchain;

	DX_CHECK_CALL(factory->CreateSwapChainForHwnd(command_queue, cwindow, &swap_chain_desc, &swap_chain_fullscreen_desc, NULL, &temp_swapchain));

	DX_CHECK_CALL(temp_swapchain->QueryInterface(IID_PPV_ARGS(&swapchain)));

	temp_swapchain->Release();

	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
	descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptor_heap_desc.NumDescriptors = BUFFER_COUNT;
	descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptor_heap_desc.NodeMask = 0;

	DX_CHECK_CALL(device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&rtv_descriptor_heap)));


	D3D12_CPU_DESCRIPTOR_HANDLE first_rtv_handle = rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	uint32_t rtv_handle_increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (uint32_t i = 0; i < BUFFER_COUNT; i++)
	{
		rtv_handles[i] = first_rtv_handle;
		rtv_handles[i].ptr += (size_t)(rtv_handle_increment * i);
	}

	get_swapchain_buffers();

	D3D12_INPUT_ELEMENT_DESC vertex_layout[] = 
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 2*sizeof(float), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}
	};

	HRESULT hr;

	ID3DBlob* vertex_shader_blob;
	ID3DBlob* pixel_shader_blob;
	ID3DBlob* error_blob = NULL;

	// Compile Vertex Shader
	hr = D3DCompile(shader_src, sizeof(shader_src) - 1, NULL, NULL, NULL,
		"VSMain", "vs_5_0", 0, 0, &vertex_shader_blob, &error_blob);

	if (FAILED(hr)) {
		if (error_blob) {
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
			error_blob->Release();
			error_blob = NULL;
		}
	}

	// Compile Pixel Shader
	hr = D3DCompile(shader_src, sizeof(shader_src) - 1, NULL, NULL, NULL,
		"PSMain", "ps_5_0", 0, 0, &pixel_shader_blob, &error_blob);

	if (FAILED(hr)) {
		if (error_blob) {
			// Log the error (e.g., OutputDebugString or print to console)
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
			error_blob->Release();
			error_blob = NULL;
		}
	}

	D3D12_ROOT_SIGNATURE_DESC root_sig_desc = {};
	root_sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* serialized_root_sig;
	hr = D3D12SerializeRootSignature(&root_sig_desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized_root_sig, &error_blob);
	if (FAILED(hr)) {
		OutputDebugStringA((char*)error_blob->GetBufferPointer());
		error_blob->Release();
		error_blob = NULL;
	}

	DX_CHECK_CALL(device->CreateRootSignature(0, serialized_root_sig->GetBufferPointer(), serialized_root_sig->GetBufferSize(), IID_PPV_ARGS(&root_signature)));

	serialized_root_sig->Release();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc = {};
	pipeline_state_desc.InputLayout.NumElements = sizeof(vertex_layout) / sizeof(vertex_layout[0]);
	pipeline_state_desc.InputLayout.pInputElementDescs = vertex_layout;
	pipeline_state_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	pipeline_state_desc.VS.pShaderBytecode = vertex_shader_blob->GetBufferPointer();
	pipeline_state_desc.VS.BytecodeLength = vertex_shader_blob->GetBufferSize();
	pipeline_state_desc.PS.pShaderBytecode = pixel_shader_blob->GetBufferPointer();
	pipeline_state_desc.PS.BytecodeLength = pixel_shader_blob->GetBufferSize();
	pipeline_state_desc.DS.pShaderBytecode = NULL;
	pipeline_state_desc.DS.BytecodeLength = 0;
	pipeline_state_desc.HS.pShaderBytecode = NULL;
	pipeline_state_desc.HS.BytecodeLength = 0;
	pipeline_state_desc.GS.pShaderBytecode = NULL;
	pipeline_state_desc.GS.BytecodeLength = 0;
	pipeline_state_desc.pRootSignature = root_signature;
	pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipeline_state_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipeline_state_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	pipeline_state_desc.RasterizerState.FrontCounterClockwise = TRUE;
	pipeline_state_desc.RasterizerState.DepthBias = 0;
	pipeline_state_desc.RasterizerState.DepthBiasClamp = 0.f;
	pipeline_state_desc.RasterizerState.SlopeScaledDepthBias = 0.f;
	pipeline_state_desc.RasterizerState.DepthClipEnable = FALSE;
	pipeline_state_desc.RasterizerState.MultisampleEnable = FALSE;
	pipeline_state_desc.RasterizerState.AntialiasedLineEnable = FALSE;
	pipeline_state_desc.RasterizerState.ForcedSampleCount = 0;
	pipeline_state_desc.StreamOutput.NumEntries = 0;
	pipeline_state_desc.StreamOutput.NumStrides = 0;
	pipeline_state_desc.StreamOutput.pSODeclaration = NULL;
	pipeline_state_desc.StreamOutput.pBufferStrides = NULL;
	pipeline_state_desc.StreamOutput.RasterizedStream = 0;
	pipeline_state_desc.NumRenderTargets = 1;
	pipeline_state_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipeline_state_desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	pipeline_state_desc.BlendState.AlphaToCoverageEnable = FALSE;
	pipeline_state_desc.BlendState.IndependentBlendEnable = FALSE;
	pipeline_state_desc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	pipeline_state_desc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	pipeline_state_desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	pipeline_state_desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	pipeline_state_desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	pipeline_state_desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	pipeline_state_desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	pipeline_state_desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	pipeline_state_desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	pipeline_state_desc.DepthStencilState.DepthEnable = FALSE;
	pipeline_state_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pipeline_state_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	pipeline_state_desc.DepthStencilState.StencilEnable = FALSE;
	pipeline_state_desc.DepthStencilState.StencilReadMask = 0;
	pipeline_state_desc.DepthStencilState.StencilWriteMask = 0;
	pipeline_state_desc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pipeline_state_desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pipeline_state_desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.SampleMask = 0xffffffff;
	pipeline_state_desc.SampleDesc.Count = 1;
	pipeline_state_desc.SampleDesc.Quality = 0;
	pipeline_state_desc.NodeMask = 0;
	pipeline_state_desc.CachedPSO.pCachedBlob = NULL;
	pipeline_state_desc.CachedPSO.CachedBlobSizeInBytes = 0;
	pipeline_state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	DX_CHECK_CALL(device->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&pipeline_state)));

	vertex_shader_blob->Release();
	pixel_shader_blob->Release();


	D3D12_HEAP_PROPERTIES upload_heap_props = {};
	upload_heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
	upload_heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	upload_heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	upload_heap_props.CreationNodeMask = 0;
	upload_heap_props.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC upload_resource_desc = {};
	upload_resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	upload_resource_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	upload_resource_desc.Width = 1024;
	upload_resource_desc.Height = 1;
	upload_resource_desc.DepthOrArraySize = 1;
	upload_resource_desc.MipLevels = 1;
	upload_resource_desc.Format = DXGI_FORMAT_UNKNOWN;
	upload_resource_desc.SampleDesc.Count = 1;
	upload_resource_desc.SampleDesc.Quality = 0;
	upload_resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	upload_resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	DX_CHECK_CALL(device->CreateCommittedResource(&upload_heap_props, D3D12_HEAP_FLAG_NONE, &upload_resource_desc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&upload_buffer)));

	D3D12_HEAP_PROPERTIES vertex_heap_props = {};
	vertex_heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
	vertex_heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	vertex_heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	vertex_heap_props.CreationNodeMask = 0;
	vertex_heap_props.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC vertex_resource_desc = {};
	vertex_resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertex_resource_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	vertex_resource_desc.Width = 1024;
	vertex_resource_desc.Height = 1;
	vertex_resource_desc.DepthOrArraySize = 1;
	vertex_resource_desc.MipLevels = 1;
	vertex_resource_desc.Format = DXGI_FORMAT_UNKNOWN;
	vertex_resource_desc.SampleDesc.Count = 1;
	vertex_resource_desc.SampleDesc.Quality = 0;
	vertex_resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vertex_resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	DX_CHECK_CALL(device->CreateCommittedResource(&vertex_heap_props, D3D12_HEAP_FLAG_NONE, &vertex_resource_desc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&vertex_buffer)));

	void* upload_buffer_host_handle;
	D3D12_RANGE upload_mapping_range = {};
	upload_mapping_range.Begin = 0;
	upload_mapping_range.End = 1024;

	DX_CHECK_CALL(upload_buffer->Map(0, &upload_mapping_range, &upload_buffer_host_handle));

	float vertices[] = {
		0.f, 0.5f, 1.f, 0.f, 0.f, 1.f,
		-0.5f, -0.5f, 0.f, 1.f, 0.f, 1.f,
		0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f,
	};

	memcpy(upload_buffer_host_handle, vertices, sizeof(vertices));

	upload_buffer->Unmap(0, &upload_mapping_range);

	DX_CHECK_CALL(command_allocator->Reset());
	DX_CHECK_CALL(command_list->Reset(command_allocator, NULL));

	command_list->CopyBufferRegion(vertex_buffer, 0, upload_buffer, 0, sizeof(vertices));

	DX_CHECK_CALL(command_list->Close());

	ID3D12CommandList* lists[] = { command_list };
	command_queue->ExecuteCommandLists(1, lists);
	signal_and_wait_fence();

	bool leave = false;
	while (leave == false)
	{
		const cwindow_event* event;
		while (event = cwindow_next_event(window))
		{
			switch (event->type)
			{

			case CWINDOW_EVENT_MOVE_SIZE: {
				printf(
					"New window dimensions:\n  width: %d\n  height: %d\n  position x: %d\n  position y: %d\n\n",
					event->info.move_size.width,
					event->info.move_size.height,
					event->info.move_size.position_x,
					event->info.move_size.position_y
				);

				width = event->info.move_size.width;
				height = event->info.move_size.height;

				flush();

				if (width != 0 && height != 0)
				{
					release_swapchain_buffers();

					DX_CHECK_CALL(swapchain->ResizeBuffers(BUFFER_COUNT, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING));

					get_swapchain_buffers();
				}
				

			} break;

			case CWINDOW_EVENT_DESTROY: {
				leave = true;
			} break;

			}
		}

		DX_CHECK_CALL(command_allocator->Reset());
		DX_CHECK_CALL(command_list->Reset(command_allocator, NULL));

		uint32_t backbuffer_index = swapchain->GetCurrentBackBufferIndex();

		D3D12_RESOURCE_BARRIER begin_barrier = {};
		begin_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		begin_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		begin_barrier.Transition.pResource = swapchain_buffers[backbuffer_index];
		begin_barrier.Transition.Subresource = 0;
		begin_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		begin_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		command_list->ResourceBarrier(1, &begin_barrier);

		float clear_color[4] = { 1.f, 1.f, 1.f, 1.f };
		command_list->ClearRenderTargetView(rtv_handles[backbuffer_index], clear_color, 0, NULL);
		
		command_list->OMSetRenderTargets(1, &rtv_handles[backbuffer_index], false, NULL);

		D3D12_VERTEX_BUFFER_VIEW vertex_view = {};
		vertex_view.SizeInBytes = sizeof(vertices);
		vertex_view.StrideInBytes = 6 * sizeof(float);
		vertex_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();

		command_list->SetPipelineState(pipeline_state);
		command_list->SetGraphicsRootSignature(root_signature);

		command_list->IASetVertexBuffers(0, 1, &vertex_view);
		command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D12_VIEWPORT view_port = {};
		view_port.TopLeftX = 0;
		view_port.TopLeftY = 0;
		view_port.Width = width;
		view_port.Height = height;
		view_port.MinDepth = 1.f;
		view_port.MaxDepth = 0.f;
		command_list->RSSetViewports(1, &view_port);

		RECT scissor_rect = {};
		scissor_rect.left = 0;
		scissor_rect.top = 0;
		scissor_rect.right = width;
		scissor_rect.bottom = height;
		command_list->RSSetScissorRects(1, &scissor_rect);

		command_list->DrawInstanced(3, 1, 0, 0);

		D3D12_RESOURCE_BARRIER end_barrier = {};
		end_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		end_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		end_barrier.Transition.pResource = swapchain_buffers[backbuffer_index];
		end_barrier.Transition.Subresource = 0;
		end_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		end_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

		command_list->ResourceBarrier(1, &end_barrier);

		DX_CHECK_CALL(command_list->Close());

		ID3D12CommandList* lists[] = { command_list };
		command_queue->ExecuteCommandLists(1, lists);
		signal_and_wait_fence();

		DX_CHECK_CALL(swapchain->Present(1, 0));
	}

	flush();

	vertex_buffer->Release();
	upload_buffer->Release();

	pipeline_state->Release();

	root_signature->Release();

	release_swapchain_buffers();
	
	rtv_descriptor_heap->Release();

	swapchain->Release();

	command_list->Release();
	command_allocator->Release();

	fence->Release();
	CloseHandle(fence_event);

	command_queue->Release();
	device->Release();

	factory->Release();

	OutputDebugStringA("\n\n\nDEBUG INFO\n\n\n");

	DX_CHECK_CALL(dxgidebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL)));
	dxgidebug->Release();
	d3d12debug->Release();

	cwindow_destroy(window);
	cwindow_context_destroy(window_context);

	return 0;
}
