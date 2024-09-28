#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_joystick.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

//Constants
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 700
#define PLAYER_SPEED 7
#define COIN_COUNT 40
#define OBSTACLE_COUNT 60
#define ENEMY_COUNT 7
#define POWERUP_COUNT 3
#define POWERUP_DURATION 8000 //5 seconds in milliseconds
#define GRID_SIZE 100 //Adjust grid size based on game world dimensions
#define JOYSTICK_DEAD_ZONE 100000
#define FILENAME "highScore.txt"
#define CLOSE_DISTANCE_THRESHOLD 2000


//Here we're just defining the different sdl pointers that we shall use but without assigning a value to them
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;


SDL_Texture* playerTexture = NULL;
SDL_Texture* player2Texture = NULL;
SDL_Texture* enemyTexture = NULL;
SDL_Texture* enemyWeakTexture = NULL;
SDL_Texture* coinTexture = NULL;
SDL_Texture* iconTexture = NULL;
SDL_Texture* obstacleTexture = NULL;
SDL_Texture* powerUPTexture = NULL;
Mix_Chunk* coinSound = NULL;
TTF_Font* font = NULL;

typedef struct
{
    int x[POWERUP_COUNT];
    int y[POWERUP_COUNT];
    int isActive;
    Uint32 activationTime;
} PowerUp;

//Define the necessary variables as global variables
int playerX, playerY;
int player2X, player2Y;
int numOfEnemies;
int enemySpeed;
int obstacleCount;
int enemyX[ENEMY_COUNT], enemyY[ENEMY_COUNT];
int coins[COIN_COUNT][2];
int obstacles[OBSTACLE_COUNT][2];
int coinCount = 0;
int Player1score = 0;
int Player2score = 0;
int TotalScore = 0;
int isPaused = 0;
int life;
int life2;
int mode = 0;
int highScore = 0;
int joystickIndexPlayer2 = -1;
int multiPlayerCount = 2;
int selectLevel = 0; // Level selection
char powerUpFlag[] = "ON";
char name[20] = "";
char name2[20] = "";
PowerUp powerUp;
bool isUp = false;
bool isDown = false;
bool isLeft = false;
bool isRight = false;
bool pause = false;
bool isLife = true;
bool isLife2 = true;

SDL_Joystick* player1Controller = NULL;
SDL_Joystick* player2Controller = NULL;

//Function prototypes
void init();
void handleInput();
void update();
int checkCollisionWithPlayer(int x, int y);
int checkCollisionWithPlayer2(int x, int y);
int checkCollisionWithObstacles(int x, int y);
void render();
void pauseGame();
void Smile();
void adjustDirectionToAvoidObstacle(int* objectX, int* objectY, int speed);
void saveHighScore();
void loadHighScore();
void updateHighScore(int score);
void renderText(SDL_Renderer* introRenderer,const char* text, int x, int y, SDL_Color color);
void displayIntroScreen();
void gameoverScreen(int score1, int score2);

int main(int argc, char* args[]){

    displayIntroScreen();

    init();


    while (1)
    {
        handleInput();
        if (!isPaused)
        { //If the game wasn't paused, continue updating every movements
            update();
        }
        render();
        SDL_Delay(16); // Cap frame rate to approximately 60 FPS i.e 1000ms/60
    }

    // Cleanup and close SDL to save memory
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(player2Texture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(enemyWeakTexture);
    SDL_DestroyTexture(coinTexture);
    SDL_DestroyTexture(iconTexture);
    SDL_DestroyTexture(obstacleTexture);
    SDL_DestroyTexture(powerUPTexture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    Mix_CloseAudio();
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

void init()
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    // Load high score
    loadHighScore();

    // Initialize joystick subsystem
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);

    //check for available joysticks
    int numJoysticks = SDL_NumJoysticks();
    printf("Number of joysticks: %d\n", numJoysticks);

    //Open player 1 controller
    player1Controller = SDL_JoystickOpen(0);
    if (player1Controller == NULL)
    {
        printf("Failed to open player 1 controller! SDL_Error: %s\n", SDL_GetError());
    }

    //Open player 2 controller
    player2Controller = SDL_JoystickOpen(1);
    if (player2Controller == NULL)
    {
        printf("Failed to open player 1 controller! SDL_Error: %s\n", SDL_GetError());
    }



    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize. SDL_mixer Error: %s\n", Mix_GetError());
        //We initialize the mixer and check if the initialization was successful
    }

    window = SDL_CreateWindow("Pac-Man Sensei", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN || SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface* iconSurface = IMG_Load("pacman.png");
    iconTexture = SDL_CreateTextureFromSurface(renderer, iconSurface);
    SDL_SetWindowIcon(window, iconSurface);
    SDL_FreeSurface(iconSurface);

    SDL_Surface* playerSurface = IMG_Load("PAC_MAN D.bmp");
    playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    SDL_FreeSurface(playerSurface);

    SDL_Surface* player2Surface = IMG_Load("test.png");
    player2Texture = SDL_CreateTextureFromSurface(renderer, player2Surface);
    SDL_FreeSurface(player2Surface);

    SDL_Surface* enemySurface = IMG_Load("enemy.png");
    enemyTexture = SDL_CreateTextureFromSurface(renderer, enemySurface);
    SDL_FreeSurface(enemySurface);

    SDL_Surface* enemyWeakSurface = IMG_Load("weak.jpg");
    enemyWeakTexture = SDL_CreateTextureFromSurface(renderer, enemyWeakSurface);
    SDL_FreeSurface(enemyWeakSurface);


    SDL_Surface* coinSurface = IMG_Load("coin.png");
    coinTexture = SDL_CreateTextureFromSurface(renderer, coinSurface);
    SDL_FreeSurface(coinSurface);

    SDL_Surface* obstacleSurface = IMG_Load("obstacle2.png"); // You need to have an obstacle image
    obstacleTexture = SDL_CreateTextureFromSurface(renderer, obstacleSurface);
    SDL_FreeSurface(obstacleSurface);

    SDL_Surface* powerupSurface = IMG_Load("booster.bmp");
    powerUPTexture = SDL_CreateTextureFromSurface(renderer, powerupSurface);
    SDL_FreeSurface(powerupSurface);

    font = TTF_OpenFont("arial.ttf", 24); // We load the font type and size

    //Now we add background music to the game(yeah I have time sensei *-*)
    Mix_Music* backgroundMusic = Mix_LoadMUS("background_music.wav");
    if(backgroundMusic == NULL)
    {
        printf("Failed to load background_music! SDL_mixer error: %s\n", Mix_GetError());
        //Music didn't load successfully so we get the error code that occurred
    }

    if(!isPaused)
    {
        Mix_PlayMusic(backgroundMusic, -1); //-1 means loop indefinitely
    }
    else
    {
        Mix_PlayMusic(NULL, -1);
    }

    for(int i = 0; i < POWERUP_COUNT; i++)
    {
        powerUp.x[i] = rand() % SCREEN_WIDTH;
        powerUp.y[i] = rand() % SCREEN_HEIGHT;
    }

    powerUp.isActive = 0;
    powerUp.activationTime = 0;


    srand(time(NULL));//We seed for random time function



    //Define player's positions at the middle of the screen
    playerX = 10;
    playerY = 650;

    player2X = 20;
    player2Y = 650;

    life = 100;
    life2 = 100;

    Player1score = 0;
    Player2score = 0;
    //Define ennemies positions at random locations within the window
    for (int i = 0; i < numOfEnemies; i++)
    {
        //We make sure that no ennemy spawns at the player's position in the beginning of the game
        if((enemyX[i] != playerX && enemyY[i] != playerY) || (enemyX[i] != player2X && enemyY[i] != player2Y) ||(enemyX[i] > 0 && enemyX[i] < SCREEN_WIDTH-32 && enemyY[i] > 0 && enemyY[i] < SCREEN_HEIGHT-32))
        {
            enemyX[i] = rand() % SCREEN_WIDTH-100;
            enemyY[i] = rand() % SCREEN_HEIGHT-100;
        }
    }

    //Define coin's positions at random locations within the window
    for (int i = 0; i < COIN_COUNT; i++)
    {
        coins[i][0] = rand() % SCREEN_WIDTH;
        coins[i][1] = rand() % SCREEN_HEIGHT;
    }

    //Define obstacle positions at random locations within the window
    for (int i = 0; i < obstacleCount; i++)
    {
        do
        {
            obstacles[i][0] = rand() % SCREEN_WIDTH;
            obstacles[i][1] = rand() % SCREEN_HEIGHT-100;
        }while((abs(obstacles[i][0] - playerX) < 100 && abs(obstacles[i][1] - playerY) < 100) || (abs(obstacles[i][0] - enemyX[numOfEnemies]) < 100 && abs(obstacles[i][1] - enemyY[numOfEnemies]) < 100));
    }
}

void handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
            printf("Your score is: %d\n%s: %d\n%s: %d", Player1score + Player2score, name, Player1score, name2, Player2score);

            exit(0);
        }

        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_SPACE)
            {
                pauseGame();
            }
        }

        // Reduce memory usage when game is minimized
        switch (event.window.event)
        {
            case SDL_WINDOWEVENT_MINIMIZED:
                while (SDL_WaitEvent(&event))
                {
                    if (event.window.event == SDL_WINDOWEVENT_RESTORED)
                    {
                        break;
                    }
                }
                break;
        }
        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL); //Seed for the keyboard state(which key is being pressed)

        //Manage player's movement with keyboard
        if (!isPaused)
        {
            if (currentKeyStates[SDL_SCANCODE_UP] && playerY > 0 /*&& !(checkCollisionWithObstacles(playerX, playerY - PLAYER_SPEED))*/)
            {
                playerY -= PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_DOWN] && playerY < SCREEN_HEIGHT - 32 /*&& !(checkCollisionWithObstacles(playerX, playerY + PLAYER_SPEED))*/)
            {
                playerY += PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_LEFT] && playerX > 0 /*&& !(checkCollisionWithObstacles(playerX - PLAYER_SPEED, playerY))*/)
            {
                playerX -= PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_RIGHT] && playerX < SCREEN_WIDTH - 32 /*&& !(checkCollisionWithObstacles(playerX + PLAYER_SPEED, playerY))*/)
            {
                playerX += PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_W] && player2Y > 0 /*&& !(checkCollisionWithObstacles(playerX, playerY - PLAYER_SPEED))*/)
            {
                player2Y -= PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_S] && player2Y < SCREEN_HEIGHT - 32 /*&& !(checkCollisionWithObstacles(playerX, playerY + PLAYER_SPEED))*/)
            {
                player2Y += PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_A] && player2X > 0 /*&& !(checkCollisionWithObstacles(playerX - PLAYER_SPEED, playerY))*/)
            {
                player2X -= PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_D] && player2X < SCREEN_WIDTH - 32 /*&& !(checkCollisionWithObstacles(playerX + PLAYER_SPEED, playerY))*/)
            {
                player2X += PLAYER_SPEED;
            }
        }

        //Manage player's movement with joystick
        if (event.type == SDL_JOYBUTTONDOWN)
        {
            // Check if it's a directional button pressed
            if (event.jbutton.type == SDL_JOYBUTTONDOWN)
            {
                switch (event.jbutton.button)
                {
                    case SDL_CONTROLLER_BUTTON_DPAD_UP:
                            isUp = true;
                    break;
                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                            isDown = true;
                    break;
                    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                            isLeft = true;
                    break;
                    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                            isRight = true;
                    break;
                }
            }
        }
            else if (event.type == SDL_JOYBUTTONUP)
            {
                if (event.jbutton.type == SDL_JOYBUTTONUP)
                {
                    switch (event.jbutton.button)
                    {
                        case SDL_CONTROLLER_BUTTON_DPAD_UP:
                            isUp = false;
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                            isDown = false;
                        break;
                        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                            isLeft = false;
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                            isRight = false;
                        break;
                    }
                }
            }
            if (isUp && playerY > 0)
            {
                playerY -= PLAYER_SPEED; //move up
            }
            else if (isDown && playerY < SCREEN_HEIGHT - 32)
            {
                playerY += PLAYER_SPEED; //move down
            }
            else if (isLeft && playerX > 0)
            {
                playerX -= PLAYER_SPEED; //Move left
            }
            else if (isRight && playerX < SCREEN_WIDTH - 32)
            {
                playerX += PLAYER_SPEED; //Move right
            }

        // Handle joystick events
        if (event.type == SDL_JOYAXISMOTION) {
            // X-axis movement
            if (event.jaxis.axis == 0) {
                if (event.jaxis.value < -JOYSTICK_DEAD_ZONE && playerX > 0) {
                    playerX -= PLAYER_SPEED; // Move left
                } else if (event.jaxis.value > JOYSTICK_DEAD_ZONE && playerX < SCREEN_WIDTH - 32) {
                    playerX += PLAYER_SPEED; // Move right
                }
            }
            // Y-axis movement
            else if (event.jaxis.axis == 1) {
                if (event.jaxis.value < -JOYSTICK_DEAD_ZONE && playerY > 0) {
                    playerY -= PLAYER_SPEED; // Move up
                } else if (event.jaxis.value > JOYSTICK_DEAD_ZONE && playerY < SCREEN_HEIGHT - 32) {
                    playerY += PLAYER_SPEED; // Move down
                }
            }
        }
        // input from second controller(player2)
        int xAxis2 = SDL_JoystickGetAxis(player2Controller, 0);
        int yAxis2 = SDL_JoystickGetAxis(player2Controller, 1);
        if (xAxis2 < -25000 && player2X > 0)
        {
            player2X -= PLAYER_SPEED;
        }
        if (xAxis2 > 25000 && player2X < SCREEN_WIDTH - 32)
        {
            player2X += PLAYER_SPEED;
        }
        if (yAxis2 < -25000 && player2Y > 0)
        {
            player2Y -= PLAYER_SPEED;
        }
        if (yAxis2 > 25000 && player2Y < SCREEN_HEIGHT - 32)
        {
            player2Y += PLAYER_SPEED;
        }

        // input from first controller(player1)
        int xAxis1 = SDL_JoystickGetAxis(player1Controller, 0);
        int yAxis1 = SDL_JoystickGetAxis(player1Controller, 1);
        if (xAxis1 < -40000 && playerX > 0)
        {
            playerX -= PLAYER_SPEED; //Moving left
        }
        if (xAxis1 > 40000 && playerX < SCREEN_WIDTH - 32)
        {
            playerX += PLAYER_SPEED; //Moving right
        }
        if (yAxis1 < -40000 && playerY > 0)
        {
            playerY -= PLAYER_SPEED; //Moving up
        }
        if (yAxis1 > 40000 && playerY < SCREEN_HEIGHT - 32)
        {
            playerY += PLAYER_SPEED; //Moving down
        }

    }
}

void update()
{
    Mix_Chunk* coinSound = Mix_LoadWAV("coin_sound.wav");
    Mix_Chunk* powerupSound = Mix_LoadWAV("powerup.wav");



    if(coinSound == NULL)
     {
         printf("Failed to load coin collection sound! SDL_Mixer error: %s\n", Mix_GetError());
     }

    // Update enemy movement
    if (mode == 1)
    {
        for (int i = 0; i < numOfEnemies; i++)
        {
            //calculate direction from ennemy to player
            int deltaX = playerX - enemyX[i];
            int deltaY = playerY - enemyY[i];

            //Normalize the direction vector
            int length = sqrt(deltaX * deltaX + deltaY * deltaY);
            if(!powerUp.isActive)
            {
                if(length < 2000)
                {
                    if(length != 0)
                    {
                        deltaX = (deltaX * enemySpeed) / length;
                        deltaY = (deltaY * enemySpeed) / length;
                    }

                    enemyX[i] += deltaX;
                    enemyY[i] += deltaY;
                }
            }
            else
            {
                if (length < 2000)
                {
                    if (length != 0)
                    {
                        deltaX = (deltaX * enemySpeed) / length;
                        deltaY = (deltaY * enemySpeed) / length;
                    }
                    //restrict enemies movement to screen size
                    if (enemyX[i] > 0 && enemyX[i] < SCREEN_WIDTH)
                    {
                        enemyX[i] -= deltaX;
                    }
                    if (enemyY[i] > 0 && enemyY[i] < SCREEN_HEIGHT)
                    {
                        enemyY[i] -= deltaY;
                    }
                }
            }

            //check for collision with obstacles
            if (checkCollisionWithObstacles(enemyX[i], enemyY[i]))
            {
                //If collision, adjust enemy's direction to navigate around the obstacle

                adjustDirectionToAvoidObstacle(&enemyX[i], &enemyY[i], enemySpeed);
            }
        }
    }
    else if (mode == 2)
    {
        // Calculate distance between each enemy and each player
        float distance[numOfEnemies][2]; //Store distances for each enemy and player
        for (int i = 0; i < numOfEnemies; i++)
        {
            for (int j = 0; j < multiPlayerCount; j++)// Loop through players
            {
                // Calculate distance between enemy i and player j
                int dx = enemyX[i] - (j == 0 ? playerX : player2X);
                int dy = enemyY[i] - (j == 0 ? playerY : player2Y);
                distance[i][j] = sqrt(dx * dx + dy * dy); // Euclidean distance
            }
        }

        // Update enemy movement
        for (int i = 0; i < numOfEnemies; i++)
        {
            // Find the closest player to the current enemy
            int closestPlayer = (distance[i][0] < distance[i][1]) ? 0 : 1;

            // If th e closest player is within a certain distance, move towards that player
            if (distance[i][closestPlayer] < CLOSE_DISTANCE_THRESHOLD && !powerUp.isActive)
            {
                // Adjust enemy position to move towards the closest player
                int dx = (closestPlayer == 0 ? playerX : player2X) - enemyX[i];
                int dy = (closestPlayer == 0 ? playerY : player2Y) - enemyY[i];
                float length = sqrt(dx * dx + dy * dy);
                int moveX = (int)(dx / length * enemySpeed);
                int moveY = (int)(dy / length * enemySpeed);
                enemyX[i] += moveX;
                enemyY[i] += moveY;
            }
            else if (distance[i][closestPlayer] < CLOSE_DISTANCE_THRESHOLD && powerUp.isActive)
            {
               // Adjust enemy position to move away from the closest player
                int dx = enemyX[i] - (closestPlayer == 0 ? playerX : player2X);
                int dy = enemyY[i] - (closestPlayer == 0 ? playerY : player2Y);
                float length = sqrt(dx * dx + dy * dy);
                int moveX = (int)(dx / length * enemySpeed);
                int moveY = (int)(dy / length * enemySpeed);
                if (enemyX[i] > 0 && enemyX[i] < SCREEN_WIDTH - 32)
                {
                    enemyX[i] += moveX;
                }
                if (enemyY[i] > 0 && enemyY[i] < SCREEN_HEIGHT - 32)
                {
                    enemyY[i] += moveY;
                }
            }

            if (checkCollisionWithObstacles(enemyX[i], enemyY[i]))
            {
                adjustDirectionToAvoidObstacle(&enemyX[i], &enemyY[i], enemySpeed);
            }
        }
    }

     // Check for collisions with powerups
    for (int i = 0; i < POWERUP_COUNT; i++)
    {
        if (checkCollisionWithPlayer(powerUp.x[i], powerUp.y[i]) || checkCollisionWithPlayer2(powerUp.x[i], powerUp.y[i]))
        {
            if (!powerUp.isActive)
            {
                powerUp.x[i] = -50;
                powerUp.y[i] = -50;
                //Activate power-up effect
                powerUp.isActive = 1;

                powerUp.activationTime = SDL_GetTicks();

                Mix_PlayChannel(-1, powerupSound, 0);
            }

        }
    }


    // Check for collision  between enemies & player
    for (int i = 0; i < numOfEnemies; i++)
    {
        if (mode == 1)
        {
           if (!powerUp.isActive && checkCollisionWithPlayer(enemyX[i], enemyY[i]))
            {
                life--;
                if(life == 0)
                {
                    isLife = false;
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                    gameoverScreen(Player1score, Player2score);
//                    printf("GAME OVER!!!\nYour score: %d\n%s: %d\n%s: %d", Player1score + Player2score, name, Player1score, name2, Player2score);
//                    exit(1);
//                    system("pause");
                }
            }
        }
        else if(mode == 2)
        {
            if (!powerUp.isActive && checkCollisionWithPlayer(enemyX[i], enemyY[i]))
            {
                life--;
                if(life == 0)
                {
                    isLife = false;
                    multiPlayerCount -= 1;
                        playerX = -2500;
                        playerY = -2500;
                }
            }
            else if (powerUp.isActive && checkCollisionWithPlayer(enemyX[i], enemyY[i]))
            {
                enemyX[i] = SCREEN_WIDTH;
                enemyY[i] = SCREEN_HEIGHT-10;
            }

            if (!powerUp.isActive && checkCollisionWithPlayer2(enemyX[i], enemyY[i]))
            {
                life2--;
                if(life2 == 0)
                {
                    isLife2 = false;
                    multiPlayerCount -= 1;
                    player2X = -2500;
                    player2Y = -2500;
                }
            }
            else if (powerUp.isActive && checkCollisionWithPlayer2(enemyX[i], enemyY[i]))
            {
                enemyX[i] = SCREEN_WIDTH;
                enemyY[i] = SCREEN_HEIGHT-10;
            }

            if (multiPlayerCount == 0)
            {
                printf("Good Game Comrade! Your Scores: %d\n%s: %d\n%s: %d ", Player1score + Player2score, name, Player1score, name2, Player2score);
            }

            if (!isLife && !isLife2)
            {
                gameoverScreen(Player1score, Player2score);
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
            }
        }
    }




    //check collision with obstacles while considering power up effect
    for (int i = 0; i < obstacleCount; i++)
    {
        if(!powerUp.isActive && checkCollisionWithPlayer(obstacles[i][0], obstacles[i][1]))
        {
            //Handle obstacle collision with no power up
            //It sends the player to one end of the obstacle if the player tries to force through when a collision occured

            if(playerX < obstacles[i][0])
            {
                //PLayer is moving from left to right, so block movement to the right
                playerX = obstacles[i][0] - 32;/*Player's width*/
            }
            else if(playerX > obstacles[i][0])
            {
            //PLayer is moving from right to left, so block movement to the left
                playerX = obstacles[i][0] + 32;/*Obstacle width*/
            }

            if(playerY < obstacles[i][1])
            {
                //PLayer is moving from top to buttom, so block movement downward
                playerY = obstacles[i][1] - 32;/*Player's height*/
            }
            else if(playerY > obstacles[i][1])
            {
            //PLayer is moving from buttom to top, so block movement upward
                playerY = obstacles[i][1] + 32;/*Obstacle height*/
            }
        }

        if(!powerUp.isActive && checkCollisionWithPlayer2(obstacles[i][0], obstacles[i][1]))
        {
            //Handle obstacle collision with no power up
            //It sends the player to one end of the obstacle if the player tries to force through when a collision occured

            if(player2X < obstacles[i][0])
            {
                //PLayer is moving from left to right, so block movement to the right
                player2X = obstacles[i][0] - 32;/*Player's width*/
            }
            else if(player2X > obstacles[i][0])
            {
            //PLayer is moving from right to left, so block movement to the left
                player2X = obstacles[i][0] + 32;/*Obstacle width*/
            }

            if(player2Y < obstacles[i][1])
            {
                //PLayer is moving from top to buttom, so block movement downward
                player2Y = obstacles[i][1] - 32;/*Player's height*/
            }
            else if(player2Y > obstacles[i][1])
            {
            //PLayer is moving from buttom to top, so block movement upward
                player2Y = obstacles[i][1] + 32;/*Obstacle height*/
            }
        }
    }

    //Check if power up effect has expired
    if (powerUp.isActive && (SDL_GetTicks() - powerUp.activationTime >= POWERUP_DURATION))
    {
        powerUp.isActive = 0;
    }

    // Check for collisions with coins
    for (int i = 0; i < COIN_COUNT; i++)
    {
        if ((playerX < coins[i][0] + 34 && playerX + 32 > coins[i][0] && playerY < coins[i][1] + 34 && playerY + 32 > coins[i][1]))
        {
            coins[i][0] = -120; // "Remove" the coin
            coins[i][1] = -5;
            Player1score++;
            Mix_PlayChannel(-1, coinSound, 0);
        }
        if ((player2X < coins[i][0] + 34 && player2X + 32 > coins[i][0] && player2Y < coins[i][1] + 34 && player2Y + 32 > coins[i][1]))
        {
            coins[i][0] = -120; // "Remove" the coin
            coins[i][1] = -5;
            Player2score++;
            Mix_PlayChannel(-1, coinSound, 0);
        }
    }
    TotalScore = Player1score + Player2score;
    if (TotalScore == COIN_COUNT)//If the player collected all the coins
    {
        printf("You won comrade\nScore: %d", TotalScore);
        exit(1);
        system("pause");
    }
    if (Player1score > Player2score)
    {
        updateHighScore(Player1score);
    }
    else
    {
        updateHighScore(Player2score);
    }
}

int checkCollisionWithPlayer(int x, int y)
{
    return ((playerX < x + 32 && playerX + 32 > x && playerY < y + 32 && playerY + 32 > y));
}

int checkCollisionWithPlayer2(int x, int y)
{
    return ((player2X < x + 32 && player2X + 32 > x && player2Y < y + 32 && player2Y + 32 > y));
}

int checkCollisionWithObstacles(int x, int y)
{
    for (int i = 0; i < obstacleCount; i++)
    {
        if (x < obstacles[i][0] + 32 && x + 32 > obstacles[i][0] && y < obstacles[i][1] + 32 && y + 32 > obstacles[i][1])
        {
            return 1; // Collision with obstacle
        }
    }

    return 0; // No collision with obstacles
}

void render()
{
    SDL_RenderClear(renderer);

    // Draw obstacles
    for (int i = 0; i < obstacleCount; i++)
    {
        SDL_Rect obstacleRect = { obstacles[i][0], obstacles[i][1], 32, 32 };
        SDL_RenderCopy(renderer, obstacleTexture, NULL, &obstacleRect);
    }

    // Draw coins
    for (int i = 0; i < COIN_COUNT; i++)
    { //loop through the number of coins specified
        if (coins[i][0] != -1)
        {
            SDL_Rect coinRect = { coins[i][0], coins[i][1], 32, 32 }; //create a rectangle for the coin
            SDL_RenderCopy(renderer, coinTexture, NULL, &coinRect); //Insert the coin's texture in the coins rect and renders it
        }
    }

    // Draw powerups
    for (int i = 0; i < POWERUP_COUNT; i++)
    { //loop through the number of powerups specified
        if (powerUp.x[i] != -5)
        {
            SDL_Rect powerUpRect = {powerUp.x[i] - 32 / 2, powerUp.y[i] - 32 / 2, 32, 32};

            SDL_RenderCopy(renderer, powerUPTexture, NULL, &powerUpRect);
        }
    }

    // Draw enemies, based on whether the power-up is active or not

    SDL_Texture* currentEnemyTexture = !powerUp.isActive ? enemyTexture : enemyWeakTexture;
    for (int i = 0; i < numOfEnemies; i++)
    {
            SDL_Rect enemyRect = { enemyX[i], enemyY[i], 32, 32 };
            SDL_RenderCopy(renderer, currentEnemyTexture, NULL, &enemyRect);
    }

    // Draw players
    if (mode == 1)
    {
        SDL_Rect playerRect = { playerX, playerY, 32, 32 };
        SDL_RenderCopy(renderer, playerTexture, NULL, &playerRect);
    }
    else if (mode == 2)
    {
        SDL_Rect playerRect = { playerX, playerY, 32, 32 };
        SDL_RenderCopy(renderer, playerTexture, NULL, &playerRect);

        SDL_Rect player2Rect = { player2X, player2Y, 32, 32 };
        SDL_RenderCopy(renderer, player2Texture, NULL, &player2Rect);
    }




    // Render score
    SDL_Color textColor = {255, 0, 255, 255};
    char scoreText[20];
    char scoreText2[20];
    if (mode == 1)
    {
        sprintf(scoreText, "Score: %d", Player1score);

        SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText, textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);

        SDL_Rect textRect = {SCREEN_WIDTH - 120, 10, 100, 30};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);
    }
    else if (mode == 2)
    {
        sprintf(scoreText, "Player1: %d", Player1score);
        sprintf(scoreText2, "Player2: %d", Player2score);

        SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText, textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);

        SDL_Rect textRect = {SCREEN_WIDTH - 120, 40, 100, 30};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);

        SDL_Surface* textSurface2 = TTF_RenderText_Solid(font, scoreText2, textColor);
        SDL_Texture* textTexture2 = SDL_CreateTextureFromSurface(renderer, textSurface2);
        SDL_FreeSurface(textSurface2);

        SDL_Rect textRect2 = {SCREEN_WIDTH - 120, 10, 100, 30};
        SDL_RenderCopy(renderer, textTexture2, NULL, &textRect2);
        SDL_DestroyTexture(textTexture2);

    }


    //Display highscore
    char highScoreText[20];
    sprintf(highScoreText, "High Score: %d", highScore);
    SDL_Surface* HightextSurface = TTF_RenderText_Solid(font, highScoreText, textColor);
    SDL_Texture* HightextTexture = SDL_CreateTextureFromSurface(renderer, HightextSurface);
    SDL_Rect HightextRect = {SCREEN_WIDTH - 150, SCREEN_HEIGHT - 30, HightextSurface->w, HightextSurface->h};
    SDL_RenderCopy(renderer, HightextTexture, NULL, &HightextRect);

    SDL_FreeSurface(HightextSurface);
    SDL_DestroyTexture(HightextTexture);

    if (mode == 1)
    {
        // Render life count player 1
        SDL_Color lifeColor = {123, 150, 213, 255};
        char lifeText[20];
        sprintf(lifeText, "PLAYER: %d", life);

        SDL_Surface* lifeSurface = TTF_RenderText_Solid(font, lifeText, lifeColor);
        SDL_Texture* lifeTexture = SDL_CreateTextureFromSurface(renderer, lifeSurface);
        SDL_FreeSurface(lifeSurface);

        SDL_Rect lifeRect = {10, 10, 100, 30};
        SDL_RenderCopy(renderer, lifeTexture, NULL, &lifeRect);
        SDL_DestroyTexture(lifeTexture);
    }
    else if(mode == 2)
    {
        // Render life count player 1
        SDL_Color lifeColor = {123, 150, 213, 255};
        char lifeText[15];
        sprintf(lifeText, "PLAYER 1: %d", life);

        SDL_Surface* lifeSurface = TTF_RenderText_Solid(font, lifeText, lifeColor);
        SDL_Texture* lifeTexture = SDL_CreateTextureFromSurface(renderer, lifeSurface);
        SDL_FreeSurface(lifeSurface);

        SDL_Rect lifeRect = {10, 10, 100, 30};
        SDL_RenderCopy(renderer, lifeTexture, NULL, &lifeRect);
        SDL_DestroyTexture(lifeTexture);

        // Render life count player 2
        char lifeText2[15];
        sprintf(lifeText2, "PLAYER 2: %d", life2);

        SDL_Surface* lifeSurface2 = TTF_RenderText_Solid(font, lifeText2, lifeColor);
        SDL_Texture* lifeTexture2 = SDL_CreateTextureFromSurface(renderer, lifeSurface2);
        SDL_FreeSurface(lifeSurface2);

        SDL_Rect lifeRect2 = {10, 30, 100, 30};
        SDL_RenderCopy(renderer, lifeTexture2, NULL, &lifeRect2);
        SDL_DestroyTexture(lifeTexture2);
    }


    if (powerUp.isActive)
    {
        //We render a small text when powerup bonus is on
        SDL_Color powerUpFlagColor = {0, 255, 0, 255};
        char powerUpFlagText[3];
        sprintf(powerUpFlagText, "%s", powerUpFlag);

        SDL_Surface* powerUpFlagSurface = TTF_RenderText_Solid(font, powerUpFlagText, powerUpFlagColor);
        SDL_Texture* powerUpFlagTexture = SDL_CreateTextureFromSurface(renderer, powerUpFlagSurface);
        SDL_FreeSurface(powerUpFlagSurface);

        SDL_Rect powerUpFlagRect = {SCREEN_WIDTH/2, 10, 50, 30};
        SDL_RenderCopy(renderer, powerUpFlagTexture, NULL, &powerUpFlagRect);
        SDL_DestroyTexture(powerUpFlagTexture);
    }

        SDL_RenderPresent(renderer);
}

void pauseGame()
{
    isPaused = !isPaused;
}

void Smile()
{
    printf("     .-\"\"\"\"\"-.\t\t\t     .-\"\"\"\"\"-.\t\t\t     .-\"\"\"\"\"-.\n");
    printf("   .'          '.\t\t   .'          '.\t\t   .'          '.\n");
    printf("  /   O      O   \\\t\t  /   O      O   \\\t\t  /   O      O   \\\n");
    printf(" :                :\t\t :                :\t\t :                :\n");
    printf(" |                | \t\t |                | \t\t |                | \n");
    printf(" : ',          ,' :\t\t : ',          ,' :\t\t : ',          ,' :\n");
    printf("  \\  '-......-'  /\t\t  \\  '-......-'  /\t\t  \\  '-......-'  /\n");
    printf("   '.          .'\t\t   '.          .'\t\t   '.          .'\n");
    printf("     '-......-'\t\t\t     '-......-'\t\t\t     '-......-'\n");
}

//Let's create a function to adjust direction to navigate around onstacles
void adjustDirectionToAvoidObstacle(int* objectX, int* objectY, int speed)
{
    int newDirection = rand() % 4;

    switch (newDirection)
    {
        case 0:
            //Move up
            *objectY -= speed*2;
            break;
        case 1:
            //move down
            *objectY += speed*2;
            break;
        case 2:
            //move left
            *objectX -= speed*2;
            break;
        case 3:
            //move right
            *objectX += speed*2;
            break;
    }
}

void saveHighScore()
{
    FILE* file = fopen(FILENAME, "w");
    if (file != NULL)
    {
        fprintf(file, "%d\n", highScore);
        fclose(file);
    }
}
void loadHighScore()
{
    FILE* file = fopen(FILENAME, "r");
    if (file != NULL)
    {
        fscanf(file, "%d", &highScore);
        fclose(file);
    }
}

void updateHighScore(int score)
{
    if (score > highScore)
    {
        highScore = score;
        saveHighScore();
    }
}

void renderText(SDL_Renderer* introRenderer, const char* text, int x, int y, SDL_Color color)
{
    SDL_Surface* introSurface = TTF_RenderText_Blended(font, text, color);
    SDL_Texture* introTexture = SDL_CreateTextureFromSurface(introRenderer, introSurface);
    SDL_Rect introRect = { x, y, introSurface->w, introSurface->h };
    SDL_RenderCopy(introRenderer, introTexture, NULL, &introRect);
    SDL_FreeSurface(introSurface);
    SDL_DestroyTexture(introTexture);
}

void displayIntroScreen()
{
    //initialize sdl
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
            printf("SDL Initialization failed: %s\n", SDL_GetError());
            return;
    }

    //Init sdl_ttf
    if (TTF_Init() < 0)
    {
        printf("SDL_ttf initialization failed: %s\n", TTF_GetError());
        SDL_Quit();
        return;
    }

    //Load font
    TTF_Font* introFont = TTF_OpenFont("arial.ttf", 24);
    if(introFont == NULL)
    {
        printf("Failed to load font: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return;
    }

    //Create window
    SDL_Window* introWindow = SDL_CreateWindow("Pac-Man Intro", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if(introWindow == NULL)
    {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        TTF_Quit();
        return;
    }
    //Create renderer
    SDL_Renderer* introRenderer = SDL_CreateRenderer(introWindow, -1, SDL_RENDERER_ACCELERATED);
    if(introRenderer == NULL)
    {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(introWindow);
        SDL_Quit();
        TTF_Quit();
        return;
    }



    // Clear the screen
    SDL_SetRenderDrawColor(introRenderer, 0, 0, 0, 255); // Black color
    SDL_RenderClear(introRenderer);

    SDL_Color textColor = {255, 255, 255}; // White color
    SDL_Surface* introSurface = TTF_RenderText_Blended(introFont, "Welcome to Pac-Man Ultimate!", textColor);
    if(introSurface == NULL)
    {
        printf("Text rendering failed: %s\n", TTF_GetError());
        SDL_DestroyRenderer(introRenderer);
        SDL_DestroyWindow(introWindow);
        SDL_Quit();
        return;
    }

    SDL_Texture* introTexture = SDL_CreateTextureFromSurface(introRenderer, introSurface);
    if(introTexture == NULL)
    {
        printf("Texture creation failed: %s\n", SDL_GetError());

        SDL_FreeSurface(introSurface);
        SDL_DestroyRenderer(introRenderer);
        SDL_DestroyWindow(introWindow);
        SDL_Quit();
        return;
    }

    //Render the texture
    SDL_Rect introRect = {100, 100, introSurface->w, introSurface->h};
    SDL_RenderCopy(introRenderer, introTexture, NULL, &introRect);

    SDL_RenderPresent(introRenderer);
    // Render introductory messages
   // renderText(introRenderer," Welcome to Pac-Man Ultimate ! ", 100, 100, textColor);
 //   renderText(introRenderer," Instructions : ", 100, 150, textColor);
//    renderText(introRenderer," Use arrow keys and WASD(in two players mode) to move Pac-Man. ", 100, 200, textColor);
//    renderText(introRenderer," Press Enter to start the game. ", 100, 250, textColor);
//
//
            // Wait for Enter key to start the game
    SDL_Event e;
    int quit = 0;
    while (!quit && SDL_WaitEvent(&e))
    {
        switch (e.type)
        {
            case SDL_QUIT:
                quit = 1;
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_RETURN) {
                    quit = 1; // Exit introductory screen loop
                }
                break;
        }
    }

        // Clear the screen
        SDL_SetRenderDrawColor(introRenderer, 0, 0, 0, 255); // Black color
        SDL_RenderClear(introRenderer);

        // Render player mode selection prompt
        SDL_Surface* modeSurface = TTF_RenderText_Blended(introFont, "Choose game mode: ", textColor);
        SDL_Texture* modeTexture = SDL_CreateTextureFromSurface(introRenderer, modeSurface);
        SDL_Rect modeRect = {100, 100, modeSurface->w, modeSurface->h};

        SDL_RenderCopy(introRenderer, modeTexture, NULL, &modeRect);

        SDL_Surface* singlePlayerSurface = TTF_RenderText_Blended(introFont, "1. Single player", textColor);
        SDL_Texture* singlePlayerTexture = SDL_CreateTextureFromSurface(introRenderer, singlePlayerSurface);
        SDL_Rect singlePlayerRect = {100, 150, singlePlayerSurface->w, singlePlayerSurface->h};

        SDL_RenderCopy(introRenderer, singlePlayerTexture, NULL, &singlePlayerRect);

        SDL_Surface* twoPlayerSurface = TTF_RenderText_Blended(introFont, "2. Two players", textColor);
        SDL_Texture* twoPlayerTexture = SDL_CreateTextureFromSurface(introRenderer, twoPlayerSurface);
        SDL_Rect twoPlayerRect = {100, 200, twoPlayerSurface->w, twoPlayerSurface->h};

        SDL_RenderCopy(introRenderer, twoPlayerTexture, NULL, &twoPlayerRect);

        SDL_RenderPresent(introRenderer);

//    renderText(introRenderer," Choose game mode : ", 100, 100, textColor);
//    renderText(introRenderer," 1. Single player ", 100, 150, textColor);
//    renderText(introRenderer," 2. Two players ", 100, 200, textColor);
//
    // Player mode selection
    while(mode == 0)
    {
        if (SDL_WaitEvent(&e))
        {
            if(e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_1 || e.key.keysym.sym == SDLK_KP_1))
            {
                mode = 1; // Single player mode
            }
            else if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_2 || e.key.keysym.sym == SDLK_KP_2))
            {
                mode = 2; // Two players mode
            }
        }
    }


    if (mode == 1 || mode == 2)
    {
        // Render level selection prompt
        SDL_SetRenderDrawColor(introRenderer, 0, 0, 0, 255); // Black color
        SDL_RenderClear(introRenderer);

        SDL_Surface* selectLevelSurface = TTF_RenderText_Blended(introFont, "Select Your Level: ", textColor);
        SDL_Texture* selectLevelTexture = SDL_CreateTextureFromSurface(introRenderer, selectLevelSurface);
        SDL_Rect selectLevelRect = {100, 100, selectLevelSurface->w, selectLevelSurface->h};

        SDL_RenderCopy(introRenderer, selectLevelTexture, NULL, &selectLevelRect);

        SDL_Surface* easySurface = TTF_RenderText_Blended(introFont, "1. Easy ", textColor);
        SDL_Texture* easyTexture = SDL_CreateTextureFromSurface(introRenderer, easySurface);
        SDL_Rect easyRect = {100, 150, easySurface->w, easySurface->h};

        SDL_RenderCopy(introRenderer, easyTexture, NULL, &easyRect);

        SDL_Surface* mediumSurface = TTF_RenderText_Blended(introFont, "2. Medium ", textColor);
        SDL_Texture* mediumTexture = SDL_CreateTextureFromSurface(introRenderer, mediumSurface);
        SDL_Rect mediumRect = {100, 200, mediumSurface->w, mediumSurface->h};

        SDL_RenderCopy(introRenderer, mediumTexture, NULL, &mediumRect);

        SDL_Surface* hardSurface = TTF_RenderText_Blended(introFont, "3. Hard ", textColor);
        SDL_Texture* hardTexture = SDL_CreateTextureFromSurface(introRenderer, hardSurface);
        SDL_Rect hardRect = {100, 250, hardSurface->w, hardSurface->h};

        SDL_RenderCopy(introRenderer, hardTexture, NULL, &hardRect);

        SDL_RenderPresent(introRenderer);
//
        // Level selection
        while (selectLevel == 0)
        {
            if (SDL_WaitEvent(&e))
            {
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym <= SDLK_3)
                {
                    selectLevel = e.key.keysym.sym - SDLK_0; // Convert key code to level number (1 to 3)
                }
            }
        }
            //We set the number of ennemies, obstacle and enemy speed based on the selected level

        switch(selectLevel)
        {
            case 1:
                numOfEnemies = 3;
                obstacleCount = 25;
                enemySpeed = 3;
                break;
            case 2:
                numOfEnemies = 5;
                obstacleCount = 40;
                enemySpeed = 5;
                break;
            case 3:
                numOfEnemies = 7;
                obstacleCount = 60;
                enemySpeed = 10;
                break;
            default:
                numOfEnemies = 3;
                obstacleCount = 25;
                enemySpeed = 3;
                break;
        }

    }

    SDL_Delay(500);

    //Destroy intro screen window and renderer

    SDL_DestroyRenderer(introRenderer);

    SDL_DestroyWindow(introWindow);

    SDL_Quit();
}

void gameoverScreen(int score1, int score2)
{

    bool choosen = false;
    //initialize sdl
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
            printf("SDL Initialization failed: %s\n", SDL_GetError());
            return;
    }

    //Init sdl_ttf
    if (TTF_Init() < 0)
    {
        printf("SDL_ttf initialization failed: %s\n", TTF_GetError());
        SDL_Quit();
        return;
    }

    //Load font
    TTF_Font* gameoverFont = TTF_OpenFont("arial.ttf", 24);
    if(gameoverFont == NULL)
    {
        printf("Failed to load font: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return;
    }

    //Create window
    SDL_Window* gameoverWindow = SDL_CreateWindow("Pac-Man Gameover", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if(gameoverWindow == NULL)
    {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        TTF_Quit();
        return;
    }
    //Create renderer
    SDL_Renderer* gameoverRenderer = SDL_CreateRenderer(gameoverWindow, -1, SDL_RENDERER_ACCELERATED);
    if(gameoverRenderer == NULL)
    {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(gameoverWindow);
        SDL_Quit();
        TTF_Quit();
        return;
    }



    // Clear the screen
    SDL_SetRenderDrawColor(gameoverRenderer, 0, 0, 0, 255); // Black color
    SDL_RenderClear(gameoverRenderer);

    SDL_Color textColor = {255, 255, 255}; // White color
    SDL_Surface* gameoverSurface = TTF_RenderText_Blended(gameoverFont, "GAME OVER!!! Press Enter", textColor);
    if(gameoverSurface == NULL)
    {
        printf("Text rendering failed: %s\n", TTF_GetError());
        SDL_DestroyRenderer(gameoverRenderer);
        SDL_DestroyWindow(gameoverWindow);
        SDL_Quit();
        return;
    }

    SDL_Texture* gameoverTexture = SDL_CreateTextureFromSurface(gameoverRenderer, gameoverSurface);
    if(gameoverTexture == NULL)
    {
        printf("Texture creation failed: %s\n", SDL_GetError());

        SDL_FreeSurface(gameoverSurface);
        SDL_DestroyRenderer(gameoverRenderer);
        SDL_DestroyWindow(gameoverWindow);
        SDL_Quit();
        return;
    }


    //Render the texture
    SDL_Rect gameoverRect = {100, 100, gameoverSurface->w, gameoverSurface->h};
    SDL_RenderCopy(gameoverRenderer, gameoverTexture, NULL, &gameoverRect);

    char scoreText[50];
    char score2Text[50];
    if (mode == 1)
    {
        sprintf(scoreText, "Score: %d", score1);

        SDL_Surface* scoreSurface = TTF_RenderText_Blended(gameoverFont, scoreText, textColor);
        if(scoreSurface == NULL)
        {
            printf("Text rendering failed: %s\n", TTF_GetError());
            SDL_DestroyRenderer(gameoverRenderer);
            SDL_DestroyWindow(gameoverWindow);
            SDL_Quit();
            return;
        }

        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(gameoverRenderer, scoreSurface);
        if(scoreTexture == NULL)
        {
            printf("Texture creation failed: %s\n", SDL_GetError());

            SDL_FreeSurface(scoreSurface);
            SDL_DestroyRenderer(gameoverRenderer);
            SDL_DestroyWindow(gameoverWindow);
            SDL_Quit();
            return;
        }


        //Render the texture
        SDL_Rect scoreRect = {100, 150, scoreSurface->w, scoreSurface->h};
        SDL_RenderCopy(gameoverRenderer, scoreTexture, NULL, &scoreRect);

        SDL_RenderPresent(gameoverRenderer);
    }
    else if(mode == 2)
    {
               sprintf(scoreText, "Player 1: %d", score1);

        SDL_Surface* scoreSurface = TTF_RenderText_Blended(gameoverFont, scoreText, textColor);
        if(scoreSurface == NULL)
        {
            printf("Text rendering failed: %s\n", TTF_GetError());
            SDL_DestroyRenderer(gameoverRenderer);
            SDL_DestroyWindow(gameoverWindow);
            SDL_Quit();
            return;
        }

        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(gameoverRenderer, scoreSurface);
        if(scoreTexture == NULL)
        {
            printf("Texture creation failed: %s\n", SDL_GetError());

            SDL_FreeSurface(scoreSurface);
            SDL_DestroyRenderer(gameoverRenderer);
            SDL_DestroyWindow(gameoverWindow);
            SDL_Quit();
            return;
        }


        //Render the texture
        SDL_Rect scoreRect = {100, 150, scoreSurface->w, scoreSurface->h};
        SDL_RenderCopy(gameoverRenderer, scoreTexture, NULL, &scoreRect);

                sprintf(score2Text, "Player 2: %d", score2);

        SDL_Surface* score2Surface = TTF_RenderText_Blended(gameoverFont, score2Text, textColor);
        if(score2Surface == NULL)
        {
            printf("Text rendering failed: %s\n", TTF_GetError());
            SDL_DestroyRenderer(gameoverRenderer);
            SDL_DestroyWindow(gameoverWindow);
            SDL_Quit();
            return;
        }

        SDL_Texture* score2Texture = SDL_CreateTextureFromSurface(gameoverRenderer, score2Surface);
        if(score2Texture == NULL)
        {
            printf("Texture creation failed: %s\n", SDL_GetError());

            SDL_FreeSurface(score2Surface);
            SDL_DestroyRenderer(gameoverRenderer);
            SDL_DestroyWindow(gameoverWindow);
            SDL_Quit();
            return;
        }


        //Render the texture
        SDL_Rect score2Rect = {100, 200, score2Surface->w, score2Surface->h};
        SDL_RenderCopy(gameoverRenderer, score2Texture, NULL, &score2Rect);

        SDL_RenderPresent(gameoverRenderer);
    }


                // Wait for Enter key to start the game
        SDL_Event e;
        int quit = 0;
        while (!quit && SDL_WaitEvent(&e))
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_RETURN) {
                        quit = 1; // Exit introductory screen loop
                    }
                    break;
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(gameoverRenderer, 0, 0, 0, 255); // Black color
        SDL_RenderClear(gameoverRenderer);

        // Render player mode selection prompt
        SDL_Surface* chooseSurface = TTF_RenderText_Blended(gameoverFont, "CONTINUE?: ", textColor);
        SDL_Texture* chooseTexture = SDL_CreateTextureFromSurface(gameoverRenderer, chooseSurface);
        SDL_Rect chooseRect = {100, 100, chooseSurface->w, chooseSurface->h};

        SDL_RenderCopy(gameoverRenderer, chooseTexture, NULL, &chooseRect);

        SDL_Surface* againSurface = TTF_RenderText_Blended(gameoverFont, "1. Play Again", textColor);
        SDL_Texture* againTexture = SDL_CreateTextureFromSurface(gameoverRenderer, againSurface);
        SDL_Rect againRect = {100, 150, againSurface->w, againSurface->h};

        SDL_RenderCopy(gameoverRenderer, againTexture, NULL, &againRect);

        SDL_Surface* quitSurface = TTF_RenderText_Blended(gameoverFont, "2. EXIT", textColor);
        SDL_Texture* quitTexture = SDL_CreateTextureFromSurface(gameoverRenderer, quitSurface);
        SDL_Rect quitRect = {100, 200, quitSurface->w, quitSurface->h};

        SDL_RenderCopy(gameoverRenderer, quitTexture, NULL, &quitRect);

        SDL_RenderPresent(gameoverRenderer);

        while(!choosen)
        {
            if (SDL_WaitEvent(&e))
            {
                if(e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_1 || e.key.keysym.sym == SDLK_KP_1))
                {
                    choosen = true;
                    SDL_DestroyRenderer(gameoverRenderer);
                    SDL_DestroyWindow(gameoverWindow);
                    SDL_Quit();
                    init();
                }
                else if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_2 || e.key.keysym.sym == SDLK_KP_2))
                {
                    choosen = true;
                    SDL_Quit();
                }
            }
        }
}
