#include <stdio.h>
#include "sdl_engine.h"
#include "math_engine.h"

int main(int argc, char* args[]) {
    if (!init_sdl()) {
        printf("Failed to initialize SDL!\n");
        return -1;
    }

    int quit = 0;
    int calculate = 0;  // Flag for when to calculate

    while (!quit) {
        handle_input(&quit);

        // Only calculate when a button is pressed (for example)
        if (calculate) {
            double result = add(5, 10);
            printf("Result of 5 + 10 = %.2f\n", result);
            calculate = 0;  // Reset the flag
        }

        render_calculator();
        SDL_Delay(100);
    }

    close_sdl();
    return 0;
}

