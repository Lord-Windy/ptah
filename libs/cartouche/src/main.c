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
#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("Cartouche Demo - Testing SDL3, WebGPU, and Clay UI integration\n");
    printf("=============================================================\n");

    cartouche_context_t ctx = {0};
    
    if (cartouche_init(&ctx, "Cartouche Demo", 800, 600) != 0) {
        printf("Failed to initialize Cartouche\n");
        return 1;
    }

    printf("\nAll libraries initialized successfully!\n");
    printf("Press ESC or close window to exit...\n\n");

    // Simple main loop
    int frame_count = 0;
    while (!cartouche_should_quit(&ctx)) {
        cartouche_present(&ctx);
        
        frame_count++;
        if (frame_count % 60 == 0) {
            printf("Frame %d - All systems running\n", frame_count);
        }
        
        // Exit after a few seconds for demo purposes
        if (frame_count > 300) { // ~5 seconds at 60 FPS
            printf("Demo completed successfully!\n");
            break;
        }
    }

    cartouche_cleanup(&ctx);
    printf("Demo finished\n");
    return 0;
}