#pragma once
#include<raylib.h>
#include<raymath.h>

#include<iostream>
#include<UI.h>
#include<vector>
#include <cstring>
#include <thread>
#include <chrono>
#include <random>

#include<State.h>
#include <MenuState.h>
#include<IntroState.h>
#include<LinkedListState.h>
#include <AVLTreeState.h>
#include<HeapState.h>

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
    
    private:
        int WindowHeight;
        int WindowWidth;
        void LoadAssets();


        MenuState menu;
        IntroState intro;
        LinkedListState linkedList;
        AVLTreeState avlTree;
        HeapState heap;
};