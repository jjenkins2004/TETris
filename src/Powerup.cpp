#include "Powerup.h"

Powerup::Powerup() {
    for (int i = 0; i < 3; i++) {
        currPower.push_back(new PowerupItem());
    }
}

void Powerup::drawPowerup() {
    
    //drawing border and powerup placeholders
    DrawRectangleRoundedLines(powerBoard, 0.2, 100, 5, (Color){80, 60, 60, 255});
    DrawCircleV(circle1, 25, circleColor);
    DrawCircleV(circle1, 22, BLACK);
    DrawCircleV(circle2, 25, circleColor);
    DrawCircleV(circle2, 22, BLACK);
    DrawCircleV(circle3, 25, circleColor);
    DrawCircleV(circle3, 22, BLACK);

    //drawing text
    Color DarkRed = {static_cast<unsigned char>(255-level*1.4), 0, 0, 255};
    DrawTextEx(allFont, "1", (Vector2){240, 715}, 15, 0, DarkRed);
    DrawTextEx(allFont, "1", (Vector2){240, 715}, 15, 0, Fade((Color){65, 170, 255, 255}, 1-level/60.0));
    DrawTextEx(allFont, "2", (Vector2){310, 715}, 15, 0, DarkRed);
    DrawTextEx(allFont, "2", (Vector2){310, 715}, 15, 0, Fade((Color){65, 170, 255, 255}, 1-level/60.0));
    DrawTextEx(allFont, "3", (Vector2){380, 715}, 15, 0, DarkRed);
    DrawTextEx(allFont, "3", (Vector2){380, 715}, 15, 0, Fade((Color){65, 170, 255, 255}, 1-level/60.0));

    //drawing powerups
    for (int i = 0; i < 3; i++) {   
        if (currPower[i]->id != "null") {
            currPower[i]->DrawItem();
        }
    }

    item* it = spawnedPower.head;
    bool removed = false;
    while(it != nullptr) {
        if (it->curr->removed) {
            it->curr->spaz();
            if (it->curr->fade <= 0) {
                    item* temp = it; it = it->next;
                    removed = true;
                    bool added = false;
                    if (temp->curr->id == "multiplier" && !temp->curr->positive) {
                        Multiplier* m = dynamic_cast<Multiplier*>(temp->curr);
                        std::cout << "added " << m->multiplier << std::endl;
                        src->addMultiplier(m->multiplier);
                    }
                    else {
                        for (int i = 0; i < 3; i++) {
                            if (currPower[i]->id == "null") {
                                currPower[i] = temp->curr;
                                if (i == 0) {
                                    currPower[i]->pos.x = circle1.x; currPower[i]->pos.y = circle1.y;
                                }
                                else if (i == 1) {
                                    currPower[i]->pos.x = circle2.x; currPower[i]->pos.y = circle2.y;
                                }
                                else if (i == 2) {
                                    currPower[i]->pos.x = circle3.x; currPower[i]->pos.y = circle3.y;
                                }
                                currPower[i]->fade = 1;
                                added = true;
                                break;
                            }
                        }
                    }
                    if (!added) {
                        delete temp->curr;
                    }
                    spawnedPower.remove(temp);
                }
        }
        else {
            it->curr->moveItem();
            it->curr->DrawItem();
            it->curr->time--;
            if (CheckCollisionPointCircle(GetMousePosition(), it->curr->pos, 30) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                it->curr->removed = true;
            }
        }
        if (it != nullptr && it->curr->time <= 0 && !it->curr->removed) {
            it->curr->fade-=0.01;
            if (it->curr->fade <= 0) {
                 item* temp = it; it = it->next;
                 delete temp->curr;
                 spawnedPower.remove(temp);
                 removed = true;
            }
        }
        if (!removed) {
            it = it->next;
        }
        else {
            removed = false;
        }
    }
}

void Powerup::spawnPowerup() {
    bool positive = false;
    if (GetRandomValue(1,2) == 1) {
        positive = true;
    }
    if (positive) {
        int rand = GetRandomValue(1, 50);
        //x2 multiplier
        if (rand <= 10) {
            spawnedPower.push_back(new Multiplier(2, 600, x2, (Vector2){0, static_cast<float>(GetRandomValue(20, 35))}));
        }
        //x1.5 multiplier
        else if (rand <= 25 && rand > 10) {
            spawnedPower.push_back(new Multiplier(1.5, 1200, x1_5, (Vector2){0, static_cast<float>(GetRandomValue(0, 20))}));
        }
        //x1.2 multiplier
        else if (rand <=50 && rand > 25) {
            spawnedPower.push_back(new Multiplier(1.2, 1500, x1_2, (Vector2){0, static_cast<float>(GetRandomValue(0, 15))}));
        }  
        //
    }
    else {
        int rand = GetRandomValue(1, 3);
        //negative multiplier
        if (rand <= 1) {
            spawnedPower.push_back(new Multiplier(-1, 1500, xNegative, (Vector2){0, static_cast<float>(GetRandomValue(0,20))}));
        }
        else if (rand <= 3) {
            spawnedPower.push_back(new Multiplier(0.7, 1500, x0_7, (Vector2){0, static_cast<float>(GetRandomValue(0,20))}));
        }
    }
}

void Powerup::usePowerup(int k) {
    PowerupItem* p;
    p = currPower[k-1];
    currPower[k-1] = new PowerupItem();
    if (p->id == "multiplier") {
        Multiplier* m = dynamic_cast<Multiplier*>(p);
        src->addMultiplier(m->multiplier);
        delete m;
    }
}