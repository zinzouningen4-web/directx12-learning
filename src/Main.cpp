#include <windows.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include <d3d12sdklayers.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")
using Microsoft::WRL::ComPtr;

// 失敗した処理名とエラーコードを表示して終了する
void CheckResult(HRESULT result, const wchar_t* operation)
{
    if (FAILED(result))
    {
        wchar_t text[512]{};

        wsprintfW(
            text,
            L"%s\nHRESULT: 0x%08X",
            operation,
            static_cast<unsigned int>(result));

        MessageBoxW(
            nullptr,
            text,
            L"DirectX 12 エラー",
            MB_OK | MB_ICONERROR);

        ExitProcess(1);
    }
}


// Windows から届く、ウィンドウへの通知を処理する関数
LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        // ウィンドウが破棄されたら、アプリの終了を要求する
        PostQuitMessage(0);
        return 0;
    }

    // 自分で処理しない通知は、Windows の標準処理に任せる
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

// Windows アプリの開始地点
int WINAPI wWinMain(
    HINSTANCE instance,
    HINSTANCE,
    PWSTR,
    int showCommand)
{
    //ウィンドウの種類を登録する
    constexpr wchar_t className[] = L"DirectX12StudyWindow";

    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground =
        reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (RegisterClassW(&windowClass) == 0)
    {
        MessageBoxW(
            nullptr,
            L"ウィンドウクラスの登録に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);
        return 1;
    }

    // ② 中身の領域が 1280 × 720 になるよう、外枠の大きさを求める
    constexpr DWORD windowStyle = WS_OVERLAPPEDWINDOW;



    RECT windowRect{ 0, 0, 1280, 720 };

    if (!AdjustWindowRect(&windowRect, windowStyle, FALSE))
    {
        return 1;
    }

    // ウィンドウを作る
    HWND hwnd = CreateWindowExW(
        0,                                      // 拡張スタイル
        className,                              // 登録した種類の名前
        L"DirectX 12 勉強 - 最初のウィンドウ",    // タイトル
        windowStyle,                            // 通常のウィンドウ
        CW_USEDEFAULT,                          // 表示位置 X
        CW_USEDEFAULT,                          // 表示位置 Y
        windowRect.right - windowRect.left,     // 外枠を含む幅
        windowRect.bottom - windowRect.top,     // 外枠を含む高さ
        nullptr,                                // 親ウィンドウなし
        nullptr,                                // メニューなし
        instance,                               // このアプリ
        nullptr);                               // 追加データなし

    if (hwnd == nullptr)
    {
        MessageBoxW(
            nullptr,
            L"ウィンドウの作成に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);
        return 1;
    }

    // 作ったウィンドウを表示する
    ShowWindow(hwnd, showCommand);



    // DirectX 12 の Device を保持する変数
    ComPtr<ID3D12Device> device;





#if defined(_DEBUG)
    // DirectX 12のデバッグ機能を取得する
    ComPtr<ID3D12Debug> debugController;

    CheckResult(
        D3D12GetDebugInterface(
            IID_PPV_ARGS(debugController.GetAddressOf())),
        L"デバッグレイヤーの取得");

    // 必ずDeviceを作る前に有効化する
    debugController->EnableDebugLayer();

    OutputDebugStringW(
        L"[Study] DirectX 12 debug layer enabled.\n");
#endif

    // Device を作る
    const HRESULT deviceResult = D3D12CreateDevice(
        nullptr,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(device.GetAddressOf()));

    // 作成に失敗したら終了する
    if (FAILED(deviceResult))
    {
        MessageBoxW(
            hwnd,
            L"DirectX 12 の Device 作成に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);

        DestroyWindow(hwnd);
        return 1;
    }

#if defined(_DEBUG)
    // DirectX 12の診断メッセージを管理する機能
    ComPtr<ID3D12InfoQueue> infoQueue;

    CheckResult(
        device.As(&infoQueue),
        L"Info Queue の取得");

    // 重大な問題を検出したらデバッガーで停止する
    CheckResult(
        infoQueue->SetBreakOnSeverity(
            D3D12_MESSAGE_SEVERITY_CORRUPTION,
            TRUE),
        L"CORRUPTION の停止設定");

    // エラーを検出したらデバッガーで停止する
    CheckResult(
        infoQueue->SetBreakOnSeverity(
            D3D12_MESSAGE_SEVERITY_ERROR,
            TRUE),
        L"ERROR の停止設定");
#endif

    // 成功したことをタイトルで確認する
    SetWindowTextW(
        hwnd,
        L"DirectX 12 勉強 - Device 作成成功");

    // 作るコマンドキューの設定
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    // コマンドキューを保持する変数
    ComPtr<ID3D12CommandQueue> commandQueue;

    // Deviceを使ってコマンドキューを作る
    const HRESULT queueResult = device->CreateCommandQueue(
        &queueDesc,
        IID_PPV_ARGS(commandQueue.GetAddressOf()));

    if (FAILED(queueResult))
    {
        MessageBoxW(
            hwnd,
            L"コマンドキューの作成に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);

        DestroyWindow(hwnd);
        return 1;
    }

    SetWindowTextW(
        hwnd,
        L"DirectX 12 勉強 - Command Queue 作成成功");

    // スワップチェーンなどを作るための窓口
    ComPtr<IDXGIFactory4> factory;

    const HRESULT factoryResult = CreateDXGIFactory2(
        0,
        IID_PPV_ARGS(factory.GetAddressOf()));

    if (FAILED(factoryResult))
    {
        MessageBoxW(
            hwnd,
            L"DXGI Factory の作成に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);

        DestroyWindow(hwnd);
        return 1;
    }

    // 表示用の画像を2枚用意する
    constexpr UINT frameCount = 2;

    // スワップチェーンの設定
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = 0;
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = frameCount;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    // まず IDXGISwapChain1 として受け取る
    ComPtr<IDXGISwapChain1> swapChain1;

    const HRESULT swapChainResult = factory->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        swapChain1.GetAddressOf());

    if (FAILED(swapChainResult))
    {
        MessageBoxW(
            hwnd,
            L"スワップチェーンの作成に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);

        DestroyWindow(hwnd);
        return 1;
    }

    // 後で描画対象の画像番号を取得できるようにする
    ComPtr<IDXGISwapChain3> swapChain;

    const HRESULT interfaceResult = swapChain1.As(&swapChain);

    if (FAILED(interfaceResult))
    {
        MessageBoxW(
            hwnd,
            L"IDXGISwapChain3 の取得に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);

        DestroyWindow(hwnd);
        return 1;
    }

    SetWindowTextW(
        hwnd,
        L"DirectX 12 勉強 - Swap Chain 作成成功");

    // RTVを2個格納するための設定
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = frameCount;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    // RTVを格納するヒープ
    ComPtr<ID3D12DescriptorHeap> rtvHeap;

    const HRESULT rtvHeapResult = device->CreateDescriptorHeap(
        &rtvHeapDesc,
        IID_PPV_ARGS(rtvHeap.GetAddressOf()));

    if (FAILED(rtvHeapResult))
    {
        MessageBoxW(
            hwnd,
            L"RTV用ヒープの作成に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);

        DestroyWindow(hwnd);
        return 1;
    }

    // スワップチェーンの画像への参照を保持する
    ComPtr<ID3D12Resource> backBuffers[frameCount];

    // 隣のRTVへ移動するときの間隔を取得する
    const UINT rtvDescriptorSize =
        device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // RTV用ヒープの先頭位置を取得する
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        rtvHeap->GetCPUDescriptorHandleForHeapStart();

    // 画像ごとにRTVを作る
    for (UINT i = 0; i < frameCount; ++i)
    {
        // スワップチェーンが持っている画像を取得する
        const HRESULT bufferResult = swapChain->GetBuffer(
            i,
            IID_PPV_ARGS(backBuffers[i].GetAddressOf()));

        if (FAILED(bufferResult))
        {
            MessageBoxW(
                hwnd,
                L"バックバッファの取得に失敗しました。",
                L"エラー",
                MB_OK | MB_ICONERROR);

            DestroyWindow(hwnd);
            return 1;
        }

        // この画像を描画先として使うためのRTVを作る
        device->CreateRenderTargetView(
            backBuffers[i].Get(),
            nullptr,
            rtvHandle);

        // 次のRTVを格納する位置へ進める
        rtvHandle.ptr += rtvDescriptorSize;
    }

    SetWindowTextW(
        hwnd,
        L"DirectX 12 勉強 - RTV 作成処理完了");

    // 命令の保存用メモリを管理するオブジェクト
    ComPtr<ID3D12CommandAllocator> commandAllocator;

    const HRESULT allocatorResult = device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(commandAllocator.GetAddressOf()));

    if (FAILED(allocatorResult))
    {
        MessageBoxW(
            hwnd,
            L"Command Allocator の作成に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);

        DestroyWindow(hwnd);
        return 1;
    }

    // GPUへの命令を記録するオブジェクト
    ComPtr<ID3D12GraphicsCommandList> commandList;

    const HRESULT commandListResult = device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(commandList.GetAddressOf()));

    if (FAILED(commandListResult))
    {
        MessageBoxW(
            hwnd,
            L"Command List の作成に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);

        DestroyWindow(hwnd);
        return 1;
    }

    // 作成直後は記録中なので、いったん記録を終了する
    const HRESULT closeResult = commandList->Close();

    if (FAILED(closeResult))
    {
        MessageBoxW(
            hwnd,
            L"Command List の Close に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);

        DestroyWindow(hwnd);
        return 1;
    }

    SetWindowTextW(
        hwnd,
        L"DirectX 12 勉強 - Command List 作成成功");

    // GPUの処理完了を確認するFence
    ComPtr<ID3D12Fence> fence;

    const HRESULT fenceResult = device->CreateFence(
        0,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(fence.GetAddressOf()));

    if (FAILED(fenceResult))
    {
        MessageBoxW(
            hwnd,
            L"Fence の作成に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);

        DestroyWindow(hwnd);
        return 1;
    }

    // 次にGPUへ渡す完了番号
    UINT64 fenceValue = 1;

    // Fenceの到達通知を受け取るWindowsのイベント
    HANDLE fenceEvent = CreateEventW(
        nullptr,
        FALSE,
        FALSE,
        nullptr);

    if (fenceEvent == nullptr)
    {
        MessageBoxW(
            hwnd,
            L"Fence用イベントの作成に失敗しました。",
            L"エラー",
            MB_OK | MB_ICONERROR);

        DestroyWindow(hwnd);
        return 1;
    }

    SetWindowTextW(
        hwnd,
        L"DirectX 12 勉強 - Fence 作成成功");

    // R"( ... )" は、改行を含む文字列を書くC++の記法
    const char shaderSource[] = R"(
struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VSOutput VSMain(uint vertexId : SV_VertexID)
{
    float2 positions[3] =
    {
        float2( 0.0,  0.6),
        float2( 0.6, -0.6),
        float2(-0.6, -0.6)
    };

    float4 colors[3] =
    {
        float4(1.0, 0.0, 0.0, 1.0),
        float4(0.0, 1.0, 0.0, 1.0),
        float4(0.0, 0.0, 1.0, 1.0)
    };

    VSOutput output;
    output.position = float4(positions[vertexId], 0.0, 1.0);
    output.color = colors[vertexId];
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    return input.color;
}
)";

    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(_DEBUG)
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> shaderErrors;

    HRESULT shaderResult = D3DCompile(
        shaderSource,
        sizeof(shaderSource) - 1,
        "Triangle",
        nullptr,
        nullptr,
        "VSMain",
        "vs_5_0",
        shaderFlags,
        0,
        vertexShader.GetAddressOf(),
        shaderErrors.GetAddressOf());

    if (shaderErrors)
    {
        OutputDebugStringA(
            static_cast<const char*>(shaderErrors->GetBufferPointer()));
    }

    CheckResult(shaderResult, L"頂点シェーダーのコンパイル");

    shaderErrors.Reset();

    shaderResult = D3DCompile(
        shaderSource,
        sizeof(shaderSource) - 1,
        "Triangle",
        nullptr,
        nullptr,
        "PSMain",
        "ps_5_0",
        shaderFlags,
        0,
        pixelShader.GetAddressOf(),
        shaderErrors.GetAddressOf());

    if (shaderErrors)
    {
        OutputDebugStringA(
            static_cast<const char*>(shaderErrors->GetBufferPointer()));
    }

    CheckResult(shaderResult, L"ピクセルシェーダーのコンパイル");

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};

    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> rootSignatureErrors;

    const HRESULT serializeResult = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        rootSignatureBlob.GetAddressOf(),
        rootSignatureErrors.GetAddressOf());

    if (rootSignatureErrors)
    {
        OutputDebugStringA(
            static_cast<const char*>(
                rootSignatureErrors->GetBufferPointer()));
    }

    CheckResult(serializeResult, L"Root Signature のシリアライズ");

    ComPtr<ID3D12RootSignature> rootSignature;

    CheckResult(
        device->CreateRootSignature(
            0,
            rootSignatureBlob->GetBufferPointer(),
            rootSignatureBlob->GetBufferSize(),
            IID_PPV_ARGS(rootSignature.GetAddressOf())),
        L"Root Signature の作成");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

    psoDesc.pRootSignature = rootSignature.Get();

    psoDesc.VS = {
        vertexShader->GetBufferPointer(),
        vertexShader->GetBufferSize()
    };

    psoDesc.PS = {
        pixelShader->GetBufferPointer(),
        pixelShader->GetBufferSize()
    };

    // 三角形の内側を塗る。今回は裏向きの面も省略しない
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;

    // 色を混ぜるブレンド処理は使わない
    D3D12_RENDER_TARGET_BLEND_DESC& blend =
        psoDesc.BlendState.RenderTarget[0];

    blend.BlendEnable = FALSE;
    blend.LogicOpEnable = FALSE;
    blend.SrcBlend = D3D12_BLEND_ONE;
    blend.DestBlend = D3D12_BLEND_ZERO;
    blend.BlendOp = D3D12_BLEND_OP_ADD;
    blend.SrcBlendAlpha = D3D12_BLEND_ONE;
    blend.DestBlendAlpha = D3D12_BLEND_ZERO;
    blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blend.LogicOp = D3D12_LOGIC_OP_NOOP;
    blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // 今回は奥行き判定を使わない
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.DepthWriteMask =
        D3D12_DEPTH_WRITE_MASK_ZERO;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = swapChainDesc.Format;
    psoDesc.SampleDesc.Count = 1;

    ComPtr<ID3D12PipelineState> pipelineState;

    CheckResult(
        device->CreateGraphicsPipelineState(
            &psoDesc,
            IID_PPV_ARGS(pipelineState.GetAddressOf())),
        L"三角形用PSOの作成");

    // 実際のバックバッファの大きさを使う
    const D3D12_RESOURCE_DESC backBufferDesc =
        backBuffers[0]->GetDesc();

    D3D12_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(backBufferDesc.Width);
    viewport.Height = static_cast<float>(backBufferDesc.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissorRect{};
    scissorRect.right = static_cast<LONG>(backBufferDesc.Width);
    scissorRect.bottom = static_cast<LONG>(backBufferDesc.Height);

    // ③ 通知を処理しながら、ゲームを動かし続ける
    MSG message{};
    bool running = true;

    while (running)
    {
        // 届いている通知を順番に取り出す
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            // アプリの終了要求
            if (message.message == WM_QUIT)
            {
                running = false;
                break;
            }

            // キー入力から文字入力の通知を生成する
            TranslateMessage(&message);

            // 通知を WindowProc に渡す
            DispatchMessageW(&message);
        }

        // 終了要求があったら、更新や描画をせずに抜ける
        if (!running)
        {
            break;
        }

        // ここにゲームの更新処理を書く
        // Update();

        // ここに DirectX 12 の描画処理を書く
        // Render();

        // 描画処理がまだないので、一時的にCPUを休ませる
        // 1. 今回描画する画像の番号を取得する
        const UINT frameIndex = swapChain->GetCurrentBackBufferIndex();

        // 2. 命令を記録する準備
        // 前のフレームの最後でGPUの完了を待っているので再利用できる
        CheckResult(
            commandAllocator->Reset(),
            L"Command Allocator の Reset");

        CheckResult(
            commandList->Reset(commandAllocator.Get(), pipelineState.Get()),
            L"Command List の Reset");

        // 3. 画像を「表示用」から「描画用」へ切り替える
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = backBuffers[frameIndex].Get();
        barrier.Transition.Subresource =
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        commandList->ResourceBarrier(1, &barrier);

        // 4. 今回の画像に対応するRTVの位置を求める
        D3D12_CPU_DESCRIPTOR_HANDLE currentRtv =
            rtvHeap->GetCPUDescriptorHandleForHeapStart();

        currentRtv.ptr +=
            static_cast<SIZE_T>(frameIndex) * rtvDescriptorSize;

        // 5. 青く塗る命令を記録する
        const float clearColor[] = { 0.0f, 0.2f, 0.8f, 1.0f };

        commandList->ClearRenderTargetView(
            currentRtv,
            clearColor,
            0,
            nullptr);

        // 三角形の描画設定
        commandList->SetGraphicsRootSignature(rootSignature.Get());

        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);

        // 描画結果を書き込むRTVを指定する
        commandList->OMSetRenderTargets(
            1,
            &currentRtv,
            FALSE,
            nullptr);

        // 頂点を3個ずつ組み合わせて三角形にする
        commandList->IASetPrimitiveTopology(
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 3頂点で構成される図形を1個描く
        commandList->DrawInstanced(
            3,
            1,
            0,
            0);

        // 6. 画像を「描画用」から「表示用」へ戻す
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        commandList->ResourceBarrier(1, &barrier);

        // 7. 記録を終了する
        CheckResult(
            commandList->Close(),
            L"Command List の Close");

        // 8. 記録した命令をGPUへ送る
        ID3D12CommandList* commandLists[] = { commandList.Get() };

        commandQueue->ExecuteCommandLists(
            1,
            commandLists);

        // 9. 描いた画像の表示を要求する
        CheckResult(
            swapChain->Present(1, 0),
            L"Present");

        // 10. このフレームの完了番号をキューに入れる
        const UINT64 waitValue = fenceValue;

        CheckResult(
            commandQueue->Signal(fence.Get(), waitValue),
            L"Fence の Signal");

        ++fenceValue;

        // 11. GPUがこの完了番号まで到達するのを待つ
        if (fence->GetCompletedValue() < waitValue)
        {
            CheckResult(
                fence->SetEventOnCompletion(waitValue, fenceEvent),
                L"Fence の完了通知登録");

            const DWORD waitResult =
                WaitForSingleObject(fenceEvent, INFINITE);

            if (waitResult != WAIT_OBJECT_0)
            {
                const HRESULT waitError =
                    (waitResult == WAIT_FAILED)
                    ? HRESULT_FROM_WIN32(GetLastError())
                    : E_FAIL;

                CheckResult(waitError, L"GPUの完了待ち");
            }
        }

        // GPUが利用できなくなっていないか確認する
        CheckResult(
            device->GetDeviceRemovedReason(),
            L"GPUの状態確認");
    }

    CloseHandle(fenceEvent);

    return static_cast<int>(message.wParam);

}
