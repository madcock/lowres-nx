//
// Copyright 2016 Timo Kloss
//
// This file is part of LowRes Core.
//
// LowRes Core is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes Core is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes Core.  If not, see <http://www.gnu.org/licenses/>.
//

#include "interpreter.h"
#include <string.h>

const char *TokenStrings[] = {
    NULL,
    
    NULL,
    NULL,
    NULL,
    NULL,
    
    ":",
    ",",
    ";",
    NULL,
    
    "=",
    ">=",
    "<=",
    "<>",
    ">",
    "<",
    "(",
    ")",
    "+",
    "-",
    "*",
    "/",
    "^",
    
    "AND",
    "DATA",
    "DIM",
    "ELSE",
    "END",
    "FOR",
    "GOSUB",
    "GOTO",
    "IF",
    "INPUT",
    "LET",
    "MOD",
    "NEXT",
    "NOT",
    "ON",
    "OR",
    "PRINT",
    "RANDOMIZE",
    "READ",
    "REM",
    "RESTORE",
    "RETURN",
    "STEP",
    "THEN",
    "TO",
    "XOR"
};

ErrorCode LRC_tokenizeProgram(Interpreter *interpreter, const char *sourceCode)
{
    const char *charSetDigits = "0123456789";
    const char *charSetLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
    const char *charSetAlphaNum = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";
    
    memset(interpreter, 0, sizeof(Interpreter));
    const char *character = sourceCode;
    
    while (*character)
    {
        if (interpreter->numTokens >= MAX_TOKENS)
        {
            return ErrorTooManyTokens;
        }
        Token *token = &interpreter->tokens[interpreter->numTokens];
        
        // line break
        if (*character == '\n')
        {
            token->type = TokenEol;
            interpreter->numTokens++;
            character++;
            continue;
        }
        
        // space
        if (*character == ' ' || *character == '\t')
        {
            character++;
            continue;
        }
        
        // string
        if (*character == '"')
        {
            character++;
            const char *firstCharacter = character;
            while (*character && *character != '"')
            {
                character++;
                if (*character == '\n')
                {
                    return ErrorExpectedEndOfString;
                }
            }
            token->type = TokenString;
            interpreter->numTokens++;
            character++;
            continue;
        }
        
        // number
        if (strchr(charSetDigits, *character))
        {
            float number = 0;
            int afterDot = 0;
            while (*character)
            {
                if (strchr(charSetDigits, *character))
                {
                    int digit = (int)*character - (int)'0';
                    if (afterDot == 0)
                    {
                        number *= 10;
                        number += digit;
                    }
                    else
                    {
                        number += (float)digit / afterDot;
                        afterDot *= 10;
                    }
                    character++;
                }
                else if (*character == '.' && afterDot == 0)
                {
                    afterDot = 10;
                    character++;
                }
                else
                {
                    break;
                }
            }
            token->type = TokenFloat;
            token->floatValue = number;
            interpreter->numTokens++;
            continue;
        }
        
        // Keyword
        TokenType foundKeywordToken = TokenUndefined;
        for (int i = 0; i < Token_count; i++)
        {
            const char *keyword = TokenStrings[i];
            if (keyword)
            {
                size_t keywordLen = strlen(keyword);
                int keywordIsAlphaNum = strchr(charSetAlphaNum, keyword[0]) != NULL;
                for (int pos = 0; pos <= keywordLen && character[pos]; pos++)
                {
                    char textCharacter = character[pos];
                    
                    if (pos < keywordLen)
                    {
                        char symbCharacter = keyword[pos];
                        if (symbCharacter != textCharacter)
                        {
                            // not matching
                            break;
                        }
                    }
                    else if (keywordIsAlphaNum && strchr(charSetAlphaNum, textCharacter))
                    {
                        // matching, but word is longer, so seems to be an identifier
                        break;
                    }
                    else
                    {
                        // symbol found!
                        foundKeywordToken = i;
                        character += keywordLen;
                        break;
                    }
                }
                if (foundKeywordToken != TokenUndefined)
                {
                    break;
                }
            }
        }
        if (foundKeywordToken != TokenUndefined)
        {
            if (foundKeywordToken == TokenREM)
            {
                // REM comment, skip until end of line
                while (*character)
                {
                    character++;
                    if (*character == '\n')
                    {
                        break;
                    }
                }
            }
            else
            {
                token->type = foundKeywordToken;
                interpreter->numTokens++;
            }
            continue;
        }
        
        // Identifier
        if (strchr(charSetLetters, *character))
        {
            const char *firstCharacter = character;
            while (*character)
            {
                if (strchr(charSetAlphaNum, *character))
                {
                    character++;
                }
                else
                {
                    break;
                }
            }
            if (*character == ':')
            {
                token->type = TokenLabel;
                character++;
            }
            else
            {
                token->type = TokenIdentifier;
            }
            interpreter->numTokens++;
            continue;
        }
        
        // Unexpected character
        return ErrorUnexpectedCharacter;
    }
    return ErrorNone;
}