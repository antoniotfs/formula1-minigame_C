/**
 * main.h
 * Created on Aug, 23th 2023
 * Author: Tiago Barros#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // Para sleep
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#define MAX_LAPS 10
#define TIME_LIMIT 60 // Tempo limite em segundos

int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

int main() {
    int lapsCompleted = 0;
    int speed = 0;
    int timeSpent = 0;
    time_t startTime, currentTime;

    printf("Minigame Fórmula 1\n");
    printf("Controle: A = Acelerar, D = Desacelerar, Q = Sair\n");
    printf("Você tem %d segundos para completar o máximo de voltas!\n", TIME_LIMIT);

    startTime = time(NULL);

    while (lapsCompleted < MAX_LAPS && timeSpent < TIME_LIMIT) {
        currentTime = time(NULL);
        timeSpent = (int)(currentTime - startTime);

        printf("\rVoltas completas: %d | Velocidade: %d km/h | Tempo: %d segundos", 
               lapsCompleted, speed, timeSpent);
        fflush(stdout);

        if (speed >= 50) {
            lapsCompleted++;
            speed = 0;
        }

        if (kbhit()) {
            char input = getchar();
            switch (input) {
                case 'A': case 'a':
                    speed += 10;
                    if (speed > 200) speed = 200;
                    break;
                case 'D': case 'd':
                    speed -= 10;
                    if (speed < 0) speed = 0;
                    break;
                case 'Q': case 'q':
                    goto end;
            }
        }

        usleep(100000); // Atraso de 100ms (100000 microssegundos)
    }

end:
    printf("\nTempo esgotado ou máximo de voltas alcançado!\n");
    printf("Voltas completas: %d\n", lapsCompleted);
    printf("Velocidade final: %d km/h\n", speed);

    return 0;
}

 * Based on "From C to C++ course - 2002"
*/

#include <string.h>

#include "screen.h"
#include "keyboard.h"
#include "timer.h"

int x = 34, y = 12;
int incX = 1, incY = 1;

void printHello(int nextX, int nextY)
{
    screenSetColor(CYAN, DARKGRAY);
    screenGotoxy(x, y);
    printf("           ");
    x = nextX;
    y = nextY;
    screenGotoxy(x, y);
    printf("Hello World");
}

void printKey(int ch)
{
    screenSetColor(YELLOW, DARKGRAY);
    screenGotoxy(35, 22);
    printf("Key code :");

    screenGotoxy(34, 23);
    printf("            ");
    
    if (ch == 27) screenGotoxy(36, 23);
    else screenGotoxy(39, 23);

    printf("%d ", ch);
    while (keyhit())
    {
        printf("%d ", readch());
    }
}

int main() 
{
    static int ch = 0;

    screenInit(1);
    keyboardInit();
    timerInit(50);

    printHello(x, y);
    screenUpdate();

    while (ch != 10) //enter
    {
        // Handle user input
        if (keyhit()) 
        {
            ch = readch();
            printKey(ch);
            screenUpdate();
        }

        // Update game state (move elements, verify collision, etc)
        if (timerTimeOver() == 1)
        {
            int newX = x + incX;
            if (newX >= (MAXX -strlen("Hello World") -1) || newX <= MINX+1) incX = -incX;
            int newY = y + incY;
            if (newY >= MAXY-1 || newY <= MINY+1) incY = -incY;

            printKey(ch);
            printHello(newX, newY);

            screenUpdate();
        }
    }

    keyboardDestroy();
    screenDestroy();
    timerDestroy();

    return 0;
}
