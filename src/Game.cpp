#include "raylib.h"
#include "Grid.h"
#include <cmath>

#define screenWidth 800
#define screenHeight 800

int menu();
int pause();
int usePower(PowerupItem* p, Score* src, Grid* grid);

int main() {
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "TETris");
    InitAudioDevice();
    
    ChangeDirectory(GetApplicationDirectory());
    Grid* grid = nullptr;
    Score* score;
    Tet* tet;
    Powerup* powerUp;

    Font titleFont = LoadFont("resources/titleFont.ttf");
    int levelcounter = 0;
    int horizontalcounter = 0;
    int downcounter = 0;
    int powerupcounter = 0;
    int spawnpower = GetRandomValue(10*60, 15*60);
    bool start = true;
    int level = 0;
    std::vector<std::pair<double, int>> speedChange; //for our speedchange powerup, double represents how changed and int is time left of effect

    Camera2D camera = {0};
    camera.target = (Vector2){screenWidth/2, screenHeight/2};
    camera.offset = (Vector2){screenWidth/2, screenHeight/2};
    camera.rotation = 180;
    camera.zoom = 1;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Main Menu into tet monologue
        //----------------------------------------------------------------------------------
        if (start) {
            if (grid != nullptr) {
                delete grid;
                delete score;
                delete tet;
                delete powerUp;
            }
            grid = new Grid();
            score = new Score();
            tet = new Tet(score);
            powerUp = new Powerup();
            grid->setScoreBoard(score);
            grid->setTet(tet);
            grid->setPowerup(powerUp);
            powerUp->setScore(score);
            level = 0;
            levelcounter = 0;
            horizontalcounter = 0;
            downcounter = 0;
            //temp
            grid->randomRotate();
            start = false; continue;
            if (menu() == -1) break;
            start = false;
            if (tet->tetMonologue() == -1) break;
            double fade = 1;
            bool quit = true;
            while (!WindowShouldClose()) {
                BeginDrawing();
                    ClearBackground(BLACK);
                    grid->drawAll(true, true);
                    DrawRectangle(0, 0, 800, 800, Fade(BLACK, fade));
                EndDrawing();
                fade-=0.01;
                if (fade <= 0) {
                    quit = false;
                    break;
                }
            }
            if (quit) break;

        }
        //----------------------------------------------------------------------------------
        // Update
        //----------------------------------------------------------------------------------
        bool checkRows = false;

        //updating functions
        grid->updateAll();

        //updating our block every so often, speed starts at 120 frames per movement but value of logistic growth function
        //using level as x that approaches 120
        level = grid->getLevel();
        int num = 120 - (240/(1+pow(M_E, (-0.05*level)))-120);
        for (std::vector<std::pair<double, int>>::iterator it = speedChange.begin(); it != speedChange.end(); it++) {
            if (!(num < 0 && it->first < 0)) num*=it->first;
            if (num < 1 && num > 0) num = 1;
            it->second--;
            if (it->second == 0) {
                if (it->first < 0) levelcounter = 0;
                speedChange.erase(it);
                it--;
            }
        }
        if (levelcounter >= num) {
            levelcounter = 0;
            grid->moveDown();
            score->addScore(1);
            checkRows = true;
        }
        levelcounter++;

        if (powerupcounter == spawnpower) {
            powerUp->spawnPowerup(true);
            powerupcounter = 0;
            spawnpower = GetRandomValue(3*60, 5*60);
        }
        powerupcounter++;

        //for tetPower
        std::string power = tet->checkTetPower();
        if (power == "level1") grid->increaseLevel(15);
        else if (power == "level2") grid->increaseLevel(25);
        else if (power == "blind1") grid->blind(GetRandomValue(5, 7));
        else if (power == "blind2") grid->blind(GetRandomValue(8, 12));
        else if (power == "zblock") grid->changeNext(5);
        else if (power == "sblock") grid->changeNext(7);

        //checking if keys are pressed and doing the corresponding action
        if (IsKeyPressed(KEY_RIGHT)) {
            grid->moveRight();
            horizontalcounter = -7;
        }

        else if (IsKeyDown(KEY_RIGHT)) {
            if (horizontalcounter == 3) {
                horizontalcounter = 0;
                grid->moveRight();
            }
            else ++horizontalcounter;
        }

        else if (IsKeyPressed(KEY_LEFT)) {
            grid->moveLeft();
            horizontalcounter = -7;
        }

        else if (IsKeyDown(KEY_LEFT)) {
            if (horizontalcounter == 3) {
                horizontalcounter = 0;
                grid->moveLeft();
            }
            else ++horizontalcounter;
        }
    
        if (IsKeyPressed(KEY_DOWN)) {
            grid->moveDown();
            score->addScore(1);
            checkRows = true;
        }

        else if (IsKeyDown(KEY_DOWN)) {
            if (downcounter == 3) {
                downcounter = 0;
                grid->moveDown();
                score->addScore(1);
                checkRows = true;
            }
            else ++downcounter;
        }

        else if (IsKeyPressed(KEY_SPACE)) {
            Sound drop = LoadSound("resources/audio/blockdropping.wav");
            PlaySound(drop);
            score->addScore(grid->drop()*2);
            checkRows = true;
        }

        if (IsKeyPressed(KEY_UP)) grid->rotate();

        //hold current block
        if (IsKeyPressed(KEY_C)) grid->hold();

        //ask powerup class if there are any fast speedchange
        std::vector<std::pair<double, int>> temp = powerUp->checkFastSpeed();
        speedChange.insert(speedChange.end(), temp.begin(), temp.end());

        //check if any powerups are being used
        PowerupItem* p = nullptr;
        if (IsKeyPressed(KEY_ONE)) p = powerUp->usePowerup(1);
        else if (IsKeyPressed(KEY_TWO)) p = powerUp->usePowerup(2);
        else if (IsKeyPressed(KEY_THREE)) p = powerUp->usePowerup(3);
        if (p != nullptr && p->id == "speedchange") {
            SpeedChange* s = dynamic_cast<SpeedChange*>(p);
            if (s->variant == 0) speedChange.push_back(std::make_pair(-1, 1800));
            else if (s->variant == -1) speedChange.push_back(std::make_pair(1.3, 1800));
            else if (s->variant == -2) speedChange.push_back(std::make_pair(1.5, 1800));
        }
        else if (usePower(p, score, grid) == -1) break;

        //pause menu
        if (IsKeyPressed(KEY_P)) {
            int p = pause();
            if (p == -1) break; //quit game
            else if (p == 0) {
                //quit to start
                start = true;
                continue;
            }
        }
        
        //do we need to check if a row is complete?
        if (checkRows) {
            std::vector<int> rowsRemoved = grid->checkRowComplete();
            levelcounter = 0;
            //removes our grid rows and fixes, if -1 is returned then program needs to exit
            if (!rowsRemoved.empty()) {
                if (grid->removeRow(rowsRemoved) == -1 || grid->fixRows(rowsRemoved) == -1) {
                    CloseWindow();
                    return 0;
                }
            }
        }


        //checking if the game should be over
        if (grid->checkGameOver()) {
            start = true; bool quit = true;
            int endcounter = 0; double fade = 0;
            while (!WindowShouldClose()) {
                ++endcounter;
                fade +=0.01;
                if (endcounter == 450) {
                    quit = false;
                    break;
                }
                BeginDrawing();
                    ClearBackground((Color){static_cast<unsigned char>(0+level/2), 0, 0});
                    grid->drawAll(true, true);
                    DrawRectangle(0, 400-MeasureTextEx(titleFont, "YOU LOSE", 150, 5).y/2-10, 800, MeasureTextEx(titleFont, "YOU LOSE", 150, 5).y+20, Fade(BLACK, fade));
                    DrawTextEx(titleFont, "YOU LOSE", (Vector2){400-MeasureTextEx(titleFont, "YOU LOSE", 150, 5).x/2, 400-MeasureTextEx(titleFont, "YOU LOSE", 150, 5).y/2}, 150, 5, Fade(WHITE, fade));
                EndDrawing();
            }
            if (quit) break;
            else continue;
        }


        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
            BeginDrawing();
                grid->drawAll(true, true);
            EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    if (grid != nullptr) {
        delete grid;
        delete score;
        delete tet;
        delete powerUp;
    }
    CloseWindow();
    CloseAudioDevice();
    return 0;
}

int menu() {
    //loading font
    Font titleFont = LoadFont("resources/titleFont.ttf");
    //defining the play button rectangle
    Rectangle playbutton; playbutton.x = 275; playbutton.y = 400; playbutton.width = 240; playbutton.height = 100;

    //conunter for our background cubes 
    const std::vector<Color> colors = {SKYBLUE, DARKBLUE, ORANGE, YELLOW, LIME, PURPLE, RED};
    std::vector<std::pair<Rectangle, Color>> cubes;
    int addCubeCounter = 0;
    int cubeSize = 20;
    bool fade = false;
    double fadeTracker = 0;


    while (!WindowShouldClose()) {
        BeginDrawing();

            ClearBackground(BLACK);

            //cool background cubes falling to make menu a little more intersting
            if (addCubeCounter == 30) {
                Rectangle c; c.x = GetRandomValue(0, 800-cubeSize); c.y = -cubeSize; c.width = cubeSize; c.height = cubeSize;
                cubes.push_back(std::make_pair(c, colors[GetRandomValue(0, 6)]));
                addCubeCounter = 0;
            }
            else addCubeCounter++;

            for (std::vector<std::pair<Rectangle, Color>>::iterator cube = cubes.begin(); cube != cubes.end(); cube++) {
                    cube->first.y+=1;
                DrawRectangleRec((*cube).first, (*cube).second);
                DrawRectangleLinesEx((*cube).first, 4, Fade(BLACK, 0.2));
            }
            if (cubes.size() != 0 && cubes[0].first.y > 800+cubeSize) {
                cubes.assign(cubes.begin()+1, cubes.end());
            }


            //drawing our title and play button
            DrawTextEx(titleFont, "MistaTet", (Vector2){400-MeasureTextEx(titleFont, "MistaTet", 150, 5).x/2, 200}, 150, 5, WHITE);
            DrawRectangleRounded(playbutton, 0.2, 100, LIGHTGRAY);
            DrawRectangleRoundedLines(playbutton, 0.2, 100, 10, GRAY);
            DrawTextEx(titleFont, "start", Vector2{playbutton.x+playbutton.width/2-MeasureTextEx(titleFont, "start", 60, 0).x/2, playbutton.y+playbutton.height/2-MeasureTextEx(titleFont, "start", 60, 0).y/2}, 60, 0, WHITE);

            //checking if mouse is over the play button and if play button is clicked
            float mX = GetMouseX(); float mY = GetMouseY();
            if (CheckCollisionPointRec((Vector2){mX, mY}, playbutton)) {
                DrawRectangleRounded(playbutton, 0.2, 100, Fade(WHITE, 0.3));
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) fade = true;
            }

            //once play button is hit then fade to black for tet monologue
            if (fade) {
                DrawRectangle(0, 0, 800, 800, Fade(BLACK, fadeTracker));
                fadeTracker+=0.01;
            }
            if (fadeTracker >= 1.5) return 0;

        EndDrawing();
    }
    return -1;
}

int pause() {
    Texture2D tetHead = LoadTexture("resources/mistaTet4Forward.png");
    Font allFont = LoadFont("resources/allFont.ttf");
    Rectangle src = (Rectangle){0, 0, 70, 70}, dest = (Rectangle) {400, 275, 70*6, 70*6}; 
    int bobCounter = 0, bobSpeed = 10, yvel = -1, startWait = 60, counter = 0, add = 1;
    double fade = 0;
    bool wait, quit = false;
    while (!WindowShouldClose()) {
        if (counter == 15) {
                src.x+=70*add;
            if (src.x == 280) add = -1;
            if (src.x == 0) add = 1;
            counter = 0;
        }
        if (wait) {
            if (bobCounter == 50) {
                bobCounter = 0;
                wait = false;
            }
        }
        else if (bobCounter == bobSpeed) {
            dest.y += yvel;
            if ((dest.y <= 255 && yvel < 0) || (dest.y >= 270 && yvel > 0)) bobSpeed++;
            else if (bobSpeed != 5) bobSpeed--;
            if (dest.y == 250) {
                wait = true;
                yvel = 1;
            }
            if (dest.y == 275) {
                yvel = -1;
                wait = true;
            }
            bobCounter = 0;
        }

        if (startWait == 0) {
            if (IsKeyPressed(KEY_P)) return 1;
            else if (IsKeyPressed(KEY_M)) quit = true;
            else if (IsKeyDown(KEY_Q)) return -1;
        }
        else --startWait;
        
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(tetHead, src, dest, (Vector2){35*6, 35*6}, 0, WHITE);
            DrawTextEx(allFont, "P to resume", (Vector2){400-MeasureTextEx(allFont, "P to resume", 25, 1).x/2, 500}, 25, 1, WHITE);
            DrawTextEx(allFont, "M to quit", (Vector2){400-MeasureTextEx(allFont, "M to quit", 25, 1).x/2, 550}, 25, 1, WHITE);
            DrawTextEx(allFont, "Q to exit game", (Vector2){400-MeasureTextEx(allFont, "Q to exit game", 25, 1).x/2, 600}, 25, 1, WHITE);
            if (quit) {
                DrawRectangle(0, 0, 800, 800, Fade(WHITE, fade));
                fade+=0.01;
                if (fade >= 1) return 0;
            }
        EndDrawing();
        ++counter;
        ++bobCounter;
    }
    return -1;
}

int usePower(PowerupItem* p, Score* scr, Grid* grid) {
    if (p == nullptr) return 0;
    if (p->id == "multiplier") {
        PlaySound(LoadSound("resources/audio/usepowerup.wav"));
        Multiplier* m = dynamic_cast<Multiplier*>(p);
        scr->addMultiplier(m->multiplier);
        delete m;
    }
    else if (p->id == "laser") {
        delete p;
        return grid->lasers();
    }
    else if (p->id == "bomb") {
        delete p;
        return grid->bomb();
    }
    else if (p->id == "nuke") {
        delete p;
        return grid->nuke();
    }
    else if (p->id == "threeblock") {
        PlaySound(LoadSound("resources/audio/usepowerup.wav"));
        ThreeBlock* t = dynamic_cast<ThreeBlock*>(p);
        grid->changeNext(t->blockID);
        delete t;
    }
    else if (p->id == "plusmultiplier") {
        PlaySound(LoadSound("resources/audio/usepowerup.wav"));
        PlusMultiplier* m = dynamic_cast<PlusMultiplier*>(p);
        scr->addPermanentMultiplier(m->multiplier);
    }
    return 0;
}