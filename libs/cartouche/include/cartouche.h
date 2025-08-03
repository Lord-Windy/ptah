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

#ifndef CARTOUCHE_H
#define CARTOUCHE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void* sdl_window;
    void* wgpu_surface;
    void* clay_context;
} cartouche_context_t;

int cartouche_init(cartouche_context_t* ctx, const char* title, int width, int height);
void cartouche_cleanup(cartouche_context_t* ctx);
int cartouche_should_quit(cartouche_context_t* ctx);
void cartouche_present(cartouche_context_t* ctx);

#ifdef __cplusplus
}
#endif

#endif // CARTOUCHE_H