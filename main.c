#include <raylib.h>
#include <raymath.h>

#define NOB_IMPLEMENTATION
#include "./nob.h"

#define ASSETS_DIR "./assets"
#define MOXICA_CHESS_ICONS_DIR ASSETS_DIR "/moxica_chess_set/Icons"
#define MOXICA_CHESS_PIECES_DIR ASSETS_DIR "/moxica_chess_set/Pieces"

#define PIECE_DIM_X 50
#define PIECE_DIM_Y 100

typedef struct {
  Vector2 position;
  Vector2 dims;
  Texture2D tex;
} Button;

typedef enum {
  SIDE_WHITE,
  SIDE_BLACK
} Side;

typedef enum {
  PT_KING,
  PT_QUEEN,
  PT_BISHOP,
  PT_ROOK,
  PT_KNIGHT,
  PT_PAWN
} PieceType;

typedef struct {
  Texture2D icon;
  Texture2D piece;
  Side side;
  PieceType pt;
  size_t value;
  const char* rank;
  size_t file;
} Piece;

typedef struct {
  Side side;
  Piece king;
  Piece queen;
  Piece bishop;
  Piece rook;
  Piece knight;
  Piece pawn; 
} Kingdom;

void DrawButton(Button b) {
  Rectangle rec = { .x = b.position.x, .y = b.position.y, .width = b.dims.x, .height = b.dims.y };
  DrawRectangleLinesEx(rec, 2, WHITE);
  size_t PADDING = 3;
  Rectangle dest = { .x = rec.x + PADDING, .y = rec.y + PADDING, .width = rec.width - PADDING*2, .height = rec.height - PADDING*2 };
  DrawTexturePro(b.tex, (Rectangle){0, 0, b.tex.width, b.tex.height}, dest, (Vector2){0, 0}, 0, WHITE);
}

size_t GetPieceValue(PieceType pt) {
  switch (pt) {
    case PT_KING: return 0;
    case PT_QUEEN: return 9;
    case PT_BISHOP: return 5;
    case PT_ROOK: return 5;
    case PT_KNIGHT: return 3;
    case PT_PAWN: return 1;
    default: return 0;
  }
}

const char* GetPieceImageName(PieceType pt) {
  switch (pt) {
    case PT_KING: return "king";
    case PT_QUEEN: return "queen";
    case PT_BISHOP: return "bishop";
    case PT_ROOK: return "rook";
    case PT_KNIGHT: return "knight";
    case PT_PAWN: return "pawn";
    default: return "";
  }
}

const char* GetSideName(Side side) {
  if (side == SIDE_WHITE) return "white";
  return "black";
}

Piece CreatePiece(Side side, PieceType pt) {
  Piece p = {0};
  p.side = side;
  p.pt = pt;
  p.value = GetPieceValue(pt);
  p.rank = "X";
  p.file = 0;
  p.icon = LoadTexture(temp_sprintf("%s/%s_%s.png", MOXICA_CHESS_ICONS_DIR, GetSideName(side), GetPieceImageName(pt)));
  p.piece= LoadTexture(temp_sprintf("%s/%s_%s.png", MOXICA_CHESS_PIECES_DIR, GetSideName(side), GetPieceImageName(pt)));
  return p;
}

Kingdom CreateKingdom(Side side) {
  Kingdom k = {0};
  k.side = side;
  k.king = CreatePiece(side, PT_KING);
  k.queen = CreatePiece(side, PT_QUEEN);
  k.bishop = CreatePiece(side, PT_BISHOP);
  k.rook = CreatePiece(side, PT_ROOK);
  k.knight = CreatePiece(side, PT_KNIGHT);
  k.pawn = CreatePiece(side, PT_PAWN);
  return k;
}

void DrawPiece(Piece p, Vector2 pos) {
  Rectangle rec = { .x = pos.x, .y = pos.y, .width = PIECE_DIM_X, .height = PIECE_DIM_Y };
  DrawRectangleLinesEx(rec, 2, WHITE);
  size_t PADDING = 3;
  Rectangle dest = { .x = rec.x + PADDING, .y = rec.y + PADDING, .width = rec.width - PADDING*2, .height = rec.height - PADDING*2 };
  //DrawTexturePro(p.icon, (Rectangle){0, 0, p.icon.width, p.icon.height}, dest, (Vector2){0, 0}, 0, WHITE);
  DrawTexturePro(p.piece, (Rectangle){0, 0, p.piece.width, p.piece.height}, dest, (Vector2){0, 0}, 0, WHITE);
}

void DrawKingdom(Kingdom k, Vector2 pos) {
  DrawPiece(k.king, pos); 
}

int main(void)
{
    InitWindow(800, 480, "Chess Score Keeper");
    SetTargetFPS(60);

    Kingdom white = CreateKingdom(SIDE_WHITE);
    Kingdom black = CreateKingdom(SIDE_BLACK);
   
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        DrawKingdom(white, (Vector2){10, 10});
        DrawKingdom(black, (Vector2){10, 10+PIECE_DIM_Y+5});
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
