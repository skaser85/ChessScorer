#include <raylib.h>
#include <raymath.h>

#define NOB_IMPLEMENTATION
#include "./nob.h"

#define ASSETS_DIR "./assets"
#define PIECES_DIR ASSETS_DIR"/pieces"
#define FONTS_DIR ASSETS_DIR"/fonts"

#define PIECE_DIM_X 55 
#define PIECE_DIM_Y PIECE_DIM_X*2 
#define BUTTON_PADDING PIECE_DIM_X*0.2
#define BUTTON_MARGIN PIECE_DIM_X*0.45
#define BUTTON_FONT_SIZE PIECE_DIM_Y*0.5
#define BUTTON_DIM_X BUTTON_FONT_SIZE*1.5 
#define BUTTON_DIM_Y BUTTON_FONT_SIZE+(BUTTON_PADDING*2) 

#define TITLE_FONT_SIZE PIECE_DIM_Y*0.5
#define TITLE_Y_PADDING TITLE_FONT_SIZE/3

typedef struct {
  const char* text;
  bool active;
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
  Texture2D texture;
  Side side;
  PieceType pt;
  size_t value;
  const char* rank;
  size_t file;
  bool active;
} Piece;

typedef struct {
  Piece *items;
  size_t count;
  size_t capacity;
} Pieces;

typedef struct {
  Side side;
  Pieces pieces;
} Kingdom;


typedef struct {
  Font *items;
  size_t count;
  size_t capacity;
} Fonts;

typedef struct {
  Button *items;
  size_t count;
  size_t capacity;
} Buttons;

bool streq(const char* s1, const char* s2) {
  return strcmp(s1, s2) == 0;
}

Fonts LoadFonts() {
  Fonts fonts = {0};
  Font f = LoadFontEx(FONTS_DIR"/archeologicaps/Archeologicaps.ttf", TITLE_FONT_SIZE, NULL, 0);
  da_append(&fonts, f);
  f = LoadFontEx(FONTS_DIR"/atiba/Atiba.ttf", TITLE_FONT_SIZE, NULL, 0);
  da_append(&fonts, f);
  f = LoadFontEx(FONTS_DIR"/audiowide/Audiowide-Regular.ttf", TITLE_FONT_SIZE, NULL, 0);
  da_append(&fonts, f);
  return fonts;
}

void DrawButton(Button *b,  Font font, Vector2 pos, Vector2 dims) {
  Vector2 textDims = MeasureTextEx(font, b->text, BUTTON_FONT_SIZE, 1);
  Rectangle rec = { .x = pos.x, .y = pos.y, .width = dims.x, .height = dims.y };
  if (b->active) {
    DrawRectangleLinesEx(rec, 1, LIME);
  }
  DrawTextEx(font, b->text, (Vector2){(pos.x+rec.width/2)-(textDims.x/2), (pos.y+rec.height/2)-(textDims.y/2) }, BUTTON_FONT_SIZE, 1, WHITE);
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

const char* GetPieceLetter(PieceType pt) {
  switch (pt) {
    case PT_KING: return "K";
    case PT_QUEEN: return "Q";
    case PT_BISHOP: return "B";
    case PT_ROOK: return "R";
    case PT_KNIGHT: return "K";
    case PT_PAWN: return " ";
    default: return "";
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

Piece CreatePiece(Side side, PieceType pt, const char* piecesName) {
  Piece p = {0};
  p.active = false;
  p.side = side;
  p.pt = pt;
  p.value = GetPieceValue(pt);
  p.rank = "X";
  p.file = 0;
  p.texture = LoadTexture(temp_sprintf("%s/%s/%s_%s.png", PIECES_DIR, piecesName, GetSideName(side), GetPieceImageName(pt)));
  return p;
}

Kingdom CreateKingdom(Side side, const char* piecesName) {
  Kingdom k = {0};
  k.side = side;
  k.pieces = (Pieces){0};
  da_append(&k.pieces, CreatePiece(side, PT_KING, piecesName));
  da_append(&k.pieces, CreatePiece(side, PT_QUEEN, piecesName));
  da_append(&k.pieces, CreatePiece(side, PT_BISHOP, piecesName));
  da_append(&k.pieces, CreatePiece(side, PT_ROOK, piecesName));
  da_append(&k.pieces, CreatePiece(side, PT_KNIGHT, piecesName));
  da_append(&k.pieces, CreatePiece(side, PT_PAWN, piecesName));
  return k;
}

Buttons CreateLetterButtons() {
  Buttons bs = {0};
  Button b = { .text = "a", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "b", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "c", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "d", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "e", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "f", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "g", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "h", .active = false };
  da_append(&bs, b);
  return bs;
}

Buttons CreateNumberButtons() {
  Buttons bs = {0};
  Button b = { .text = "1", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "2", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "3", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "4", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "5", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "6", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "7", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "8", .active = false };
  da_append(&bs, b);
  return bs;
}

Buttons CreateActionButtons() {
  Buttons bs = {0};
  Button b = { .text = "x", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "+", .active = false };
  da_append(&bs, b);
  b = (Button){ .text = "#", .active = false };
  da_append(&bs, b);
  return bs;
}

void DrawPiece(Piece *p, Vector2 pos) {
  Rectangle rec = { .x = pos.x, .y = pos.y, .width = PIECE_DIM_X, .height = PIECE_DIM_Y };
  if (p->active) {
    DrawRectangleLinesEx(rec, 2, LIME);
  }
  float factor = 0.3f;
  Rectangle dest = { .x = rec.x + BUTTON_PADDING*factor, .y = rec.y + BUTTON_PADDING*factor, .width = rec.width - BUTTON_PADDING*2*factor, .height = rec.height - BUTTON_PADDING*2*factor };
  DrawTexturePro(p->texture, (Rectangle){0, 0, p->texture.width, p->texture.height}, dest, (Vector2){0, 0}, 0, WHITE);
}

void DrawKingdom(Kingdom *k, Vector2 *pos, Vector2 mouse, Piece **activePiece) {
  for (size_t i = 0; i < k->pieces.count; ++i) {
    Piece *currentPiece = &k->pieces.items[i];
    Rectangle rec = { .x = pos->x, .y = pos->y, .width = PIECE_DIM_X, .height = PIECE_DIM_Y };

    if (!currentPiece->active && CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      if (*activePiece != NULL) {
        (*activePiece)->active = false;
      }
      currentPiece->active = true;
      *activePiece = currentPiece;
    }

    DrawPiece(currentPiece, *pos);

    pos->x += PIECE_DIM_X + BUTTON_PADDING + BUTTON_MARGIN;
  }
}

Vector2 GetSideTextPosition(Side side, Font font) {
  const char* text = side == SIDE_WHITE ? "WHITE" : "BLACK";
  Vector2 dims = MeasureTextEx(font, text, 28, 1);
  int w = GetScreenWidth();
  int x = 0;
  if (side == SIDE_WHITE) x = (w/4*1)-(dims.x);
  else x = (w/4*3)-(dims.x);
  return (Vector2) { x, TITLE_Y_PADDING };
}

int DrawTitle(Side *side, Font font, Vector2 mouse) {
  Vector2 wPos= GetSideTextPosition(SIDE_WHITE, font);
  Vector2 bPos = GetSideTextPosition(SIDE_BLACK, font);
  
  int title_bottom = (TITLE_FONT_SIZE+(TITLE_Y_PADDING*2));

  Rectangle whiteRec = { .x = 0, .y = 0, .width = GetScreenWidth()/2, .height = title_bottom };
  Rectangle blackRec = { .x = GetScreenWidth()/2, .y = 0, .width = GetScreenWidth()/2, .height = title_bottom };

  Color wColor = *side == SIDE_WHITE ? BLACK : WHITE;
  Color bColor = *side == SIDE_WHITE ? WHITE : BLACK; 

  DrawRectangleRec(*side == SIDE_WHITE ? whiteRec : blackRec, WHITE); 
  DrawLine(0, title_bottom, GetScreenWidth(), title_bottom, WHITE);
  DrawTextEx(font, "WHITE", wPos, TITLE_FONT_SIZE, 1, wColor);
  DrawTextEx(font, "BLACK", bPos, TITLE_FONT_SIZE, 1, bColor);

  if (*side == SIDE_WHITE && CheckCollisionPointRec(mouse, blackRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) *side = SIDE_BLACK;
  if (*side == SIDE_BLACK && CheckCollisionPointRec(mouse, whiteRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) *side = SIDE_WHITE; 

  return title_bottom;
}

void DrawButtonsRow(Buttons bs, Vector2 *rowPos, Font font, Vector2 mouse, Button **activeTracker) {
  for (size_t i = 0; i < bs.count; ++i) {
    Button *currentButton = &bs.items[i];
    
    Rectangle rec = { .x = rowPos->x, .y = rowPos->y, .width = BUTTON_DIM_X, .height = BUTTON_DIM_Y };
    if (!currentButton->active && CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      
      if (*activeTracker != NULL) {
        (*activeTracker)->active = false;
      }
      
      currentButton->active = true;
      *activeTracker = currentButton;
    }

    DrawButton(currentButton, font, *rowPos, (Vector2){BUTTON_DIM_X, BUTTON_DIM_Y});
    
    rowPos->x += PIECE_DIM_X + BUTTON_PADDING + BUTTON_MARGIN;
  }
}


int main(void)
{
      // 1. Initialize with temporary values so we can probe the hardware
    InitWindow(100, 100, "Chess Score Keeper");

    // 2. Query the physical monitor's current resolution dimensions
    int monitor = GetCurrentMonitor();
    int screenWidth = GetMonitorWidth(monitor);
    int screenHeight = GetMonitorHeight(monitor);

    // 3. Resize the window buffer to match your screen perfectly
    SetWindowSize(screenWidth, screenHeight);

    // 4. Toggle the hardware fullscreen state flags
    ToggleFullscreen();

    if (!IsWindowReady()) {
        fprintf(stderr, "ERROR: Raylib window context failed to open.\n");
        return 1;
    }

    SetTargetFPS(60);

    Fonts fonts = LoadFonts();
    Font current_font = fonts.items[2];

    Buttons letters = CreateLetterButtons();
    Buttons numbers = CreateNumberButtons();
    Buttons actionButtons = CreateActionButtons();

    const char* PIECES = "moxica";
    //const char* FONT_NAME = "archeologicaps";

    Kingdom white = CreateKingdom(SIDE_WHITE, PIECES);
    Kingdom black = CreateKingdom(SIDE_BLACK, PIECES);
  
    Side turn = SIDE_WHITE;

    Piece *activePiece = NULL;
    Button *activeAction = NULL;
    Button *activeLetter = NULL;
    Button *activeNumber = NULL;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        
        Vector2 mouse = GetMousePosition();
        
        int title_bottom = DrawTitle(&turn, current_font, mouse);
        
        Vector2 rowPos = (Vector2){BUTTON_MARGIN, title_bottom+BUTTON_PADDING};
        
        Kingdom *kingdom = turn == SIDE_WHITE ? &white : &black;
        DrawKingdom(kingdom, &rowPos, mouse, &activePiece);
        
        int bottom = rowPos.y;
        rowPos.x = (PIECE_DIM_X + BUTTON_MARGIN + BUTTON_PADDING) * 6;
        rowPos.y = (rowPos.y + PIECE_DIM_Y) - (BUTTON_DIM_Y);
        DrawButtonsRow(actionButtons, &rowPos, current_font, mouse, &activeAction); 
        
        rowPos.y = bottom;
        
        rowPos.x = BUTTON_MARGIN;
        rowPos.y = (rowPos.y + PIECE_DIM_Y + BUTTON_MARGIN);
        DrawButtonsRow(letters, &rowPos, current_font, mouse, &activeLetter);
        
        rowPos.x = BUTTON_MARGIN;
        rowPos.y = (rowPos.y + (BUTTON_DIM_Y) + BUTTON_MARGIN);
        DrawButtonsRow(numbers, &rowPos, current_font, mouse, &activeNumber);

        const char* pieceText = activePiece ? GetPieceLetter(activePiece->pt) : " ";
        const char* letterText = activeLetter ? activeLetter->text : " ";
        const char* numberText = activeNumber ? activeNumber->text : " ";
        const char* actionText = activeAction ? activeAction->text : "";
        const char* move = temp_sprintf("Move: %s%s%s%s%s%s", 
            (streq(actionText, "x") && streq(pieceText, " ")) ? "x" : "", 
            pieceText, 
            (streq(actionText, "x") && !streq(pieceText, " ")) ? "x" : "",
            letterText, 
            numberText,
            ""); 
        DrawTextEx(current_font, move, (Vector2){ BUTTON_PADDING, GetScreenHeight() - (BUTTON_PADDING + TITLE_FONT_SIZE)}, TITLE_FONT_SIZE, 1, WHITE);

        Button reset = { .text = "RESET", .active = false };
        Vector2 dims = MeasureTextEx(current_font, reset.text, BUTTON_FONT_SIZE*0.8, 1);
        Vector2 size = { .x = dims.x + (BUTTON_PADDING*2), .y = (BUTTON_FONT_SIZE*0.8+BUTTON_PADDING) };
        Rectangle rec = { .x = GetScreenWidth()/2, .y = GetScreenHeight() - size.y - BUTTON_PADDING, .width = size.x, .height = size.y };
        DrawRectangleRec(rec, RED);
        DrawTextEx(current_font, reset.text, (Vector2){rec.x+BUTTON_PADDING,rec.y+BUTTON_PADDING/2}, BUTTON_FONT_SIZE*0.8, 1, WHITE);
        if (CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          if (activePiece) activePiece->active = false;
          activePiece = NULL;
          if (activeAction) activeAction->active = false;
          activeAction = NULL;
          if (activeLetter) activeLetter->active = false;
          activeLetter = NULL;
          if (activeNumber) activeNumber->active = false;
          activeNumber = NULL; 
        }

        Button submit = { .text = "SUBMIT", .active = false };
        dims = MeasureTextEx(current_font, submit.text, BUTTON_FONT_SIZE*0.8, 1);
        size = (Vector2){ .x = dims.x + (BUTTON_PADDING*4), .y = (BUTTON_FONT_SIZE*0.8+BUTTON_PADDING) };
        rec = (Rectangle){ .x = rec.x+rec.width+BUTTON_PADDING, .y = GetScreenHeight() - size.y - BUTTON_PADDING, .width = size.x, .height = size.y };
        DrawRectangleRec(rec, GREEN);
        DrawTextEx(current_font, submit.text, (Vector2){rec.x+BUTTON_PADDING*2,rec.y+BUTTON_PADDING/2}, BUTTON_FONT_SIZE*0.8, 1, WHITE);
        if (CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          // TODO: record the move
          turn = turn == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;
          if (activePiece) activePiece->active = false;
          activePiece = NULL;
          if (activeAction) activeAction->active = false;
          activeAction = NULL;
          if (activeLetter) activeLetter->active = false;
          activeLetter = NULL;
          if (activeNumber) activeNumber->active = false;
          activeNumber = NULL; 
        }
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
