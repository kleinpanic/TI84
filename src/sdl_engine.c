#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include "sdl_engine.h"

// Screen and window properties
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 640

// Button properties
#define BUTTON_WIDTH 40
#define BUTTON_HEIGHT 30
#define BUTTON_SPACING 5

// TI-84 display dimensions
#define DISPLAY_WIDTH 260
#define DISPLAY_HEIGHT 100
#define DISPLAY_X 30
#define DISPLAY_Y 20

// Buffer to hold the expression shown on the calculator display
char expression[256] = "";  // Screen buffer

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;

// Function prototypes
void draw_button(int x, int y, int w, int h, SDL_Color color, const char* label);
void update_screen();
void append_to_expression(char c);
void handle_key(SDL_Keycode key);
void handle_mouse_click(int x, int y);

// Initialize SDL and SDL_ttf
int init_sdl() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        return 0;
    }

    window = SDL_CreateWindow("TI-84 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Load font (adjust the path to where the font file is located)
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 18);
    if (font == NULL) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        return 0;
    }

    return 1;
}

// Clean up SDL and TTF resources
void close_sdl() {
    TTF_CloseFont(font);
    font = NULL;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
}

// Draw a button with text on it
void draw_button(int x, int y, int w, int h, SDL_Color color, const char* label) {
    // Draw the button background
    SDL_Rect rect = { x, y, w, h };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);

    // Render the text label on the button
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, label, textColor);
    SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Get the dimensions of the text to center it
    int text_width = 0, text_height = 0;
    TTF_SizeText(font, label, &text_width, &text_height);
    SDL_Rect textRect = {x + (w - text_width) / 2, y + (h - text_height) / 2, text_width, text_height};
    
    SDL_RenderCopy(renderer, text, NULL, &textRect);

    // Clean up
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(text);
}

// Update the calculator screen with the current expression
void update_screen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw the calculator screen area
    SDL_Rect display_rect = { DISPLAY_X, DISPLAY_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT };
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);  // Light gray display
    SDL_RenderFillRect(renderer, &display_rect);

    // Render the current expression on the screen
    SDL_Color textColor = {0, 0, 0, 255};  // Black text
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, expression, textColor);
    SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, textSurface);
    
    int text_width = 0, text_height = 0;
    TTF_SizeText(font, expression, &text_width, &text_height);
    SDL_Rect textRect = { DISPLAY_X + 10, DISPLAY_Y + (DISPLAY_HEIGHT - text_height) / 2, text_width, text_height };
    SDL_RenderCopy(renderer, text, NULL, &textRect);

    // Clean up
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(text);
}

// Append characters to the screen's expression buffer
void append_to_expression(char c) {
    int len = strlen(expression);
    if (len < sizeof(expression) - 1) {
        expression[len] = c;
        expression[len + 1] = '\0';
    }
}

// Render the entire calculator layout, including buttons
void render_calculator() {
    update_screen();  // Update the screen first

    // Define button color
    SDL_Color button_color = {100, 100, 100, 255};  // Gray  
    SDL_Color purple_button_color = {128, 0, 128, 255};  // Purple
    SDL_Color blue_button_color = {0, 0, 255, 255};  // Blue
    SDL_Color green_button_color = {0, 255, 0, 255};  // Green
    SDL_Color black_button_color = {0, 0, 0, 255};  // Black

    // Right-side buttons layout
    int right_x = 220;  // X position of the right-side buttons
    int start_y = 500;  // Move the buttons down, so they are closer to the bottom

    // Draw right side buttons: "/", "*", "-", "+", "Enter"
    draw_button(right_x, start_y - 120, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "/");
    draw_button(right_x, start_y - 80, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "*");
    draw_button(right_x, start_y -40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "-");
    draw_button(right_x, start_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "+");
    draw_button(right_x, start_y + 40, BUTTON_WIDTH, BUTTON_HEIGHT * 2, button_color, "Enter");

    int start_x = 70;
    
    draw_button(start_x, start_y + 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "0");
    draw_button(start_x + 50, start_y + 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, ".");
    draw_button(start_x + 100, start_y + 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "(-)");

    draw_button(start_x, start_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "1");
    draw_button(start_x + 50, start_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "2");
    draw_button(start_x + 100, start_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "3");

    start_y -= 40;  // Move up for the next row
    draw_button(start_x, start_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "4");
    draw_button(start_x + 50, start_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "5");
    draw_button(start_x + 100, start_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "6");

    start_y -= 40;  // Move up for the next row
    draw_button(start_x, start_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "7");
    draw_button(start_x + 50, start_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "8");
    draw_button(start_x + 100, start_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "9");


    // Additional buttons: "(" and ")" above 8 and 9, "," above 7
    draw_button(start_x + 50, start_y - 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "(");  // Above "8"
    draw_button(start_x + 100, start_y - 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, ")");  // Above "9"
    draw_button(start_x, start_y - 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, ",");  


    // Left column buttons
    int left_x = 20;
    // "ON" button to the left of "0", larger (same size as Enter)
    draw_button(left_x, start_y + 120, BUTTON_WIDTH, BUTTON_HEIGHT * 2, button_color, "ON");

    // "log" left of 7, "ln" left of 4, "q" left of 1, "x^2" left of ","
    draw_button(left_x, start_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "log");
    draw_button(left_x, start_y + 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "ln");
    draw_button(left_x, start_y + 80, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "q");
    draw_button(left_x, start_y - 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "x^2");

    // Adding new buttons above the current layout
    start_y -= 40;  // Move further up for the next row of buttons
    draw_button(left_x, start_y - 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "X^-1");  // Above "x^2"
    draw_button(start_x, start_y - 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "SIN");  // Above ","
    draw_button(start_x + 50, start_y - 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "COS");  // Above "("
    draw_button(start_x + 100, start_y - 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "TAN");  // Above ")"
    draw_button(right_x, start_y - 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "^");  // Above "/"

    // Top row: Clear, VARS, PRGM, APPS (purple), MATH
    draw_button(right_x, start_y - 80, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "CLEAR");  // Above "^"
    draw_button(right_x - 50, start_y - 80, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "VARS");
    draw_button(right_x - 100, start_y - 80, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "PRGM");
    draw_button(right_x - 150, start_y - 80, BUTTON_WIDTH, BUTTON_HEIGHT, purple_button_color, "APPS");
    draw_button(right_x - 200, start_y - 80, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "MATH");  // Above "X^-1"

    // New row: STAT, DEL, X, MODE
    draw_button(right_x - 100, start_y - 120, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "STAT");  // Above PRGM
    draw_button(right_x - 100, start_y - 160, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "DEL");  // Above STAT
    draw_button(right_x - 150, start_y - 120, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "X");  // Above APPS
    draw_button(right_x - 150, start_y - 160, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "MODE");  // Above X

    // Arrow keys (cross layout)
    int arrow_center_x = right_x;  // Center of the arrow key layout
    int arrow_center_y = start_y - 160;  // Vertical center of the arrow key layout

    draw_button(arrow_center_x, arrow_center_y - 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "UP");    // UP
    draw_button(arrow_center_x, arrow_center_y + 40, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "DOWN");  // DOWN
    draw_button(arrow_center_x - 50, arrow_center_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "LEFT");  // LEFT
    draw_button(arrow_center_x + 50, arrow_center_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_color, "RIGHT"); // RIGHT

    // ALPHA (green) and 2ND (blue) buttons above MATH
    draw_button(right_x - 200, start_y - 120, BUTTON_WIDTH, BUTTON_HEIGHT, green_button_color, "ALPHA");
    draw_button(right_x - 200, start_y - 160, BUTTON_WIDTH, BUTTON_HEIGHT, blue_button_color, "2ND");

    // Present the updated rendering
    SDL_RenderPresent(renderer);
}

// Handle key press events
void handle_key(SDL_Keycode key) {
    switch (key) {
        case SDLK_1:
            append_to_expression('1');
            break;
        case SDLK_2:
            append_to_expression('2');
            break;
        case SDLK_PLUS:
        case SDLK_KP_PLUS:
            append_to_expression('+');
            break;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            append_to_expression('-');
            break;
        case SDLK_SLASH:
        case SDLK_KP_DIVIDE:
            append_to_expression('/');
            break;
        case SDLK_ASTERISK:
        case SDLK_KP_MULTIPLY:
            append_to_expression('*');
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            // Enter key handling can execute the expression in the future
            printf("Enter key pressed\n");
            break;
        default:
            break;
    }
}

// Detect mouse click events on buttons
void handle_mouse_click(int x, int y) {
    // Check if the click happened within the button regions
    if (x >= 240 && x <= 280 && y >= 310 && y <= 370) {
        printf("Enter button clicked\n");
    } else if (x >= 240 && x <= 280 && y >= 270 && y <= 300) {
        printf("+ button clicked\n");
        append_to_expression('+');
    } else if (x >= 240 && x <= 280 && y >= 230 && y <= 260) {
        printf("- button clicked\n");
        append_to_expression('-');
    } else if (x >= 240 && x <= 280 && y >= 190 && y <= 220) {
        printf("* button clicked\n");
        append_to_expression('*');
    } else if (x >= 240 && x <= 280 && y >= 150 && y <= 180) {
        printf("/ button clicked\n");
        append_to_expression('/');
    }
}

// Handle events
void handle_input(int* quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            *quit = 1;
        } else if (event.type == SDL_KEYDOWN) {
            handle_key(event.key.keysym.sym);
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            handle_mouse_click(event.button.x, event.button.y);
        }
    }
}
