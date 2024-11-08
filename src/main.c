#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#define NUM_OBSTACLES 4
#define STEP 3
#define INITIAL_TICK_RATE 50
#define MIN_TICK_RATE 5
#define MAX_TICK_RATE 100
#define GAME_DURATION 30 

int carX = 15, carY = 20;
int velocityX = 0;
int trackLineY = 0; 
int obstaclePositions[NUM_OBSTACLES][2]; 
int tickRate = INITIAL_TICK_RATE;
int gameTimer = GAME_DURATION;
time_t startTime; 
void initObstacles() {
    srand(time(NULL));
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        obstaclePositions[i][0] = ((rand() % ((MAXX - 1) / STEP)) + 1) * STEP; 
        obstaclePositions[i][1] = -(rand() % 10);
    }
}

void drawTrack() {
    screenSetColor(WHITE, DARKGRAY);
    for (int x = MINX + 10; x < MAXX - 10; x += 10) {
        screenGotoxy(15, trackLineY);
        printf("   "); 
        screenGotoxy(15, trackLineY + 1);
        printf("   "); 
        screenGotoxy(15, trackLineY + 2);
        printf("   "); 
    }
    trackLineY += 2; 
    if (trackLineY > MAXY) trackLineY = MINY; 
    for (int x = MINX + 10; x < MAXX - 10; x += 10) {
        screenGotoxy(15, trackLineY);
        printf("█"); 
        screenGotoxy(15, trackLineY + 1);
        printf("█"); 
        screenGotoxy(15, trackLineY + 2);
        printf("█"); 
    }
}

void updateObstacles() {
    screenSetColor(RED, DARKGRAY);
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        screenGotoxy(obstaclePositions[i][0], obstaclePositions[i][1]);
        printf(" ");
        obstaclePositions[i][1]++;
        if (abs(obstaclePositions[i][0] - carX) < 2 && abs(obstaclePositions[i][1] - carY) < 1) {
            time_t endTime = time(NULL); 
            double elapsedTime = difftime(endTime, startTime); 
            printf("\nGame Over! You hit an obstacle. Você jogou por %.2f segundos.\n", elapsedTime);
            exit(0);
        }

        if (obstaclePositions[i][1] < MAXY) {
            screenGotoxy(obstaclePositions[i][0], obstaclePositions[i][1]);
            printf("X");
        } else {
            obstaclePositions[i][0] = ((rand() % ((MAXX - 1) / STEP)) + 1) * STEP;
            obstaclePositions[i][1] = MINY;
        }
    }
}

void printCar(int posX, int posY) {
    screenSetColor(CYAN, DARKGRAY);
    screenGotoxy(carX, carY);
    printf("  ");
    carX = posX;
    carY = posY;
    screenGotoxy(carX, carY);
    printf("F1");
}

void handleInput(int ch) {
    switch (ch) {
        case 'd': 
            velocityX = 1;
            break;
        case 'a': 
            velocityX = -1;
            break;
        case 'w': 
            if (tickRate > MIN_TICK_RATE) tickRate -= 5;
            timerInit(tickRate);
            break;
        case 's': 
            if (tickRate < MAX_TICK_RATE) tickRate += 5;
            timerInit(tickRate);
            break;
        case 27: 
            exit(0);
    }
}

void updateTimer() {
    static int lastTick = 0;
    if (timerTimeOver() != lastTick) {
        lastTick = !lastTick;
        gameTimer -= 1;
        screenGotoxy(0, 0);
        printf("Distance remaining: %d:%02d", gameTimer / 60, gameTimer % 60);
        
        if (gameTimer <= 0) {
            time_t endTime = time(NULL); 
            double elapsedTime = difftime(endTime, startTime); 
            printf("\nWINNER! Você demorou %.2f segundos.\n", elapsedTime);
            exit(0);
        }
    }
}

int main() {
    int ch = 0;

    screenInit(1);
    keyboardInit();
    timerInit(tickRate);

    startTime = time(NULL); 
    initObstacles();
    printCar(carX, carY);
    screenUpdate();

    while (1) { 
        if (keyhit()) {
            ch = readch();
            handleInput(ch);
            screenUpdate();
        }

        if (timerTimeOver() == 1) {
            int newX = carX + velocityX;
            if (newX < MINX + 1) newX = MINX + 1;
            if (newX >= MAXX - 2) newX = MAXX - 2;
            drawTrack();
            updateObstacles();
            printCar(newX, carY);
            screenUpdate();
            updateTimer();
        }
    }

    keyboardDestroy();
    screenDestroy();
    timerDestroy();

    return 0;
}