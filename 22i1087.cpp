#include <iostream>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <chrono>
#include <random>
#include <cstdlib>
#include <semaphore.h>
#include <SFML/Graphics.hpp>

using namespace std;

// Global variables
const int normalGhostSpeed = 600; 
int ghostSpeed = normalGhostSpeed; 
bool increasedSpeed = false; /
const int numSpeedBoosts = 2;
int availableBoosts = numSpeedBoosts;
queue<int> boostQueue;
mutex boostMutex;

const int windowWidth = 1000;
const int windowHeight = 1000;
const int mazeWidth = 15;
const int mazeHeight = 15;
const int cellSize = windowWidth / mazeWidth;
char gameBoard[mazeHeight][mazeWidth];
int pacmanPositionX;
int pacmanPositionY;
int score = 0;
int boostedGhosts = 0;


vector<pair<int, int>> fruitPositions;
vector<pair<int, int>> ghostPositions;
vector<int> powerPellets;
vector<pair<int, int>> pelletPositions;
vector<pair<int, int>> powerPos;
int lives = 3;
bool gameRunning = true;
bool check = 0;
bool powerup = 0; 
mutex mtx;
condition_variable cv;
sem_t semaphore;
sem_t menuSemaphore;
sem_t ghostSemaphore;
sem_t speedBoostSemaphore;
sem_t keySemaphore;
sem_t permitSemaphore;
mutex keyMutex;
mutex permitMutex;

const char WALL = '#';
const char PATH = ' ';
const char PELLET = '.';
const char POWER_PELLET = '*';
const char PACMAN = 'P';
const char GHOST = 'G';

void initializeGameBoard();
void drawMaze(sf::RenderWindow& window);
void drawPacman(sf::RenderWindow& window);
void movePacman(char direction);
void drawGhosts(sf::RenderWindow& window);
void moveGhost(void *id);
void checkCollision();
void GameEngine(sf::RenderWindow& window);
void Menu(sf::RenderWindow& window);
void drawScore(sf::RenderWindow& window); 

const int powerupDuration = 5; 
sf::Clock powerupTimer; 
 sf::Clock gameClock; 
 sf::Time elapsed1 = gameClock.getElapsedTime();
 
void initializeGameBoard(bool powerup) {
  char hardcodedGameBoard[mazeHeight][mazeWidth] = {
       { WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL },
       { WALL, '.', ' ', '.', '.', '.', 'F', '.', '.', '.', '.', '.', '.', 'F', WALL },
       { WALL, '.', '.', '.', '.', WALL, WALL, WALL, WALL, WALL, '.', '.', '.', '.', WALL },
       { WALL, '.', WALL, WALL, '.', '.', '.', WALL, '.', '.', '.', WALL, WALL, '.', WALL },
       { WALL, 'F', WALL, '.', '.', '.', 'F', '.', '.', '.', 'F', '.', WALL, ' ', WALL },
       { WALL, '.', WALL, '.', '.', '.', '.', '.', '.', '.', '.', '.', WALL, '.', WALL },
       { WALL, '.', WALL, 'F', '.', WALL, WALL, '.', WALL, WALL, 'F', '.', WALL, '.', WALL },
       { WALL, '.', '.', '.', '.', WALL, PATH, PATH, PATH, WALL, '.', '.', '.', '.', WALL },
       { WALL, ' ', WALL, '.', '.', WALL, PATH, PATH, PATH, WALL, '.', '.', WALL, '.', WALL },
       { WALL, '.', WALL, '.', '.', WALL, WALL, WALL, WALL, WALL, '.', '.', WALL, '.', WALL },
       { WALL, '.', WALL, '.', '.', '.', '.', '.', '.', '.', '.', '.', WALL, 'F', WALL },
       { WALL, '.', WALL, WALL, '.', '.', '.', '.', '.', '.', '.', WALL, WALL, '.', WALL },
       { WALL, 'F', '.', '.', '.', WALL, WALL, WALL, WALL, WALL, '.', '.', '.', '.', WALL },
       { WALL, '.', '.', '.', '.', '.', 'F', '.', '.', 'F', '.', '.', ' ', 'F', WALL },
       { WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL }
    };                          
 
    for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            gameBoard[i][j] = hardcodedGameBoard[i][j];
          //  cout<<gameBoard[i][j]<<endl;
        }
    }
       // Add Power Pellets to the game board
    for (int i = 0; i < 3; ++i) {
        int pelletX = 2 + rand() % (mazeWidth - 2);
        int pelletY = 2 + rand() % (mazeHeight - 2);
        
        // Check if there's a pellet at the position and clear it
        for (auto it = pelletPositions.begin(); it != pelletPositions.end(); ++it) {
            if (it->first == pelletY && it->second == pelletX) {
                pelletPositions.erase(it);
                gameBoard[pelletY][pelletX] = PATH;
                break; // Break after clearing the pellet
            }
        }
        /*
        // Place the Power Pellet
        if (gameBoard[pelletY][pelletX] != WALL) {
            powerPos.push_back({ pelletX, pelletY });
            gameBoard[pelletY][pelletX] = POWER_PELLET;
        } else {
            i = i - 1; // If there's a wall, decrement the loop counter to try again
        } 
*/
    }
      for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            if (gameBoard[i][j] == '.') {
                pelletPositions.push_back({i, j});
            }
        }
    }


for (int i = 0; i < mazeHeight; ++i) {
    for (int j = 0; j < mazeWidth; ++j) {
        if (gameBoard[i][j] == 'F') {
            fruitPositions.push_back({ i, j });
       cout<<i<<" "<<j<<endl;
        }
    }
}

    pacmanPositionX = 1;	
    pacmanPositionY = 1;  
    gameBoard[pacmanPositionY][pacmanPositionX] = PACMAN;

    for (int i = 0; i < 4; ++i) {
       // int ghostX = 1 + rand() % (mazeWidth - 2);
       // int ghostY = 1 + rand() % (mazeHeight - 2);
        int ghostX = mazeWidth / 2 - 1 + i;
        int ghostY = mazeHeight/ 2 - 1 + i;
        if (gameBoard[ghostY][ghostX] != WALL){
           ghostPositions.push_back({ ghostX, ghostY });
           gameBoard[ghostY][ghostX] = GHOST;	
        }
        else{
           ghostX += 1;
           ghostY -= 1;  
           ghostPositions.push_back({ ghostX, ghostY });
           gameBoard[ghostY][ghostX] = GHOST;
        }
    }
  
} 

void drawMaze(sf::RenderWindow& window) {
    sf::RectangleShape wall(sf::Vector2f(cellSize, cellSize));
    wall.setFillColor(sf::Color::White);

    for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            if (gameBoard[i][j] == WALL) {
                wall.setPosition(j * cellSize, i * cellSize);
                window.draw(wall);
            } 
        }
    }
}
/*
void drawFruit(sf::RenderWindow& window) {
  
//cout<<'f'<<endl;
    /*
    for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            if (gameBoard[i][j] == 'F') {
                // Check if the fruit position is within maze bounds
                if (i >= 0 && i < mazeHeight && j >= 0 && j < mazeWidth) {
                    // Check if the cell is empty
                    if (gameBoard[i][j] != WALL) {
                        fruit.setPosition(i * cellSize, j * cellSize);
                        window.draw(fruit);
  //                      cout<<i<<" "<<j<<endl;
                    }
                }
            }
        }
    }
    */
    
//}

// Inside drawFruit() function
void drawFruit(sf::RenderWindow& window) {
     sf::RectangleShape fruit(sf::Vector2f(cellSize, cellSize));
    fruit.setFillColor(sf::Color::Cyan);

    // Draw each fruit at its position
    for (const auto& pos : fruitPositions) {
        fruit.setPosition(pos.first * cellSize, pos.second * cellSize);
        window.draw(fruit);
    }
}

void drawPellets(sf::RenderWindow& window) {
    sf::CircleShape pellet(cellSize / 4); // Adjust the size of the pellet
 
    pellet.setFillColor(sf::Color::Blue);

    // Draw each pellet at its position
    for (const auto& pos : pelletPositions) {
       pellet.setPosition(pos.second * cellSize + cellSize / 2 - pellet.getRadius(), pos.first * cellSize + cellSize / 2 - pellet.getRadius());
        window.draw(pellet);
    }
}

void drawPacman(sf::RenderWindow& window) {
    sf::CircleShape pacman(cellSize / 2);
    pacman.setFillColor(sf::Color::Yellow);
    pacman.setPosition(pacmanPositionX * cellSize, pacmanPositionY * cellSize);
    window.draw(pacman);
}
void initializeSemaphores() {
    // Initialize semaphores with appropriate values
    sem_init(&keySemaphore, 0, 2);
    sem_init(&permitSemaphore, 0, 2);
}

void acquireKeyAndPermit(int ghostId) {
    // Acquire key
    sem_wait(&keySemaphore);
    keyMutex.lock();
    // Acquire permit
    sem_wait(&permitSemaphore);
    permitMutex.lock();

}
void releaseKeyAndPermit(int ghostId) {
    // Release key
    keyMutex.unlock();
    sem_post(&keySemaphore);

    // Release permit
    permitMutex.unlock();
    sem_post(&permitSemaphore);
}

void movePacman(char direction) {
    unique_lock<mutex> lock(mtx);

    int newPacmanPositionX = pacmanPositionX;
    int newPacmanPositionY = pacmanPositionY;
    switch (direction) {
    case 'w':
        newPacmanPositionY--;
        break;
    case 'a':
        newPacmanPositionX--;
        break;
    case 's':
        newPacmanPositionY++;
        break;
    case 'd':
        newPacmanPositionX++;
        break;
    }

    if (gameBoard[newPacmanPositionY][newPacmanPositionX] != WALL) {
        gameBoard[pacmanPositionY][pacmanPositionX] = PATH;
        pacmanPositionX = newPacmanPositionX;
        pacmanPositionY = newPacmanPositionY;
        gameBoard[pacmanPositionY][pacmanPositionX] = PACMAN;
    }

    lock.unlock(); 	
    cv.notify_all();
}

void drawGhosts(sf::RenderWindow& window) {
    sf::CircleShape ghost(cellSize / 2);
    ghost.setFillColor(sf::Color::Red);

    for (const auto& pos : ghostPositions) {
        ghost.setPosition(pos.first * cellSize, pos.second * cellSize);
        window.draw(ghost);
    }
}

void drawScore(sf::RenderWindow& window){
    sf::Font font;
    if (!font.loadFromFile("comic.ttf")) {
        // Error loading font
        return;
    }
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setString("Score: " + to_string(score)); 
    scoreText.setCharacterSize(28); 
    scoreText.setFillColor(sf::Color::Black); /
    scoreText.setPosition(8, 2); 
    
    window.draw(scoreText);    
}
void addPellet(sf::RenderWindow& window){
    for (int i = 0; i < 3; ++i) {
        int pelletX = 2 + rand() % (mazeWidth - 2);
        int pelletY = 2 + rand() % (mazeHeight - 2);
        if (gameBoard[pelletY][pelletX] != WALL){
            powerPos.push_back({ pelletX, pelletY });
            gameBoard[pelletY][pelletX] = POWER_PELLET;
        }
        else{
            i = i - 1;
        }
    }
}

void drawPowerPellet(sf::RenderWindow& window) {
    sf::CircleShape power(cellSize / 2, 6);
    power.setFillColor(sf::Color::Cyan);

    for (const auto& pos : powerPos) {
      
        if (pos.second >= 0 && pos.second < mazeHeight && pos.first >= 0 && pos.first < mazeWidth) {
           
            for (auto it = pelletPositions.begin(); it != pelletPositions.end(); ++it) {
                if (it->first == pos.first && it->second == pos.second) {
                
                    pelletPositions.erase(it);
                    gameBoard[pos.second][pos.first] = PATH;
                    break; 
                }
            }
            power.setPosition(pos.first * cellSize, pos.second * cellSize);
            window.draw(power);
        }
    }
}

/*
void moveGhost(void *id_ptr) {
    int ghostId = *(int*)id_ptr;
    default_random_engine generator;
    uniform_int_distribution<int> distribution(0, 3);

    while (true) {
        // Sleep before acquiring the lock to prevent busy waiting
          this_thread::sleep_for(chrono::milliseconds(100));

        // Acquire the lock
        unique_lock<mutex> lock(mtx);

        int dx = 0, dy = 0;
        int direction = distribution(generator);
        switch (direction) {
        case 0:
            dy = -1;
            break;
        case 1:
            dy = 1;
            break;
        case 2:
            dx = -1;
            break;
        case 3:
            dx = 1;
            break;
        }

        int newGhostX = ghostPositions[ghostId].first + dx;
        int newGhostY = ghostPositions[ghostId].second + dy;

       if (gameBoard[newGhostY][newGhostX] != WALL && gameBoard[newGhostY][newGhostX] != GHOST) {
            // Update ghost position 
            gameBoard[ghostPositions[ghostId].second][ghostPositions[ghostId].first] = PATH;	
            ghostPositions[ghostId].first = newGhostX;
            ghostPositions[ghostId].second = newGhostY;
            gameBoard[ghostPositions[ghostId].second][ghostPositions[ghostId].first] = GHOST;
        }
        // Unlock the mutex before notifying
        lock.unlock();

        // Notify other threads that the move is complete
        cv.notify_all();

       struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 500 * 1000000; // 500 milliseconds

        // Sleep to slow down ghost movement
        nanosleep(&ts, NULL);
    }
} */

void moveGhost(void *id_ptr) {
    int ghostId = *(int*)id_ptr;
    default_random_engine generator;
    uniform_int_distribution<int> distribution(0, 3);

    // Initialize ghost's current direction randomly
    int dx = 0, dy = 0;
    int direction = distribution(generator);
    switch (direction) {
        case 0: 
            dy = -1;
            break;
        case 1:
            dy = 1;
            break;
        case 2:
            dx = -1;
            break;
        case 3:
            dx = 1;
            break;
    }	

    while (true) {
        unique_lock<mutex> lock(mtx);
        acquireKeyAndPermit(ghostId);

        int newGhostX = ghostPositions[ghostId].first + dx;
        int newGhostY = ghostPositions[ghostId].second + dy;

        if (gameBoard[newGhostY][newGhostX] != WALL) {
            
            gameBoard[ghostPositions[ghostId].second][ghostPositions[ghostId].first] = PATH;
            ghostPositions[ghostId].first = newGhostX;
            ghostPositions[ghostId].second = newGhostY;
            gameBoard[ghostPositions[ghostId].second][ghostPositions[ghostId].first] = GHOST;
        } else {
         
            direction = distribution(generator);
            switch (direction) {
                case 0:
                    dx = 0;
                    dy = -1;
                    break;
                case 1:
                    dx = 0;
                    dy = 1;
                    break;
                case 2:
                    dx = -1;
                    dy = 0;
                    break;
                case 3:
                    dx = 1;
                    dy = 0; 
                    break;
            }
        }
releaseKeyAndPermit(ghostId);
        lock.unlock();
        cv.notify_all();

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = ghostSpeed * 1000000;
         if (ghostId==0) {
        cout<<elapsed1.asSeconds()<<endl;
         if(elapsed1.asSeconds() > 15.000){
  cout<<"in 1"<<endl;
  
    if (sem_trywait(&speedBoostSemaphore) == 0) {
      
        cout<<"in 2"<<endl;
        ghostSpeed = 300; /
        sem_post(&speedBoostSemaphore);
    }
    }
}
        nanosleep(&ts, NULL);
    }
}

void checkCollision(bool powerup) {
    // Collision detection between Pac-Man and ghosts
    for (auto& pos : ghostPositions) {
        if (pacmanPositionX == pos.first && pacmanPositionY == pos.second) {
            if (!powerup) {
                lives--;
                if (lives == 0) {
                    cout << "Game Over!" << endl;
                    sf::RenderWindow window(sf::VideoMode(320, 480), "ENDING SCREEN");
                    sf::Sprite sprite;
                    sf::Texture texture;
                    texture.loadFromFile("img/gameover.png");
                    sprite.setTexture(texture);
                    sprite.setPosition(0, 0);
                    while (window.isOpen()) {
                        window.clear();
                        window.draw(sprite);
                        window.display();
                        sf::Event event;
                        while (window.pollEvent(event)) {
                            if (event.type == sf::Event::Closed)
                                window.close();
                        }
                    }
                    std::cout << "Your score was: " << std::endl;
                    std::cout << score;
                    exit(0);
                } else {
                    cout << "Remaining Lives: " << lives << endl;
                    pacmanPositionX = mazeWidth / 2;
                    pacmanPositionY = mazeHeight / 2;
                    score -= 10;
                    gameBoard[pacmanPositionY][pacmanPositionX] = PACMAN;
                }
            } else {
            
                int centerX = mazeWidth / 2;
                int centerY = mazeHeight / 2;
                pos = {centerX, centerY};
                // Update game board
                gameBoard[pos.second][pos.first] = PATH; 
                gameBoard[centerY][centerX] = GHOST; 
                cout << "GHOST DEAD" << endl;
                score += 5;
                 if (powerup && powerupTimer.getElapsedTime().asSeconds() >= powerupDuration) {
     cout<<"power up timer finished"<<endl;
        powerup = false;
    }
            }
            
        }
    }
     for (auto it = powerPos.begin(); it != powerPos.end(); ++it) {
        const auto& pos = *it;
        if (pacmanPositionX == pos.first && pacmanPositionY == pos.second) {
            cout << pacmanPositionY << " , " << pacmanPositionX << " collided with power pellet " << endl;
         
            powerup = true;
            powerupTimer.restart(); 
            powerPos.erase(it);
        }
    }
    
}

void checkCollisionFruit(int x, int y) {
    // Iterate over fruit positions to check collision
    for (auto it = fruitPositions.begin(); it != fruitPositions.end(); ++it) {
        if (it->first == x && it->second == y) {
            score += 10; 
            fruitPositions.erase(it);
            return; 
        }
    } 		
}
void checkCollisionPellet(int x, int y) {
    // Check collision with pellets
    for (auto it = pelletPositions.begin(); it != pelletPositions.end(); ++it) {
        if (it->first == y && it->second == x) {
            // Update score
            score += 2;
            pelletPositions.erase(it);
            //  gameBoard[x][y] = PATH;
           return;
        }
    }
}
///////////////////////////////////////////

void GameEngine(sf::RenderWindow& window) {
bool once=0;
    bool check = false;
    srand(time(NULL));

    initializeGameBoard(powerup);
    pthread_t ghostThreads[4];
    int ghostIds[4]; // Create an array to hold ghost IDs

    for (int i = 0; i < 4; ++i) {
        ghostIds[i] = i;
        pthread_create(&ghostThreads[i], NULL, (void *(*)(void *))moveGhost, (void *)&ghostIds[i]);
    }
 sf::Clock clock;
    while (window.isOpen() && gameRunning) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                gameRunning = false;
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed) {
                // Handle user input for moving Pac-Man
                char direction = ' ';
                switch (event.key.code) {
                case sf::Keyboard::W:
                    direction = 'w';
                    break;
                case sf::Keyboard::A:
                    direction = 'a';
                    break;
                case sf::Keyboard::S:
                    direction = 's';
                    break;
                case sf::Keyboard::D:
                    direction = 'd';
                    break;
case sf::Keyboard::P:
{
    // Static variable to track if the game is paused
    static bool paused = false;
  sf::Sprite sprite;
    sf::Texture texture;
    texture.loadFromFile("img/menu2.png");
    sprite.setTexture(texture);
    sprite.setPosition(200, 200);
 
    if (!paused)
    {
        paused = true; 

        while (window.isOpen() && paused)
        {
            // Process events
            sf::Event event;
            while (window.pollEvent(event))
            {
                // Check if the window is closed
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
            
                else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P)
                {
                    paused = false; 
                }
            }

            // Clear the window
            window.clear();
            // Draw the menu sprite
            window.draw(sprite);
            // Display the window
            window.display();
        }
    }

    else
    {
        paused = false; 
    }
    break; /
}
           
                default:
                    break;
                }
                cout<<"----"<<endl;
                movePacman(direction);
                cout<<pacmanPositionX<<" "<<pacmanPositionY<<endl;
                checkCollisionFruit(pacmanPositionX, pacmanPositionY);
                checkCollisionPellet(pacmanPositionX, pacmanPositionY); // Add this line
                //checkCollision();
                checkCollision(powerup);
            }
        }
      
        sf::Time elapsed = clock.getElapsedTime();
        // Output the elapsed time in seconds to the terminal
        //cout << "Time elapsed: " << elapsed.asSeconds() << " seconds" << endl;

        window.clear(sf::Color::Black);
     // initializeGameBoard( powerup); 
        drawMaze(window);
        drawPacman(window);
       // increaseGhostSpeed(10); 
     /*
       if (elapsed.asSeconds() > 10.000 && elapsed.asSeconds() < 18.000){
           ghostSpeed = 200;
       }
       else{
           ghostSpeed = 600;
       }
       */
        if (elapsed.asSeconds() > 8.000 && elapsed.asSeconds() < 20.000 ){ // !check){        
        
      // Set the flag to true to indicate that power pellets need to be added
           if (check == false){           
             addPellet(window);
     	      
            }
            check = true;
            drawPowerPellet(window); // Add power pellets to the screen
            powerup = 1; // Set the powerup flag to true
            //powerPelletsAdded = true;
        }
        else{
        powerup=false;
        }
         
         if(elapsed.asSeconds()>20.00)
         {
         
         if(once==0){
         
         for(int i=0 ; i< mazeWidth ; i++){
          for(int j=0 ; i<mazeHeight ; j++){
          
         if(gameBoard[i][j]==POWER_PELLET)
         {
         
         gameBoard[i][j]=PATH;
         }
         
         }
         
         
         }
         
       }
        
    }      
     //   powerPelletsAdded = true;
     //   window.clear(sf::Color::Black);
     //   drawMaze(window);
     //   drawPacman(window);
        drawGhosts(window);
        drawScore(window);
        drawFruit(window);
        drawPellets(window); // Add this line
            
        window.display();
    }

    sem_post(&semaphore);
}

void mainmenu(sf::RenderWindow& window) {
    sf::Sprite sprite5;
    sf::Texture texture5;
    texture5.loadFromFile("img/welcome.png");
    sprite5.setTexture(texture5);
    sprite5.setPosition(0, 0);

    bool startPressed = false;
    bool controlsPressed = false;

    while (window.isOpen()) {
        window.clear();
        window.draw(sprite5);
        window.display();

        sf::Event event;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1) || sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad1)) {
            startPressed = true;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
            controlsPressed = true;
        }

        if (startPressed) {
            break;
        }

      if (controlsPressed) {
    sf::RenderWindow controlsWindow(sf::VideoMode(1000, 1000), "Controls");
    sf::Sprite controlsSprite;
    sf::Texture controlsTexture;
    controlsTexture.loadFromFile("img/con.jpg");
    controlsSprite.setTexture(controlsTexture);
    controlsSprite.setPosition(0, 0);

    while (controlsWindow.isOpen()) {
        sf::Event controlsEvent;
        while (controlsWindow.pollEvent(controlsEvent)) {
            if (controlsEvent.type == sf::Event::Closed) {
                controlsWindow.close();
            }
      
            if (controlsEvent.type == sf::Event::KeyPressed && controlsEvent.key.code == sf::Keyboard::B) {
                controlsWindow.close(); /
                break; 
            }
        }

        controlsWindow.clear();
        controlsWindow.draw(controlsSprite);
        controlsWindow.display();
    }

    controlsPressed = false; 
}
    }

    sem_post(&menuSemaphore);
}

int main() {
    // Initialize the semaphore
    sem_init(&semaphore, 0, 0); //ghost boost
    sem_init(&menuSemaphore, 0, 0);
     
      initializeSemaphores();
   int numSpeedBoosts = 2; // Assuming you have 2 speed boosts
    sem_init(&speedBoostSemaphore, 0, numSpeedBoosts);
    
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "SFML Maze");

    pthread_t menuThread;
    pthread_create(&menuThread, NULL, (void *(*)(void *))mainmenu, (void *)&window);

    // Wait for the menu thread to finish
    sem_wait(&menuSemaphore);

    // Start the game engine
    GameEngine(window);

    // Wait for the game loop to finish
    sem_wait(&semaphore);

    // Close the game window
    window.close();

    // Destroy the semaphore
    sem_destroy(&semaphore);
    sem_destroy(&menuSemaphore);

    return 0;
}
