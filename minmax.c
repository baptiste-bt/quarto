#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define BOARD_SIZE 4
#define EMPTY -1

// Encoding the pieces as integers
typedef enum {
    TALL_LIGHT_ROUND_SOLID = 0,
    TALL_LIGHT_ROUND_HOLLOW,
    // ... other pieces ...
    SHORT_DARK_SQUARE_HOLLOW = 15
} Piece;

// The game state
typedef struct {
    int board[BOARD_SIZE][BOARD_SIZE];
    bool availablePieces[16]; // true if the piece is available
} GameState;

typedef struct {
    int score;
    int x;
    int y;
    int piece;
} MoveScore;

// Function declarations
void printBoard(GameState *state, bool printAsInt);
void printPiece(int piece);
int evaluateBoard(GameState *state);
MoveScore minimax(GameState state, int depth, int pieceToPlace, bool isMax, int alpha, int beta);
int min(int a, int b);
int max(int a, int b);
bool checkLine(GameState *state, int startX, int startY, int deltaX, int deltaY);
void unitTestEvaluate();
// void unitTestFindBestMove();

int evaluateCount = 0;

int main(int argc, char *argv[]) {

    int depth;

    // Check if depth is provided as an argument
    if (argc == 2) {
        depth = atoi(argv[1]);
        if (depth <= 0) {
            printf("Invalid depth. Please provide a positive integer.\n");
            return 1;
        }
    } else {
        printf("Usage: %s <depth>\n", argv[0]);
        return 1;
    }

    // Example initialization
    // GameState state = { {{EMPTY, EMPTY, EMPTY, EMPTY},
    //                      {EMPTY, EMPTY, EMPTY, EMPTY},
    //                      {EMPTY, EMPTY, EMPTY, EMPTY},
    //                      {EMPTY, EMPTY, EMPTY, EMPTY}},
    //                     {false, true, true, true, true, true, true, true,
    //                      true, true, true, true, true, true, true, true} };

    // Initialize the game state with a few pieces
    // GameState state = { 
    //     { {0, 1, 2, EMPTY},
    //     {EMPTY, EMPTY, EMPTY, 13},
    //     {EMPTY, EMPTY, 3, EMPTY},
    //     {15, EMPTY, EMPTY, EMPTY} },
    //     {false, false, false, false, true, true, true, true, true, true, true, true, true, false, true, false}
    // };

    //  0:  SDQO 
    //  1:  TDQO 
    //  2:  SLQO 
    //  3:  TLQO 
    //  4:  SDRO 
    //  5:  TDRO 
    //  6:  SLRO 
    //  7:  TLRO 
    //  8:  SDQH 
    //  9:  TDQH 
    // 10:  SLQH 
    // 11:  TLQH 
    // 12:  SDRH 
    // 13:  TDRH 
    // 14:  SLRH 
    // 15:  TLRH 

    GameState state = { 
        { {0, 1, 2, 12},
        {EMPTY, 4, EMPTY, 13},
        {11, EMPTY, 3, EMPTY},
        {15, 5, EMPTY, 14} },
        {false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false}
    }; // Available pieces: 5, 6, 7, 8, 9, 10, 14

     printBoard(&state, false);

    int pieceToPlace = 10;
    // Check if the piece is available
    if (!state.availablePieces[pieceToPlace]) {
        printf("Piece %d is not available.\n", pieceToPlace);
        return 1;
    }
    // Remove the piece from the available pieces
    state.availablePieces[pieceToPlace] = false;

    MoveScore moveValue = minimax(state, depth, pieceToPlace, true, -1000, 1000);
    
    printf("\n---------------------------------------------------------------------\n");
    printf("Best Move: Piece %d at (%d, %d) with score %d\n", pieceToPlace, moveValue.x, moveValue.y, moveValue.score);
    printf("Piece to give to the opponent: %d\n", moveValue.piece);
    // Print the board after the move
    state.board[moveValue.x][moveValue.y] = pieceToPlace;
    printf("evaluateBoard(Move) = %d\n", evaluateBoard(&state));
    printBoard(&state, false);
    printBoard(&state, true);

    printf("evaluateBoard was called %d times\n", evaluateCount);

    // unitTestEvaluate();
    // unitTestFindBestMove();

    // print all pieces with their number
    // for (int i = 0; i < 16; i++) {
    //     printf("%2d: ", i);
    //     printPiece(i);
    //     printf("\n");
    // }
    return 0;
}

void printBoard(GameState *state, bool printAsInt) {
    printf("Current Board State:\n");
    for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            int piece = state->board[x][y];
            if (piece == EMPTY) {
                printf("   .  "); // Empty space
            } else {
                if (printAsInt) {
                    printf(" %3d  ", piece);
                    continue;
                }
                // Decode the piece attributes
                char height = (piece & 1) ? 'T' : 'S'; // Tall or Short
                char color = (piece & 2) ? 'L' : 'D';  // Light or Dark
                char shape = (piece & 4) ? 'R' : 'Q';  // Round or Square
                char consistency = (piece & 8) ? 'H' : 'O'; // Hollow or Solid

                printf(" %c%c%c%c ", height, color, shape, consistency);
            }
        }
        printf("\n");
    }
}

void printPiece(int piece) {
    // Decode the piece attributes
    char height = (piece & 1) ? 'T' : 'S'; // Tall or Short
    char color = (piece & 2) ? 'L' : 'D';  // Light or Dark
    char shape = (piece & 4) ? 'R' : 'Q';  // Round or Square
    char consistency = (piece & 8) ? 'H' : 'O'; // Hollow or Solid

    printf(" %c%c%c%c ", height, color, shape, consistency);
}


int evaluateBoard(GameState *state) {
    evaluateCount++;
    int score = 0;

    // Winning and losing scores
    const int WIN_SCORE = 1000;

    // Check all rows, columns, and diagonals
    for (int i = 0; i < BOARD_SIZE; i++) {
        // Check rows and columns
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (checkLine(state, i, 0, 0, 1) || checkLine(state, 0, j, 1, 0)) {
                return WIN_SCORE;
            }
        }

        // Check diagonals
        if (checkLine(state, 0, 0, 1, 1) || checkLine(state, 0, BOARD_SIZE - 1, 1, -1)) {
            return WIN_SCORE;
        }
    }

    // Other heuristics can be added here to evaluate non-winning board states

    return score;
}

bool checkLine(GameState *state, int startX, int startY, int deltaX, int deltaY) {
    int attributes[4] = {0, 0, 0, 0}; // Track common attributes

    for (int i = 0; i < BOARD_SIZE; i++) {
        int piece = state->board[startX + i * deltaX][startY + i * deltaY];
        if (piece == EMPTY) {
            return false; // Line is not complete
        }
        for (int j = 0; j < 4; j++) {
            attributes[j] |= (piece & (1 << j));
        }
    }

    // Check if all pieces in the line share at least one attribute
    for (int j = 0; j < 4; j++) {
        if (attributes[j] == 0 || attributes[j] == 15) {
            return true; // Winning line found
        }
    }

    return false; // No winning line
}

MoveScore minimax(GameState state, int depth, int pieceToPlace, bool isMax, int alpha, int beta) {
    int score = evaluateBoard(&state);
    MoveScore bestMoveScore;

    // Terminal node (win/lose) or maximum depth reached
    if (depth == 0 || score == 1000 || score == -1000) {
        bestMoveScore.score = score;
        return bestMoveScore;
    }

    if (isMax) {
        bestMoveScore.score = -1000;

        // Place the piece given by the opponent
        for (int x = 0; x < BOARD_SIZE; x++) {
            for (int y = 0; y < BOARD_SIZE; y++) {
                if (state.board[x][y] != EMPTY) continue;

                // Make the move
                state.board[x][y] = pieceToPlace;

                // Give each available piece to the opponent
                for (int nextPiece = 0; nextPiece < 16; nextPiece++) {
                    if (!state.availablePieces[nextPiece]) continue;

                    // Call minimax recursively and choose the maximum value
                    MoveScore currentMoveScore = minimax(state, depth - 1, nextPiece, !isMax, alpha, beta);
                    if (currentMoveScore.score > bestMoveScore.score) {
                        bestMoveScore = currentMoveScore;
                        bestMoveScore.x = x;
                        bestMoveScore.y = y;
                        bestMoveScore.piece = nextPiece;
                    }

                    // Alpha Beta Pruning
                    alpha = max(alpha, bestMoveScore.score);
                    if (beta <= alpha) {
                        break;
                    }
                }

                // Undo the move
                state.board[x][y] = EMPTY;

                if (beta <= alpha) {
                    break;
                }
            }
        }
        return bestMoveScore;
    } else {
        bestMoveScore.score = 1000;

        // Place the piece given by the opponent
        for (int x = 0; x < BOARD_SIZE; x++) {
            for (int y = 0; y < BOARD_SIZE; y++) {
                if (state.board[x][y] != EMPTY) continue;

                // Make the move
                state.board[x][y] = pieceToPlace;

                // Give each available piece to the opponent
                for (int nextPiece = 0; nextPiece < 16; nextPiece++) {
                    if (!state.availablePieces[nextPiece]) continue;

                    // Call minimax recursively and choose the minimum value
                    MoveScore currentMoveScore = minimax(state, depth - 1, nextPiece, !isMax, alpha, beta);
                    if (currentMoveScore.score < bestMoveScore.score) {
                        bestMoveScore = currentMoveScore;
                        bestMoveScore.x = x;
                        bestMoveScore.y = y;
                        bestMoveScore.piece = nextPiece;
                    }

                    // Alpha Beta Pruning
                    beta = min(beta, bestMoveScore.score);
                    if (beta <= alpha) {
                        break;
                    }
                }

                // Undo the move
                state.board[x][y] = EMPTY;

                if (beta <= alpha) {
                    break;
                }
            }
        }
        return bestMoveScore;
    }
}


// Utility functions to find max and min values
int max(int a, int b) {
    return (a > b) ? a : b;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}


void unitTestEvaluate() {
    GameState rowWin = { 
        { {0, 1, 2, 3},    // All pieces share at least one attribute
        {EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY} },
        {false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true}
    };

    GameState colWin = { 
        { {0, EMPTY, EMPTY, EMPTY},    // All pieces share at least one attribute
        {1, EMPTY, EMPTY, EMPTY},
        {2, EMPTY, EMPTY, EMPTY},
        {3, EMPTY, EMPTY, EMPTY} },
        {false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true}
    };

    GameState diagWin = { 
        { {0, EMPTY, EMPTY, EMPTY},    // All pieces share at least one attribute
        {EMPTY, 1, EMPTY, EMPTY},
        {EMPTY, EMPTY, 2, EMPTY},
        {EMPTY, EMPTY, EMPTY, 3} },
        {false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true}
    };

    GameState noWin = { 
        { {0, 1, EMPTY, EMPTY},
        {2, 3, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY} }, // No complete line with common attributes
        {false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true}
    };

    GameState mixedAttributes = { 
        { {0, 4, EMPTY, EMPTY},
        {8, 12, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY} }, // Pieces with different attributes
        {false, false, true, true, false, true, true, true, false, true, true, true, false, false, true, true}
    };

    GameState nearlyComplete = { 
        { {0, 1, 2, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY} }, // One piece short of a complete line
        {false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true}
    };

    printf("evaluateBoard(rowWin) = %d\n", evaluateBoard(&rowWin));
    printf("evaluateBoard(colWin) = %d\n", evaluateBoard(&colWin));
    printf("evaluateBoard(diagWin) = %d\n", evaluateBoard(&diagWin));
    printf("evaluateBoard(noWin) = %d\n", evaluateBoard(&noWin));
    printf("evaluateBoard(mixedAttributes) = %d\n", evaluateBoard(&mixedAttributes));
    printf("evaluateBoard(nearlyComplete) = %d\n", evaluateBoard(&nearlyComplete));
}

// void unitTestFindBestMove() {
//     GameState defensiveMove = { 
//         { {0, 1, 2, EMPTY},
//         {EMPTY, EMPTY, EMPTY, EMPTY},
//         {EMPTY, EMPTY, EMPTY, EMPTY},
//         {EMPTY, EMPTY, EMPTY, EMPTY} },
//         {false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true}
//     };
//     // Expected: The best move is to block the opponent by placing a piece in the last empty space of the first row.

//     GameState winningMove = { 
//         { {0, EMPTY, EMPTY, EMPTY},
//         {1, EMPTY, EMPTY, EMPTY},
//         {2, EMPTY, EMPTY, EMPTY},
//         {EMPTY, EMPTY, EMPTY, EMPTY} },
//         {false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true}
//     };
//     // Expected: The best move is to complete the column with the winning piece.
//     int bestPiece, bestX, bestY;
//     int depth = 2;

//     // Test 1
//     findBestMove(&defensiveMove, &bestPiece, &bestX, &bestY, depth);
//     printf("Best Move: Piece %d at (%d, %d)\n", bestPiece, bestX, bestY);
//     // Play the move and print the board
//     defensiveMove.board[bestX][bestY] = bestPiece;
//     defensiveMove.availablePieces[bestPiece] = false;
//     printBoard(&defensiveMove);

//     // Test 2
//     findBestMove(&winningMove, &bestPiece, &bestX, &bestY, depth);
//     printf("Best Move: Piece %d at (%d, %d)\n", bestPiece, bestX, bestY);
//     // Play the move and print the board
//     winningMove.board[bestX][bestY] = bestPiece;
//     winningMove.availablePieces[bestPiece] = false;
//     printBoard(&winningMove);

//     // Test 3
//     //  Reinitialize the game states
//     defensiveMove = (GameState) { 
//         { {0, 1, 2, EMPTY},
//         {EMPTY, EMPTY, EMPTY, EMPTY},
//         {EMPTY, EMPTY, EMPTY, EMPTY},
//         {EMPTY, EMPTY, EMPTY, EMPTY} },
//         {false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true}
//     };
//     int piece = 15;
//     findBestMoveGivenAPiece(&defensiveMove, &bestPiece, &bestX, &bestY, depth, piece);
//     printf("Best Move: Piece %d at (%d, %d)\n", bestPiece, bestX, bestY);
//     // Play the move and print the board
//     defensiveMove.board[bestX][bestY] = bestPiece;
//     defensiveMove.availablePieces[bestPiece] = false;
//     // Evaluate the board
//     printf("evaluateBoard(defensiveMove) = %d\n", evaluateBoard(&defensiveMove));
//     printBoard(&defensiveMove);


//     // Test 4
//     //  Reinitialize the game states
//     winningMove = (GameState) { 
//         { {0, EMPTY, EMPTY, EMPTY},
//         {1, EMPTY, EMPTY, EMPTY},
//         {2, EMPTY, EMPTY, EMPTY},
//         {EMPTY, EMPTY, EMPTY, EMPTY} },
//         {false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true}
//     };
//     findBestMoveGivenAPiece(&winningMove, &bestPiece, &bestX, &bestY, depth, piece);
//     printf("Best Move: Piece %d at (%d, %d)\n", bestPiece, bestX, bestY);
//     // Play the move and print the board
//     winningMove.board[bestX][bestY] = bestPiece;
//     winningMove.availablePieces[bestPiece] = false;
//     // Evaluate the board
//     printf("evaluateBoard(winningMove) = %d\n", evaluateBoard(&winningMove));
//     printBoard(&winningMove);

// }

