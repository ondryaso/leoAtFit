/*
 * @file proj3.c
 * @author Ondřej Ondryáš (xondry02)
 * @date 2019-12-15
 * @brief Triangular maze solver
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

// Return codes for methods and the program itself
#define LOAD_OK 0
#define ERR_PARSE_SIZE 1
#define ERR_INVALID_SIZE 2
#define ERR_PARSE 3
#define ERR_PARSE_OVERFLOW 4
#define ERR_INVALID_BORDERS 5
#define ERR_ALLOC 6
#define ERR_INVALID_STARTING_CELL 7
#define ERR_ARGS 8

// Maps a 2D-array index to a 1D-index, used to get the index of a specific cell in the cells array
#define A_VAL(col, row, width) ((col) + (row) * (width))
// Returns the right-most bit => 1 if the cell represented by the number i has the left border
#define M_LEFT(i) ((i)&1)
// Returns the second bit => 1 if the cell represented by the number i has the right border
#define M_RIGHT(i) (((i) >> 1) & 1)
// Retruns the third bit => 1 if the cell represented by the number i has the horizontal border
#define M_HOR(i) (((i) >> 2) & 1)

// Constants representing the borders. LEFT, RIGHT and HORIZONTAL also represent the number of bits
// the value has to be shifted to the right to get the corresponding border bit.
#define BORDER_LEFT 0
#define BORDER_RIGHT 1
#define BORDER_HORIZONTAL 2
#define BORDER_HTOP 3
#define BORDER_HBTM 4

// Constants representing the rule used for solving.
#define RULE_LEFT 0
#define RULE_RIGHT 1

#define STR_HELP "Available arguments:\n--help: prints this help\n"                                                                                                           \
                 "--test [file path]: tests whether the specified file contains a valid map\n"                                                                                \
                 "--rpath [entry cell row] [entry cell column] [file path]: finds a path through the maze using the right-hand rule, starting on the specified coordinates\n" \
                 "--lpath [entry cell row] [entry cell column] [file path]: finds a path through the maze using the left-hand rule, starting on the specified coordinates\n"

typedef struct
{
    int rows;
    int cols;
    unsigned char *cells;
} Map;

// Creates a new Map structure and allocates its cells array.
// If the allocation fails, set rows/columns count to zero to indicate an invalid object.
// If one of rows, columns is 0, sets both to zero to indicate an invalid object.
Map map_ctor(unsigned int rows, unsigned int columns)
{
    Map ret;
    ret.rows = rows;
    ret.cols = columns;
    ret.cells = NULL;

    if (rows != 0 && columns != 0)
    {
        ret.cells = calloc(rows * columns, sizeof(char));
    }

    if (ret.cells == NULL)
    {
        ret.rows = 0;
        ret.cols = 0;
    }

    return ret;
}

// Map destructor. Deallocates the cells array if the Map is valid.
// Doesn't do anything when the map is invalid or NULL.
void map_dtor(Map *map)
{
    if (map == NULL)
        return;
    if (map->rows == 0 || map->cols == 0 || map->cells == NULL)
        return;
    free(map->cells);
}

// Validates whether the borders of adjacent cells match each other. Returns true if they do.
// When verbose is set to true, prints out the indexes of invalid cells to stderr.
// When the map is invalid or NULL, returns false.
bool validate_map(Map *map, bool verbose)
{
    if (map == NULL)
    {
        return false;
    }

    int total = map->cols * map->rows;
    if (total == 0)
    {
        return false;
    }

    bool result = true;

    for (int i = 0; i < total; i++)
    {
        if (i != 0 && ((i - 1) % map->cols) != (map->cols - 1))
        {
            if (M_LEFT(map->cells[i]) != M_RIGHT(map->cells[i - 1]))
            {
                result = false;
                if (verbose)
                    fprintf(stderr, "Right border of cell #%d != Left border of cell #%d\n", i - 1, i);
            }
        }

        if ((i + 1) != total && ((i + 1) % map->cols) != 0)
        {
            if (M_RIGHT(map->cells[i]) != M_LEFT(map->cells[i + 1]))
            {
                result = false;
                if (verbose)
                    fprintf(stderr, "Right border of cell #%d != Left border of cell #%d\n", i, i + 1);
            }
        }

        if (i + map->cols >= total)
            continue; // Don't check the horizontal side when there are no more rows under this one

        if (map->cols % 2 == 1)
        {
            // When width is odd, all the odd triangles have the horizontal side on the bottom
            // and must be checked with the triangle on the position of i+mapWidth. The others
            // don't have to be checked.
            if (i % 2 == 1)
            {
                if (M_HOR(map->cells[i]) != M_HOR(map->cells[i + map->cols]))
                {
                    result = false;
                    if (verbose)
                        fprintf(stderr, "HBTM border of #%d != HTOP border of cell #%d\n", i, i + map->cols);
                }
            }
        }
        else
        {
            // When width is even, the even triangles in the odd lines have to be checked and vice-versa.
            if (i % 2 == (((i / map->cols) + 1) % 2))
            {
                if (M_HOR(map->cells[i]) != M_HOR(map->cells[i + map->cols]))
                {
                    result = false;
                    if (verbose)
                        fprintf(stderr, "HBTM border of #%d != HTOP border of cell #%d\n", i, i + map->cols);
                }
            }
        }
    }

    return result;
}

// Loads a text file with a map, parses it, creates a corresponding Map object and fills it with the loaded data.
// Performs a border validation using validate_map().
// Returns LOAD_OK when the map is loaded correctly and valid, or one of the ERR_ codes when an error occurs.
// The pointer to the created Map object is passed to the caller using the **map argument.
// The verbose argument is passed to the validate_map() call.
int parse_map(FILE *file, Map **map, bool verbose)
{
    int width = 0, height = 0;
    if (fscanf(file, "%d %d ", &height, &width) != 2)
    {
        return ERR_PARSE_SIZE;
    }

    if (width <= 0 || height <= 0)
        return ERR_INVALID_SIZE;

    Map *ret = malloc(sizeof(Map));

    if (ret == NULL)
        return ERR_ALLOC;

    *ret = map_ctor(height, width);

    if (ret->cells == NULL)
    {
        free(ret);
        return ERR_ALLOC;
    }

    int tmp = 0;
    int load = 0;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            tmp = fscanf(file, "%d", &load);
            // If scanning reaches EOF before the end of these two cycles, there aren't enough values => ERR_PARSE
            if (tmp != 1)
            {
                map_dtor(ret);
                free(ret);
                return ERR_PARSE;
            }

            if (load < 0 || load > 7)
            {
                map_dtor(ret);
                free(ret);
                return ERR_PARSE_OVERFLOW;
            }

            ret->cells[A_VAL(col, row, width)] = load;
        }
    }

    if (!validate_map(ret, verbose))
    {
        map_dtor(ret);
        free(ret);
        return ERR_INVALID_BORDERS;
    }

    *map = ret;
    return LOAD_OK;
}

// Prints an error message about the specified ERR_ code to stderr.
void print_map_error(int error)
{
    switch (error)
    {
    case ERR_PARSE_SIZE:
        fprintf(stderr, "Couldn't parse the information about map size.\n");
        break;
    case ERR_INVALID_SIZE:
        fprintf(stderr, "Invalid map size.\n");
        break;
    case ERR_PARSE:
        fprintf(stderr, "Invalid map format (parsing error).\n");
        break;
    case ERR_PARSE_OVERFLOW:
        fprintf(stderr, "Invalid map format (the cell is specified by a 3-bit number).\n");
        break;
    case ERR_INVALID_BORDERS:
        fprintf(stderr, "Invalid map specification (some of the adjacent borders don't match).\n");
        break;
    case ERR_ALLOC:
        fprintf(stderr, "Couldn't allocate memory.\n");
        break;
    default:
        break;
    }
}

// Returns one of BORDER_HBTM, BORDER_HTOP signalizing whether the specified cell
// has its horizontal border on the bottom or on the top.
int horizontal_border_side(int r, int c, int width)
{
    size_t index = A_VAL(c, r, width);

    if (width % 2 == 1) // Width is odd
    {
        // All the odd triangles have their horizontal border on the bottom
        if (index % 2 == 1)
        {
            return BORDER_HBTM;
        }
        else
        {
            return BORDER_HTOP;
        }
    }
    else
    {
        // When width is even, the triangle has its horizontal side on the top when its parity matches the row's parity
        if (index % 2 == (((index / width) % 2)))
        {
            return BORDER_HTOP;
        }
        else
        {
            return BORDER_HBTM;
        }
    }
}

// Checks whether the specified cell has a solid border or a free space at one of the specified sides (one of the BORDER_ values).
bool isborder(Map *map, int r, int c, int border)
{
    if (map == NULL && map->cells == NULL)
        return false;
    if (r >= map->rows && c >= map->cols)
        return false;
    if (border < 0 || border > 4)
        return false;

    size_t index = A_VAL(c, r, map->cols);
    unsigned char val = map->cells[index];

    if (border == BORDER_HTOP || border == BORDER_HBTM)
    {
        unsigned char hor = (val >> BORDER_HORIZONTAL) & 1;

        if (hor == 0)
            return false;

        if (border == horizontal_border_side(r, c, map->cols))
            return true;

        return false;
    }

    // Properly defined BORDER_LEFT/RIGHT/HORIZONTAL shifts the bits
    // so that we always end up with either 0 or 1 that signalises a space or a border.
    return (val >> border) & 1;
}

// Decides which border is the entry one for the specified cell. 
// Returns BORDER_LEFT, BORDER_RIGHT or BORDER_HORIZONTAL accordingly.
// The cell has to be at one of the edges of the map, otherwise ERR_INVALID_STARTING_CELL is returned.
// (This function is not implemented according to the spec, as allowed on the forum.)
int start_border(Map *map, int r, int c)
{
    if (c < 0 || c >= map->cols || r < 0 || r >= map->rows)
    {
        return ERR_INVALID_STARTING_CELL;
    }

    if (c == 0)
    {
        if (!isborder(map, r, c, BORDER_LEFT))
            return BORDER_LEFT;
    }

    if (c == (map->cols - 1))
    {
        if (!isborder(map, r, c, BORDER_RIGHT))
            return BORDER_RIGHT;
    }

    if (r == 0 || r == (map->rows - 1))
    {
        int hborder = horizontal_border_side(r, c, map->cols);
        if (hborder == BORDER_HTOP && r != 0 && map->rows != 1)
        {
            return ERR_INVALID_STARTING_CELL;
        }

        if (hborder == BORDER_HBTM && r != (map->rows - 1))
        {
            return ERR_INVALID_STARTING_CELL;
        }

        if (!isborder(map, r, c, hborder))
        {
            return BORDER_HORIZONTAL;
        }
    }

    return ERR_INVALID_STARTING_CELL;
}

// Traverses the map from the specified entry cell using the specified rule, starting from the side of entry_border.
// Prints the path to stdout.
// Returns when the traversal process steps outside of the map.
void traverse(Map *map, int r, int c, int rule, int entry_border)
{
    while (true)
    {
        // When looking for the next direction to go, the border ID is either incremented
        // or decremented in each step. When using the right-hand rule, the 60° rotation
        // means going L->H, H->R and R->L on a triangle with its horizontal side on the bottom.
        // That means going 0->2, 2->1 and 1->0 in terms of border IDs, so the step is a -1 mod 3.
        // If the horizontal side is on the top, its reversed: L->R, R->H and H->L, so 0->1, 1->2, 2->0.
        // Using the left-hand rule simply means going in the other direction.

        int current_hor_border_side = horizontal_border_side(r, c, map->cols);
        int op = (current_hor_border_side == BORDER_HBTM) ? -1 : 1;

        if (rule == RULE_LEFT)
            op = -op;

        // Adjust coordinates for the output.
        printf("%d,%d\n", r + 1, c + 1);

        // Add _op_ unless we find a side with no border. This should always break,
        // because a cell with all borders solid wouldn't be stepped into.
        int max_iterations_counter = 0;
        while (true)
        {
            entry_border += op;
            if (entry_border > 2)
                entry_border = 0;
            if (entry_border < 0)
                entry_border = 2;

            if (!isborder(map, r, c, entry_border))
                break;

            max_iterations_counter++;
            if (max_iterations_counter > 3)
            {
                fprintf(stderr, "Invalid cell state – traversing started on an invalid field?");
                return;
            }
        }

        // Move to the next cell and change coordinates and entry border accordingly
        if (entry_border == BORDER_LEFT)
        {
            c--;

            if (c == -1)
            {
                return;
            }
        }
        else if (entry_border == BORDER_RIGHT)
        {
            c++;

            if (c == map->cols)
            {
                return;
            }
        }
        else
        {
            if (current_hor_border_side == BORDER_HBTM)
            {
                r++;

                if (r == map->rows)
                {
                    return;
                }
            }
            else
            {
                r--;

                if (r == -1)
                {
                    return;
                }
            }
        }

        // Left->Right, Right->Left, Hor stays Hor
        if (entry_border != BORDER_HORIZONTAL)
        {
            entry_border = (entry_border == BORDER_LEFT) ? BORDER_RIGHT : BORDER_LEFT;
        }
    }
}

// Handles the --lpath or --rpath functionality. Loads the map from the specified file, parses and validates it,
// decides which border is the starting one and runs the traversal. Returns 0 or one of the ERR_ codes.
int perform_traversal(char *file_name, int r, int c, int rule)
{
    FILE *mapFile = fopen(file_name, "r");

    if (mapFile == NULL)
    {
        perror("Loading map file failed");
        return errno;
    }

    Map *m = 0;
    int res = parse_map(mapFile, &m, false);
    fclose(mapFile);

    if (res != LOAD_OK)
    {
        print_map_error(res);
        map_dtor(m);
        free(m);
        return res;
    }

    // Adjust coordinates (the program works with zero-based indexes).
    r--;
    c--;
    int stb = start_border(m, r, c);

    if (stb == ERR_INVALID_STARTING_CELL)
    {
        fprintf(stderr, "Invalid starting cell specified.\n");
        map_dtor(m);
        free(m);
        return ERR_INVALID_STARTING_CELL;
    }

    traverse(m, r, c, rule, stb);

    map_dtor(m);
    free(m);
    return 0;
}

// Handles the --test functionality. Loads the map from the specified file, parses and validates it.
// Prints "Valid" when the map is valid, or "Invalid" when it's not.
// Passes true to the verbose argument of parse_map().
int perform_test(char *file_name)
{
    FILE *mapFile = fopen(file_name, "r");

    if (mapFile == NULL)
    {
        perror("Loading map file failed");
        return errno;
    }

    Map *m = 0;
    int res = parse_map(mapFile, &m, true);
    fclose(mapFile);

    if (res == LOAD_OK)
    {
        printf("Valid\n");
    }
    else
    {
        printf("Invalid\n");
    }

    map_dtor(m);
    free(m);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        // Invalid argument count, return the ERR_ARGS code.
        fprintf(stderr, STR_HELP);
        return ERR_ARGS;
    }

    if(strcmp(argv[1], "--help") == 0) {
        printf(STR_HELP);
        return 0;
    }

    if (strcmp(argv[1], "--test") == 0)
    {
        if (argc < 3)
        {
            // Invalid argument count, return the ERR_ARGS code.
            fprintf(stderr, STR_HELP);
            return ERR_ARGS;
        }

        return perform_test(argv[2]);
    }

    if (argc < 5)
    {
        // Invalid argument count, return the ERR_ARGS code.
        fprintf(stderr, STR_HELP);
        return ERR_ARGS;
    }

    if (strcmp(argv[1], "--rpath") == 0)
    {
        return perform_traversal(argv[4], atoi(argv[2]), atoi(argv[3]), RULE_RIGHT);
    }

    if (strcmp(argv[1], "--lpath") == 0)
    {
        return perform_traversal(argv[4], atoi(argv[2]), atoi(argv[3]), RULE_LEFT);
    }
}
