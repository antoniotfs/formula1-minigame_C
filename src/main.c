#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <termios.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#define NUM_OBSTACLES 2
#define STEP 3
#define INITIAL_TICK_RATE 60
#define MIN_TICK_RATE 5
#define MAX_TICK_RATE 100
#define GAME_DURATION 31
#define RECORDS_FILE "records.txt" 

int gameState = 1;
int carX = 15, carY = 25;
int velocityX = 0;
int trackLineY = 0; 
int **obstaclePositions = NULL;
int tickRate = INITIAL_TICK_RATE;
int gameTimer = GAME_DURATION;
int gameMode = 0; 
double gamePoint = 0;
time_t startTime;

typedef struct {
    char name[50];
    double score;
} Record;

void enableEcho() {
    struct termios term;
    tcgetattr(0, &term);           
    term.c_lflag |= ECHO;          
    tcsetattr(0, TCSANOW, &term);  
}

void disableEcho() {
    struct termios term;
    tcgetattr(0, &term);           
    term.c_lflag &= ~ECHO;         
    tcsetattr(0, TCSANOW, &term);  
}

Record* loadRecords() {
    Record* records = malloc(2 * sizeof(Record)); 
    FILE* file = fopen(RECORDS_FILE, "r");
    if (!file) {
        strcpy(records[0].name, "Ninguém");
        records[0].score = 0.0;
        strcpy(records[1].name, "Ninguém");
        records[1].score = 0.0;
    } else {
        fread(records, sizeof(Record), 2, file);
        fclose(file);
    }
    return records;
}

void saveRecord(int mode, const char* name, double score) {
    Record* records = loadRecords(); 
    strcpy(records[mode].name, name);
    records[mode].score = score;

    FILE* file = fopen(RECORDS_FILE, "w");
    if (!file) {
        perror("Erro ao salvar os recordes");
        free(records);
        return;
    }
    fwrite(records, sizeof(Record), 2, file);
    fclose(file);
    free(records); 
}

void checkAndSaveRecord(int mode, double currentScore, int gTimer) {
    if (gTimer > 0 && gTimer < GAME_DURATION) {
        return;
    }

    Record* records = loadRecords();
    if (currentScore > records[mode].score) {
        printf("\nParabéns! Você bateu o recorde!\n");
        screenSetNormal();
        printf("Digite seu nome: ");
        enableEcho();
        char name[50];
        scanf("%49s", name);
        disableEcho();

        saveRecord(mode, name, currentScore);
    }
    free(records); 
}

void displayRecord(int mode) {
    Record* records = loadRecords();
    printf("Recorde Atual (%s): %.2f por %s\n", (mode == 0) ? "30 Segundos" : "Infinito", records[mode].score, records[mode].name);
    free(records); 
}

void initObstacles() {
    obstaclePositions = malloc(NUM_OBSTACLES * sizeof(int*));
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        obstaclePositions[i] = malloc(2 * sizeof(int));
    }

    srand(time(NULL));
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        int isUnique;
        do {
            isUnique = 1;
            obstaclePositions[i][0] = ((rand() % (((MAXX - 4) - (MINX + 2)) / STEP + 1)) + (MINX + 2) / STEP) * STEP;
            for (int j = 0; j < i; j++) {
                if (abs(obstaclePositions[i][0] - obstaclePositions[j][0]) < 3) {
                    isUnique = 0;
                    break;
                }
            }
        } while (!isUnique);
        obstaclePositions[i][1] = -(rand() % 10);
    }
}

void freeObstacles() {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        free(obstaclePositions[i]);
    }
    free(obstaclePositions);
}

void resetGame() {
    screenInit(1);
    carX = 15;
    velocityX = 0;
    trackLineY = 0;
    tickRate = INITIAL_TICK_RATE;
    gameTimer = GAME_DURATION;
    gamePoint = 0;
    initObstacles();
    timerInit(tickRate);
    displayRecord(gameMode);
}

void drawTrack() {
    screenSetColor(WHITE, DARKGRAY);
    for (int x = MINX + 10; x < MAXX - 10; x += 10) {
        screenGotoxy(15, trackLineY);
        printf(" ");
        screenGotoxy(15, trackLineY + 1);
        printf(" ");
        screenGotoxy(15, trackLineY + 2);
        printf(" ");
        screenGotoxy(15, trackLineY + 3);
        printf(" ");
    }
    trackLineY += 2;
    if ((trackLineY + 3) > MAXY) trackLineY = MINY;
    for (int x = MINX + 10; x < MAXX - 10; x += 10) {
        screenGotoxy(15, trackLineY);
        printf("█");
        screenGotoxy(15, trackLineY + 1);
        printf("█");
        screenGotoxy(15, trackLineY + 2);
        printf("█");
        screenGotoxy(15, trackLineY + 3);
        printf("█");
    }
}

void drawObstacle(int posX, int posY) {
    screenSetColor(RED, DARKGRAY);
    screenGotoxy(posX, posY);
    printf("┏┳┓");
    screenGotoxy(posX, posY + 1);
    printf("0╋0");
    screenGotoxy(posX, posY + 2);
    printf("┣╋┫");
    screenGotoxy(posX, posY + 3);
    printf("0╋0");
    screenGotoxy(posX, posY + 4);
    printf("━┻━");
}

void clearCar(int posX, int posY) {
    for (int i = 0; i < 5; i++) {
        screenGotoxy(posX, posY + i);
        printf("   ");
    }
}

void updateObstacles() {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        clearCar(obstaclePositions[i][0], obstaclePositions[i][1]);
        obstaclePositions[i][1]++; 

        if (obstaclePositions[i][1] >= carY && obstaclePositions[i][1] < carY + 5 && abs(obstaclePositions[i][0] - carX) < 3) {
            screenClear();
            printf("\nGame Over! Você fez %.2f pontos.\nPressione Enter para voltar ao menu.\n", gamePoint);
            while (readch() != '\n');

            gameState = 0;
            return;
        }

        if (obstaclePositions[i][1] < MAXY - 4) {
            drawObstacle(obstaclePositions[i][0], obstaclePositions[i][1]);
        } else {
            int isUnique;
            do {
                isUnique = 1;
                obstaclePositions[i][0] = ((rand() % (((MAXX - 4) - (MINX + 2)) / STEP + 1)) + (MINX + 2) / STEP) * STEP;
                for (int j = 0; j < NUM_OBSTACLES; j++) {
                    if (i != j && abs(obstaclePositions[i][0] - obstaclePositions[j][0]) < 3) {
                        isUnique = 0;
                        break;
                    }
                }
            } while (!isUnique);
            obstaclePositions[i][1] = MINY;
        }
    }
}

void printCar(int posX, int posY) {
    screenSetColor(CYAN, DARKGRAY);
    screenGotoxy(posX, posY);
    printf("┏┳┓");
    screenGotoxy(posX, posY + 1);
    printf("0╋0");
    screenGotoxy(posX, posY + 2);
    printf("┣╋┫");
    screenGotoxy(posX, posY + 3);
    printf("0╋0");
    screenGotoxy(posX, posY + 4);
    printf("━┻━");
}

void handleInput(int ch) {
    switch (ch) {
        case 'd':
            velocityX = STEP;
            break;
        case 'a':
            velocityX = -STEP;
            break;
        case 'w':
            if (tickRate > MIN_TICK_RATE) tickRate -= 2;
            timerInit(tickRate);
            break;
        case 's':
            if (tickRate < MAX_TICK_RATE) tickRate += 2;
            timerInit(tickRate);
            break;
        case 27:
            exit(0);
    }
}

void updateTimer() {
    if (gameMode == 0) { 
        static time_t lastUpdateTime = 0;
        time_t currentTime = time(NULL);
        if (currentTime != lastUpdateTime) {
            lastUpdateTime = currentTime;
            gameTimer -= 1;
            gamePoint += (1000.0 / tickRate);
            screenGotoxy(0, 0);
            printf("Tempo Restante: %d:%02d", gameTimer / 60, gameTimer % 60);

            if (gameTimer <= 0) {
                screenClear();
                printf("\nWINNER! Você fez %.2f pontos.\nPressione Enter para voltar ao menu.\n", gamePoint);
                while (readch() != '\n');
                return;
            }
        }
    } else { 
        gamePoint += (100.0 / tickRate);
        screenGotoxy(0, 0);
        printf("Pontos: %.2f", gamePoint);
    }
}

void showMenu() {
    int option = 0;
    while (1) {
        screenClear();
        printf("Selecione o modo de jogo:\n\n");
        if (option == 0) {
            printf("-> Jogo de 30 Segundos\n");
            printf("   Jogo Infinito\n");
        } else {
            printf("   Jogo de 30 Segundos\n");
            printf("-> Jogo Infinito\n");
        }
        printf("\nUse 'A' e 'D' para mudar a opção, 'Enter' para selecionar.\n");

        int ch = readch();
        if (ch == 'a' || ch == 'A') {
            option = 0;
        } else if (ch == 'd' || ch == 'D') {
            option = 1;
        } else if (ch == '\n' || ch == '\r') { 
            gameMode = option;
            break;
        }
        if (ch == 27) { 
            exit(0);
        }
        screenUpdate();
    }
}

int main() {
    int ch = 0;

    keyboardInit();
    timerInit(tickRate);

    while (1) {
        showMenu();
        resetGame();
        startTime = time(NULL);
        printCar(carX, carY);
        screenUpdate();

        gameState = 1;
        while (gameState) {
            if (keyhit()) {
                ch = readch();
                handleInput(ch);
                screenUpdate();
            }

            if (timerTimeOver() == 1) {
                int newX = carX + velocityX;
                clearCar(carX, carY);

                if (newX < MINX + 1) newX = MINX + 1;
                if (newX >= MAXX - 5) newX = MAXX - 5;

                drawTrack();
                updateObstacles();

                if (!gameState) break;


                printCar(newX, carY);
                updateTimer();
                screenUpdate();

                carX = newX;
                velocityX = 0;

                if (gameTimer <= 0 || (gameMode == 1 && gamePoint < 0)) {
                    break;
                }
            }
        }
        checkAndSaveRecord(gameMode, gamePoint, gameTimer);
    }

    freeObstacles();
    keyboardDestroy();
    screenDestroy();
    timerDestroy();

    return 0;
}
