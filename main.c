#include <raylib.h>
#include <raymath.h>

#define NOB_IMPLEMENTATION
#include "./nob.h"

#define ASSETS_DIR "./assets"
#define MOXICA_CHESS_ICONS_DIR ASSETS_DIR "/moxica_chess_set/Icons"
#define MOXICA_CHESS_PIECES_DIR ASSETS_DIR "/moxica_chess_set/Pieces"

int main(void)
{
    InitWindow(800, 480, "Chess Score Keeper");
    SetTargetFPS(60);

    Texture2D whiteKing = LoadTexture(MOXICA_CHESS_ICONS_DIR"/White_king.png");
    Texture2D blackKing = LoadTexture(MOXICA_CHESS_ICONS_DIR"/Black_king.png");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        DrawTextureEx(whiteKing, CLITERAL(Vector2){10, 10}, 0, 0.25f, WHITE);
        DrawTextureEx(blackKing, CLITERAL(Vector2){100, 10}, 0, 0.25f, WHITE);
        EndDrawing();
    }

    UnloadTexture(whiteKing);
    UnloadTexture(blackKing);
    CloseWindow();
    return 0;
}
