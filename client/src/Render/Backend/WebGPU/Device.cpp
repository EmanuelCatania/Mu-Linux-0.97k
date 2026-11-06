// Sin PCH: el backend WebGPU no depende de los encabezados del cliente.
#include "Render/Backend/WebGPU/Device.h"
#include <cstdio>
#include <cstring>
#include <string>

extern "C" void DbgLogPublic(const char*);

namespace WebGPU {
namespace {

struct State {
    WGPUInstance instance = nullptr;
    WGPUAdapter adapter = nullptr;
    WGPUDevice device = nullptr;
    WGPUQueue queue = nullptr;
    WGPUSurface surface = nullptr;
    WGPUTextureFormat format = WGPUTextureFormat_Undefined;
    WGPUPresentMode immediateMode = WGPUPresentMode_Fifo;
    unsigned width = 0, height = 0;
    bool vsync = true;
    bool configured = false;
    WGPUTexture frameTexture = nullptr;
    WGPUTextureView frameView = nullptr;
    std::string description;
} g;

std::string Text(WGPUStringView view) {
    if (!view.data) return {};
    return view.length == WGPU_STRLEN ? std::string(view.data) : std::string(view.data, view.length);
}

void Log(const char* format, const std::string& detail) {
    char line[512];
    std::snprintf(line, sizeof line, format, detail.c_str());
    DbgLogPublic(line);
}

const char* ApiName(WGPUBackendType type) {
    switch (type) {
    case WGPUBackendType_D3D12: return "D3D12";
    case WGPUBackendType_D3D11: return "D3D11";
    case WGPUBackendType_Vulkan: return "Vulkan";
    case WGPUBackendType_OpenGL: return "OpenGL";
    case WGPUBackendType_Metal: return "Metal";
    default: return "?";
    }
}

WGPUBackendType BackendType(Api api) {
    switch (api) {
    case Api::D3D12: return WGPUBackendType_D3D12;
    case Api::D3D11: return WGPUBackendType_D3D11;
    case Api::Vulkan: return WGPUBackendType_Vulkan;
    default: return WGPUBackendType_Undefined;
    }
}

// Espera síncrona de un pedido asíncrono de Dawn (adaptador, dispositivo).
template <class Result>
bool Wait(WGPUFuture future, const Result& result) {
    WGPUFutureWaitInfo wait = WGPU_FUTURE_WAIT_INFO_INIT;
    wait.future = future;
    return wgpuInstanceWaitAny(g.instance, 1, &wait, UINT64_MAX) == WGPUWaitStatus_Success && result.done;
}

struct AdapterResult { WGPUAdapter adapter = nullptr; bool done = false; };
struct DeviceResult { WGPUDevice device = nullptr; bool done = false; };

void OnAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* user, void*) {
    auto* result = static_cast<AdapterResult*>(user);
    result->done = true;
    if (status == WGPURequestAdapterStatus_Success) result->adapter = adapter;
    else Log("WebGPU: sin adaptador: %s", Text(message));
}

void OnDevice(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* user, void*) {
    auto* result = static_cast<DeviceResult*>(user);
    result->done = true;
    if (status == WGPURequestDeviceStatus_Success) result->device = device;
    else Log("WebGPU: no se pudo crear el dispositivo: %s", Text(message));
}

void OnUncapturedError(WGPUDevice const*, WGPUErrorType, WGPUStringView message, void*, void*) {
    // Un error que se repite cada cuadro llenaría el log: sólo los primeros.
    static unsigned count = 0;
    if (++count <= 100) Log("WebGPU: error: %s", Text(message));
    else if (count == 101) DbgLogPublic("WebGPU: demasiados errores, no se registran más");
}

void OnDeviceLost(WGPUDevice const*, WGPUDeviceLostReason reason, WGPUStringView message, void*, void*) {
    if (reason == WGPUDeviceLostReason_Destroyed || reason == WGPUDeviceLostReason_CallbackCancelled) return;
    Log("WebGPU: dispositivo perdido: %s", Text(message));
}

bool CreateSurface(const Surface& source) {
#ifdef _WIN32
    WGPUSurfaceSourceWindowsHWND hwnd = WGPU_SURFACE_SOURCE_WINDOWS_HWND_INIT;
    hwnd.hwnd = source.window;
    hwnd.hinstance = source.instance;
    WGPUSurfaceDescriptor descriptor = WGPU_SURFACE_DESCRIPTOR_INIT;
    descriptor.nextInChain = &hwnd.chain;
    g.surface = wgpuInstanceCreateSurface(g.instance, &descriptor);
#else
    (void)source;   // Linux x64: superficie Xlib/Wayland en una etapa siguiente.
#endif
    return g.surface != nullptr;
}

bool RequestAdapter(Api api) {
    WGPURequestAdapterOptions options = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
    options.backendType = BackendType(api);
    options.powerPreference = WGPUPowerPreference_HighPerformance;
    options.compatibleSurface = g.surface;
    AdapterResult result;
    WGPURequestAdapterCallbackInfo callback = WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT;
    callback.mode = WGPUCallbackMode_WaitAnyOnly;
    callback.callback = OnAdapter;
    callback.userdata1 = &result;
    if (!Wait(wgpuInstanceRequestAdapter(g.instance, &options, callback), result) || !result.adapter) return false;
    g.adapter = result.adapter;
    WGPUAdapterInfo info = WGPU_ADAPTER_INFO_INIT;
    if (wgpuAdapterGetInfo(g.adapter, &info) == WGPUStatus_Success) {
        g.description = std::string(ApiName(info.backendType)) + " | " + Text(info.device);
        wgpuAdapterInfoFreeMembers(info);
    }
    return true;
}

bool RequestDevice() {
    WGPUDeviceDescriptor descriptor = WGPU_DEVICE_DESCRIPTOR_INIT;
    descriptor.uncapturedErrorCallbackInfo.callback = OnUncapturedError;
    descriptor.deviceLostCallbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    descriptor.deviceLostCallbackInfo.callback = OnDeviceLost;
    DeviceResult result;
    WGPURequestDeviceCallbackInfo callback = WGPU_REQUEST_DEVICE_CALLBACK_INFO_INIT;
    callback.mode = WGPUCallbackMode_WaitAnyOnly;
    callback.callback = OnDevice;
    callback.userdata1 = &result;
    if (!Wait(wgpuAdapterRequestDevice(g.adapter, &descriptor, callback), result) || !result.device) return false;
    g.device = result.device;
    g.queue = wgpuDeviceGetQueue(g.device);
    return true;
}

// Formato y modo sin vsync que ofrece la superficie con este adaptador.
void ReadCapabilities() {
    WGPUSurfaceCapabilities caps = WGPU_SURFACE_CAPABILITIES_INIT;
    if (wgpuSurfaceGetCapabilities(g.surface, g.adapter, &caps) != WGPUStatus_Success) return;
    for (size_t i = 0; i < caps.formatCount; ++i) {
        // Formato lineal (sin sRGB): el 0.97k compone en espacio de pantalla.
        if (caps.formats[i] == WGPUTextureFormat_BGRA8Unorm || caps.formats[i] == WGPUTextureFormat_RGBA8Unorm) {
            g.format = caps.formats[i];
            break;
        }
    }
    if (g.format == WGPUTextureFormat_Undefined && caps.formatCount) g.format = caps.formats[0];
    for (size_t i = 0; i < caps.presentModeCount; ++i) {
        if (caps.presentModes[i] == WGPUPresentMode_Immediate) { g.immediateMode = WGPUPresentMode_Immediate; break; }
        if (caps.presentModes[i] == WGPUPresentMode_Mailbox) g.immediateMode = WGPUPresentMode_Mailbox;
    }
    wgpuSurfaceCapabilitiesFreeMembers(caps);
}

void DropFrame() {
    if (g.frameView) { wgpuTextureViewRelease(g.frameView); g.frameView = nullptr; }
    if (g.frameTexture) { wgpuTextureRelease(g.frameTexture); g.frameTexture = nullptr; }
}

} // namespace

bool Initialize(Api api, const Surface& surface, unsigned width, unsigned height, bool vsync) {
    if (g.device) return true;
    WGPUInstanceFeatureName features[] = { WGPUInstanceFeatureName_TimedWaitAny };
    WGPUInstanceDescriptor descriptor = WGPU_INSTANCE_DESCRIPTOR_INIT;
    descriptor.requiredFeatureCount = 1;
    descriptor.requiredFeatures = features;
    g.instance = wgpuCreateInstance(&descriptor);
    const bool ok = g.instance && CreateSurface(surface) && RequestAdapter(api) && RequestDevice();
    if (!ok) { Release(); return false; }
    ReadCapabilities();
    Log("WebGPU: %s", g.description);
    if (!Configure(width, height, vsync)) { Release(); return false; }
    return true;
}

bool Configure(unsigned width, unsigned height, bool vsync) {
    if (!g.device || !g.surface || g.format == WGPUTextureFormat_Undefined || !width || !height) return false;
    DropFrame();
    WGPUSurfaceConfiguration config = WGPU_SURFACE_CONFIGURATION_INIT;
    config.device = g.device;
    config.format = g.format;
    config.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
    config.width = width;
    config.height = height;
    config.alphaMode = WGPUCompositeAlphaMode_Opaque;
    config.presentMode = vsync ? WGPUPresentMode_Fifo : g.immediateMode;
    wgpuSurfaceConfigure(g.surface, &config);
    g.width = width; g.height = height; g.vsync = vsync; g.configured = true;
    const char* mode = config.presentMode == WGPUPresentMode_Fifo ? "Fifo (vsync)"
        : config.presentMode == WGPUPresentMode_Immediate ? "Immediate" : "Mailbox";
    char line[96];
    std::snprintf(line, sizeof line, "WebGPU: superficie %ux%u, %s", width, height, mode);
    DbgLogPublic(line);
    return true;
}

void Release() {
    DropFrame();
    // La superficie primero: su swapchain usa el dispositivo al soltarse.
    if (g.surface && g.configured) wgpuSurfaceUnconfigure(g.surface);
    if (g.surface) wgpuSurfaceRelease(g.surface);
    if (g.queue) wgpuQueueRelease(g.queue);
    if (g.device) { wgpuDeviceDestroy(g.device); wgpuDeviceRelease(g.device); }
    if (g.adapter) wgpuAdapterRelease(g.adapter);
    if (g.instance) wgpuInstanceRelease(g.instance);
    g = State{};
}

bool IsReady() { return g.device && g.configured; }

bool BeginFrame() {
    if (!IsReady()) return false;
    DropFrame();
    WGPUSurfaceTexture frame = WGPU_SURFACE_TEXTURE_INIT;
    wgpuSurfaceGetCurrentTexture(g.surface, &frame);
    switch (frame.status) {
    case WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal:
    case WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal:
        break;
    case WGPUSurfaceGetCurrentTextureStatus_Outdated:
    case WGPUSurfaceGetCurrentTextureStatus_Lost:
        if (frame.texture) wgpuTextureRelease(frame.texture);
        Configure(g.width, g.height, g.vsync);
        return false;
    default:
        if (frame.texture) wgpuTextureRelease(frame.texture);
        return false;
    }
    g.frameTexture = frame.texture;
    g.frameView = wgpuTextureCreateView(g.frameTexture, nullptr);
    return g.frameView != nullptr;
}

void Present() {
    if (!g.frameView) return;
    wgpuSurfacePresent(g.surface);
    DropFrame();
    wgpuInstanceProcessEvents(g.instance);
}

unsigned Width() { return g.width; }
unsigned Height() { return g.height; }
WGPUDevice GetDevice() { return g.device; }
WGPUQueue GetQueue() { return g.queue; }
WGPUTextureFormat SurfaceFormat() { return g.format; }
WGPUTextureView FrameView() { return g.frameView; }
const char* Description() { return g.description.c_str(); }

} // namespace WebGPU
