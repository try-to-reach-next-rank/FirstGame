#innclude<bít/stdc++.h>
using namespace std;

struct Object{
    int x, y;
    int speed;
    int direction; // 0: up, 1: right, 2: down, 3: left, 4: up-right, 5: down-right, 6: down-left, 7: up-left
    bool active;
    Uint32 lastShotTime;
};

struct Meteor {
    SDL_Rect rect;        // Vị trí và kích thước
    int speed;            // Tốc độ rơi
    bool active;          // Trạng thái hoạt động
};

struct Bullets{
    int x, y;
    int direction;
    bool active;
    bool isPlayerBullet;
};

Object Player;
vector<Bullets> bullets;
vector<Object> enimies;
vector<Meteor> meteors;

bool keyStates[SDL_NUM_SCANCODES] = { false }; // Track the state of each key

bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void gamePlay() {
    Player.speed = 5;
    Player.active = true;
    Player.lastShotTime = 0;

}
