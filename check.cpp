#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
Mix_Chunk* hoverSound = nullptr;
Mix_Chunk* clickSound = nullptr;
Mix_Music* backgroundMusic = nullptr;

enum GameState { MENU, PLAYING, SHOWGUIDANCE };
GameState gameState = MENU;

const char* menuOptions[] = {"Play", "Guide", "Exit"};
int selectedOption = 0;
bool mouseDown = false;
int lastHovered = -1;

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return false;
    if (TTF_Init() < 0) return false;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return false;

    window = SDL_CreateWindow("Menu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;

    font = TTF_OpenFont("assets/font.ttf", 48);
    if (!font) return false;

    hoverSound = Mix_LoadWAV("assets/hover.wav");
    clickSound = Mix_LoadWAV("assets/click.wav");
    backgroundMusic = Mix_LoadMUS("assets/menu_music.mp3");

    if (backgroundMusic) Mix_PlayMusic(backgroundMusic, -1);

    return true;
}

void cleanup() {
    Mix_FreeChunk(hoverSound);
    Mix_FreeChunk(clickSound);
    Mix_FreeMusic(backgroundMusic);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

void handleMenuEvents(bool& running) {
    SDL_Event e;
    int centerX = SCREEN_WIDTH / 2 - 100;
    int startY = SCREEN_HEIGHT / 2 - 60;
    int itemHeight = 80;
    int itemWidth = 200;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
            return;
        }

        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_UP) selectedOption = (selectedOption + 2) % 3;
            if (e.key.keysym.sym == SDLK_DOWN) selectedOption = (selectedOption + 1) % 3;
            if (e.key.keysym.sym == SDLK_RETURN) {
                Mix_PlayChannel(-1, clickSound, 0);
                if (selectedOption == 0) gameState = PLAYING;
                else if (selectedOption == 1) gameState = SHOWGUIDANCE;
                else if (selectedOption == 2) running = false;
            }
        }

        if (e.type == SDL_MOUSEMOTION) {
            int mouseX = e.motion.x;
            int mouseY = e.motion.y;
            // Tạo biến cục bộ mousePoint thay vì dùng rvalue
            SDL_Point mousePoint = { mouseX, mouseY };
            for (int i = 0; i < 3; ++i) {
                SDL_Rect rect = { centerX, startY + i * itemHeight, itemWidth, itemHeight };
                if (SDL_PointInRect(&mousePoint, &rect)) {
                    if (lastHovered != i) {
                        selectedOption = i;
                        lastHovered = i;
                        Mix_PlayChannel(-1, hoverSound, 0);
                    }
                }
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            mouseDown = true;
        }

        if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
            mouseDown = false;
            int mouseX = e.button.x;
            int mouseY = e.button.y;
            // Tạo biến cục bộ mousePoint thay vì dùng rvalue
            SDL_Point mousePoint = { mouseX, mouseY };
            for (int i = 0; i < 3; ++i) {
                SDL_Rect rect = { centerX, startY + i * itemHeight, itemWidth, itemHeight };
                if (SDL_PointInRect(&mousePoint, &rect)) {
                    selectedOption = i;
                    Mix_PlayChannel(-1, clickSound, 0);
                    if (selectedOption == 0) gameState = PLAYING;
                    else if (selectedOption == 1) gameState = SHOWGUIDANCE;
                    else if (selectedOption == 2) running = false;
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (!init()) {
        std::cerr << "Initialization failed\n";
        return -1;
    }

    bool running = true;
    while (running) {
        if (gameState == MENU) {
            handleMenuEvents(running);
            // Ở đây bạn có thể gọi renderMenu() nếu muốn hiển thị menu
        }
        // Thêm code xử lý PLAYING, SHOWGUIDANCE...
    }

    cleanup();
    return 0;
}
