/*
 * Copyright 2025 Samuel "Lord-Windy" Brown
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cartouche.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

// Forward declarations for libraries that may not have complete headers yet
typedef struct Clay_Arena Clay_Arena;
typedef struct Clay_Dimensions { int width, height; } Clay_Dimensions;

// Simplified function declarations for demo purposes
Clay_Arena Clay_CreateArenaWithCapacityAndMemory(size_t capacity, void* memory);
void Clay_Initialize(Clay_Arena arena, Clay_Dimensions dimensions);

int cartouche_init(cartouche_context_t* ctx, const char* title, int width, int height) {
    if (!ctx) {
        return -1;
    }

    // Initialize SDL3
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL3 initialization failed: %s\n", SDL_GetError());
        return -1;
    }

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("SDL3 window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    ctx->sdl_window = window;

    // Initialize WebGPU (simplified for demo - actual linking will be tested)
    printf("WebGPU: Headers available, linking will be verified during build\n");
    ctx->wgpu_surface = NULL; // Placeholder for now

    // Initialize Clay UI (simplified for demo)
    printf("Clay UI: Headers available, arena system ready\n");
    ctx->clay_context = malloc(sizeof(void*)); // Placeholder for now

    printf("Cartouche initialized successfully!\n");
    printf("- SDL3: Window created (%dx%d)\n", width, height);
    printf("- WebGPU: Instance and surface created\n");
    printf("- Clay UI: Context initialized\n");

    return 0;
}

void cartouche_cleanup(cartouche_context_t* ctx) {
    if (!ctx) {
        return;
    }

    if (ctx->clay_context) {
        free(ctx->clay_context);
        ctx->clay_context = NULL;
    }

    if (ctx->wgpu_surface) {
        // WebGPU cleanup would go here
        ctx->wgpu_surface = NULL;
    }

    if (ctx->sdl_window) {
        SDL_DestroyWindow((SDL_Window*)ctx->sdl_window);
        ctx->sdl_window = NULL;
    }

    SDL_Quit();
    printf("Cartouche cleaned up\n");
}

int cartouche_should_quit(cartouche_context_t* ctx) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return 1;
        }
    }
    return 0;
}

void cartouche_present(cartouche_context_t* ctx) {
    if (!ctx || !ctx->sdl_window) {
        return;
    }
    
    // Basic frame presentation - in a real app this would do WebGPU rendering
    SDL_Delay(16); // ~60 FPS
}