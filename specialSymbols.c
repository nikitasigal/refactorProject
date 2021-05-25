#include "specialSymbols.h"

void processSpecialSymbols(char *input, int inputSize, char *output, int *outputSize) {
    char singleSymbols[] = {'-', '+', '/', '^', '=', '<', '>',
                            '%', '&', '|', '?'};   //& and * are too complex and are to be treated separately
    char doubleSymbols[][2] = {"==", "!=", ">>", "<<", "||", "&&", ">=", "<=", "+=", "-=", "*=", "/=", "^=", "|=",
                               "&="};    //-- and ++ are exceptions
    char exceptionSymbols[][2] = {"--", "++", "->"};
    char tripleSymbols[][3] = {">>=", "<<="};

    bool isPrevSpace = false;

    for (int i = 0; i < inputSize;) {
        // Before everything else - skip commented segments, strings, chars, #include and #define instructions
        if (inputSize - i >= 2 && input[i] == '/') {
            if (input[i + 1] == '/') {
                while (input[i] != '\n' && i < inputSize)
                    sprintf(output + (*outputSize)++, "%c", input[i++]);
                sprintf(output + (*outputSize)++, "\n");
                i++;
                isPrevSpace = true;
                continue;
            }
            if (input[i + 1] == '*') {
                sprintf(output + (*outputSize), "//");
                (*outputSize)+=2;
                i+=2;
                while (input[i] != '*' || input[i + 1] != '/') {
                    sprintf(output + (*outputSize)++, "%c", input[i]);
                    if (input[i] == '\n'){
                        sprintf(output + (*outputSize), "//");
                        (*outputSize)+=2;
                    }
                    i++;
                }
                sprintf(output + *outputSize, "\n");
                (*outputSize)++;
                i += 2;
                isPrevSpace = true;
                continue;
            }
        }
        if (input[i] == '\'') {
            sprintf(output + (*outputSize)++, "'");
            i++;
            while ((input[i - 1] == '\\' && input[i - 2] != '\\') || input[i] != '\'')
                sprintf(output + (*outputSize)++, "%c", input[i++]);
            sprintf(output + (*outputSize)++, "'");
            i++;
            isPrevSpace = false;
            continue;
        }
        if (input[i] == '"') {
            sprintf(output + (*outputSize)++, "\"");
            i++;
            while (input[i - 1] == '\\' || input[i] != '"')
                sprintf(output + (*outputSize)++, "%c", input[i++]);
            sprintf(output + (*outputSize)++, "\"");
            i++;
            isPrevSpace = false;
            continue;
        }
        if (input[i] == '#') {
            while (input[i] != '\n')
                sprintf(output + (*outputSize)++, "%c", input[i++]);
            sprintf(output + (*outputSize)++, "%c", input[i++]);
            isPrevSpace = true;
            continue;
        }

        // First - check or triples (if possible)
        if (inputSize - i >= 3) {
            bool isTriple = false;
            for (int j = 0; j < 2; ++j)
                if (!strncmp(tripleSymbols[j], input + i, 3))
                    isTriple = true;
            if (isTriple) {
                if (!isPrevSpace)
                    sprintf(output + (*outputSize)++, " ");
                sprintf(output + *outputSize, "%c%c%c ", input[i], input[i + 1], input[i + 2]);
                *outputSize += 4;
                i += 3;
                isPrevSpace = true;
                continue;
            }
        }

        // Second - check for doubles and exceptions
        if (inputSize - i >= 2) {
            bool isDouble = false;
            for (int j = 0; j < 15; ++j)
                if (!strncmp(doubleSymbols[j], input + i, 2))
                    isDouble = true;
            if (isDouble) {
                if (output[*outputSize - 1] != ' ')
                    sprintf(output + (*outputSize)++, " ");
                sprintf(output + *outputSize, "%c%c ", input[i], input[i + 1]);
                *outputSize += 3;
                i += 2;
                isPrevSpace = true;
                continue;
            }

            bool isException = false;
            for (int j = 0; j < 3; ++j)
                if (!strncmp(exceptionSymbols[j], input + i, 2))
                    isException = true;
            if (isException) {
                sprintf(output + *outputSize, "%c%c", input[i], input[i + 1]);
                *outputSize += 2;
                i += 2;
                isPrevSpace = false;
                continue;
            }
        }

        // Third - singles
        {
            bool isSingle = false;
            for (int j = 0; j < 11; ++j)
                if (input[i] == singleSymbols[j])
                    isSingle = true;
            if (isSingle) {
                //if (!isPrevSpace)
                if (output[*outputSize - 1] != ' ')
                    sprintf(output + (*outputSize)++, " ");
                sprintf(output + *outputSize, "%c ", input[i]);
                *outputSize += 2;
                i++;
                isPrevSpace = true;
                continue;
            }
        }

        //Forth - complicated symbols
        if (input[i] == '&') {
            sprintf(output + (*outputSize)++, "&");
            i++;
            isPrevSpace = true; // This way it will be attached to the next symbol
            continue;
        }
        if (input[i] == '*') {  // Disambiguation between ptr and multiplication
            char previousSymbol;
            if (i > 0 && input[i - 1] != ' ' && input[i - 1] != '\n')
                previousSymbol = input[i - 1];
            else if (i > 1)
                previousSymbol = input[i - 2];
            else
                previousSymbol = 0;
            if (isalnum(previousSymbol) || previousSymbol == ')') {
                // Multiplication
                //if (!isPrevSpace)
                if (output[*outputSize - 1] != ' ')
                    sprintf(output + (*outputSize)++, " ");
                sprintf(output + *outputSize, "* ");
                *outputSize += 2;
                i++;
                isPrevSpace = true;
                continue;
            } else {
                sprintf(output + (*outputSize)++, "*");
                i++;
                isPrevSpace = true; // This way it will be attached to the next symbol
                continue;
            }
        }
        if (input[i] == '(') {
            sprintf(output + (*outputSize)++, "(");
            i++;
            isPrevSpace = true;
            continue;
        }

        if (input[i] == ',') {
            if (i > 0 && output[(*outputSize) - 1] == ' ')
                (*outputSize)--;
            sprintf(output + *outputSize, ", ");
            *outputSize += 2;
            i++;
            isPrevSpace = true;
            continue;
        }

        if (input[i] == ')' || input[i] == ']') {
            if (i > 0 && output[(*outputSize) - 1] == ' ')
                (*outputSize)--;
            if (input[i] == ')')
                sprintf(output + (*outputSize)++, ")");
            else
                sprintf(output + (*outputSize)++, "]");
            i++;
            isPrevSpace = true;
            continue;
        }

        if (input[i] == '[' || input[i] == ';') {
            if (i > 0 && output[(*outputSize) - 1] == ' ')
                (*outputSize)--;
            if (input[i] == '[')
                sprintf(output + (*outputSize)++, "[");
            else
                sprintf(output + (*outputSize)++, ";");
            i++;
            isPrevSpace = true;
            continue;
        }



        if (input[i] == ' ' || input[i] == '\n' || input[i] == '\t') {
            if (!isPrevSpace) {
                sprintf(output + (*outputSize)++, " ");
                isPrevSpace = true;
            }
        } else {
            sprintf(output + (*outputSize)++, "%c", input[i]);
            isPrevSpace = false;
        }
        i++;
    }

}