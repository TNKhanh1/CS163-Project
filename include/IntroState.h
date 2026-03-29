#pragma once
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <random>
#include <cstring>
#include "UI.h"
#include "State.h"

// Định nghĩa cấu trúc bong bóng
struct BUBBLE {
    Vector2 position;
    float scale;
    float speed;
    float wobbleOffset;
};

class IntroState {
public:
    IntroState();
    ~IntroState();

    void loadAssets();
    void update(float deltaTime);
    void draw();

    State NextState;

private:
    int WindowWidth;
    int WindowHeight;

    Font introFont;
    Texture2D bubbleTex;
    back_ground* introBg;
    button* startBtn;

    std::vector<BUBBLE> bubbles;
    float bubbleSpawnTimer;
    std::mt19937 rng;

    double startTime;
    double introDuration;
    bool isFadingOut;
    float transitionAlpha;
    bool isInitialized;
};