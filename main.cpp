#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <bits/stdc++.h>

using namespace std;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const char* SCREEN_TITLE = "CeadHup";

enum GameState { MENU, DIFFICULTY, PLAYING, SHOWGUIDANCE, EXIT, PAUSED , GAMEOVER};
GameState gameState = MENU;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* menuBG = nullptr;
SDL_Texture* gameBG = nullptr;
SDL_Texture* guideBG[2] = { nullptr, nullptr };
SDL_Texture* dificulBG[4] = {nullptr, nullptr, nullptr, nullptr};
SDL_Texture* sound[2] = {nullptr, nullptr};
SDL_Texture* pausegame[2] = {nullptr, nullptr};
TTF_Font* font = nullptr;
Mix_Music* bgm = nullptr;
Mix_Music* gamesound = nullptr;
Mix_Music* othersound = nullptr;
Mix_Music* winsound  = nullptr;
Mix_Chunk* hoverSound = nullptr;
Mix_Chunk* clickSound = nullptr;
Mix_Chunk* shootSound = nullptr;
Mix_Chunk* specialSound = nullptr;
Mix_Chunk* jumpSound = nullptr;
Mix_Chunk* dirtSound = nullptr;
Mix_Chunk* wormSound = nullptr;
Mix_Chunk* carrotSound = nullptr;
Mix_Chunk* hitSound = nullptr;
Mix_Chunk* hittedSound = nullptr;
Mix_Chunk* knockout = nullptr;

int DifFrameCount = 0;
int selectedOption = 0; // Dùng cho menu và cho difficulty
bool mouseDown = false;
int lastHovered = -1;
int difficulty = 0;    // 0: Easy, 1: Hard, 2: Hell
int pausedSelectedOption = 0; // 0: Resume, 1: Quit to Main Menu
Uint32 lastDifUpdate = 0 ;
bool isvictory;

// Physics constants
const int JUMP_STRENGTH = -700;
const float GRAVITY_UP = 2000.0f;
const float GRAVITY_DOWN = 2500.0f;
const float MAX_FALL_SPEED = 15.0f;
const int BULLET_SPEED = 12;
const int SPECIAL_COOLDOWN = 5000;
const int DASH_DURATION = 150;
const int DASH_SPEED = 25;
const int DASH_COOLDOWN = 500;
const int dashDistance = SCREEN_WIDTH / 7;

// Gameplay constants
const int MAX_HP = 3;
const int MAX_SPECIAL = 5;
const int HEART_SIZE = 30;
const int SPECIAL_ICON_SIZE = 25;

// Boss constants
const int BOSS_WIDTH = 300;
const int BOSS_HEIGHT = SCREEN_HEIGHT;
int BOSS_PHASE1_HP = 0;
int BOSS_PHASE2_HP = 500;
int BOSS_PHASE3_HP = 1000;

// Global systems
int currentHP = MAX_HP;
int specialCharges = 0;
static Uint32 lastSpecialGain = 0;
bool gameOver = false;
int dashTargetX;
static Uint32 lastDashTime = 0;
const int SPECIAL_FIRE_RATE = 500;
static Uint32 lastSpecialFireTime = 0;
const int FIRE_RATE = 200;
static Uint32 lastFireTime = 0;
bool lastPushRight;
bool pref_isRunning;
bool pref_isShooting;
bool pref_isSpawning;
int damage = 100;


const int SLIDER_X = 40;
const int SLIDER_Y = 20;
const int SLIDER_WIDTH = 100;
const int SLIDER_HEIGHT = 10;
const int SLIDER_HANDLE_SIZE = 20;

void fadeTransition(GameState newState);

SDL_Rect volumeIconRect;
SDL_Rect pauseIconRect;

struct Player {
    int x, y, w, h;
    float velY;
    bool isJumping;
    bool isFalling;
    bool facingRight;
    bool isDashing;
    bool isCrouching;
    bool isStanding;
    bool isRunning;
    bool isShooting;
    Uint32 dashStartTime;
    int baseSpeed;
    SDL_Rect hitbox;
};

struct Bullet {
    int x, y, w, h;
    int dx, dy;
    bool active;
    bool isSpecial;
    SDL_Rect hitbox;
    SDL_Texture* phase1Texture;
    int phase1FrameWidth;
    int phase1FrameHeight;
    int phase1FrameCount;
    int phase1CurrentFrame;
    Uint32 phase1StartTime;
    Uint32 phase1FrameDuration;
    SDL_Texture* phase2Texture;
    int phase2FrameWidth;
    int phase2FrameHeight;
    int phase2FrameCount;
    int phase2CurrentFrame;
    Uint32 phase2StartTime;
    Uint32 phase2FrameDuration;
    int phase;
    SDL_Texture* carrotTexture;
    int carrotFrameWidth;
    int carrotFrameHeight;
    int carrotFrameCount;
    int carrotCurrentFrame;
    Uint32 carrotStartTime;
    Uint32 carrotFrameDuration;
    SDL_Texture* playerBulletTexture;
    int playerBulletFrameWidth;
    int playerBulletFrameHeight;
    int playerBulletFrameCount;
    int playerBulletCurrentFrame;
    Uint32 playerBulletStartTime;
    Uint32 playerBulletFrameDuration;
    bool is_right;
};

struct Meteor {
    int x, y, w, h;
    int dx, dy;
    bool active;
    SDL_Rect hitbox;
    SDL_Texture* meteorTexture;
    int currentFrame;
    Uint32 MeteorStartTime;
};

struct Boss {
    int x, y, w, h;
    int health;
    Uint32 lastGroundAttack;
    Uint32 lastHomingAttack;
    Uint32 lastMeteorAttack;
    bool canAttack;
    SDL_Rect hitbox;
    SDL_Texture* phase3Texture;
    int phase3FrameWidth;
    int phase3FrameHeight;
    int phase3FrameCount;
    int phase3CurrentFrame;
    Uint32 phase3StartTime;
    Uint32 phase3FrameDuration;
    bool isPhase3;
    SDL_Texture* phase12Texture;
    int phase12FrameWidth;
    int phase12FrameHeight;
    int phase12FrameCount;
    int phase12CurrentFrame;
    Uint32 phase12StartTime;
    Uint32 phase12FrameDuration;
    SDL_Texture* runningTexture;
    int runningFrameWidth;
    int runningFrameHeight;
    int runningFrameCount;
    int runningCurrentFrame;
    Uint32 runningStartTime;
    Uint32 runningFrameDuration;
    bool isRunning;
};

struct VolumeSlider {
    SDL_Rect rect;
    SDL_Rect handleRect;
    int minValue;
    int maxValue;
    int currentValue;
    bool isDragging;
};

VolumeSlider volumeSlider;

vector<Bullet> bullets;
vector<Meteor> meteors;
vector<Bullet> bossBullets;
Player player;
Boss boss;
Uint32 lastMeteorSpawn = 0;
bool movingToStart = true;
float logicX = 0;
int targetX = SCREEN_WIDTH / 3;

// Animation variables
SDL_Texture* playerRunTexture;
int playerRunCurrentFrame = 0;
Uint32 playerRunStartTime = 0;
bool runFacingRight = true;

SDL_Texture* playerShootTexture;
int playerShootCurrentFrame = 0;
Uint32 playerShootStartTime = 0;

SDL_Texture* playerJumpTexture;
int playerJumpCurrentFrame = 0;
Uint32 playerJumpStartTime = 0;


SDL_Texture* playerDashTexture;
int playerDashCurrentFrame = 0;
Uint32 playerDashStartTime = 0;

SDL_Texture* playerIdleTexture;
int playerIdleCurrentFrame = 0;
Uint32 playerIdleStartTime = 0;

SDL_Texture* playerRunShootTexture;
int playerRunShootCurrentFrame = 0;
Uint32 playerRunShootStartTime = 0;

SDL_Texture* hpTexture;
SDL_Texture* specialTexture;


void spawnBoss(SDL_Renderer* renderer) {
    boss = {
        SCREEN_WIDTH - BOSS_WIDTH - 50,
        SCREEN_HEIGHT - BOSS_HEIGHT + BOSS_HEIGHT / 2,
        BOSS_WIDTH / 2,
        BOSS_HEIGHT / 2,
        BOSS_PHASE1_HP,
        0,
        0,
        0,
        false,
        { SCREEN_WIDTH - BOSS_WIDTH - 50, SCREEN_HEIGHT - BOSS_HEIGHT + BOSS_HEIGHT / 2, BOSS_WIDTH / 2, BOSS_HEIGHT / 2 },
        nullptr,
        102,
        111,
        22,
        0,
        0,
        100,
        false,
        nullptr,
        122,
        120,
        17,
        0,
        0,
        100,
        nullptr,
        2291 / 20,
        108,
        20,
        0,
        0,
        150,
        false
    };

    boss.phase3Texture = IMG_LoadTexture(renderer, "your_phase3_image.png");
    boss.phase12Texture = IMG_LoadTexture(renderer, "your_phase12_image.png");
    boss.runningTexture = IMG_LoadTexture(renderer, "boss_post_move_image.png");
}

void spawnPlayer() {
    player.x = 0;
    player.y = 650;
    player.w = 50;
    player.h = 75;
    player.velY = 0.0f;
    player.isJumping = false;
    player.isFalling = false;
    player.facingRight = true;
    player.isDashing = false;
    player.isCrouching = false;
    player.isStanding = false;
    player.isShooting = false;
    player.isRunning = false;
    player.dashStartTime = 0;
    player.baseSpeed = 5;
    player.hitbox = { player.x, player.y, player.w, player.h };
    logicX = player.hitbox.x;
}

void moveToTargetX() {
    const float speed = 2.5f;
    if (abs(logicX - targetX) > speed) {
        logicX += (logicX < targetX) ? speed : -speed;
        player.x = static_cast<int>(logicX);
        player.hitbox.x = player.x;

    } else {
        movingToStart = false;
        player.x = targetX;
        player.hitbox.x = player.x;
        boss.canAttack = true;
    }

    if (player.isRunning&& !player.isShooting) {
        Uint32 currentTime = SDL_GetTicks();
        if ((currentTime - playerRunStartTime > 50 || pref_isRunning == 1) && gameState == PLAYING) {
            playerRunCurrentFrame = (playerRunCurrentFrame + 1) % 16;
            playerRunStartTime = currentTime;
        }
    }else{
        playerRunCurrentFrame = 0;
    }
    if (boss.isRunning) {
            Uint32 currentTime = SDL_GetTicks();
            Uint32 frameTime = currentTime - boss.runningStartTime;
            if (frameTime > boss.runningFrameDuration && gameState == PLAYING) {
                boss.runningCurrentFrame = (boss.runningCurrentFrame + 1) % boss.runningFrameCount;
                boss.runningStartTime = currentTime;
            }
        }
}


void dashToTargetX(float& logicX, int& playerX, int targetX) {
    const float speed = 25.0f;
    if (abs(logicX - targetX) > speed) {
        logicX += (logicX < targetX) ? speed : -speed;
        playerX = static_cast<int>(logicX);
    } else {
        playerX = targetX;
    }
}

void spawnMeteor(SDL_Renderer * renderer) {
    Meteor m;
    m.w = 50 + rand() % 50;
    m.h = m.w;
    m.x = rand() % (SCREEN_WIDTH - m.w);
    m.y = -m.h;

    int playerCenterX = player.x + player.w / 2;
    int playerCenterY = player.y + player.h / 2;
    int meteorCenterX = m.x + m.w / 2;
    int meteorCenterY = m.y + m.h / 2;

    int diffX = playerCenterX - meteorCenterX;
    int diffY = playerCenterY - meteorCenterY;
    float length = sqrt(diffX * diffX + diffY * diffY);

    float speed = 3 + rand() % 12;
    if (length > 0) {
        m.dx = (diffX / length) * speed;
        m.dy = (diffY / length) * speed;
    } else {
        m.dx = 0;
        m.dy = speed;
    }

    m.active = true;
    m.hitbox = { m.x, m.y, m.w, m.h };
    m.meteorTexture =  IMG_LoadTexture(renderer, "thien_thach.png");;
    m.currentFrame = 0;
    m.MeteorStartTime = SDL_GetTicks();
    meteors.push_back(m);
}

void bossAttack(SDL_Renderer* renderer) {
    Uint32 currentTime = SDL_GetTicks();
    if (boss.canAttack) {
        if (boss.health > BOSS_PHASE3_HP) {
            boss.x = SCREEN_WIDTH / 2 - boss.w / 2;
            boss.y = SCREEN_HEIGHT / 2 - boss.h / 2;

            if (currentTime - boss.lastHomingAttack > 800) {
                Bullet b;
                b.x = boss.x;
                b.y = boss.y + BOSS_HEIGHT / 2;
                b.w = 45;
                b.h = 45;
                b.dx = 0;
                b.dy = 0;
                b.active = true;
                b.isSpecial = true;
                b.hitbox = { b.x, b.y, b.w, b.h };
                b.carrotTexture = IMG_LoadTexture(renderer, "carrot_bullet.png");
                b.carrotFrameWidth = 86;
                b.carrotFrameHeight = 142;
                b.carrotFrameCount = 4;
                b.carrotCurrentFrame = 0;
                b.carrotStartTime = currentTime;
                b.carrotFrameDuration = 100;
                b.phase = 3;
                b.is_right = 1;
                bossBullets.push_back(b);
                Mix_PlayChannel(-1, carrotSound, 0);
                boss.lastHomingAttack = currentTime;
            }
        } else if (boss.health > BOSS_PHASE2_HP) {
            if (currentTime - boss.lastMeteorAttack > 2000) {
                spawnMeteor(renderer);
                boss.lastMeteorAttack = currentTime;
            }

            if (currentTime - boss.lastGroundAttack > 1000) {
                int dx = player.x + player.w / 2 - (boss.x + boss.w / 2);
                int dy = player.y + player.h / 2 - (boss.y + boss.h / 2);
                float length = sqrt(dx * dx + dy * dy);

                if (length > 0) {
                    Bullet b;
                    b.x = boss.x + boss.w / 2;
                    b.y = boss.y + boss.h / 2;
                    b.w = 67;
                    b.h = 67;
                    b.dx = (int)(dx / length * 10);
                    b.dy = (int)(dy / length * 10);
                    b.active = true;
                    b.isSpecial = false;
                    b.hitbox = { b.x, b.y, b.w, b.h };
                    b.phase2Texture = IMG_LoadTexture(renderer, "your_bullet_phase2_image.png");
                    b.phase2FrameWidth = 134;
                    b.phase2FrameHeight = 142;
                    b.phase2FrameCount = 4;
                    b.phase2CurrentFrame = 0;
                    b.phase2StartTime = currentTime;
                    b.phase2FrameDuration = 100;
                    b.phase = 2;
                    b.is_right = 1;
                    bossBullets.push_back(b);
                    Mix_PlayChannel(-1, wormSound, 0);
                }
                boss.lastGroundAttack = currentTime;
            }
        } else {
            if (currentTime - boss.lastGroundAttack > 1900) {
                Bullet b;
                b.x = boss.x - 50;
                b.y = SCREEN_HEIGHT - 75;
                b.w = 90;
                b.h = 90;
                b.dx = -8;
                b.dy = 0;
                b.active = true;
                b.isSpecial = false;
                b.hitbox = { b.x, b.y, b.w, b.h };
                b.phase1Texture = IMG_LoadTexture(renderer, "your_bullet_phase1_image.png");
                b.phase1FrameWidth = 134;
                b.phase1FrameHeight = 137;
                b.phase1FrameCount = 8;
                b.phase1CurrentFrame = 0;
                b.phase1StartTime = currentTime;
                b.phase1FrameDuration = 100;
                b.phase = 1;
                b.is_right = 1;
                bossBullets.push_back(b);
                Mix_PlayChannel(-1, dirtSound, 0);
                boss.lastGroundAttack = currentTime;
            }
        }
    }
}

void updateBossBullets() {
    for (auto& b : bossBullets) {
        if (b.isSpecial) {
            int dx = player.x + player.w / 2 - b.x;
            int dy = player.y + player.h / 2 - b.y;
            float length = sqrt(dx * dx + dy * dy);
            if (length > 0) {
                b.dx = (dx / length )*2;
                b.dy = (dy / length )*2;
            }
        }

        b.x += b.dx;
        b.y += b.dy;
        b.hitbox.x = b.x;
        b.hitbox.y = b.y;

        if (SDL_HasIntersection(&b.hitbox, &player.hitbox)) {
            Mix_PlayChannel(-1, hittedSound, 0);
            currentHP--;
            b.active = false;
        }
    }

    bossBullets.erase(remove_if(bossBullets.begin(), bossBullets.end(), [](const Bullet& b) { return !b.active; }), bossBullets.end());
}

void handleInput(bool& running, Player& player, float deltaTime, SDL_Renderer* renderer) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)running = false;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            if (gameState == PLAYING) {
                gameState = PAUSED;
                Mix_PauseMusic();
            } else if (gameState == PAUSED) {
                gameState = PLAYING;
                Mix_ResumeMusic();
            }
        }
    }

    if (gameOver) return;
    SDL_PumpEvents();
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    pref_isRunning = player.isRunning;
    pref_isShooting = player.isShooting;
    if (keys[SDL_SCANCODE_LEFT] && !movingToStart) player.facingRight = false;
    if (keys[SDL_SCANCODE_RIGHT] && !movingToStart) player.facingRight = true;

    player.isStanding = keys[SDL_SCANCODE_C];
    player.isCrouching = keys[SDL_SCANCODE_DOWN];

    if (player.isCrouching) {
        player.h = 50;
    } else {
        player.h = 75;
    }
    player.isRunning = false;
    boss.isRunning = false;
    if(movingToStart){
        player.isRunning = true;
        boss.isRunning = true;
    }
    if(keys[SDL_SCANCODE_LEFT]){
        runFacingRight = false;
    }
    else if(keys[SDL_SCANCODE_RIGHT]){
        runFacingRight = true;
    }
    if (!player.isStanding && !movingToStart && !player.isDashing) {
        if (keys[SDL_SCANCODE_LEFT]) {
            player.x = max(0, player.x - player.baseSpeed);
            logicX = player.x;
            player.isRunning = true;
            //runFacingRight = false;
            playerRunStartTime = SDL_GetTicks();
        }
        else if (keys[SDL_SCANCODE_RIGHT]) {
            player.x = min(SCREEN_WIDTH - player.w, player.x + player.baseSpeed);
            logicX = player.x;
            player.isRunning = true;
            //runFacingRight = true;
            playerRunStartTime = SDL_GetTicks();
        }else{
            player.isRunning = false;
        }
    }

    if (keys[SDL_SCANCODE_Z]) {
        if (!player.isJumping && !player.isFalling) {
            Mix_PlayChannel(-1, jumpSound, 0);
            player.velY = JUMP_STRENGTH;
            player.isJumping = true;
            player.isFalling = false;
            playerJumpStartTime = SDL_GetTicks();
        }
    }

    if ((keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT]) && !player.isDashing && (SDL_GetTicks() - lastDashTime >= DASH_COOLDOWN)) {
        player.isDashing = true;
        player.dashStartTime = SDL_GetTicks();
    }else{
        player.isDashing = false;
    }


    if (!movingToStart) {
        if (keys[SDL_SCANCODE_X]) {
            if (!player.isJumping && !player.isFalling) {
                int dx = 0, dy = 0;
                bool diagonalShot = false;
                Uint32 currentTime = SDL_GetTicks();
                if (currentTime - lastFireTime >= FIRE_RATE) {
                    if (keys[SDL_SCANCODE_UP]) {
                        dy = -BULLET_SPEED;
                        dx = player.facingRight ? BULLET_SPEED : -BULLET_SPEED;
                        diagonalShot = true;
                    }

                    if (!diagonalShot) dx = player.facingRight ? BULLET_SPEED : -BULLET_SPEED;

                    Bullet b;
                    b.x = player.x + player.w / 2 - 5;
                    b.y = player.y + player.h / 2 - 5;
                    b.w = 22;
                    b.h = 22;
                    b.dx = dx;
                    b.dy = dy;
                    b.active = true;
                    b.isSpecial = false;
                    b.hitbox = { b.x, b.y, b.w, b.h };
                    b.playerBulletTexture = IMG_LoadTexture(renderer, "player_bullet.png");
                    b.playerBulletFrameWidth = 892 / 6;
                    b.playerBulletFrameHeight = 47;
                    b.playerBulletFrameCount = 6;
                    b.playerBulletCurrentFrame = 0;
                    b.playerBulletStartTime = currentTime;
                    b.playerBulletFrameDuration = 100;
                    b.phase = 0;
                    b.is_right = player.facingRight;
                    Mix_PlayChannel(-1, shootSound, 0);
                    bullets.push_back(b);
                    lastFireTime = currentTime;
                }
                player.isShooting = true;
                playerShootStartTime = SDL_GetTicks();
            }
        }else if (keys[SDL_SCANCODE_SPACE] && specialCharges > 0) {
            if (!player.isJumping && !player.isFalling) {
                int dx = 0, dy = 0;
                bool diagonalShot = false;
                Uint32 currentTime = SDL_GetTicks();
                if (currentTime - lastSpecialFireTime >= SPECIAL_FIRE_RATE) {
                    if (keys[SDL_SCANCODE_UP]) {
                        dy = -BULLET_SPEED;
                        dx = player.facingRight ? BULLET_SPEED : -BULLET_SPEED;
                        diagonalShot = true;
                    }

                    if (!diagonalShot) dx = player.facingRight ? BULLET_SPEED : -BULLET_SPEED;

                    Bullet b;
                    b.x = player.x + player.w / 2 - 20;
                    b.y = player.y + player.h / 2 - 20;
                    b.w = 102;
                    b.h = 102;
                    b.dx = dx;
                    b.dy = dy;
                    b.active = true;
                    b.isSpecial = true;
                    b.hitbox = { b.x, b.y, b.w, b.h };
                    b.playerBulletTexture = IMG_LoadTexture(renderer, "special_attack.png");
                    b.playerBulletFrameWidth = 2162 / 8;
                    b.playerBulletFrameHeight = 115;
                    b.playerBulletFrameCount = 8;
                    b.playerBulletCurrentFrame = 0;
                    b.playerBulletStartTime = currentTime;
                    b.playerBulletFrameDuration = 100;
                    b.phase = 0;
                    b.is_right = player.facingRight;
                    bullets.push_back(b);
                    Mix_PlayChannel(-1, specialSound, 0);
                    specialCharges--;
                    lastSpecialFireTime = currentTime;
                }
                player.isShooting = true;
                playerShootStartTime = SDL_GetTicks();
            }
        }else{
            player.isShooting = false;
        }
    }

}

void updateGame(Player& player, float deltaTime, SDL_Renderer* renderer, int damage) {
    if (gameOver) return;

    bossAttack(renderer);
    updateBossBullets();
    if (SDL_GetTicks() - lastSpecialGain > 5000) {
        if (specialCharges < MAX_SPECIAL) {
            specialCharges++;
            lastSpecialGain = SDL_GetTicks();
        }
    }

    if (movingToStart) {
        moveToTargetX();
        return;
    }

    if (player.isJumping || player.isFalling) {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - playerJumpStartTime > 50 && gameState == PLAYING) {
            playerJumpCurrentFrame = (playerJumpCurrentFrame + 1) % 8;
            playerJumpStartTime = currentTime;
        }
        if (player.velY < 0) {
            player.velY += GRAVITY_UP * deltaTime;
        } else {
            player.velY += GRAVITY_DOWN * deltaTime;
            player.isFalling = true;
        }
        player.y += player.velY * deltaTime;
    }

    if (player.y >= 650 ) {
        if(player.isCrouching){
            player.y = 675;
        }else player.y = 650;
        if (player.velY >= 0) {
            player.isJumping = false;
            player.isFalling = false;
        }
    } else if (player.velY > 0 && player.isJumping) {
        player.isJumping = false;
        player.isFalling = true;
    }
    if (player.isDashing) {
        Uint32 dashDuration = SDL_GetTicks() - player.dashStartTime;
        if (dashDuration < DASH_DURATION) {
            player.x += player.facingRight ? DASH_SPEED : -DASH_SPEED;
        } else {
            player.isDashing = false;
            lastDashTime = SDL_GetTicks();
        }
        Uint32 currentTime = SDL_GetTicks();

        if (currentTime - player.dashStartTime > 50 && gameState == PLAYING) {
            playerDashCurrentFrame = (playerDashCurrentFrame + 1) % 8;
            player.dashStartTime = currentTime;
        }
        Uint32 dashDurationTime = SDL_GetTicks() - player.dashStartTime;
        if (dashDurationTime >= DASH_DURATION) {
            player.isDashing = false;
            lastDashTime = SDL_GetTicks();
            playerDashCurrentFrame = 0;
        }
    }
        cerr<<logicX << " "<<player.velY<<endl;
    for (auto& bullet : bullets) {
        bullet.x += bullet.dx;
        bullet.y += bullet.dy;
        bullet.hitbox.x = bullet.x;
        bullet.hitbox.y = bullet.y;
        if (bullet.x < 0 || bullet.x > SCREEN_WIDTH + 100){
            bullet.active = false;
            bullet.x = 0;
        }
    }
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return !b.active; }), bullets.end());

    if (boss.health >= BOSS_PHASE2_HP && SDL_GetTicks() - lastMeteorSpawn > 700) {
        spawnMeteor(renderer);
        lastMeteorSpawn = SDL_GetTicks();
    }
    for (auto& m : meteors) {
        if (m.active) {
            m.x += m.dx;
            m.y += m.dy;
            m.hitbox.x = m.x;
            m.hitbox.y = m.y;
            if (m.y > SCREEN_HEIGHT + 100) m.active = false;
        }
    }
    meteors.erase(remove_if(meteors.begin(), meteors.end(), [](const Meteor& m) { return !m.active; }), meteors.end());

    player.hitbox.x = player.x;
    player.hitbox.y = player.y;
    boss.hitbox.x = boss.x;
    boss.hitbox.y = boss.y;

    for (auto& m : meteors) {
        if (m.active && SDL_HasIntersection(&player.hitbox, &m.hitbox)) {
            currentHP--;
            m.active = false;
            break;
            Mix_PlayChannel(-1, hittedSound, 0);
        }
    }

    for (auto& b : bullets) {
        if (SDL_HasIntersection(&b.hitbox, &boss.hitbox)) {
            boss.health += b.isSpecial ? 5*damage : damage;
            b.active = false;
            Mix_PlayChannel(-1, hitSound, 0);
        }
    }

    if (boss.health >= BOSS_PHASE3_HP && !boss.isPhase3) {
        boss.isPhase3 = true;
        boss.phase3StartTime = SDL_GetTicks();
    }

    if (player.isRunning&& !player.isShooting) {
        Uint32 currentTime = SDL_GetTicks();
        if ((currentTime - playerRunStartTime > 50 || pref_isRunning == 1) && gameState == PLAYING) {
            playerRunCurrentFrame = (playerRunCurrentFrame + 1) % 16;
            playerRunStartTime = currentTime;
        }
    }else{
        playerRunCurrentFrame = 0;
    }

    if (player.isShooting && !player.isRunning) {
        Uint32 currentTime = SDL_GetTicks();
        if ((currentTime - playerShootStartTime > 33 || pref_isShooting == 1) && gameState == PLAYING) {
            playerShootCurrentFrame = (playerShootCurrentFrame + 1) % 6;
            playerShootStartTime = currentTime;
        }
        //if (playerShootCurrentFrame >= 5) player.isShooting = false;
    }else{
        playerShootCurrentFrame = 0;
    }
    if(player.isRunning && player.isShooting ){
        Uint32 currentTime = SDL_GetTicks();
        if ((currentTime - playerRunShootStartTime > 25 || (pref_isRunning == 1 && pref_isShooting == 1)) && gameState == PLAYING) {
            playerRunShootCurrentFrame = (playerRunShootCurrentFrame + 1) % 16;
            playerRunShootStartTime = currentTime;
        }
    }else{
       playerRunShootCurrentFrame = 0;
    }

    if (!player.isDashing && !player.isRunning && !player.isJumping && !player.isFalling && !player.isShooting){
        player.isStanding = true;
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - playerIdleStartTime > 100 && gameState == PLAYING) {
            playerIdleCurrentFrame = (playerIdleCurrentFrame + 1) % 5;
            playerIdleStartTime = currentTime;
        }
    } else {
        player.isStanding = false;
        playerIdleCurrentFrame = 0;
    }
    if(currentHP  <= 0){
        //gameOver = true;
       // fadeTransition(GAMEOVER);
      //  isvictory = false;
    }if(difficulty == 0){
        if(boss.health >= BOSS_PHASE3_HP){
            gameOver = true;
            fadeTransition(GAMEOVER);
            isvictory = true;
        }
    }
}

// -------------------- KHỞI TẠO --------------------
bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        cerr << "SDL_Init failed: " << SDL_GetError() << endl;
        return false;
    }
    if (TTF_Init() == -1) {
        cerr << "TTF_Init failed: " << TTF_GetError() << endl;
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cerr << "Mix_OpenAudio failed: " << Mix_GetError() << endl;
        return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        cerr << "IMG_Init failed: " << IMG_GetError() << endl;
        return false;
    }

    window = SDL_CreateWindow(SCREEN_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    font = TTF_OpenFont("AlegreyaSansSC-ExtraBold.ttf", 32);

    shootSound = Mix_LoadWAV("shoot.wav");
    jumpSound = Mix_LoadWAV("jump.wav");
    hoverSound = Mix_LoadWAV("hover.wav");
    clickSound = Mix_LoadWAV("click.wav");
    specialSound = Mix_LoadWAV("specialSound.wav");
    hitSound = Mix_LoadWAV("player_shoot_hit_07.wav");
    hittedSound = Mix_LoadWAV("player_death_01.wav");
    wormSound = Mix_LoadWAV("veggies_Potato_Spit_Worm_02.wav");
    dirtSound = Mix_LoadWAV("veggies_Potato_Spit_01.wav");
    carrotSound = Mix_LoadWAV("veggies_Carrot_MindMeld_Start.wav");
    knockout = Mix_LoadWAV("cuphead-knockout.wav");

    bgm = Mix_LoadMUS("01 The Delicious Last Course.mp3");
    gamesound = Mix_LoadMUS("gameplay.mp3");
    winsound = Mix_LoadMUS("victory.mp3");
    othersound = Mix_LoadMUS("videoplayback.mp3");

    Mix_PlayMusic(bgm, -1);

    menuBG = IMG_LoadTexture(renderer,"Menu_Background.png");
    gameBG = IMG_LoadTexture(renderer,"GameBackground.png");
    guideBG[0] = IMG_LoadTexture(renderer,"GuidanceBackGround1.png");
    guideBG[1] = IMG_LoadTexture(renderer,"GuidanceBackGround2.png");
    dificulBG[0] = IMG_LoadTexture(renderer,"choose1.png");
    dificulBG[1] = IMG_LoadTexture(renderer,"choose2.png");
    dificulBG[2] = IMG_LoadTexture(renderer,"choose3.png");
    dificulBG[3] = IMG_LoadTexture(renderer, "choose2.png");
    playerRunTexture = IMG_LoadTexture(renderer, "player_run_animation.png");
    playerShootTexture = IMG_LoadTexture(renderer, "stand_shoot.png");
    playerJumpTexture = IMG_LoadTexture(renderer, "player_jump_animation.png");
    playerDashTexture = IMG_LoadTexture(renderer, "player_dash.png");
    playerIdleTexture = IMG_LoadTexture(renderer, "player_idle.png");
    playerRunShootTexture = IMG_LoadTexture(renderer, "player_shoot_animation.png");
    hpTexture = IMG_LoadTexture(renderer, "heart.png");
    specialTexture = IMG_LoadTexture(renderer, "speialAttack.png");
    sound[0] = IMG_LoadTexture(renderer, "sound.png");
    sound[1] = IMG_LoadTexture(renderer, "mute.png");
    pausegame[0] = IMG_LoadTexture(renderer, "resume.png");
    pausegame[1] = IMG_LoadTexture(renderer, "pause.png");


    volumeSlider.rect = { SLIDER_X, SLIDER_Y, SLIDER_WIDTH, SLIDER_HEIGHT };
    volumeSlider.handleRect = { SLIDER_X + SLIDER_WIDTH - SLIDER_HANDLE_SIZE, SLIDER_Y - (SLIDER_HANDLE_SIZE - SLIDER_HEIGHT) / 2, SLIDER_HANDLE_SIZE, SLIDER_HANDLE_SIZE };
    volumeSlider.minValue = 0;
    volumeSlider.maxValue = MIX_MAX_VOLUME;
    volumeSlider.currentValue = MIX_MAX_VOLUME; // Get current volume or set to max
    volumeSlider.isDragging = false;

    volumeIconRect = { SLIDER_X - 40, SLIDER_Y - 20,  30, 30};
    pauseIconRect = {1200, SLIDER_Y - 20, 50, 50};

    return true;
}

void cleanup() {
    SDL_DestroyTexture(menuBG);
    SDL_DestroyTexture(gameBG);
    SDL_DestroyTexture(playerRunTexture);
    SDL_DestroyTexture(playerShootTexture);
    SDL_DestroyTexture(playerJumpTexture);
    SDL_DestroyTexture(playerDashTexture);
    SDL_DestroyTexture(playerIdleTexture);
    SDL_DestroyTexture(playerRunShootTexture);
    SDL_DestroyTexture(hpTexture);
    SDL_DestroyTexture(specialTexture);
    for (int i = 0; i < 2; i++) {
        SDL_DestroyTexture(guideBG[i]);
    }
    for(int i = 0; i< 4 ; ++i){
        SDL_DestroyTexture(dificulBG[i]);
    }
    Mix_FreeMusic(bgm);
    Mix_FreeMusic(othersound);
    Mix_FreeMusic(gamesound);
    Mix_FreeMusic(winsound);
    Mix_FreeChunk(hoverSound);
    Mix_FreeChunk(clickSound);
    Mix_FreeChunk(shootSound);
    Mix_FreeChunk(jumpSound);
    Mix_FreeChunk(knockout);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// -------------------- HIỆU ỨNG FADE --------------------
// Sử dụng step nhỏ hơn và delay nhỏ để chuyển cảnh mượt hơn
void fadeOut(int step = 1) {
    for (int alpha = 0; alpha <= 255; alpha += step) {
        switch (gameState) {
            case MENU:
                if (menuBG)
                    SDL_RenderCopy(renderer, menuBG, NULL, NULL);
                else {
                    SDL_SetRenderDrawColor(renderer, 30, 30, 70, 255);
                    SDL_RenderClear(renderer);
                }
                break;
            case DIFFICULTY:
                // RenderDifficulty sẽ gọi sau
                break;
            case PLAYING:
                if (gameBG)
                    SDL_RenderCopy(renderer, gameBG, NULL, NULL);
                else {
                    SDL_SetRenderDrawColor(renderer, 150, 20, 20, 255);
                    SDL_RenderClear(renderer);
                }
                break;
            case SHOWGUIDANCE:
                if (guideBG[0])
                    SDL_RenderCopy(renderer, guideBG[0], NULL, NULL);
                else {
                    SDL_SetRenderDrawColor(renderer, 0, 100, 150, 255);
                    SDL_RenderClear(renderer);
                }
                break;
            default: break;
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
        SDL_RenderFillRect(renderer, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(2);
    }
}

void fadeIn(int step = 1) {
    for (int alpha = 255; alpha >= 0; alpha -= step) {
        switch (gameState) {
            case MENU:
                if (menuBG)
                    SDL_RenderCopy(renderer, menuBG, NULL, NULL);
                else {
                    SDL_SetRenderDrawColor(renderer, 30, 30, 70, 255);
                    SDL_RenderClear(renderer);
                }
                break;
            case DIFFICULTY:
                // RenderDifficulty sẽ gọi sau
                break;
            case PLAYING:
                if (gameBG)
                    SDL_RenderCopy(renderer, gameBG, NULL, NULL);
                else {
                    SDL_SetRenderDrawColor(renderer, 150, 20, 20, 255);
                    SDL_RenderClear(renderer);
                }
                break;
            case SHOWGUIDANCE:
                if (guideBG[0])
                    SDL_RenderCopy(renderer, guideBG[0], NULL, NULL);
                else {
                    SDL_SetRenderDrawColor(renderer, 0, 100, 150, 255);
                    SDL_RenderClear(renderer);
                }
                break;
            default: break;
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
        SDL_RenderFillRect(renderer, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(2);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void fadeTransition(GameState newState) {
    fadeOut();
    gameState = newState;
    Mix_HaltMusic();

    switch (gameState) {
        case PLAYING:
            Mix_PlayMusic(gamesound, -1);
            break;
        case MENU:
            Mix_PlayMusic(bgm, -1);
            break;
        case GAMEOVER:
            if(isvictory) Mix_PlayMusic(winsound,-1);
            else    Mix_PlayChannel(0,knockout, -1);
            break;
        default:
            Mix_PlayMusic(othersound, -1);
            break;
    }

    fadeIn();
}


void renderText(const char* text, int x, int y, bool highlight) {
    SDL_Color color = highlight ? SDL_Color{255, 255, 0, 255} : SDL_Color{255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderGameOver(bool &running) {
    SDL_Color text_color = { 255, 0, 0, 255 };
    SDL_Surface* knocked_out_surface;
    if(isvictory) knocked_out_surface = TTF_RenderText_Solid(font, "Knock out", text_color);
    else knocked_out_surface = TTF_RenderText_Solid(font, "Knocked out", text_color);
    SDL_Texture* knocked_out_texture = SDL_CreateTextureFromSurface(renderer, knocked_out_surface);
    SDL_Rect knocked_out_rect;
    knocked_out_rect.w = knocked_out_surface->w;
    knocked_out_rect.h = knocked_out_surface->h;
    knocked_out_rect.x = (SCREEN_WIDTH - knocked_out_rect.w) / 2;
    knocked_out_rect.y = SCREEN_HEIGHT / 2 - knocked_out_rect.h;
    SDL_FreeSurface(knocked_out_surface);


    SDL_Surface* press_enter_surface = TTF_RenderText_Solid(font, "Press Enter to Main Menu...", text_color);
    SDL_Texture* press_enter_texture = SDL_CreateTextureFromSurface(renderer, press_enter_surface);
    SDL_Rect press_enter_rect;
    press_enter_rect.w = press_enter_surface->w;
    press_enter_rect.h = press_enter_surface->h;
    press_enter_rect.x = (SCREEN_WIDTH - press_enter_rect.w) / 2;
    press_enter_rect.y = SCREEN_HEIGHT / 2 + 20;
    SDL_FreeSurface(press_enter_surface);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, knocked_out_texture, NULL, &knocked_out_rect);
    SDL_RenderCopy(renderer, press_enter_texture, NULL, &press_enter_rect);
    SDL_RenderPresent(renderer);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_RETURN && gameOver) {
                gameOver = false;
                fadeTransition(MENU);
            }
        }
    }

    SDL_DestroyTexture(knocked_out_texture);
    SDL_DestroyTexture(press_enter_texture);
}


void renderMenu() {
    if (menuBG)
        SDL_RenderCopy(renderer, menuBG, NULL, NULL);
    else {
        SDL_SetRenderDrawColor(renderer, 30, 30, 70, 255);
        SDL_RenderClear(renderer);
    }
    const char* menuItems[] = {"Play", "Guide", "Exit"};
    int centerX = SCREEN_WIDTH / 2 - 50;
    int startY = SCREEN_HEIGHT / 2 - 50;
    for (int i = 0; i < 3; i++) {
        bool isSelected = (selectedOption == i);

        float scale = 1.0f;
        if (isSelected) {
            scale = mouseDown ? 1.15f : 1.10f;
        }

        SDL_Surface* tempSurface = TTF_RenderText_Blended(font, menuItems[i], SDL_Color{255,255,255,255});
        int baseW = tempSurface->w;
        int baseH = tempSurface->h;
        SDL_FreeSurface(tempSurface);
        int newW = static_cast<int>(baseW * scale);
        int newH = static_cast<int>(baseH * scale);
        int posX = centerX - (newW - baseW) / 2;
        int posY = startY + i * 60 - (newH - baseH) / 2;
        renderText(menuItems[i], posX, posY, isSelected);
    }

    if(volumeSlider.currentValue > 0) SDL_RenderCopy(renderer, sound[0], nullptr, &volumeIconRect);
    else SDL_RenderCopy(renderer, sound[1], nullptr, &volumeIconRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &volumeSlider.rect);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer, &volumeSlider.handleRect);
    SDL_RenderPresent(renderer);
}

void handleMenuEvents(bool &running) {
    SDL_Event e;
    int centerX = SCREEN_WIDTH / 2 - 50;
    int startY = SCREEN_HEIGHT / 2 - 50;
    int itemHeight = 60;
    int itemWidth = 100;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
            return;
        }
        if (e.type == SDL_KEYDOWN) {
            int prevOption = selectedOption;
            switch(e.key.keysym.sym) {
                case SDLK_UP:
                    selectedOption = (selectedOption > 0) ? selectedOption - 1 : 2;
                    break;
                case SDLK_DOWN:
                    selectedOption = (selectedOption < 2) ? selectedOption + 1 : 0;
                    break;
                case SDLK_RETURN:
                    if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                    if (selectedOption == 0) fadeTransition(DIFFICULTY);
                    else if (selectedOption == 1) fadeTransition(SHOWGUIDANCE);
                    else if (selectedOption == 2) running = false;
                    break;
                default: break;
            }
            if (prevOption != selectedOption && hoverSound) {
                Mix_PlayChannel(-1, hoverSound, 0);
            }
        }
        if (e.type == SDL_MOUSEMOTION) {
            int mouseX = e.motion.x, mouseY = e.motion.y;
            SDL_Point mousePoint = { mouseX, mouseY };
            for (int i = 0; i < 3; i++) {
                SDL_Rect itemRect = { centerX, startY + i * itemHeight, itemWidth, itemHeight };
                if (SDL_PointInRect(&mousePoint, &itemRect)) {
                    if (lastHovered != i) {
                        lastHovered = i;
                        selectedOption = i;
                        if (hoverSound) Mix_PlayChannel(-1, hoverSound, 0);
                    }
                }
            }

            if (volumeSlider.isDragging) {
                volumeSlider.handleRect.x = max(SLIDER_X, min(mouseX - SLIDER_HANDLE_SIZE / 2, SLIDER_X + SLIDER_WIDTH - SLIDER_HANDLE_SIZE / 2));
                volumeSlider.currentValue = volumeSlider.minValue + (volumeSlider.maxValue - volumeSlider.minValue) * (volumeSlider.handleRect.x - SLIDER_X) / SLIDER_WIDTH;
                Mix_VolumeMusic(volumeSlider.currentValue);
            }
        }
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            mouseDown = true;
            int mouseX = e.motion.x, mouseY = e.motion.y;
            if (mouseX >= volumeSlider.handleRect.x && mouseX < volumeSlider.handleRect.x + volumeSlider.handleRect.w &&
                mouseY >= volumeSlider.handleRect.y && mouseY < volumeSlider.handleRect.y + volumeSlider.handleRect.h) {
                volumeSlider.isDragging = true;
            }
        }
        if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
            mouseDown = false;
            int mouseX = e.button.x, mouseY = e.button.y;
            SDL_Point mousePoint = { mouseX, mouseY };
            for (int i = 0; i < 3; i++) {
                SDL_Rect itemRect = { centerX, startY + i * itemHeight, itemWidth, itemHeight };
                if (SDL_PointInRect(&mousePoint, &itemRect)) {
                    selectedOption = i;
                    if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                    if (selectedOption == 0) fadeTransition(DIFFICULTY);
                    else if (selectedOption == 1) fadeTransition(SHOWGUIDANCE);
                    else if (selectedOption == 2) running = false;
                }
            }

            volumeSlider.isDragging = false;
        }

    }
}

void renderDifficulty() {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastDifUpdate > 200) {
        DifFrameCount = (DifFrameCount + 1) % 4;
        lastDifUpdate = currentTime;
    }


    SDL_RenderCopy(renderer, dificulBG[DifFrameCount], NULL, NULL);
    const char* diffOptions[] = {"Easy", "Hard", "Hell", "Back"};
    int centerX = SCREEN_WIDTH / 2 - 70;
    int startY = SCREEN_HEIGHT / 2 - 50;

    renderText("Select Difficulty", SCREEN_WIDTH / 2 - 120, startY - 70, false);

    for (int i = 0; i < 4; i++) {
        bool isSelected = (selectedOption == i);
        float scale = isSelected ? (mouseDown ? 1.15f : 1.10f) : 1.0f;
        SDL_Surface* tempSurface = TTF_RenderText_Blended(font, diffOptions[i], SDL_Color{255,255,255,255});
        int baseW = tempSurface->w;
        int baseH = tempSurface->h;
        SDL_FreeSurface(tempSurface);

        int newW = static_cast<int>(baseW * scale);
        int newH = static_cast<int>(baseH * scale);
        int posX = centerX - (newW - baseW) / 2;
        int posY = startY + i * 60 - (newH - baseH) / 2;

        renderText(diffOptions[i], posX, posY, isSelected);
    }
    SDL_RenderPresent(renderer);
}

void handleDifficultyEvents(bool &running) {
    SDL_Event e;
    int centerX = SCREEN_WIDTH / 2 - 70;
    int startY = SCREEN_HEIGHT / 2 - 50;
    int itemHeight = 60;
    int itemWidth = 140;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
            return;
        }
        if (e.type == SDL_KEYDOWN) {
            int prevOption = selectedOption;
            switch(e.key.keysym.sym) {
                case SDLK_UP:
                    selectedOption = (selectedOption > 0) ? selectedOption - 1 : 3;
                    break;
                case SDLK_DOWN:
                    selectedOption = (selectedOption < 3) ? selectedOption + 1 : 0;
                    break;
                case SDLK_RETURN:
                    if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                    if (selectedOption == 3) {
                        fadeTransition(MENU);
                    } else {
                        difficulty = selectedOption;
                        fadeTransition(PLAYING);
                    }
                    break;
                case SDLK_ESCAPE:
                    fadeTransition(MENU);
                    break;
                default: break;
            }
            if (prevOption != selectedOption && hoverSound) {
                Mix_PlayChannel(-1, hoverSound, 0);
            }
        }
        if (e.type == SDL_MOUSEMOTION) {
            int mouseX = e.motion.x, mouseY = e.motion.y;
            SDL_Point mousePoint = { mouseX, mouseY };
            for (int i = 0; i < 4; i++) {
                SDL_Rect rect = { centerX, startY + i * itemHeight, itemWidth, itemHeight };
                if (SDL_PointInRect(&mousePoint, &rect)) {
                    if (lastHovered != i) {
                        lastHovered = i;
                        selectedOption = i;
                        if (hoverSound) Mix_PlayChannel(-1, hoverSound, 0);
                    }
                }
            }
        }
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            mouseDown = true;
        }
        if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
            mouseDown = false;
            int mouseX = e.button.x, mouseY = e.button.y;
            SDL_Point mousePoint = { mouseX, mouseY };
            for (int i = 0; i < 4; i++) {
                SDL_Rect rect = { centerX, startY + i * itemHeight, itemWidth, itemHeight };
                if (SDL_PointInRect(&mousePoint, &rect)) {
                    selectedOption = i;
                    if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                    if (selectedOption == 3) {
                        fadeTransition(MENU);
                    } else {
                        difficulty = selectedOption;
                        fadeTransition(PLAYING);
                    }
                }
            }
        }
    }
}


void handlePauseEvents(bool& running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
        }
        // Xử lý nút ESC để tiếp tục game
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            gameState = PLAYING;
            Mix_ResumeMusic();
        }
    }
}



void renderGame(SDL_Renderer* renderer) {
    SDL_RenderCopy(renderer, gameBG, NULL, NULL);

    if (!gameOver && gameState) {
        if (boss.isRunning) {
            SDL_Rect srcRect = { boss.runningCurrentFrame * boss.runningFrameWidth, 0, boss.runningFrameWidth, boss.runningFrameHeight };
            SDL_Rect destRect = { boss.hitbox.x, boss.hitbox.y, boss.hitbox.w, boss.hitbox.h };
            SDL_RenderCopy(renderer, boss.runningTexture, &srcRect, &destRect);
        } else if (boss.isPhase3) {
            Uint32 currentTime = SDL_GetTicks();
            Uint32 frameTime = currentTime - boss.phase3StartTime;
            if (frameTime > boss.phase3FrameDuration && gameState == PLAYING) {
                boss.phase3CurrentFrame = (boss.phase3CurrentFrame + 1) % boss.phase3FrameCount;
                boss.phase3StartTime = currentTime;
            }
            SDL_Rect srcRect = { boss.phase3CurrentFrame * boss.phase3FrameWidth, 0, boss.phase3FrameWidth, boss.phase3FrameHeight };
            SDL_Rect destRect = { boss.hitbox.x, boss.hitbox.y, boss.hitbox.w, boss.hitbox.h };
            SDL_RenderCopy(renderer, boss.phase3Texture, &srcRect, &destRect);
        } else {
            Uint32 currentTime = SDL_GetTicks();
            Uint32 frameTime = currentTime - boss.phase12StartTime;
            if (frameTime > boss.phase12FrameDuration && gameState == PLAYING) {
                boss.phase12CurrentFrame = (boss.phase12CurrentFrame + 1) % boss.phase12FrameCount;
                boss.phase12StartTime = currentTime;
            }
            SDL_Rect srcRect = { boss.phase12CurrentFrame * boss.phase12FrameWidth, 0, boss.phase12FrameWidth, boss.phase12FrameHeight };
            SDL_Rect destRect = { boss.hitbox.x, boss.hitbox.y, boss.hitbox.w, boss.hitbox.h };
            SDL_RenderCopy(renderer, boss.phase12Texture, &srcRect, &destRect);
        }

        for (auto& b : bossBullets) {
            if (b.phase == 1 && b.phase1Texture) {
                Uint32 currentTime = SDL_GetTicks();
                Uint32 frameTime = currentTime - b.phase1StartTime;
                if (frameTime > b.phase1FrameDuration && gameState == PLAYING) {
                    b.phase1CurrentFrame = (b.phase1CurrentFrame + 1) % b.phase1FrameCount;
                    b.phase1StartTime = currentTime;
                }
                SDL_Rect srcRect = { b.phase1CurrentFrame * b.phase1FrameWidth, 0, b.phase1FrameWidth, b.phase1FrameHeight };
                SDL_Rect destRect = { b.hitbox.x, b.hitbox.y, b.hitbox.w, b.hitbox.h };
                SDL_RenderCopy(renderer, b.phase1Texture, &srcRect, &destRect);
            } else if (b.phase == 2 && b.phase2Texture ) {
                Uint32 currentTime = SDL_GetTicks();
                Uint32 frameTime = currentTime - b.phase2StartTime;
                if (frameTime > b.phase2FrameDuration && gameState == PLAYING) {
                    b.phase2CurrentFrame = (b.phase2CurrentFrame + 1) % b.phase2FrameCount;
                    b.phase2StartTime = currentTime;
                }
                SDL_Rect srcRect = { b.phase2CurrentFrame * b.phase2FrameWidth, 0, b.phase2FrameWidth, b.phase2FrameHeight };
                SDL_Rect destRect = { b.hitbox.x, b.hitbox.y, b.hitbox.w, b.hitbox.h };
                SDL_RenderCopy(renderer, b.phase2Texture, &srcRect, &destRect);
            } else if (b.phase == 3 && b.carrotTexture) {
                Uint32 currentTime = SDL_GetTicks();
                Uint32 frameTime = currentTime - b.carrotStartTime;
                if (frameTime > b.carrotFrameDuration && gameState == PLAYING){
                    b.carrotCurrentFrame = (b.carrotCurrentFrame + 1) % b.carrotFrameCount;
                    b.carrotStartTime = currentTime;
                }
                SDL_Rect srcRect = { b.carrotCurrentFrame * b.carrotFrameWidth, 0, b.carrotFrameWidth, b.carrotFrameHeight };
                SDL_Rect destRect = { b.hitbox.x, b.hitbox.y, b.hitbox.w, b.hitbox.h };
                SDL_RenderCopy(renderer, b.carrotTexture, &srcRect, &destRect);
            }
        }

        for (auto& bullet : bullets) {
            if (bullet.playerBulletTexture) {
                Uint32 currentTime = SDL_GetTicks();
                Uint32 frameTime = currentTime - bullet.playerBulletStartTime;
                if (frameTime > bullet.playerBulletFrameDuration && gameState == PLAYING) {
                    bullet.playerBulletCurrentFrame = (bullet.playerBulletCurrentFrame + 1) % bullet.playerBulletFrameCount;
                    bullet.playerBulletStartTime = currentTime;
                }
                SDL_Rect srcRect = { bullet.playerBulletCurrentFrame * bullet.playerBulletFrameWidth, 0, bullet.playerBulletFrameWidth, bullet.playerBulletFrameHeight };
                SDL_Rect destRect = { bullet.hitbox.x, bullet.hitbox.y, bullet.hitbox.w, bullet.hitbox.h };
                SDL_RendererFlip flip = bullet.is_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
                SDL_RenderCopyEx(renderer, bullet.playerBulletTexture, &srcRect, &destRect, 0, NULL, flip);
            }
        }
        for (auto& meteor : meteors) {
            if(meteor.meteorTexture){
                Uint32 currentTime = SDL_GetTicks();
                Uint32 frameTime = currentTime - meteor.MeteorStartTime;
                if(frameTime > 50 && gameState == PLAYING){
                    meteor.currentFrame = (meteor.currentFrame+1)%15;
                    meteor.MeteorStartTime = currentTime;
                }
                SDL_Rect srcRect = { meteor.currentFrame * (1700 / 15), 0, 1700 / 15, 146 };
                SDL_Rect destRect = { meteor.hitbox.x, meteor.hitbox.y, meteor.hitbox.w, meteor.hitbox.h };
                SDL_RenderCopy(renderer, meteor.meteorTexture, &srcRect, &destRect);
            }
        }

        for (int i = 0; i < currentHP; ++i) {
            SDL_Rect heartRect = { 10 + i * (HEART_SIZE + 5), 10, HEART_SIZE, HEART_SIZE };
            SDL_RenderCopy(renderer, hpTexture, nullptr, &heartRect);
        }
        for (int i = 0; i < specialCharges; i++) {
            SDL_Rect specialRect = { 10 + i * (SPECIAL_ICON_SIZE + 5), 50, SPECIAL_ICON_SIZE, SPECIAL_ICON_SIZE };
            SDL_RenderCopy(renderer, specialTexture, nullptr, &specialRect);
        }
        if ((player.isJumping || player.isFalling )&& !movingToStart) {
            SDL_Rect srcRect = { playerJumpCurrentFrame * (704 / 8), 0, 704 / 8, 109 };
            SDL_Rect destRect = { player.x, player.y, player.w, player.h };
            SDL_RendererFlip flip = player.facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
            SDL_RenderCopyEx(renderer, playerJumpTexture, &srcRect, &destRect, 0, NULL, flip);
        } else if (player.isDashing && !movingToStart) {
            SDL_Rect srcRect = { playerDashCurrentFrame * (2000 / 8 ), 0, 2000 / 8, 125 };
            SDL_Rect destRect = { player.x, player.y, player.w, player.h };
            SDL_RendererFlip flip = player.facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
            SDL_RenderCopyEx(renderer, playerDashTexture, &srcRect, &destRect, 0, NULL, flip);
        } else if (player.isRunning && player.isShooting && !movingToStart) {
            SDL_Rect srcRect = { playerRunShootCurrentFrame * (1885 / 16), 0, 1885 / 16, 132 };
            SDL_Rect destRect = { player.x, player.y, player.w, player.h };
            SDL_RendererFlip flip = runFacingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
            SDL_RenderCopyEx(renderer, playerRunShootTexture, &srcRect, &destRect, 0, NULL, flip);
         } else if (player.isShooting && !movingToStart) {
            SDL_Rect srcRect = { playerShootCurrentFrame * (822 / 6), 0, 822 / 6, 165 };
            SDL_Rect destRect = { player.x, player.y, player.w, player.h };
            SDL_RendererFlip flip = runFacingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
            SDL_RenderCopyEx(renderer, playerShootTexture, &srcRect, &destRect, 0, NULL, flip);
        } else if (player.isRunning) {
            SDL_Rect srcRect = { playerRunCurrentFrame * (1800 / 16), 0, 1800 / 16, 138 };
            SDL_Rect destRect = { player.x, player.y, player.w, player.h };
            SDL_RendererFlip flip = runFacingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
            SDL_RenderCopyEx(renderer, playerRunTexture, &srcRect, &destRect, 0, NULL, flip);
        } else {
            SDL_Rect srcRect = { playerIdleCurrentFrame * (500 / 5), 0, 500 / 5, 155 };
            SDL_Rect destRect = { player.x, player.y, player.w, player.h };
            SDL_RendererFlip flip = player.facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
            SDL_RenderCopyEx(renderer, playerIdleTexture, &srcRect, &destRect, 0, NULL, flip);
        }
        if (gameState == PLAYING) {
            SDL_RenderCopy(renderer, pausegame[0], nullptr, &pauseIconRect);
        }
        else if (gameState == PAUSED) {
            SDL_RenderCopy(renderer, pausegame[1], nullptr, &pauseIconRect);
        }
    }
    SDL_RenderPresent(renderer);
}

void handleGameEvents(bool &running) {
    if(DIFFICULTY == 2){
        damage = 1;
    }
    currentHP = MAX_HP;
    specialCharges = 0;
    gameOver = false;
    movingToStart = true;
    logicX = 0;
    targetX = SCREEN_WIDTH / 3;
    player.x = 0;
    player.y = 650;
    player.velY = 0.0f;
    player.isJumping = false;
    player.isFalling = false;
    player.facingRight = true;
    player.isDashing = false;
    player.isCrouching = false;
    player.isStanding = false;
    player.isRunning = false;
    player.isShooting = false;
    boss.x = SCREEN_WIDTH - BOSS_WIDTH - 50;
    boss.y = SCREEN_HEIGHT - BOSS_HEIGHT + BOSS_HEIGHT / 2;
    boss.health = BOSS_PHASE1_HP;
    boss.canAttack = false;
    boss.isPhase3 = false;
    bullets.clear();
    bossBullets.clear();
    meteors.clear();
    spawnPlayer();
    spawnBoss(renderer);
    boss.runningStartTime = SDL_GetTicks();
    Uint32 lastFrame = SDL_GetTicks();
    while (running && !gameOver) {
        Uint32 currentFrame = SDL_GetTicks();
        float deltaTime = (currentFrame - lastFrame) / 1000.0f;
        lastFrame = currentFrame;
        if (gameState == PLAYING) {
            handleInput(running, player, deltaTime, renderer);
            updateGame(player, deltaTime, renderer, damage);
        }
        else if (gameState == PAUSED) {
            handlePauseEvents(running);
        }

        renderGame(renderer);
        SDL_Delay(16);
    }
}

int currentGuideBG = 0;
void renderGuide() {
    if (guideBG[currentGuideBG]) {
        SDL_RenderCopy(renderer, guideBG[currentGuideBG], NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 100, 150, 255);
        SDL_RenderClear(renderer);
    }
    renderText("Guide Screen (Press ESC to return)", 400, 350, false);
    renderText("Press LEFT/RIGHT to change guide BG", 380, 400, false);
    SDL_RenderPresent(renderer);
}

void handleGuideEvents(bool &running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
            return;
        }
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                fadeTransition(MENU);
            }
            if (e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_RIGHT) {
                // Fade effect for guide background transition
                for (int alpha = 0; alpha <= 255; alpha += 25) {
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
                    SDL_RenderFillRect(renderer, NULL);
                    SDL_RenderPresent(renderer);
                    SDL_Delay(5);
                }
                currentGuideBG = 1 - currentGuideBG;
                for (int alpha = 255; alpha >= 0; alpha -= 25) {
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
                    SDL_RenderFillRect(renderer, NULL);
                    SDL_RenderPresent(renderer);
                    SDL_Delay(5);
                }
            }
        }
    }
}

// -------------------- MAIN --------------------
int main(int argc, char* argv[]) {
    if (!initSDL()) {
        cerr << "Failed to initialize SDL\n";
        return -1;
    }

    bool running = true;
    while (running) {
        switch (gameState) {
            case MENU:
                handleMenuEvents(running);
                renderMenu();
                break;
            case DIFFICULTY:
                handleDifficultyEvents(running);
                renderDifficulty();
                break;
            case PLAYING:
                handleGameEvents(running);
                break;
            case SHOWGUIDANCE:
                handleGuideEvents(running);
                renderGuide();
                break;
            case PAUSED:
                handlePauseEvents(running);
                break;
            case GAMEOVER:
                renderGameOver(running);
                break;
            case EXIT:
                running = false;
                break;
            default:
                break;
        }
        SDL_Delay(16);
    }

    cleanup();
    return 0;
}
