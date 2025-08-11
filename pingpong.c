/**
 Boilerplate code implementing the GUI for the pingpong game. Update the file accordingly as necessary. 
 You are allowed to update functions, add new functions, modify the stuctures etc. Keep the output graphics intact.
 
 CS3205 - Assignment 2 (Holi'25)
 Instructor: Ayon Chakraborty
 **/
 
#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define WIDTH 80
#define HEIGHT 30
#define OFFSETX 10
#define OFFSETY 5

typedef struct {
    int x, y;
    int dx, dy;
} Ball;

typedef struct {
    int x; 
    int width;
} Paddle;

typedef struct {
    Ball ball;
    Paddle paddleA, paddleB;
    int penaltyA, penaltyB;
    int game_running;
} GameState;

const int stride = 1;
int server_fd, client_fd;
Ball ball;
Paddle paddleA,paddleB;
int game_running = 1;
int penaltyA = 0;
int penaltyB = 0;
GameState game;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void init();
void end_game();
void draw(WINDOW *win);
void *move_ballA(void *args);
void *move_ballB(void *args);
void update_paddleA(int ch);
void update_paddleB(int ch);
void reset_ball();
void server();
void client();


int main(int argc, char *argv[]) {

    ball = (Ball){WIDTH / 2, HEIGHT / 2, 1, 1};
    paddleA = (Paddle){WIDTH / 2 - 3, 10};
    paddleB = (Paddle){WIDTH / 2 - 3, 10};
    game = (GameState){ball, paddleA, paddleB, penaltyA, penaltyB,game_running};
    init();

    // pthread_t ball_thread;
    if (strcmp(argv[1],"server")==0){
        printf("Server\n");
        // pthread_create(&ball_thread, NULL, move_ballA, NULL);
        struct sockaddr_in address;
        int addrlen = sizeof(address);

        server_fd = socket(AF_INET, SOCK_STREAM, 0); //step 1
        memset(&address, '\0', sizeof(address));  //step 2, next 3 lines
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(atoi(argv[2]));

        bind(server_fd, (struct sockaddr *)&address, sizeof(address)); //step 3
        listen(server_fd, 5); //step 4

        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        printf("Client connected\n");
        server();
        close(client_fd);
    }
    else if(strcmp(argv[1],"client")==0){
        // pthread_create(&ball_thread, NULL, move_ballB, NULL);
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        char buffer[100] = {0};
    
        client_fd = socket(AF_INET, SOCK_STREAM, 0);
        memset(&address, '\0', sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(12345);
        printf("IP: %s\n",argv[2]);
        inet_pton(AF_INET,argv[2], &address.sin_addr); //change the IP accordingly
    
        connect(client_fd, (struct sockaddr *)&address, addrlen);
        printf("Connected to server\n");
        client();
        close(client_fd);
    }
    return 0;

}

void server() {
    pthread_t ball_thread;
    pthread_create(&ball_thread, NULL, move_ballA, NULL);
    while (game_running) {  
        int ch = getch();
        if (ch == 'q') {
            game_running = 0; /* if we want to exit by pressing 'q' */
            break;
        }
        if (ch != ERR) {
            update_paddleA(ch);
        }
        game = (GameState){ball, paddleA, paddleB, penaltyA, penaltyB,game_running};
        draw(stdscr);
    }
    pthread_join(ball_thread, NULL);
    end_game();
}

void client() {
    pthread_t ball_thread;
    pthread_create(&ball_thread, NULL, move_ballB, NULL);
    while (game_running) {
        int ch = getch();
        if (ch == 'q') {
            game_running = 0; /* if we want to exit by pressing 'q' */
            break;
        }
        if (ch != ERR) {
            update_paddleB(ch);
        }
        draw(stdscr);
    }
    pthread_join(ball_thread, NULL);
    end_game();
}

void init() {
    initscr();
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_WHITE);
    init_pair(2, COLOR_YELLOW, COLOR_RED);
    timeout(1);                    
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    noecho(); 
}

void end_game() {
    endwin();  // End curses mode
}

void draw(WINDOW *win) {
    erase();  // Clear the screen

    // Draw the border
    attron(COLOR_PAIR(1));
    for (int i = OFFSETX; i <= OFFSETX + WIDTH; i++) {
        mvprintw(OFFSETY-1, i, " ");
    }
    mvprintw(OFFSETY-1, OFFSETX + 3, "CS3205 NetPong, Ball: %d, %d", ball.x, ball.y);
    mvprintw(OFFSETY-1, OFFSETX + WIDTH-25, "Player A: %d, Player B: %d", penaltyA, penaltyB);
        
    for (int i = OFFSETY; i < OFFSETY + HEIGHT; i++) {
        mvprintw(i, OFFSETX, "  ");
        mvprintw(i, OFFSETX + WIDTH - 1, "  ");
    }
    for (int i = OFFSETX; i < OFFSETX + WIDTH; i++) {
        mvprintw(OFFSETY, i, " ");
        mvprintw(OFFSETY + HEIGHT - 1, i, " ");
    }
    attroff(COLOR_PAIR(1));
    
    // Draw the ball
    mvprintw(OFFSETY + ball.y, OFFSETX + ball.x, "o");

    // Draw the paddle
    attron(COLOR_PAIR(2));
    for (int i = 0; i < paddleA.width; i++) {
        mvprintw(OFFSETY + HEIGHT-2, OFFSETX + paddleA.x + i, " ");
    }
    for (int i = 0; i < paddleB.width; i++) {
        mvprintw(OFFSETY+1, OFFSETX + paddleB.x + i, " ");
    }
    attroff(COLOR_PAIR(2));
    
    refresh();
}

void *move_ballB(void *args){
    while(game_running){
        game = (GameState){ball, paddleA, paddleB, penaltyA, penaltyB,game_running};
        send(client_fd, &game, sizeof(game), 0);
        while(recv(client_fd, &game, sizeof(game), 0)==0){

        }
        ball = game.ball;
        paddleA = game.paddleA;
        penaltyA = game.penaltyA;
        penaltyB = game.penaltyB;
        game_running = game.game_running;
    }
    return NULL;
}
void *move_ballA(void *args) {
    while (game_running) {

        while(recv(client_fd, &game, sizeof(game), 0)==0){

        }

        paddleB = game.paddleB;
        game_running = game.game_running;
        // Move the ball
        ball.x += ball.dx;
        ball.y += ball.dy;

        // Ball bounces off top
        // if (ball.y <= 1) {
        //     ball.dy = -ball.dy;
        // }
        if (ball.y == 2 && ball.x >= paddleB.x -1 && ball.x < paddleB.x + paddleB.width + 1) {
            ball.dy = -ball.dy;
        }
        
        // Ball goes past paddle (Game Over)
        if (ball.y <= 1) {
            penaltyA++;
            reset_ball();
        }

        // Ball bounces off left and right walls
        if (ball.x <= 2 || ball.x >= WIDTH - 2) {
            ball.dx = -ball.dx;
        }

        // Ball hits the paddle
        if (ball.y == HEIGHT - 3 && ball.x >= paddleA.x -1 && ball.x < paddleA.x + paddleA.width + 1) {
            ball.dy = -ball.dy;
        }
        
        // Ball goes past paddle (Game Over)
        if (ball.y >= HEIGHT - 2) {
            penaltyB++;
            reset_ball();
        }
        game = (GameState){ball, paddleA, paddleB, penaltyA, penaltyB,game_running};
        send(client_fd, &game, sizeof(game), 0);
        // Slow down ball movement
        usleep(100000); 
    }
    send(client_fd, &game, sizeof(game), 0);
    return NULL;
}

void update_paddleA(int ch) {
    if (ch == KEY_LEFT && paddleA.x > 2) {
        paddleA.x -= stride;  // Move paddle left
    }
    if (ch == KEY_RIGHT && paddleA.x < WIDTH - paddleA.width - 1) {
        paddleA.x += stride;  // Move paddle right
    }
}

void update_paddleB(int ch) {
    if (ch == KEY_LEFT && paddleB.x > 2) {
        paddleB.x -= stride;  // Move paddle left
    }
    if (ch == KEY_RIGHT && paddleB.x < WIDTH - paddleB.width - 1) {
        paddleB.x += stride;  // Move paddle right
    }
}

void reset_ball() {
    ball.x = WIDTH / 2;
    ball.y = HEIGHT / 2;
    ball.dx = 1;
    ball.dy = 1;
}