#pragma once
#include<raylib.h>
#include<raymath.h>
#include<iostream>
#include<UI.h>
#include<vector>
#include<State.h>
#include <MenuState.h>
#include <cstring>
#include <thread>
#include <chrono>
#include <random>
using namespace std;

class MainLoop
{
    public:
    MainLoop();
    ~MainLoop();


    State current_state;
    vector<State> history_state;


    void update(float delta_time);
    void run();
    void draw();
    void BackState();
    void NextState();
    void Intro();
    
    private:
        int WindowHeight;
        int WindowWidth;
        void LoadAssets();


        MenuState menu;
};