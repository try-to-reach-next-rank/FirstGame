#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <bits/stdc++.h>

using namespace std;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const char* SCREEN_TITLE = "CeadHup";

enum GameState { MENU, PLAYING, GUIDE, EXIT };

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* backgroundTexture = nullptr;
TTF_Font* font = nullptr;
Mix_Music* bgm = nullptr;
GameState gameState = MENU;
int selectedOption = 0;


SDL_Texture* loadTexture(const char* path) {
    SDL_Texture* texture = nullptr;
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cerr << "Failed to load image: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return false;
    if (TTF_Init() == -1) return false;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return false;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return false;

    window = SDL_CreateWindow(SCREEN_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;

    font = TTF_OpenFont("AlegreyaSansSC-Light.ttf", 64);
    if (!font) return false;

    bgm = Mix_LoadMUS("01 The Delicious Last Course.mp3");
    if (!bgm) return false;
    Mix_PlayMusic(bgm, -1);

    backgroundTexture = loadTexture("DALL·E-2025-02-25-16.05.png");
    if (!backgroundTexture) return false;

    return true;
}

// Hàm hiển thị văn bản
void renderText(const char* text, int x, int y, bool isSelected) {
    SDL_Color color = isSelected ? SDL_Color{255, 255, 0, 255} : SDL_Color{255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderMenu() {
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    const char* menuItems[] = { "Play", "Guide", "Exit" };
    int centerX = SCREEN_WIDTH / 2 - 50;
    int startY = SCREEN_HEIGHT / 2 - 50 ;

    for (int i = 0; i < 3; ++i) {
        renderText(menuItems[i], centerX, startY + i * 50, selectedOption == i);
    }

    SDL_RenderPresent(renderer);
}

void handleMenuEvents(bool& running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { running = false; return; }
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_UP) selectedOption = (selectedOption > 0) ? selectedOption - 1 : 2;
            if (e.key.keysym.sym == SDLK_DOWN) selectedOption = (selectedOption < 2) ? selectedOption + 1 : 0;
            if (e.key.keysym.sym == SDLK_RETURN) {
                if (selectedOption == 0) gameState = PLAYING;
                if (selectedOption == 1) std::cout << "Show Guide\n";
                if (selectedOption == 2) running = false;
            }
        }
    }
}

void renderGame() {
    SDL_SetRenderDrawColor(renderer, 50, 50, 255, 255);
    SDL_RenderClear(renderer);
    renderText("Game is running... Press ESC to return", SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2, false);
    SDL_RenderPresent(renderer);
}

void handleGameEvents(bool& running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { running = false; return; }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            gameState = MENU;
        }
    }
}

// Hàm chính
int main(int argc, char* argv[]) {
    if (!initSDL()) {
        std::cerr << "Failed to initialize SDL\n";
        return -1;
    }

    bool running = true;
    while (running) {
        if (gameState == MENU) {
            handleMenuEvents(running);
            renderMenu();
        } else if (gameState == PLAYING) {
            handleGameEvents(running);
            renderGame();
        }
        SDL_Delay(16);
    }

    SDL_DestroyTexture(backgroundTexture);
    Mix_FreeMusic(bgm);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
