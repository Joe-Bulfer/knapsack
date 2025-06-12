/*
gcc map-maker.c -o map-maker -I ../raylib-quickstart/build/external/raylib-master/src/ -L ../raylib-quickstart/bin/Debug/ -lraylib -lm -lpthread -ldl -lX11 && ./map-maker
*/

#define RAYGUI_IMPLEMENTATION

#include <string.h> //for LoadMap
#include <stdbool.h>

#include "raylib.h"
#include "raygui.h"
#include "raymath.h"
#include <stdio.h>

#define DB_X 1000
#define DB_Y 20
#define DB_F 20
#define DB_CL                 (Color){ 155, 21, 41, 255 }
#define G 500 
#define FPS 30 
#define TRANSPARENT_WHITE (Color){255, 255, 255, 128}
#define ERASER_PINK (Color){255, 105, 180, 128}

typedef struct Player {
    Rectangle rect;
    float speed;
    bool canJump;
} Player;

typedef struct Block {
    Rectangle rect;
    Texture2D tex;
    int blockID;
} Block;

typedef struct { //LoadMap returns this
    Block *blocks;
    int length; //number of blocks
} Map;

typedef struct InventorySlot {
    Rectangle rect;
    Texture2D tex;
    int blockID;
} InventorySlot;

int editMode = 1;

int canPlace = 0;
Vector2 mousePos;
int activeSlot = 0;
#define INVENTORY_SIZE 5
#define MAX_BLOCKS 190
Block blocks[MAX_BLOCKS];
int blockCount = 0;

// Function declarations
void PlayerControls(Player *player);
bool IsPlayerAgainstWall(Player *player, Block *envItems, int envItemsLength, float moveSpeed);
void PlayerControlsWithCollision(Player *player, Block *envItems, int envItemsLength,float *deltaTime);
void UpdateCameraCenter(Camera2D *camera, Player *player, int width, int height);
void UpdateActiveSlot();
void DrawInventory(InventorySlot *inventory);
void PlaceBlocks(Vector2 mousePos, InventorySlot *inventory, Block *blocks, int *blockCount);
void RemoveBlock(Vector2 mousePos, Block *blocks, int *blockCount);
void SaveMap(char *textBoxText, Block *blocks, int blockCount);
#define MAXCHAR 100
Map LoadMap(char *fileName);
void checkCollisions( Block *envItems, int envItemsLength, Player *player);

bool topCollision = false;

int main(void)
{
    bool textBoxEditMode = false;
    char textBoxText[64] = "Enter Map Name";
    const int screenWidth = 1400;
    const int screenHeight = 850;

    InitWindow(screenWidth, screenHeight, "map-maker");

    Player player = { 0 };
    player.rect = (Rectangle){ 300, 300, 30, 30 }; 
    player.speed = 0;
    player.canJump = false;

    Texture2D sandTex = LoadTexture("../old-raylib/assets/rocky-sand-10x10.png");        
    Texture2D dirtTex = LoadTexture("../old-raylib/assets/dirt-10x10.png");        
    Texture2D grassTex = LoadTexture("../old-raylib/assets/grass-10x10.png");        
    Texture2D waterTex = LoadTexture("../old-raylib/assets/water-10x10.png");        
    Texture2D eraserTex = LoadTexture("../old-raylib/assets/eraser-10x10.png");        
            
    
    //to try to load a map, just untoggle this, may or may not work.
    
    Map map = LoadMap("../old-raylib/new-testrtfsaeaadwdwaswasd.txt");
    int lenMapBlocks = map.length;
    //int blockCount = map.length;
    Block* mapBlocks = map.blocks;
    //int lenMapBlocks = sizeof(&mapBlocks) / sizeof(&mapBlocks[0]);
    //int lenMapBlocks  = *(&mapBlocks + 1) - mapBlocks;

    //int lenMapBlocks;
    for (int i = 0; i < lenMapBlocks ; i++) {
    
        switch (mapBlocks[i].blockID) {
            case 0: mapBlocks[i].tex = sandTex; break;
            case 1: mapBlocks[i].tex = dirtTex; break;
            case 2: mapBlocks[i].tex = grassTex; break;
            case 3: mapBlocks[i].tex = waterTex; break;
            //default: mapBlocks[i].tex = sandTex; break;
        }
        blocks[i] = mapBlocks[i];
        blockCount ++;
    }
    
    

    InventorySlot inventory[INVENTORY_SIZE] = {
        {{30, 30, 80, 80}, sandTex,0},
        {{130, 30, 80, 80}, dirtTex,1},
        {{230, 30, 80, 80}, grassTex,2},
        {{330, 30, 80, 80}, waterTex,3},
        {{430, 30, 80, 80}, eraserTex,4}
    };

    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.rect.x, player.rect.y };
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.zoom = 3.0f;

    SetTargetFPS(FPS); 

    // Main game loop
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        camera.zoom += ((float)GetMouseWheelMove() * 0.10f);
        if (camera.zoom > 4.0f) camera.zoom = 4.0f;
        else if (camera.zoom < 0.25f) camera.zoom = 0.25f;

        if (IsKeyPressed(KEY_R)) camera.zoom = 3.0;
        if (IsKeyPressed(KEY_P)) editMode = !editMode ;


        UpdateCameraCenter(&camera, &player, screenWidth, screenHeight);

        // Update the active inventory slot
        UpdateActiveSlot();

        // Draw
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
        
        // Draw the placed blocks
        for (int i = 0; i < blockCount; i++) {
            DrawTexture(blocks[i].tex, blocks[i].rect.x, blocks[i].rect.y, WHITE);
        }
       

        mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
        // this floor function gives grid like appearance
        mousePos.x = floorf(mousePos.x / 10) * 10.0f;
        mousePos.y = floorf(mousePos.y / 10) * 10.0f;

        if (!editMode){
            PlayerControlsWithCollision(&player,blocks,blockCount,&deltaTime);
            DrawRectangleRec(player.rect,RED);

            for (int i = 0;i<blockCount;i++){
                checkCollisions(&blocks[i],blockCount,&player);
            }
        }

        if (editMode){
            PlayerControls(&player);

            switch(activeSlot )
                {
                case 0: 
                    PlaceBlocks(mousePos, inventory, blocks, &blockCount);
                    DrawTexture(sandTex, mousePos.x, mousePos.y, TRANSPARENT_WHITE);
                    break;
                case 1:
                    PlaceBlocks(mousePos, inventory, blocks, &blockCount);
                    DrawTexture(dirtTex, mousePos.x, mousePos.y, TRANSPARENT_WHITE);
                    break;
                case 2:
                    PlaceBlocks(mousePos, inventory, blocks, &blockCount);
                    DrawTexture(grassTex, mousePos.x, mousePos.y, TRANSPARENT_WHITE);
                    break;
                case 3:
                    PlaceBlocks(mousePos, inventory, blocks, &blockCount);
                    DrawTexture(waterTex, mousePos.x, mousePos.y, TRANSPARENT_WHITE);
                    break;
                case 4:// eraser
                    RemoveBlock(mousePos, blocks, &blockCount);
                    DrawRectangle(mousePos.x, mousePos.y,10,10,ERASER_PINK);
                    break;
            }
        }


        EndMode2D();

        // Draw Inventory UI
        if (editMode){
            if (GuiTextBox((Rectangle){ 500, 715, 225, 70 }, textBoxText, 199, textBoxEditMode)) textBoxEditMode = !textBoxEditMode;
            DrawInventory(inventory);
        }

        DrawFPS(10, 10);
        // Debug
        DrawText(TextFormat("Player position: %.2f, %.2f", player.rect.x, player.rect.y), DB_X, DB_Y + 25, DB_F, DB_CL);
        DrawText(TextFormat("Block count: %d", blockCount), DB_X, DB_Y + 50, DB_F, DB_CL);
        DrawText(TextFormat("Active slot: %d", activeSlot), DB_X, DB_Y + 75, DB_F, DB_CL);
        DrawText(TextFormat("canJump: %d topCollision: %d", player.canJump,topCollision), DB_X, DB_Y + 100, DB_F, DB_CL);
        //DrawText(TextFormat("lenMapBlocks : %d",lenMapBlocks ), DB_X, DB_Y + 125, DB_F, DB_CL);
        //DrawText(TextFormat("textBoxText: %s",textBoxText), DB_X, DB_Y + 125, DB_F, DB_CL);

        EndDrawing();
    }
    SaveMap(textBoxText,blocks,blockCount);
    // Clean up textures
    for (int i = 0;i<INVENTORY_SIZE;i++){
        UnloadTexture(inventory[i].tex);
    }

    CloseWindow(); // Close the window and OpenGL context

    return 0;
}

void PlayerControlsWithCollision(Player *player, Block *envItems, int envItemsLength, float *deltaTime) {
    if (IsKeyDown(KEY_A)) {
        player->rect.x -= 4;
    } else if (IsKeyDown(KEY_D)) {
        player->rect.x += 4;
    }

    if (IsKeyPressed(KEY_SPACE) && player->canJump)
    {
        topCollision = false;
        player->speed = -400;
        player->canJump = false;
    }


    if (!topCollision) {
        player->rect.y += player->speed * (*deltaTime);
        player->speed += G*(*deltaTime);
        player->canJump = false;
    }

    else player->canJump = true;

    player->rect.y += 1; // wierd thing I have to do to constantly stay colliding with ground
}

// Function to move the player during edit mode
void PlayerControls(Player *player) {
    if (IsKeyDown(KEY_A)) {
        player->rect.x -= 4;
    } else if (IsKeyDown(KEY_D)) {
        player->rect.x += 4;
    }

    if (IsKeyDown(KEY_W)) {
        player->rect.y -= 4;
    } else if (IsKeyDown(KEY_S)) {
        player->rect.y += 4;
    }
}

// Function to update the camera center based on player position
void UpdateCameraCenter(Camera2D *camera, Player *player, int width, int height) {
    camera->offset = (Vector2){ width / 2.0f, height / 2.0f };
    camera->target = (Vector2){ player->rect.x, player->rect.y };
}

// Function to update the active inventory slot
void UpdateActiveSlot() {
    if (IsKeyPressed(KEY_ONE)) {
        activeSlot = 0;
    } else if (IsKeyPressed(KEY_TWO)) {
        activeSlot = 1;
    }
    else if (IsKeyPressed(KEY_THREE)) {
        activeSlot = 2;
    }
    else if (IsKeyPressed(KEY_FOUR)) {
        activeSlot = 3;
    }
    else if (IsKeyPressed(KEY_FIVE)) {
        activeSlot = 4;
    }
}

// Function to draw the inventory UI
void DrawInventory(InventorySlot *inventory) {
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        // Highlight the active slot with a border
        Color slotColor = (i == activeSlot) ? GOLD : (Color){ 128, 128, 128, 255 };

        // Draw the inventory slot with a background color
        DrawRectangleRec(inventory[i].rect, slotColor);

        DrawTextureEx(inventory[i].tex, (Vector2){inventory[i].rect.x + 20, inventory[i].rect.y + 20}, 0,4,WHITE);
    }
}

// Place blocks based on the active inventory slot
void PlaceBlocks(Vector2 mousePos, InventorySlot *inventory, Block *blocks, int *blockCount) {
    if (IsMouseButtonDown(0) && *blockCount < MAX_BLOCKS) {
        int canPlace = 1;
        
        // Check if the spot is already occupied
        for (int i = 0; i < *blockCount; i++) {
            Vector2 envPos = (Vector2){ blocks[i].rect.x, blocks[i].rect.y };
            if (Vector2Equals(envPos, mousePos)) {
                canPlace = 0;
                break;
            }
        }

        if (canPlace) {
            Texture2D currentTex = inventory[activeSlot].tex;
            int currentID = inventory[activeSlot].blockID;
            Block newBlock = { 
                (Rectangle){ mousePos.x, mousePos.y, 10, 10 },
                currentTex,currentID
            };
            blocks[*blockCount] = newBlock;
            (*blockCount)++;
        }
    }
}

//eraser
void RemoveBlock(Vector2 mousePos, Block *blocks, int *blockCount)
{
    if (IsMouseButtonDown(0)){
        for (int i = 0; i < *blockCount; i++) {
            if (CheckCollisionPointRec(mousePos, blocks[i].rect)) {
                // Shift blocks down in the array
                for (int j = i; j < *blockCount - 1; j++) {
                    blocks[j] = blocks[j + 1];
                }
                (*blockCount)--;  // Decrease the block count
                break;
            }
        }
    }
}
void SaveMap(char *textBoxText, Block *blocks, int blockCount) {
    FILE *file = fopen(textBoxText, "w");
    //FILE *file = fopen("saved_map.txt", "w");
    if (file == NULL) {
        //printf("Failed to open file for saving!\n");
        return;
    }

    for (int i = 0; i < blockCount; i++) {
        // Write the block's position and texture index to the file
        fprintf(file, "%.0f,%.0f,%d\n", blocks[i].rect.x, blocks[i].rect.y, blocks[i].blockID);
    }

    fclose(file);
    //printf("Map saved successfully!\n");
}

void checkCollisions(Block *envItems, int envItemsLength, Player *player) {
    Vector2 centerA, centerB;
    Vector2 subtract;
    Vector2 halfWidthA, halfWidthB;
    float minDistX, minDistY;
    topCollision = false;  

    for (int i = 0; i < envItemsLength; i++) {
        Rectangle boxA = envItems[i].rect;

        if (CheckCollisionRecs(boxA, player->rect)) {
            centerA = (Vector2){boxA.x + boxA.width / 2, boxA.y + boxA.height / 2};
            centerB = (Vector2){player->rect.x + player->rect.width / 2, player->rect.y + player->rect.height / 2};
            subtract = Vector2Subtract(centerA, centerB);
            halfWidthA = (Vector2){boxA.width * 0.5f, boxA.height * 0.5f};
            halfWidthB = (Vector2){player->rect.width * 0.5f, player->rect.height * 0.5f};
            minDistX = halfWidthA.x + halfWidthB.x - fabsf(subtract.x);
            minDistY = halfWidthA.y + halfWidthB.y - fabsf(subtract.y);

            if (minDistX < minDistY) {
                //collide left or right
                player->rect.x -= copysignf(minDistX, subtract.x); 
            } else {
                //collide top or bottom
                if (subtract.y > 0){
                    topCollision = true;
                    player->speed = 0;
                    //player->rect.y += 1; //prevent flickering
                }
                player->rect.y -= copysignf(minDistY, subtract.y); 
                player->speed = 0; // bounce off bottom of object
            } 
        }
    }
}

Map LoadMap(char *fileName){
    FILE *fp;
    char ch;
    int lines = 0;

    fp = fopen(fileName, "r");

    // Count the number of lines the file to determine number of blocks
    // this will be used to allocate space for blocks array
    while (!feof(fp)) {
        ch = fgetc(fp);
        if (ch == '\n') {
            lines++;
        }
    }

    Block* blocks = (Block*)malloc(lines * sizeof(Block));

    // Check if memory allocation was successful
    if (blocks == NULL) {
        //printf("Memory allocation failed!\n");
        exit(1); // Exit the program if allocation fails
    }

    char row[MAXCHAR];
    char *token; //for comma seperation

    // Re-open the file to read blocks
    fp = fopen(fileName, "r");
    int blockIndex = 0;

    // Read each line (block) from the file
    while (fgets(row, MAXCHAR, fp) != NULL) {

        // Tokenize the row using commas as the separator
        token = strtok(row, ",");

        // First token: x position (convert to float)
        if (token != NULL) {
            blocks[blockIndex].rect.x = strtof(token, NULL); // Convert to float
        }

        // Second token: y position (convert to float)
        token = strtok(NULL, ",");
        if (token != NULL) {
            blocks[blockIndex].rect.y = strtof(token, NULL); // Convert to float
        }

        // Third token: blockID (convert to integer)
        token = strtok(NULL, ",");
        if (token != NULL) {
            int id = atoi(token); // Convert to integer
            blocks[blockIndex].blockID = id;

        // blocks are always 10x10
        blocks[blockIndex].rect.width = 10;
        blocks[blockIndex].rect.height = 10;
        
        blockIndex++; // move to next block
        }
    }

    fclose(fp);//close file
    Map map = { blocks, lines };
    return map;
}

