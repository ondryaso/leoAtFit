/*
 * @file proj1.c
 * @author Ondřej Ondryáš (xondry02)
 * @date 2019-11-17
 * @brief An implementation of phone dialer search functionality.
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// 100 actual characters + \r + \n + \0
#define MAX_LINE_LENGTH 103
// When set to 1, the program will perform the non-premium funcionality
#define DISABLE_INTERRUPTIONS 0

int parse_arguments(int argc, char **argv, char **input);
bool matches_input(const char *name, const char *input);
bool is_valid_entry(const char *name, const char *phone, const char *input);

// Digit-characters mapping strings. It's an array of arrays with a string containing
// all possible matches for a digit, located at an index corresponding to the digit.
// Thus, nums[4] contains all characters that would be a valid match for the digit 4.
const char nums[10][10] = {"0+", "1", "2abcABC", "3defDEF", "4ghiGHI", "5jklJKL", "6mnoMNO", "7pqrsPQRS", "8tuvTUV", "9wxyzWXYZ"};

// The matching algorithm goes through the name character by character and checks whether
// the current character is a part of the predefined digit-characters mapping string
// on the position of _numCounter_. The _numCounter_ variable starts at 0 and increments
// when the check is true, moving to the next digit in the input. When _numCounter_ reaches
// the length of the input string, it means that a match for each digit was found, true
// is returned.
bool matches_input(const char *name, const char *input)
{
    size_t numCounter = 0;
    size_t nameLen = strlen(name);
    size_t inputLen = strlen(input);

    // The name won't match the input when it's shorter and nothing will match
    // anything when they're empty strings.
    if (nameLen == 0 || inputLen == 0 || nameLen < inputLen)
    {
        return false;
    }

    for (size_t i = 0; i < nameLen; i++)
    {
        int digit = input[numCounter] - 48; // In this case, the character is guaranteed to be a digit, but a check could be made here.

        if (strchr(nums[digit], name[i]) != NULL)
        {
            numCounter++;
        }
#if DISABLE_INTERRUPTIONS
        else
        {
            // If interruptions are not allowed, _numCounter_ must be zeroed on a failed match.
            // Only setting _numCounter_ to zero would introduce problems when the beginning of
            // a sequence would repeat – "Lloyd" wouldn't be matched with 56 ("lo"), because
            // the second L would zero _numCounter_, even though it should have stayed at one.
            // Using recursive calls to the method with the name string shifted by one is the
            // easiest solution with no side effects.
            return matches_input(name + 1, input);
        }
#endif

        if (numCounter == inputLen)
        {
            return true;
        }
    }

    return false;
}

bool is_valid_entry(const char *name, const char *phone, const char *input)
{
    if (input[0] == '\0')
        return true; // When the input is empty, it always matches.
    if (matches_input(phone, input))
        return true;
    if (matches_input(name, input))
        return true;

    return false;
}

// Parses arguments from argv and stores the required values into input and allowInterruptions.
int parse_arguments(int argc, char **argv, char **input)
{
    // Argc should always have at least one element. This is a kinda useless sanity check.
    if (argc == 0)
    {
        fprintf(stderr, "Unspecified behaviour.");
        return 1;
    }

    // If argc is 1, the first element is the program name, process that as a blank input.
    if (argc == 1)
    {
        *input = "";
        return 0;
    }

    // If argc is 2 (or bigger), load the input number.
    if (argc >= 2)
    {
        // Check if input only consists of numbers.
        char *p = argv[1];
        while (*p != '\0')
        {
            if (!isdigit(*p))
            {
                fprintf(stderr, "A number is expected as the first argument. All the other characters have to be valid digits.\n");
                return 2;
            }

            p++;
        }

        // An input check could be implemented here, but it's not required by the spec.
        *input = argv[1];
    }

    // Zero is ok.
    return 0;
}

int main(int argc, char **argv)
{
    char nameLine[MAX_LINE_LENGTH];
    char phoneLine[MAX_LINE_LENGTH];
    char *input;
    char *nextBuffer = nameLine;

    bool found = false;

    int code = parse_arguments(argc, argv, &input);
    if (code != 0)
    {
        return code;
    }

    // Load lines until end-of-file is reached.
    while (fgets(nextBuffer, MAX_LINE_LENGTH, stdin) != NULL)
    {
        // Cut off the newline characters. This should handle both DOS and Unix-style line endings.
        char *cr = strchr(nextBuffer, '\r');
        if (cr == NULL)
        {
            nextBuffer[strcspn(nextBuffer, "\n")] = 0;
        }
        else
        {
            cr[0] = 0;
        }

        if (nextBuffer == nameLine)
        {
            // The first line containing a name was loaded, now change the target buffer
            // and do the next cycle iteration.
            nextBuffer = phoneLine;
        }
        else
        {
            if (is_valid_entry(nameLine, phoneLine, input))
            {
                printf("%s, %s\n", nameLine, phoneLine);
                found = true;
            }

            nextBuffer = nameLine;
        }
    }

    if (!found)
    {
        printf("Not found\n");
    }

    return 0;
}