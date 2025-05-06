#include "GraphicUtils.h"

#include "Engine.h"
#include "stb_image_write.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#endif

#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

using namespace Supernova;

void Editor::GraphicUtils::saveImage(int width, int height, FramebufferRender& framebuffer){
    uint8_t* pixels = nullptr;
    bool needDelete = false;

    GraphicBackend backend = Engine::getGraphicBackend();

    if (backend == GraphicBackend::GLCORE || backend == GraphicBackend::GLES3) {
        // OpenGL
        pixels = new uint8_t[width * height * 4];
        needDelete = true;

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.getGLHandler());
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
#if defined(__APPLE__)
    else if (backend == GraphicBackend::METAL) {
        // Metal
        auto metalTexPtr = framebuffer.getColorTexture().getMetalHandler();
        id<MTLTexture> tex = (__bridge id<MTLTexture>)metalTexPtr;
        id<MTLDevice> device = [tex device];
        NSUInteger bytesPerRow = width * 4;
        NSUInteger dataSize = bytesPerRow * height;
        id<MTLBuffer> buffer = [device newBufferWithLength:dataSize options:MTLResourceStorageModeShared];

        id<MTLCommandQueue> queue = [device newCommandQueue];
        id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
        id<MTLBlitCommandEncoder> blit = [cmdBuf blitCommandEncoder];

        [blit copyFromTexture:tex
                 sourceSlice:0
                 sourceLevel:0
                sourceOrigin:MTLOriginMake(0, 0, 0)
                  sourceSize:MTLSizeMake(width, height, 1)
                   toBuffer:buffer
           destinationOffset:0
      destinationBytesPerRow:bytesPerRow
    destinationBytesPerImage:dataSize];
        [blit endEncoding];
        [cmdBuf commit];
        [cmdBuf waitUntilCompleted];

        pixels = (uint8_t*)[buffer contents];
        needDelete = false; // Don't delete!
    }
#endif
#if defined(_WIN32)
    else if (backend == GraphicBackend::D3D11) {
        // D3D11
        auto rtvPtr = framebuffer.getD3D11HandlerColorRTV();
        ID3D11RenderTargetView* rtv = reinterpret_cast<ID3D11RenderTargetView*>(const_cast<void*>(rtvPtr));

        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;
        ID3D11Resource* renderTargetResource = nullptr;
        D3D11_TEXTURE2D_DESC desc = {};
        ID3D11Texture2D* staging = nullptr;
        pixels = new uint8_t[width * height * 4];
        needDelete = true;

        rtv->GetDevice(&device);
        device->GetImmediateContext(&context);
        rtv->GetResource(&renderTargetResource);
        ID3D11Texture2D* srcTex = nullptr;
        renderTargetResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&srcTex);
        srcTex->GetDesc(&desc);

        // Create staging texture
        D3D11_TEXTURE2D_DESC stagingDesc = desc;
        stagingDesc.Usage = D3D11_USAGE_STAGING;
        stagingDesc.BindFlags = 0;
        stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        stagingDesc.MiscFlags = 0;
        device->CreateTexture2D(&stagingDesc, nullptr, &staging);

        // Copy to staging
        context->CopyResource(staging, srcTex);

        // Map and read
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        context->Map(staging, 0, D3D11_MAP_READ, 0, &mapped);

        for (int y = 0; y < height; ++y) {
            memcpy(&pixels[y * width * 4],
                (uint8_t*)mapped.pData + y * mapped.RowPitch,
                width * 4);
        }

        // Cleanup
        context->Unmap(staging, 0);
        staging->Release();
        srcTex->Release();
        renderTargetResource->Release();
        context->Release();
        device->Release();
    }
#endif

    stbi_write_png("output.png", width, height, 4, pixels, width * 4);

    if (needDelete && pixels) {
        delete[] pixels;
    }
}