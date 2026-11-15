#include <time.h>

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
    PT_NONE = 0,
    PT_KING,
    PT_QUEEN,
    PT_BISHOP,
    PT_ROOK,
    PT_KNIGHT,
    PT_PAWN
} PieceType;

size_t GetPieceValue(PieceType pt) {
    switch (pt) {
        case PT_KING:   return 0;
        case PT_QUEEN:  return 9;
        case PT_BISHOP: return 5;
        case PT_ROOK:   return 5;
        case PT_KNIGHT: return 3;
        case PT_PAWN:   return 1;
        case PT_NONE:
        default:        return 0;
    }
}

const char* GetPieceLetter(PieceType pt) {
    switch (pt) {
        case PT_KING:   return "K";
        case PT_QUEEN:  return "Q";
        case PT_BISHOP: return "B";
        case PT_ROOK:   return "R";
        case PT_KNIGHT: return "N";
        case PT_PAWN:
        case PT_NONE:
        default:        return "";
    }
}

const char* GetPieceImageName(PieceType pt) {
    switch (pt) {
        case PT_KING:   return "king";
        case PT_QUEEN:  return "queen";
        case PT_BISHOP: return "bishop";
        case PT_ROOK:   return "rook";
        case PT_KNIGHT: return "knight";
        case PT_PAWN:   return "pawn";
        case PT_NONE:
        default:        return "";
    }
}

typedef enum {
  SCREEN_START_GAME,
  SCREEN_PLAYER_SETUP,
  SCREEN_MOVES_LIST,
  SCREEN_MOVE_SELECTION
} ScreenState;

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

typedef struct {
    const char **items;
    size_t count;
    size_t capacity;
} PlayerList;

typedef struct {
    size_t count;
    size_t fullmove;
    Piece piece;
    const char *fromFile;   // Disambiguation file ("a"-"h", or "" if none)
    const char *fromRank;   // Disambiguation rank ("1"-"8", or "" if none)
    const char *rank;       // Destination rank ("1"-"8")
    const char *file;       // Destination file ("a"-"h")
    const char *promotion;  // "=Q", "=R", "=B", "=N", or ""
    const char *castle;     // "O-O", "O-O-O", or ""
    bool capture;
    bool check;
    bool checkmate;
    struct tm *timestamp;
} Ply;

typedef struct {
  Ply *items;
  size_t count;
  size_t capacity;
} Plys;

typedef struct {
  const char* name;
  Side side;
  Plys plys; 
} Player;

typedef struct {
  size_t count;
  Ply white;
  Ply black;
} Fullmove;

typedef struct {
  Fullmove *items;
  size_t count;
  size_t capacity;
} Fullmoves;

typedef struct {
  size_t id;
  struct tm *timestamp;
  Player white;
  Player black;
  Fullmoves moves;
} Match;

typedef struct {
    Font font;
    Font movesListFont;
    Kingdom white;
    Kingdom black;
    Side turn;
    const char *piecesName;
    Piece *activePiece;
    Button *activeAction;
    Button *activeLetter;
    Button *activeNumber;
    Button *activePromotion;
    Button *activeCastle;
    const char *fromFile;
    const char *fromRank;
    bool disambigMode;
    ScreenState screen;
    Buttons letters;
    Buttons numbers;
    Buttons actions;
    Buttons promotions;
    Buttons castles;
    PlayerList availablePlayers;
    Match match;
    size_t movesListPage;
} Game;

struct tm *timestamp() {
  time_t t = time(NULL);
  return localtime(&t);
}

const char* dump_ts(struct tm *time_info) {
 // Format matches standard PGN date strings nicely: YYYY.MM.DD HH:MM:SS
 return TextFormat("%04d.%02d.%02d %02d:%02d:%02d", 
                    time_info->tm_year + 1900, 
                    time_info->tm_mon + 1, 
                    time_info->tm_mday,
                    time_info->tm_hour, 
                    time_info->tm_min, 
                    time_info->tm_sec);
}

bool streq(const char* s1, const char* s2) {
  return strcmp(s1, s2) == 0;
}

const char* GetScreenStateText(ScreenState state) {
  switch (state) {
    case SCREEN_START_GAME: return "START_GAME";
    case SCREEN_PLAYER_SETUP: return "PLAYER_SETUP";
    case SCREEN_MOVES_LIST: return "MOVES_LIST";
    case SCREEN_MOVE_SELECTION: return "MOVE_SELECTION";
    default: return temp_sprintf("%d", state);
  }
}

Fonts LoadFonts() {
  Fonts fonts = {0};
  Font f = LoadFontEx(FONTS_DIR"/archeologicaps/Archeologicaps.ttf", TITLE_FONT_SIZE, NULL, 0);
  da_append(&fonts, f);
  f = LoadFontEx(FONTS_DIR"/atiba/Atiba.ttf", TITLE_FONT_SIZE, NULL, 0);
  da_append(&fonts, f);
  f = LoadFontEx(FONTS_DIR"/audiowide/Audiowide-Regular.ttf", TITLE_FONT_SIZE, NULL, 0);
  da_append(&fonts, f);
  f = LoadFontEx(FONTS_DIR"/semi-coder/Semi-Coder-Regular.otf", TITLE_FONT_SIZE, NULL, 0);
  da_append(&fonts, f);
  return fonts;
}

Buttons CreatePromotionButtons() {
    Buttons bs = {0};
    da_append(&bs, ((Button){ .text = "=Q", .active = false }));
    da_append(&bs, ((Button){ .text = "=R", .active = false }));
    da_append(&bs, ((Button){ .text = "=B", .active = false }));
    da_append(&bs, ((Button){ .text = "=N", .active = false }));
    return bs;
}

Buttons CreateCastleButtons() {
    Buttons bs = {0};
    da_append(&bs, ((Button){ .text = "O-O",   .active = false }));
    da_append(&bs, ((Button){ .text = "O-O-O", .active = false }));
    return bs;
}

const char *FormatPlyPGN(Ply *ply) {
    const char *suffix = ply->checkmate ? "#" : (ply->check ? "+" : "");

    // Castling overrides piece/square coordinates
    if (ply->castle && strlen(ply->castle) > 0) {
        return temp_sprintf("%s%s", ply->castle, suffix);
    }

    const char *piece  = GetPieceLetter(ply->piece.pt);
    const char *fFile  = ply->fromFile ? ply->fromFile : "";
    const char *fRank  = ply->fromRank ? ply->fromRank : "";
    const char *cap    = ply->capture ? "x" : "";
    const char *file   = ply->file ? ply->file : "";
    const char *rank   = ply->rank ? ply->rank : "";
    const char *promo  = ply->promotion ? ply->promotion : "";

    return temp_sprintf("%s%s%s%s%s%s%s%s", piece, fFile, fRank, cap, file, rank, promo, suffix);
}

void DrawButton(Button *b,  Font font, Vector2 pos, Vector2 dims) {
  Vector2 textDims = MeasureTextEx(font, b->text, BUTTON_FONT_SIZE, 1);
  Rectangle rec = { .x = pos.x, .y = pos.y, .width = dims.x, .height = dims.y };
  if (b->active) {
    DrawRectangleLinesEx(rec, 1, LIME);
  }
  DrawTextEx(font, b->text, (Vector2){(pos.x+rec.width/2)-(textDims.x/2), (pos.y+rec.height/2)-(textDims.y/2) }, BUTTON_FONT_SIZE, 1, WHITE);
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

PlayerList CreatePlayerList() {
  PlayerList pl = {0};
  da_append(&pl, "Aaron");
  da_append(&pl, "Holden");
  da_append(&pl, "Juli");
  da_append(&pl, "Mitchell");
  da_append(&pl, "Steve");
  return pl;
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

void DrawScreen_StartGame(Game *game, Vector2 mouse) {
  Vector2 dims = MeasureTextEx(game->font, "START GAME", TITLE_FONT_SIZE, 1);
  Vector2 size = { .x = dims.x + (BUTTON_PADDING*2), .y = TITLE_FONT_SIZE+BUTTON_PADDING*2 };
  Rectangle rec = { .x = GetScreenWidth()/2-size.x/2, .y = GetScreenHeight()/2-size.y/2, .width = size.x, .height = size.y };
  DrawRectangleRec(rec, PINK);
  DrawRectangleLinesEx(rec, 2, WHITE);
  DrawTextEx(game->font, "START GAME", (Vector2){rec.x+BUTTON_PADDING,rec.y+BUTTON_PADDING}, TITLE_FONT_SIZE, 1, WHITE);
  if (CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      game->match.timestamp = timestamp();
      game->match.moves = (Fullmoves){0};
      game->movesListPage = 0;
      game->screen = SCREEN_PLAYER_SETUP;
  }

}
void DrawScreen_PlayerSetup(Game *game, Vector2 mouse) {
    // Title header
    DrawTextEx(game->font, "Player Setup", (Vector2){ BUTTON_MARGIN, BUTTON_MARGIN }, TITLE_FONT_SIZE, 1, WHITE);

    float startY = BUTTON_MARGIN + TITLE_FONT_SIZE + BUTTON_PADDING;
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();
    
    // Define column center positions at 25% and 75% of the screen width
    float leftCenterMark = screenWidth * 0.25f;
    float rightCenterMark = screenWidth * 0.75f;
    float boxWidth = 200;

    // Small font and text-driven compact box height
    float nameFontSize = BUTTON_FONT_SIZE * 0.75f; 
    Vector2 textSample = MeasureTextEx(game->font, "Tg", nameFontSize, 1);
    float boxHeight = textSample.y + (BUTTON_PADDING * 0.4f);

    // Column Headers centered at 25% and 75%
    Vector2 whiteTitleDims = MeasureTextEx(game->font, "White Player", BUTTON_FONT_SIZE * 0.5, 1);
    DrawTextEx(game->font, "White Player", (Vector2){ leftCenterMark - (whiteTitleDims.x / 2), startY }, BUTTON_FONT_SIZE * 0.5, 1, LIGHTGRAY);

    Vector2 blackTitleDims = MeasureTextEx(game->font, "Black Player", BUTTON_FONT_SIZE * 0.5, 1);
    DrawTextEx(game->font, "Black Player", (Vector2){ rightCenterMark - (blackTitleDims.x / 2), startY }, BUTTON_FONT_SIZE * 0.5, 1, LIGHTGRAY);

    float listY = startY + (BUTTON_FONT_SIZE * 0.5) + BUTTON_PADDING * 0.4f;

    // Render available players list for both columns
    for (size_t i = 0; i < game->availablePlayers.count; ++i) {
        const char *player = game->availablePlayers.items[i];
        float itemY = listY + i * (boxHeight + BUTTON_PADDING * 0.2f); // Very tight vertical spacing

        // --- White Player Column Item ---
        float leftBoxX = leftCenterMark - (boxWidth / 2);
        Rectangle whiteRec = { .x = leftBoxX, .y = itemY, .width = boxWidth, .height = boxHeight };
        bool isWhiteSelected = (game->match.white.name && strcmp(game->match.white.name, player) == 0);
        
        DrawRectangleRec(whiteRec, isWhiteSelected ? PURPLE : DARKGRAY);
        DrawRectangleLinesEx(whiteRec, 1, WHITE);
        
        Vector2 textDims = MeasureTextEx(game->font, player, nameFontSize, 1);
        Vector2 textPos = { 
            whiteRec.x + (whiteRec.width - textDims.x) / 2, 
            whiteRec.y + (whiteRec.height - textDims.y) / 2 
        };
        DrawTextEx(game->font, player, textPos, nameFontSize, 1, WHITE);
        
        if (CheckCollisionPointRec(mouse, whiteRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            game->match.white.name = player;
            game->match.white.side = SIDE_WHITE;
        }

        // --- Black Player Column Item ---
        float rightBoxX = rightCenterMark - (boxWidth / 2);
        Rectangle blackRec = { .x = rightBoxX, .y = itemY, .width = boxWidth, .height = boxHeight };
        bool isBlackSelected = (game->match.black.name && strcmp(game->match.black.name, player) == 0);
        
        DrawRectangleRec(blackRec, isBlackSelected ? PURPLE : DARKGRAY);
        DrawRectangleLinesEx(blackRec, 1, WHITE);
        
        Vector2 blackTextDims = MeasureTextEx(game->font, player, nameFontSize, 1);
        Vector2 blackTextPos = { 
            blackRec.x + (blackRec.width - blackTextDims.x) / 2, 
            blackRec.y + (blackRec.height - blackTextDims.y) / 2 
        };
        DrawTextEx(game->font, player, blackTextPos, nameFontSize, 1, WHITE);
        
        if (CheckCollisionPointRec(mouse, blackRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            game->match.black.name = player;
            game->match.black.side = SIDE_BLACK;
        }
    }

    // Bottom Navigation Buttons (BACK and START) safely positioned above screen bottom
    Button back = { .text = "BACK", .active = false };
    Vector2 dims = MeasureTextEx(game->font, back.text, BUTTON_FONT_SIZE * 0.7, 1);
    Vector2 size = { .x = dims.x + (BUTTON_PADDING * 2), .y = (BUTTON_FONT_SIZE * 0.7 + BUTTON_PADDING) };
    
    Button next = { .text = "START", .active = false };
    Vector2 nextDims = MeasureTextEx(game->font, next.text, BUTTON_FONT_SIZE * 0.7, 1);
    Vector2 nextSize = { .x = nextDims.x + (BUTTON_PADDING * 4), .y = (BUTTON_FONT_SIZE * 0.7 + BUTTON_PADDING) };

    float totalWidth = size.x + BUTTON_PADDING + nextSize.x;
    float startX = screenWidth - totalWidth - BUTTON_MARGIN;
    float buttonY = screenHeight - size.y - BUTTON_MARGIN;

    // Back Button
    Rectangle recBack = { .x = startX, .y = buttonY, .width = size.x, .height = size.y };
    DrawRectangleRec(recBack, DARKGRAY);
    DrawRectangleLinesEx(recBack, 1, WHITE);
    DrawTextEx(game->font, back.text, (Vector2){ recBack.x + BUTTON_PADDING, recBack.y + BUTTON_PADDING / 2 }, BUTTON_FONT_SIZE * 0.7, 1, WHITE);
    if (CheckCollisionPointRec(mouse, recBack) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game->screen = SCREEN_START_GAME;
    }

    // Next/Start Button
    Rectangle recNext = { .x = recBack.x + recBack.width + BUTTON_PADDING, .y = buttonY, .width = nextSize.x, .height = nextSize.y };
    DrawRectangleRec(recNext, GREEN);
    DrawRectangleLinesEx(recNext, 1, WHITE);
    DrawTextEx(game->font, next.text, (Vector2){ recNext.x + BUTTON_PADDING * 2, recNext.y + BUTTON_PADDING / 2 }, BUTTON_FONT_SIZE * 0.7, 1, WHITE);
    if (CheckCollisionPointRec(mouse, recNext) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game->screen = SCREEN_MOVES_LIST;
    }
}

const char *GetFullMoveText(Fullmove *fm) {
    const char *whiteMove = FormatPlyPGN(&fm->white);

    const char *blackMove = "";
    bool blackMoved = (fm->black.castle && strlen(fm->black.castle) > 0) ||
                      (fm->black.file && strlen(fm->black.file) > 0);
    if (blackMoved) {
        blackMove = FormatPlyPGN(&fm->black);
    }

    return temp_sprintf("%7s,%7s", whiteMove, blackMove);
}

void DrawScreen_MovesList(Game *game, Vector2 mouse) {
    const size_t colCount = 3;
    const size_t rowsPerCol = 14;
    const size_t movesPerPage = colCount * rowsPerCol; // 42 moves per page

    // Calculate total pages (minimum 1 page even when moves.count == 0)
    size_t totalPages = (game->match.moves.count == 0)
        ? 1
        : ((game->match.moves.count - 1) / movesPerPage) + 1;

    // Clamp current page in case moves were reset
    if (game->movesListPage >= totalPages) {
        game->movesListPage = totalPages - 1;
    }

    // Header Title
    DrawTextEx(
        game->font,
        temp_sprintf("Match Moves List (%s to move)", game->turn == SIDE_WHITE ? "WHITE" : "BLACK"),
        (Vector2){BUTTON_MARGIN, BUTTON_MARGIN},
        TITLE_FONT_SIZE * 0.6f,
        1,
        WHITE
    );

    // 1. Top-Right "Move" Button (Full Size)
    const char *btnText = "Move";
    Vector2 dims = MeasureTextEx(game->font, btnText, BUTTON_FONT_SIZE, 1);

    float moveBtnWidth = dims.x + BUTTON_PADDING * 2;
    float moveBtnHeight = BUTTON_DIM_Y * 0.75f;
    float moveSidebarX = GetScreenWidth() - moveBtnWidth - BUTTON_MARGIN;

    Rectangle moveRec = {
        .x = moveSidebarX,
        .y = BUTTON_PADDING - 5,
        .width = moveBtnWidth,
        .height = moveBtnHeight
    };

    DrawRectangleRec(moveRec, BLUE);
    DrawTextEx(
        game->font,
        btnText,
        (Vector2){moveRec.x + BUTTON_PADDING, moveRec.y + (moveRec.height - dims.y) / 2.0f},
        BUTTON_FONT_SIZE,
        1,
        WHITE
    );

    if (CheckCollisionPointRec(mouse, moveRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game->screen = SCREEN_MOVE_SELECTION;
    }

    // 2. Half-Size Vertical Pagination Strip on the Right Edge
    float navWidth = moveBtnWidth * 0.5f;
    float navBtnHeight = moveBtnHeight * 0.5f;
    float navFontSize = BUTTON_FONT_SIZE * 0.35f;
    float navSidebarX = GetScreenWidth() - navWidth - (BUTTON_MARGIN * 0.5f);
    float currentControlY = moveRec.y + moveRec.height + BUTTON_MARGIN;

    // "PREV" Button
    const char *prevText = "PREV";
    Vector2 prevDims = MeasureTextEx(game->font, prevText, navFontSize, 1);
    Rectangle prevRec = {
        .x = navSidebarX,
        .y = currentControlY,
        .width = navWidth,
        .height = navBtnHeight
    };

    bool canGoPrev = (game->movesListPage > 0);
    DrawRectangleRec(prevRec, canGoPrev ? DARKGRAY : GetColor(0x252525FF));
    DrawRectangleLinesEx(prevRec, 1, canGoPrev ? WHITE : GRAY);
    DrawTextEx(
        game->font,
        prevText,
        (Vector2){prevRec.x + (prevRec.width - prevDims.x) / 2.0f, prevRec.y + (prevRec.height - prevDims.y) / 2.0f},
        navFontSize,
        1,
        canGoPrev ? WHITE : GRAY
    );

    if (canGoPrev && CheckCollisionPointRec(mouse, prevRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game->movesListPage--;
    }

    currentControlY += navBtnHeight + (BUTTON_PADDING * 0.6f);

    // Page Indicator ("1/2") Centered in Compact Strip
    const char *pageText = temp_sprintf("%zu/%zu", game->movesListPage + 1, totalPages);
    Vector2 pageDims = MeasureTextEx(game->font, pageText, navFontSize, 1);
    DrawTextEx(
        game->font,
        pageText,
        (Vector2){navSidebarX + (navWidth - pageDims.x) / 2.0f, currentControlY},
        navFontSize,
        1,
        LIGHTGRAY
    );

    currentControlY += pageDims.y + (BUTTON_PADDING * 0.6f);

    // "NEXT" Button
    const char *nextText = "NEXT";
    Vector2 nextDims = MeasureTextEx(game->font, nextText, navFontSize, 1);
    Rectangle nextRec = {
        .x = navSidebarX,
        .y = currentControlY,
        .width = navWidth,
        .height = navBtnHeight
    };

    bool canGoNext = (game->movesListPage + 1 < totalPages);
    DrawRectangleRec(nextRec, canGoNext ? DARKGRAY : GetColor(0x252525FF));
    DrawRectangleLinesEx(nextRec, 1, canGoNext ? WHITE : GRAY);
    DrawTextEx(
        game->font,
        nextText,
        (Vector2){nextRec.x + (nextRec.width - nextDims.x) / 2.0f, nextRec.y + (nextRec.height - nextDims.y) / 2.0f},
        navFontSize,
        1,
        canGoNext ? WHITE : GRAY
    );

    if (canGoNext && CheckCollisionPointRec(mouse, nextRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game->movesListPage++;
    }

    // 3. 3-Column Scoresheet Grid with Dynamic Font Scaling
    float startY = moveRec.y + moveRec.height + BUTTON_PADDING;
    float startX = BUTTON_MARGIN;
    float fullWidth = navSidebarX - startX - (BUTTON_MARGIN * 0.5f);
    float fullHeight = GetScreenHeight() - startY - BUTTON_PADDING;
    float colWidth = (fullWidth / colCount) - 6.0f;
    float rowHeight = fullHeight / rowsPerCol;

    // Measure worst-case 19-char row ("000." + 15-char fullmove) to auto-fit column width
    float baseFontSize = BUTTON_FONT_SIZE * 0.6f;
    Vector2 sampleDims = MeasureTextEx(game->movesListFont, "000.exd8=Q#, O-O-O#", baseFontSize, 1);
    float maxEntryWidth = colWidth - 8.0f;

    float listFontSize = baseFontSize;
    if (sampleDims.x > maxEntryWidth && sampleDims.x > 0.0f) {
        listFontSize = baseFontSize * (maxEntryWidth / sampleDims.x);
    }
    if (listFontSize > rowHeight * 0.9f) {
        listFontSize = rowHeight * 0.9f;
    }

    float currentX = startX;
    float currentY = startY;
    size_t pageOffset = game->movesListPage * movesPerPage;
    size_t slot = 0;

    for (size_t col = 0; col < colCount; ++col) {
        for (; slot < rowsPerCol * (col + 1); ++slot) {
            size_t moveIdx = pageOffset + slot;
            size_t num = moveIdx + 1;

            // Keep move prefix fixed at 4 chars ("01. " or "100.") so 15-char move text aligns cleanly
            const char *numText = (num > 99)
                ? temp_sprintf("%zu.", num)
                : (num > 9 ? temp_sprintf("%zu. ", num) : temp_sprintf("0%zu. ", num));

            Vector2 numDims = MeasureTextEx(game->movesListFont, numText, listFontSize, 1);
            Vector2 numPos = { currentX + 2, currentY };
            DrawTextEx(game->movesListFont, numText, numPos, listFontSize, 1, WHITE);

            if (moveIdx < game->match.moves.count) {
                Fullmove *fm = &game->match.moves.items[moveIdx];
                numPos.x += numDims.x + 2;
                DrawTextEx(game->movesListFont, GetFullMoveText(fm), numPos, listFontSize, 1, WHITE);
            }

            currentY += rowHeight;
        }
        currentX += colWidth + 6.0f;
        currentY = startY;
    }
}

void DrawActionButtonsRow(Buttons bs, Vector2 *rowPos, Font font, Vector2 mouse) {
    for (size_t i = 0; i < bs.count; ++i) {
        Button *currentButton = &bs.items[i];
        Rectangle rec = { .x = rowPos->x, .y = rowPos->y, .width = BUTTON_DIM_X, .height = BUTTON_DIM_Y };

        if (CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            currentButton->active = !currentButton->active;

            // Keep "+" (check) and "#" (checkmate) mutually exclusive, while allowing "x" with either
            if (currentButton->active) {
                if (streq(currentButton->text, "+")) {
                    for (size_t j = 0; j < bs.count; ++j) {
                        if (streq(bs.items[j].text, "#")) bs.items[j].active = false;
                    }
                } else if (streq(currentButton->text, "#")) {
                    for (size_t j = 0; j < bs.count; ++j) {
                        if (streq(bs.items[j].text, "+")) bs.items[j].active = false;
                    }
                }
            }
        }

        DrawButton(currentButton, font, *rowPos, (Vector2){BUTTON_DIM_X, BUTTON_DIM_Y});
        rowPos->x += PIECE_DIM_X + BUTTON_PADDING + BUTTON_MARGIN;
    }
}

void ResetMoveSelection(Game *game) {
    if (game->activePiece) game->activePiece->active = false;
    game->activePiece = NULL;

    for (size_t i = 0; i < game->actions.count; ++i) {
        game->actions.items[i].active = false;
    }
    game->activeAction = NULL;

    if (game->activeLetter) game->activeLetter->active = false;
    game->activeLetter = NULL;

    if (game->activeNumber) game->activeNumber->active = false;
    game->activeNumber = NULL;

    if (game->activePromotion) game->activePromotion->active = false;
    game->activePromotion = NULL;

    if (game->activeCastle) game->activeCastle->active = false;
    game->activeCastle = NULL;

    game->fromFile = "";
    game->fromRank = "";
    game->disambigMode = false;
}

void DrawToggleButton(Button *b, Font font, Rectangle rec, float fontSize, Vector2 mouse, Button **tracker) {
    if (CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (b->active) {
            b->active = false;
            if (tracker) *tracker = NULL;
        } else {
            if (tracker && *tracker != NULL) {
                (*tracker)->active = false;
            }
            b->active = true;
            if (tracker) *tracker = b;
        }
    }

    DrawRectangleRec(rec, b->active ? DARKGREEN : DARKGRAY);
    DrawRectangleLinesEx(rec, 1, b->active ? LIME : WHITE);
    Vector2 textDims = MeasureTextEx(font, b->text, fontSize, 1);
    DrawTextEx(
        font,
        b->text,
        (Vector2){ rec.x + (rec.width - textDims.x) / 2.0f, rec.y + (rec.height - textDims.y) / 2.0f },
        fontSize,
        1,
        WHITE
    );
}
void DrawMoveSelectionGrid(Game *game, int title_bottom, Vector2 mouse) {
    float rowGap = 8.0f;
    float coordBtnHeight = 56.0f;
    float smallFont = BUTTON_FONT_SIZE * 0.55f;

    // Row 1: Pieces + Action Modifiers ("x", "+", "#")
    Vector2 rowPos = (Vector2){BUTTON_MARGIN, title_bottom + 8.0f};
    Kingdom *kingdom = game->turn == SIDE_WHITE ? &game->white : &game->black;
    DrawKingdom(kingdom, &rowPos, mouse, &game->activePiece);

    int row1Top = rowPos.y;
    rowPos.x = (PIECE_DIM_X + BUTTON_MARGIN + BUTTON_PADDING) * 6;
    rowPos.y = (row1Top + PIECE_DIM_Y) - coordBtnHeight;
    for (size_t i = 0; i < game->actions.count; ++i) {
        Button *currentButton = &game->actions.items[i];
        Rectangle rec = { .x = rowPos.x, .y = rowPos.y, .width = BUTTON_DIM_X, .height = coordBtnHeight };
        if (CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            currentButton->active = !currentButton->active;
            if (currentButton->active) {
                if (streq(currentButton->text, "+")) {
                    for (size_t j = 0; j < game->actions.count; ++j)
                        if (streq(game->actions.items[j].text, "#")) game->actions.items[j].active = false;
                } else if (streq(currentButton->text, "#")) {
                    for (size_t j = 0; j < game->actions.count; ++j)
                        if (streq(game->actions.items[j].text, "+")) game->actions.items[j].active = false;
                }
            }
        }
        DrawButton(currentButton, game->font, rowPos, (Vector2){BUTTON_DIM_X, coordBtnHeight});
        rowPos.x += PIECE_DIM_X + BUTTON_PADDING + BUTTON_MARGIN;
    }

    // Row 2: Files ("a" - "h")
    rowPos.x = BUTTON_MARGIN;
    rowPos.y = row1Top + PIECE_DIM_Y + rowGap;
    for (size_t i = 0; i < game->letters.count; ++i) {
        Button *btn = &game->letters.items[i];
        Rectangle rec = { .x = rowPos.x, .y = rowPos.y, .width = BUTTON_DIM_X, .height = coordBtnHeight };
        if (CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (game->disambigMode) {
                game->fromFile = streq(game->fromFile, btn->text) ? "" : btn->text;
                game->disambigMode = false;
            } else {
                if (game->activeLetter) game->activeLetter->active = false;
                btn->active = true;
                game->activeLetter = btn;
            }
        }

        DrawButton(btn, game->font, rowPos, (Vector2){BUTTON_DIM_X, coordBtnHeight});

        // Highlight active fromFile with an inset orange border
        if (game->fromFile && strlen(game->fromFile) > 0 && streq(game->fromFile, btn->text)) {
            Rectangle fromHighlight = {
                .x = rec.x + 2,
                .y = rec.y + 2,
                .width = rec.width - 4,
                .height = rec.height - 4
            };
            DrawRectangleLinesEx(fromHighlight, 2, ORANGE);
        }

        rowPos.x += PIECE_DIM_X + BUTTON_PADDING + BUTTON_MARGIN;
    }

    // Row 3: Ranks ("1" - "8")
    rowPos.x = BUTTON_MARGIN;
    rowPos.y += coordBtnHeight + rowGap;
    for (size_t i = 0; i < game->numbers.count; ++i) {
        Button *btn = &game->numbers.items[i];
        Rectangle rec = { .x = rowPos.x, .y = rowPos.y, .width = BUTTON_DIM_X, .height = coordBtnHeight };
        if (CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (game->disambigMode) {
                game->fromRank = streq(game->fromRank, btn->text) ? "" : btn->text;
                game->disambigMode = false;
            } else {
                if (game->activeNumber) game->activeNumber->active = false;
                btn->active = true;
                game->activeNumber = btn;
            }
        }

        DrawButton(btn, game->font, rowPos, (Vector2){BUTTON_DIM_X, coordBtnHeight});

        // Highlight active fromRank with an inset orange border
        if (game->fromRank && strlen(game->fromRank) > 0 && streq(game->fromRank, btn->text)) {
            Rectangle fromHighlight = {
                .x = rec.x + 2,
                .y = rec.y + 2,
                .width = rec.width - 4,
                .height = rec.height - 4
            };
            DrawRectangleLinesEx(fromHighlight, 2, ORANGE);
        }

        rowPos.x += PIECE_DIM_X + BUTTON_PADDING + BUTTON_MARGIN;
    }

    // Row 4: Promotions ("=Q".."=N") + Castling ("O-O", "O-O-O") + "FROM" Toggle
    float modX = BUTTON_MARGIN;
    float modY = rowPos.y + coordBtnHeight + rowGap;
    float modHeight = 46.0f;
    float promoWidth = BUTTON_DIM_X * 1.0f;

    for (size_t i = 0; i < game->promotions.count; ++i) {
        Rectangle pRec = { .x = modX, .y = modY, .width = promoWidth, .height = modHeight };
        DrawToggleButton(&game->promotions.items[i], game->font, pRec, smallFont, mouse, &game->activePromotion);
        if (game->activePromotion && game->activeCastle) {
            game->activeCastle->active = false;
            game->activeCastle = NULL;
        }
        modX += promoWidth + BUTTON_PADDING;
    }

    modX += BUTTON_MARGIN * 0.4f;
    for (size_t i = 0; i < game->castles.count; ++i) {
        float cWidth = (i == 0) ? BUTTON_DIM_X * 1.25f : BUTTON_DIM_X * 1.65f;
        Rectangle cRec = { .x = modX, .y = modY, .width = cWidth, .height = modHeight };
        DrawToggleButton(&game->castles.items[i], game->font, cRec, smallFont, mouse, &game->activeCastle);
        if (game->activeCastle && game->activePromotion) {
            game->activePromotion->active = false;
            game->activePromotion = NULL;
        }
        modX += cWidth + BUTTON_PADDING;
    }

    modX += BUTTON_MARGIN * 0.4f;
    float fromWidth = BUTTON_DIM_X * 1.2f;
    Rectangle fromRec = { .x = modX, .y = modY, .width = fromWidth, .height = modHeight };
    if (CheckCollisionPointRec(mouse, fromRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game->disambigMode = !game->disambigMode;
    }
    DrawRectangleRec(fromRec, game->disambigMode ? ORANGE : DARKGRAY);
    DrawRectangleLinesEx(fromRec, 1, game->disambigMode ? YELLOW : WHITE);
    Vector2 fromDims = MeasureTextEx(game->font, "FROM", smallFont, 1);
    DrawTextEx(
        game->font,
        "FROM",
        (Vector2){ fromRec.x + (fromRec.width - fromDims.x) / 2.0f, fromRec.y + (fromRec.height - fromDims.y) / 2.0f },
        smallFont,
        1,
        WHITE
    );
}

void DrawScreen_MoveSelection(Game *game, Vector2 mouse) {
    const char *playerName = (game->turn == SIDE_WHITE) ? game->match.white.name : game->match.black.name;
    if (!playerName) playerName = "Unknown";
    const char *sideName = (game->turn == SIDE_WHITE) ? "White" : "Black";

    const char *headerText = temp_sprintf("Player: %s (%s)", playerName, sideName);
    Vector2 textDims = MeasureTextEx(game->font, headerText, TITLE_FONT_SIZE, 1);

    int title_bottom = (TITLE_FONT_SIZE + (TITLE_Y_PADDING * 2));
    Rectangle headerRec = { .x = 0, .y = 0, .width = GetScreenWidth(), .height = title_bottom };

    DrawRectangleRec(headerRec, DARKGRAY);
    DrawLine(0, title_bottom, GetScreenWidth(), title_bottom, WHITE);
    DrawTextEx(game->font, headerText, (Vector2){ (GetScreenWidth() / 2.0f) - (textDims.x / 2.0f), TITLE_Y_PADDING }, TITLE_FONT_SIZE, 1, WHITE);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, headerRec)) {
        game->turn = (game->turn == SIDE_WHITE) ? SIDE_BLACK : SIDE_WHITE;
        ResetMoveSelection(game);
    }

    // Draw all 4 selection rows
    DrawMoveSelectionGrid(game, title_bottom, mouse);

    // Build temporary Ply for live preview & submission
    Ply previewPly = {0};
    if (game->activePiece) {
        previewPly.piece = *game->activePiece;
    } else if (!game->activeCastle) {
        previewPly.piece.side = game->turn;
        previewPly.piece.pt = PT_PAWN;
        previewPly.piece.value = GetPieceValue(PT_PAWN);
    }
    previewPly.piece     = game->activePiece ? *game->activePiece : (Piece){0};
    previewPly.fromFile  = game->fromFile ? game->fromFile : "";
    previewPly.fromRank  = game->fromRank ? game->fromRank : "";
    previewPly.file      = game->activeLetter ? game->activeLetter->text : "";
    previewPly.rank      = game->activeNumber ? game->activeNumber->text : "";
    previewPly.promotion = game->activePromotion ? game->activePromotion->text : "";
    previewPly.castle    = game->activeCastle ? game->activeCastle->text : "";
    previewPly.capture   = (game->actions.count > 0) ? game->actions.items[0].active : false;
    previewPly.check     = (game->actions.count > 1) ? game->actions.items[1].active : false;
    previewPly.checkmate = (game->actions.count > 2) ? game->actions.items[2].active : false;

    const char *pgnMove = FormatPlyPGN(&previewPly);
    const char *moveLabel = temp_sprintf("Move: %s", pgnMove);
    DrawTextEx(game->font, moveLabel, (Vector2){ BUTTON_PADDING, GetScreenHeight() - (BUTTON_PADDING + TITLE_FONT_SIZE * 0.85f) }, TITLE_FONT_SIZE * 0.85f, 1, WHITE);

    // RESET Button
    Button reset = { .text = "RESET", .active = false };
    Vector2 dims = MeasureTextEx(game->font, reset.text, BUTTON_FONT_SIZE * 0.8f, 1);
    Vector2 size = { .x = dims.x + (BUTTON_PADDING * 2), .y = (BUTTON_FONT_SIZE * 0.8f + BUTTON_PADDING) };
    Rectangle rec = { .x = GetScreenWidth() / 2.0f, .y = GetScreenHeight() - size.y - BUTTON_PADDING, .width = size.x, .height = size.y };
    DrawRectangleRec(rec, RED);
    DrawTextEx(game->font, reset.text, (Vector2){rec.x + BUTTON_PADDING, rec.y + BUTTON_PADDING / 2.0f}, BUTTON_FONT_SIZE * 0.8f, 1, WHITE);

    if (CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ResetMoveSelection(game);
    }

    // SUBMIT Button
    Button submit = { .text = "SUBMIT", .active = false };
    dims = MeasureTextEx(game->font, submit.text, BUTTON_FONT_SIZE * 0.8f, 1);
    size = (Vector2){ .x = dims.x + (BUTTON_PADDING * 4), .y = (BUTTON_FONT_SIZE * 0.8f + BUTTON_PADDING) };
    rec = (Rectangle){ .x = rec.x + rec.width + BUTTON_PADDING, .y = GetScreenHeight() - size.y - BUTTON_PADDING, .width = size.x, .height = size.y };
    DrawRectangleRec(rec, GREEN);
    DrawTextEx(game->font, submit.text, (Vector2){rec.x + BUTTON_PADDING * 2, rec.y + BUTTON_PADDING / 2.0f}, BUTTON_FONT_SIZE * 0.8f, 1, WHITE);

    if (CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        previewPly.timestamp = timestamp();

        if (game->turn == SIDE_WHITE) {
            Fullmove fm = {0};
            fm.count = game->match.moves.count + 1;
            fm.white = previewPly;
            da_append(&game->match.moves, fm);
        } else if (game->match.moves.count > 0) {
            game->match.moves.items[game->match.moves.count - 1].black = previewPly;
        }

        Player *currentPlayer = (game->turn == SIDE_WHITE) ? &game->match.white : &game->match.black;
        const char *logPlayerName = currentPlayer->name ? currentPlayer->name : GetSideName(game->turn);
        nob_log(NOB_INFO, "Move recorded [%s (%s)]: %s", logPlayerName, GetSideName(game->turn), pgnMove);

        game->turn = (game->turn == SIDE_WHITE) ? SIDE_BLACK : SIDE_WHITE;
        ResetMoveSelection(game);

        if (game->match.moves.count > 0) {
            game->movesListPage = (game->match.moves.count - 1) / 42;
        }
        game->screen = SCREEN_MOVES_LIST;
    }
}

Ply ParseSANToken(const char *token, Side side, size_t moveNum) {
    Ply ply = {0};
    ply.fullmove = moveNum;
    ply.piece.side = side;
    ply.fromFile = "";
    ply.fromRank = "";
    ply.file = "";
    ply.rank = "";
    ply.promotion = "";
    ply.castle = "";
    ply.timestamp = timestamp();

    char buf[32];
    strncpy(buf, token, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    size_t len = strlen(buf);
    if (len == 0) return ply;

    // 1. Strip trailing '+' or '#'
    if (buf[len - 1] == '+') {
        ply.check = true;
        buf[--len] = '\0';
    } else if (buf[len - 1] == '#') {
        ply.checkmate = true;
        buf[--len] = '\0';
    }

    // 2. Check for Castling ("O-O-O" or "O-O")
    if (streq(buf, "O-O-O") || streq(buf, "0-0-0")) {
        ply.castle = "O-O-O";
        ply.piece.pt = PT_KING;
        return ply;
    }
    if (streq(buf, "O-O") || streq(buf, "0-0")) {
        ply.castle = "O-O";
        ply.piece.pt = PT_KING;
        return ply;
    }

    // 3. Check for Promotion ("=Q", "=R", "=B", "=N")
    char *eq = strchr(buf, '=');
    if (eq && eq[1] != '\0') {
        switch (eq[1]) {
            case 'Q': ply.promotion = "=Q"; break;
            case 'R': ply.promotion = "=R"; break;
            case 'B': ply.promotion = "=B"; break;
            case 'N': ply.promotion = "=N"; break;
            default: break;
        }
        *eq = '\0';
        len = strlen(buf);
    }

    // 4. Identify Piece Type (uppercase first letter = piece, lowercase = pawn)
    size_t idx = 0;
    switch (buf[0]) {
        case 'K': ply.piece.pt = PT_KING;   idx = 1; break;
        case 'Q': ply.piece.pt = PT_QUEEN;  idx = 1; break;
        case 'R': ply.piece.pt = PT_ROOK;   idx = 1; break;
        case 'B': ply.piece.pt = PT_BISHOP; idx = 1; break;
        case 'N': ply.piece.pt = PT_KNIGHT; idx = 1; break;
        default:  ply.piece.pt = PT_PAWN;   idx = 0; break;
    }
    ply.piece.value = GetPieceValue(ply.piece.pt);

    // 5. Extract destination square (last 2 chars: file 'a'-'h' + rank '1'-'8')
    if (len >= idx + 2) {
        char destFile[2] = { buf[len - 2], '\0' };
        char destRank[2] = { buf[len - 1], '\0' };
        ply.file = strdup(destFile);
        ply.rank = strdup(destRank);
        len -= 2;
        buf[len] = '\0';
    }

    // 6. Scan remaining middle characters for 'x' and disambiguation (fromFile / fromRank)
    for (size_t i = idx; i < len; ++i) {
        char c = buf[i];
        if (c == 'x') {
            ply.capture = true;
        } else if (c >= 'a' && c <= 'h') {
            char f[2] = { c, '\0' };
            ply.fromFile = strdup(f);
        } else if (c >= '1' && c <= '8') {
            char r[2] = { c, '\0' };
            ply.fromRank = strdup(r);
        }
    }

    return ply;
}

bool LoadMatchFromPGN(Game *game, const char *filePath) {
    String_Builder sb = {0};
    if (!read_entire_file(filePath, &sb)) {
        nob_log(WARNING, "Could not load seed PGN file: %s", filePath);
        return false;
    }
    sb_append_null(&sb);

    game->match = (Match){0};
    game->match.timestamp = timestamp();
    game->match.white.side = SIDE_WHITE;
    game->match.black.side = SIDE_BLACK;

    char *p = sb.items;
    Side nextSide = SIDE_WHITE;

    while (*p != '\0') {
        // Skip whitespace
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p == '\0') break;

        // Parse Header Tags: [Tag "Value"]
        if (*p == '[') {
            char *endBracket = strchr(p, ']');
            if (!endBracket) break;
            *endBracket = '\0';

            char *quote1 = strchr(p + 1, '"');
            char *quote2 = quote1 ? strchr(quote1 + 1, '"') : NULL;
            if (quote1 && quote2) {
                *quote2 = '\0';
                const char *val = quote1 + 1;
                if (strncmp(p + 1, "White ", 6) == 0) {
                    game->match.white.name = strdup(val);
                } else if (strncmp(p + 1, "Black ", 6) == 0) {
                    game->match.black.name = strdup(val);
                }
            }
            p = endBracket + 1;
            continue;
        }

        // Skip PGN brace comments { ... }
        if (*p == '{') {
            char *endBrace = strchr(p, '}');
            if (!endBrace) break;
            p = endBrace + 1;
            continue;
        }

        // Read next whitespace-delimited token
        char *tokStart = p;
        while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') p++;
        char saved = *p;
        *p = '\0';

        // Skip game result markers
        if (streq(tokStart, "1-0") || streq(tokStart, "0-1") ||
            streq(tokStart, "1/2-1/2") || streq(tokStart, "*")) {
            if (saved != '\0') p++;
            continue;
        }

        // Handle move number prefixes (e.g., "1." or "12.c4" or "1...")
        char *dot = strrchr(tokStart, '.');
        if (dot) {
            tokStart = dot + 1;
            if (*tokStart == '\0') {
                if (saved != '\0') p++;
                continue;
            }
        }

        // Parse SAN move and append to Fullmoves
        if (nextSide == SIDE_WHITE) {
            Fullmove fm = {0};
            fm.count = game->match.moves.count + 1;
            fm.white = ParseSANToken(tokStart, SIDE_WHITE, fm.count);
            da_append(&game->match.moves, fm);
            nextSide = SIDE_BLACK;
        } else if (game->match.moves.count > 0) {
            Fullmove *last = &game->match.moves.items[game->match.moves.count - 1];
            last->black = ParseSANToken(tokStart, SIDE_BLACK, last->count);
            nextSide = SIDE_WHITE;
        }

        if (saved != '\0') p++;
    }

    game->turn = nextSide;
    game->movesListPage = 0;
    sb_free(sb);

    nob_log(INFO, "Seeded match from %s (%zu fullmoves loaded)", filePath, game->match.moves.count);
    return true;
}

int main(void)
{
      // 1. Initialize with temporary values so we can probe the hardware
    InitWindow(800, 480, "Chess Score Keeper");
#ifdef PI_TARGET
    // Query display size and set fullscreen specifically for the Pi
    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    int monitorHeight = GetMonitorHeight(monitor);
    
    // If you use a windowed fullscreen or toggle fullscreen:
    SetWindowSize(monitorWidth, monitorHeight);
    ToggleFullscreen(); // or InitWindow with these dimensions
#endif

    SetTargetFPS(60);

    Fonts fonts = LoadFonts();
    
    Game game = {0};
    game.font = fonts.items[2];
    game.movesListFont = fonts.items[3];
    game.piecesName = "moxica";
    game.white = CreateKingdom(SIDE_WHITE, game.piecesName);
    game.black = CreateKingdom(SIDE_BLACK, game.piecesName);
    game.turn = SIDE_WHITE;
    game.activePiece = NULL;
    game.activeAction = NULL;
    game.activeLetter = NULL;
    game.activeNumber = NULL;
    game.screen = SCREEN_START_GAME;
    game.letters = CreateLetterButtons();
    game.numbers = CreateNumberButtons();
    game.actions = CreateActionButtons();
    game.promotions = CreatePromotionButtons();
    game.castles = CreateCastleButtons();
    game.fromFile = "";
    game.fromRank = "";
    game.availablePlayers = CreatePlayerList();
    game.match = (Match){0};

    // Seed test PGN match if present in working directory
    if (file_exists("kasparov_karpov_1987_g24.pgn") == 1) {
        if (LoadMatchFromPGN(&game, "kasparov_karpov_1987_g24.pgn")) {
            game.screen = SCREEN_MOVES_LIST;
        }
    }

    while (!WindowShouldClose()) {
        size_t mark = temp_save();
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        
        Vector2 mouse = GetMousePosition();
       
        switch (game.screen) {
          case SCREEN_START_GAME: DrawScreen_StartGame(&game, mouse); break;
          case SCREEN_PLAYER_SETUP: DrawScreen_PlayerSetup(&game, mouse); break;
          case SCREEN_MOVES_LIST: DrawScreen_MovesList(&game, mouse); break;
          case SCREEN_MOVE_SELECTION: DrawScreen_MoveSelection(&game, mouse); break;
          default: nob_log(ERROR, "Unknown screen state: %s", GetScreenStateText(game.screen)); 
        } 
        
        EndDrawing();
        temp_rewind(mark);
    }

    CloseWindow();
    return 0;
}
