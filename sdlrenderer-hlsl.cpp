#include <cassert>
#include <cstdint>

// Well, if it's SDL + Direct3d HLSL we are going to need those
#include <SDL.h>
#include <d3d9.h>

// This includes the binary for the compiled pixel shader.
#include "fractal.h"

// Constants for the window
const static char window_title[] = "SDL_Renderer + HLSL";
const static int window_width = 640;
const static int window_height = 480;

bool should_quit()
{
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
    case SDL_QUIT:
      return true;
    default:; // We are only interested in the "quit" event
    }
  }

  return false;
}

// Builds a render that's guaranteed to be direct3d
SDL_Renderer *direct3d9_renderer(SDL_Window *window)
{
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d9");
  const static int use_first_renderer_supporting_flags = -1;
  SDL_Renderer *renderer = SDL_CreateRenderer(window,
                                              use_first_renderer_supporting_flags,
                                              SDL_RENDERER_ACCELERATED);

  SDL_RendererInfo renderer_details;
  SDL_GetRendererInfo(renderer, &renderer_details);

  // Since we are relying on a hint to provide a Direct3D9 renderer, we may get
  // an OpenGL one still, if Direct3D9 is not supported. To avoid hard bugs
  // down the line, we make sure that we are returning a direct3d renderer.
  //
  // In a real application we would select the shading language (HLSL or GLSL)
  // based on the renderer we get instead of forcing it to be direct3d.
  assert(!strcmp(renderer_details.name, "direct3d"));

  return renderer;
}

// This fetches the Direct3D device from inside the opaque SDL_Renderer field
// "driverdata". We copied its definition for the Direct3D renderer from the SDL
// source code to the top of this file so that works fine.
//
// After fetching the device we jsut need to call SetPixelShader and we are
// golden.
IDirect3DPixelShader9 *apply_hlsl_pixel_shader(SDL_Renderer *renderer, IDirect3DPixelShader9 *shader)
{
  IDirect3DDevice9 *device = SDL_RenderGetD3D9Device(renderer);

  IDirect3DPixelShader9 *current_shader;

  assert(D3D_OK == IDirect3DDevice9_GetPixelShader(device, &current_shader));

  assert(D3D_OK == IDirect3DDevice9_SetPixelShader(device, shader));

  return current_shader;
}

// Creates a shader doing the same thing as the above function to get the device
// and then calling CreatePixelShader with the binary array we imported from
// "fractal.h".
IDirect3DPixelShader9 *hlsl_pixel_shader(SDL_Renderer *renderer)
{
  IDirect3DDevice9 *device = SDL_RenderGetD3D9Device(renderer);

  IDirect3DPixelShader9 *shader;
  assert(D3D_OK == IDirect3DDevice9_CreatePixelShader(device, reinterpret_cast<const DWORD *>(g_ps21_main), &shader));

  return shader;
}

// Writes a byte to the first byte of a SDL texture.
void send_byte_using_texture(SDL_Texture *texture, uint8_t byte)
{
  uint32_t *pixels;
  int pitch;

  SDL_LockTexture(texture, nullptr, reinterpret_cast<void **>(&pixels), &pitch);

  pixels[0] = byte;

  SDL_UnlockTexture(texture);
}

int main(int arg_count, char **arg_vector)
{
  // 1. Create a window, renderer, texture and shader
  SDL_Window *window = SDL_CreateWindow(window_title,
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        window_width, window_height,
                                        SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer = direct3d9_renderer(window);

  SDL_Texture *texture = SDL_CreateTexture(renderer,
                                           SDL_PIXELFORMAT_RGBA8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           1, 1);
  IDirect3DPixelShader9 *shader = hlsl_pixel_shader(renderer);
  uint8_t frame_counter = 0;

  // 2. Loop until the window is closed
  while (true)
  {
    if (should_quit())
      break;

    // Allow the shader to do effects that change with time by writting to the
    // texture that the shader can read.
    send_byte_using_texture(texture, frame_counter++);

    // Do the drawing using our HLSL shader
    const auto previous_shader = apply_hlsl_pixel_shader(renderer, shader);
    {
      SDL_RenderCopy(renderer, texture, nullptr, nullptr);
      SDL_RenderFlush(renderer);
    }
    apply_hlsl_pixel_shader(renderer, previous_shader);

    // ... show the drawing
    SDL_RenderPresent(renderer);
  }

  // 3. Clear everything we allocated
  IDirect3DPixelShader9_Release(shader);

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  return 0;
}
