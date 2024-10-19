#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include "sdl_engine.h"
#include "math_engine.h"

// Screen and window properties
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 640

// Button properties
#define BUTTON_WIDTH 40
#define BUTTON_HEIGHT 30
#define BUTTON_SPACING 5

// TI-84 display dimensions
#define DISPLAY_WIDTH 280
#define DISPLAY_HEIGHT 130
#define DISPLAY_X 20
#define DISPLAY_Y 30

#define MAX_LINES 7  // Maximum number of lines to display
#define LINE_LENGTH 256  // Maximum length of a line

char screen_buffer[MAX_LINES][LINE_LENGTH] = {""};  // Circular buffer for 7 lines
int current_line = 0;  // Index of the current line being entered
int cursor_position = 0;  // Cursor position on the current line
int total_lines = 0;  // Total lines on the screen (max 7)

// Buffer to hold the expression shown on the calculator display
char expression[256] = "";  // Screen buffer

static int cursor_visible = 1;  // Blinking flag for the cursor
static Uint32 last_blink_time = 0;  // Timer for blinking

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

void toggle_cursor_blink() {
    Uint32 current_time = SDL_GetTicks();  // Get the current time in milliseconds
    if (current_time > last_blink_time + 500) {  // 500ms for blinking interval
        cursor_visible = !cursor_visible;
        last_blink_time = current_time;
    }
}

// Update the calculator screen with the current expression
void update_screen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw the calculator screen area
    SDL_Rect display_rect = { DISPLAY_X, DISPLAY_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT };
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);  // Light gray display
    SDL_RenderFillRect(renderer, &display_rect);

    // Adjustments: Setting better positioning variables
    int line_start_x = DISPLAY_X + 5;  // Start 5px from the left edge
    int line_start_y = DISPLAY_Y + 10; // Start 10px from the top edge
    int line_height = 20;              // Height between lines (spacing between expressions and results)

    // Render each line from the screen buffer
    for (int i = 0; i < MAX_LINES; i++) {
        if (strlen(screen_buffer[i]) > 0) {
            SDL_Color textColor = {0, 0, 0, 255};  // Black text
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, screen_buffer[i], textColor);
            SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, textSurface);
            
            int text_width = 0, text_height = 0;
            TTF_SizeText(font, screen_buffer[i], &text_width, &text_height);

            // For results (right-align): Check if the line contains a result (a number)
            if (i % 2 == 1) {
                // Right-align the result
                SDL_Rect textRect = { DISPLAY_X + DISPLAY_WIDTH - text_width - 5, line_start_y + i * line_height, text_width, text_height };  
                SDL_RenderCopy(renderer, text, NULL, &textRect);
            } else {
                // Left-align input expressions
                SDL_Rect textRect = { line_start_x, line_start_y + i * line_height, text_width, text_height };
                SDL_RenderCopy(renderer, text, NULL, &textRect);
            }

            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(text);
        }
    }    

    // Render the current expression
    SDL_Color textColor = {0, 0, 0, 255};  // Black text
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, expression, textColor);
    SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, textSurface);
    
    int text_width = 0, text_height = 0;
    TTF_SizeText(font, expression, &text_width, &text_height);

    // Render the expression at the top-left of the screen
    SDL_Rect textRect = { line_start_x, line_start_y + (current_line * line_height), text_width, text_height };
    SDL_RenderCopy(renderer, text, NULL, &textRect);

    // Cursor blinking logic
    toggle_cursor_blink();

    // Render the cursor after the current text in the expression if visible
    if (cursor_visible) {
        char cursor_char[] = "_";
        SDL_Surface* cursorSurface = TTF_RenderText_Solid(font, cursor_char, textColor);
        SDL_Texture* cursorTexture = SDL_CreateTextureFromSurface(renderer, cursorSurface);

        int cursor_width = 0, cursor_height = 0;
        TTF_SizeText(font, cursor_char, &cursor_width, &cursor_height);

        // Cursor should be placed right after the last character of the expression
        SDL_Rect cursorRect = { line_start_x + text_width, line_start_y + (current_line * line_height), cursor_width, cursor_height };
        SDL_RenderCopy(renderer, cursorTexture, NULL, &cursorRect);

        SDL_FreeSurface(cursorSurface);
        SDL_DestroyTexture(cursorTexture);
    }

    SDL_RenderPresent(renderer);

    // Clean up
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(text);
}

// Append characters to the screen's expression buffer
void append_to_expression(char c) {
    if (strlen(screen_buffer[current_line]) < LINE_LENGTH - 1) {
        int len = strlen(screen_buffer[current_line]);
        screen_buffer[current_line][len] = c;
        screen_buffer[current_line][len + 1] = '\0';
        cursor_position = len + 1;
        update_screen();  // Update the screen after adding a character
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
            handle_enter();  // Evaluate expression on Enter key
            break;
        default:
            break;
    }
    //update_screen();
}

// Detect mouse click events on buttons
void handle_mouse_click(int x, int y) {
    // Right-side buttons layout
    int right_x = 220;  // X position of the right-side buttons
    int start_y = 500;  // Y position of the bottom-most buttons

    // Check for right-side buttons ("/", "*", "-", "+", "Enter")
    if (x >= right_x && x <= right_x + BUTTON_WIDTH && y >= start_y - 120 && y <= start_y - 120 + BUTTON_HEIGHT) {
        printf("/ button clicked\n");
        append_to_expression('/');
    } else if (x >= right_x && x <= right_x + BUTTON_WIDTH && y >= start_y - 80 && y <= start_y - 80 + BUTTON_HEIGHT) {
        printf("* button clicked\n");
        append_to_expression('*');
    } else if (x >= right_x && x <= right_x + BUTTON_WIDTH && y >= start_y - 40 && y <= start_y - 40 + BUTTON_HEIGHT) {
        printf("- button clicked\n");
        append_to_expression('-');
    } else if (x >= right_x && x <= right_x + BUTTON_WIDTH && y >= start_y && y <= start_y + BUTTON_HEIGHT) {
        printf("+ button clicked\n");
        append_to_expression('+');

    } else if (x >= right_x && x <= right_x + BUTTON_WIDTH && y >= start_y + 40 && y <= start_y + 40 + BUTTON_HEIGHT * 2) {
        printf("Enter button clicked\n");
        handle_enter();  // Handle enter key (evaluate expression)
    }

    // Number buttons
    int start_x = 70;  // X position of the number buttons
    if (x >= start_x && x <= start_x + BUTTON_WIDTH && y >= start_y + 40 && y <= start_y + 40 + BUTTON_HEIGHT) {
        printf("0 button clicked\n");
        append_to_expression('0');
    } else if (x >= start_x + 50 && x <= start_x + 50 + BUTTON_WIDTH && y >= start_y + 40 && y <= start_y + 40 + BUTTON_HEIGHT) {
        printf(". button clicked\n");
        append_to_expression('.');
    } else if (x >= start_x + 100 && x <= start_x + 100 + BUTTON_WIDTH && y >= start_y + 40 && y <= start_y + 40 + BUTTON_HEIGHT) {
        printf("(-) button clicked\n");
        append_to_expression('-');  // Could be treated as a negation symbol

    } else if (x >= start_x && x <= start_x + BUTTON_WIDTH && y >= start_y && y <= start_y + BUTTON_HEIGHT) {
        printf("1 button clicked\n");
        append_to_expression('1');
    } else if (x >= start_x + 50 && x <= start_x + 50 + BUTTON_WIDTH && y >= start_y && y <= start_y + BUTTON_HEIGHT) {
        printf("2 button clicked\n");
        append_to_expression('2');
    } else if (x >= start_x + 100 && x <= start_x + 100 + BUTTON_WIDTH && y >= start_y && y <= start_y + BUTTON_HEIGHT) {
        printf("3 button clicked\n");
        append_to_expression('3');
    }

    // Continue for other buttons (4, 5, 6, 7, 8, 9, etc.)
    start_y -= 40;  // Adjust Y for the next row of number buttons
    if (x >= start_x && x <= start_x + BUTTON_WIDTH && y >= start_y && y <= start_y + BUTTON_HEIGHT) {
        printf("4 button clicked\n");
        append_to_expression('4');
    } else if (x >= start_x + 50 && x <= start_x + 50 + BUTTON_WIDTH && y >= start_y && y <= start_y + BUTTON_HEIGHT) {
        printf("5 button clicked\n");
        append_to_expression('5');
    } else if (x >= start_x + 100 && x <= start_x + 100 + BUTTON_WIDTH && y >= start_y && y <= start_y + BUTTON_HEIGHT) {
        printf("6 button clicked\n");
        append_to_expression('6');
    }

    start_y -= 40;  // Adjust for next row
    if (x >= start_x && x <= start_x + BUTTON_WIDTH && y >= start_y && y <= start_y + BUTTON_HEIGHT) {
        printf("7 button clicked\n");
        append_to_expression('7');
    } else if (x >= start_x + 50 && x <= start_x + 50 + BUTTON_WIDTH && y >= start_y && y <= start_y + BUTTON_HEIGHT) {
        printf("8 button clicked\n");
        append_to_expression('8');
    } else if (x >= start_x + 100 && x <= start_x + 100 + BUTTON_WIDTH && y >= start_y && y <= start_y + BUTTON_HEIGHT) {
        printf("9 button clicked\n");
        append_to_expression('9');
    }

    // Additional buttons: "(", ")", ","
    if (x >= start_x + 50 && x <= start_x + 50 + BUTTON_WIDTH && y >= start_y - 40 && y <= start_y - 40 + BUTTON_HEIGHT) {
        printf("( button clicked\n");
        append_to_expression('(');
    } else if (x >= start_x + 100 && x <= start_x + 100 + BUTTON_WIDTH && y >= start_y - 40 && y <= start_y - 40 + BUTTON_HEIGHT) {
        printf(") button clicked\n");
        append_to_expression(')');
    } else if (x >= start_x && x <= start_x + BUTTON_WIDTH && y >= start_y - 40 && y <= start_y - 40 + BUTTON_HEIGHT) {
        printf(", button clicked\n");
        append_to_expression(',');
    }

    // New row of buttons below the screen (Y=, WINDOW, ZOOM, TRACE, GRAPH)
    int new_row_start_x = 70;
    int new_row_start_y = 120;
    int small_button_height = BUTTON_HEIGHT / 2;

    if (x >= new_row_start_x && x <= new_row_start_x + BUTTON_WIDTH * 1.5 && y >= new_row_start_y && y <= new_row_start_y + small_button_height) {
        printf("Y= button clicked\n");
    } else if (x >= new_row_start_x + 90 && x <= new_row_start_x + 90 + BUTTON_WIDTH * 1.5 && y >= new_row_start_y && y <= new_row_start_y + small_button_height) {
        printf("WINDOW button clicked\n");
    } else if (x >= new_row_start_x + 180 && x <= new_row_start_x + 180 + BUTTON_WIDTH * 1.5 && y >= new_row_start_y && y <= new_row_start_y + small_button_height) {
        printf("ZOOM button clicked\n");
    } else if (x >= new_row_start_x + 270 && x <= new_row_start_x + 270 + BUTTON_WIDTH * 1.5 && y >= new_row_start_y && y <= new_row_start_y + small_button_height) {
        printf("TRACE button clicked\n");
    } else if (x >= new_row_start_x + 360 && x <= new_row_start_x + 360 + BUTTON_WIDTH * 1.5 && y >= new_row_start_y && y <= new_row_start_y + small_button_height) {
        printf("GRAPH button clicked\n");
    }

    // Add logic for other buttons in the new layout (log, ln, q, etc.)
    
    //update_screen();
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

void handle_enter() {
    printf("Enter button clicked. Expression: %s\n", expression);  // Log the expression

    // Pass the expression to the math engine
    double result = evaluate_expression(screen_buffer[current_line]);

    printf("Result of expression: %.2f\n", result);

    // Display the result right-aligned on the next line
    current_line = (current_line + 1) % MAX_LINES;
    total_lines = total_lines < MAX_LINES ? total_lines + 1 : MAX_LINES;
    snprintf(screen_buffer[current_line], LINE_LENGTH, "%10.2f", result);

    // Move to the next input line (empty line)
    current_line = (current_line + 1) % MAX_LINES;
    total_lines = total_lines < MAX_LINES ? total_lines + 1 : MAX_LINES;
    screen_buffer[current_line][0] = '\0';  // Clear the new line
    cursor_position = 0;  // Reset cursor position

    update_screen();  // Render everything    
}
