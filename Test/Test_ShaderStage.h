#include <stdexcept>

#include <d3d11.h>
#include <SimShaderStage.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

constexpr UINT CLIENT_WIDTH = 640;
constexpr UINT CLIENT_HEIGHT = 480;

class Test_ShaderStage
{
public:
    void Run(void)
    {

    }

private:
    void InitD3DContext(void)
    {
        //=================������Ⱦ����=================

        hInstance_ = GetModuleHandle(nullptr);

        WNDCLASSEX wc;
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance_;
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = L"TestWindowClass";
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
        if(!RegisterClassEx(&wc))
            throw std::runtime_error("Failed to register window class");

        RECT rect = { 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT};
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
        hWnd_ = CreateWindow(L"TestWindowClass", L"Test Window",
            WS_OVERLAPPED | WS_CAPTION,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr, nullptr, hInstance_, nullptr);
        if(!hWnd_)
        {
            UnregisterClass(L"TestWindowClass", hInstance_);
            throw std::runtime_error("Failed to create rendering window");
        }

        SetForegroundWindow(hWnd_);
        ShowWindow(hWnd_, SW_SHOW);

        //=================����D3D��Ⱦ����=================

        HRESULT hr;

        RECT clientRect;
        GetClientRect(hWnd_, &clientRect);
        LONG width = clientRect.right - clientRect.left;
        LONG height = clientRect.bottom - clientRect.top;

        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.Width = width;
        swapChainDesc.BufferDesc.Height = height;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.Flags = 0;
        swapChainDesc.OutputWindow = hWnd_;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Windowed = TRUE;

        hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc,
            &swapChain_, &D3D_, nullptr, &DC_);
        if(FAILED(hr))
        {
            DestroyD3DContext();
            throw std::runtime_error("Failed to create D3D device and swap chain");
        }

        //=================ȡ����Ⱦ������ͼ=================

        ID3D11Texture2D *backbuffer;
        hr = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer));
        if(FAILED(hr))
        {
            DestroyD3DContext();
            throw std::runtime_error("Failed to get backbuffer of swapChain");
        }

        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
        hr = D3D_->CreateRenderTargetView(backbuffer, nullptr, &renderTargetView_);
        backbuffer->Release();
        if(FAILED(hr))
        {
            DestroyD3DContext();
            throw std::runtime_error("Failed to create render target view");
        }

        //=================׼�����ģ�建��=================

        D3D11_TEXTURE2D_DESC depthStencilBufDesc;
        depthStencilBufDesc.ArraySize = 1;
        depthStencilBufDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilBufDesc.CPUAccessFlags = 0;
        depthStencilBufDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilBufDesc.Height = height;
        depthStencilBufDesc.Width = width;
        depthStencilBufDesc.MipLevels = 0;
        depthStencilBufDesc.MiscFlags = 0;
        depthStencilBufDesc.SampleDesc.Count = 1;
        depthStencilBufDesc.SampleDesc.Quality = 0;
        depthStencilBufDesc.Usage = D3D11_USAGE_DEFAULT;

        hr = D3D_->CreateTexture2D(&depthStencilBufDesc, nullptr, &depthStencilBuffer_);
        if(FAILED(hr))
        {
            DestroyD3DContext();
            throw std::runtime_error("Failed to create depth stencil buffer");
        }

        D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
        D3D11_DEPTH_STENCILOP_DESC depthStencilOp;
        depthStencilOp.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilOp.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilOp.StencilFunc = D3D11_COMPARISON_ALWAYS;
        depthStencilOp.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
        depthStencilStateDesc.DepthEnable = TRUE;
        depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
        depthStencilStateDesc.StencilEnable = FALSE;
        depthStencilStateDesc.StencilReadMask = 0xFF;
        depthStencilStateDesc.StencilWriteMask = 0xFF;
        depthStencilStateDesc.FrontFace = depthStencilOp;
        depthStencilStateDesc.BackFace = depthStencilOp;

        hr = D3D_->CreateDepthStencilState(&depthStencilStateDesc, &depthStencilState_);
        if(FAILED(hr))
        {
            DestroyD3DContext();
            throw std::runtime_error("Failed to create depth stencil state");
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
        depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Flags = 0;
        depthStencilViewDesc.Texture2D.MipSlice = 0;
        hr = D3D_->CreateDepthStencilView(depthStencilBuffer_, &depthStencilViewDesc, &depthStencilView_);
        if(FAILED(hr))
        {
            DestroyD3DContext();
            throw std::runtime_error("Failed to create depth stencil view");
        }

        DC_->OMSetRenderTargets(1, &renderTargetView_, depthStencilView_);
        DC_->OMSetDepthStencilState(depthStencilState_, 0);

        //=================�ӿ�����=================

        D3D11_VIEWPORT vp;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width = width;
        vp.Height = height;
        vp.MaxDepth = 1.0f;
        vp.MinDepth = 0.0f;
        DC_->RSSetViewports(1, &vp);
    }

    void DestroyD3DContext(void)
    {
        if(vtxBuf_)
        {
            vtxBuf_->Release();
            vtxBuf_ = nullptr;
        }

        if(shaderByteCode_)
        {
            shaderByteCode_->Release();
            shaderByteCode_ = nullptr;
        }

        if(depthStencilState_)
        {
            depthStencilState_->Release();
            depthStencilState_ = nullptr;
        }

        if(depthStencilView_)
        {
            depthStencilView_->Release();
            depthStencilView_ = nullptr;
        }

        if(depthStencilBuffer_)
        {
            depthStencilBuffer_->Release();
            depthStencilBuffer_ = nullptr;
        }

        if(renderTargetView_)
        {
            renderTargetView_->Release();
            renderTargetView_ = nullptr;
        }

        if(swapChain_)
        {
            swapChain_->Release();
            swapChain_ = nullptr;
        }

        if(DC_)
        {
            DC_->Release();
            DC_ = nullptr;
        }

        if(D3D_)
        {
            D3D_->Release();
            D3D_ = nullptr;
        }

        if(hWnd_)
        {
            DestroyWindow(hWnd_);
            UnregisterClass(L"TestWindowClass", hInstance_);
        }
    }

private:
    HINSTANCE hInstance_ = NULL;
    HWND hWnd_ = NULL;

    IDXGISwapChain *swapChain_ = nullptr;

    ID3D11Device *D3D_ = nullptr;
    ID3D11DeviceContext *DC_ = nullptr;

    ID3D11Texture2D *depthStencilBuffer_ = nullptr;
    ID3D11DepthStencilState *depthStencilState_ = nullptr;
    ID3D11DepthStencilView *depthStencilView_ = nullptr;
    ID3D11RenderTargetView *renderTargetView_ = nullptr;

    ID3D11Buffer *vtxBuf_ = nullptr;
    ID3D10Blob *shaderByteCode_ = nullptr;
};
