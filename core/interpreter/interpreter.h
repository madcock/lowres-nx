//
// Copyright 2016-2017 Timo Kloss
//
// This file is part of LowRes NX.
//
// LowRes NX is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes NX is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes NX.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef interpreter_h
#define interpreter_h

#include <stdio.h>
#include <stdbool.h>
#include "interpreter_config.h"
#include "token.h"
#include "error.h"
#include "value.h"
#include "labels.h"
#include "variables.h"
#include "data.h"
#include "big_endian.h"
#include "text_lib.h"

#define BAS_TRUE -1.0f
#define BAS_FALSE 0.0f

struct Core;

extern const char *CharSetDigits;
extern const char *CharSetLetters;
extern const char *CharSetAlphaNum;
extern const char *CharSetHex;

enum Pass {
    PassPrepare,
    PassRun
};

enum State {
    StateNoProgram,
    StateEvaluate,
    StateWait,
    StateInput,
    StateEnd
};

enum Mode {
    ModeNone,
    ModeMain,
    ModeInterrupt
};

struct Symbol {
    char name[SYMBOL_NAME_SIZE];
};

struct RomDataEntry {
    BigEndianUInt16 start;
    BigEndianUInt16 length;
};

enum InterruptType {
    InterruptTypeRaster,
    InterruptTypeVBL
};

struct Interpreter {
    enum Pass pass;
    enum State state;
    enum Mode mode;
    struct Token *pc;
    enum ErrorCode exitErrorCode;
    
    struct Token tokens[MAX_TOKENS];
    int numTokens;
    struct Symbol symbols[MAX_SYMBOLS];
    int numSymbols;
    
    struct LabelStackItem labelStackItems[MAX_LABEL_STACK_ITEMS];
    int numLabelStackItems;
    struct JumpLabelItem jumpLabelItems[MAX_JUMP_LABEL_ITEMS];
    int numJumpLabelItems;
    bool isSingleLineIf;
    
    struct SimpleVariable simpleVariables[MAX_SIMPLE_VARIABLES];
    int numSimpleVariables;
    struct ArrayVariable arrayVariables[MAX_ARRAY_VARIABLES];
    int numArrayVariables;
    struct RCString *nullString;
    
    struct Token *firstData;
    struct Token *lastData;
    struct Token *currentDataToken;
    struct Token *currentDataValueToken;
    
    struct Token *currentOnRasterToken;
    struct Token *currentOnVBLToken;
    
    int waitCount;
    bool exitEvaluation;
    
    struct TextLib textLib;
};

enum ErrorCode itp_compileProgram(struct Core *core, const char *sourceCode);
void itp_resetProgram(struct Core *core);
void itp_runProgram(struct Core *core);
void itp_runInterrupt(struct Core *core, enum InterruptType type);
void itp_freeProgram(struct Core *core);
enum ErrorCode itp_getExitErrorCode(struct Core *core);
int itp_getPcPositionInSourceCode(struct Core *core);

union Value *itp_readVariable(struct Core *core, enum ValueType *type, enum ErrorCode *errorCode);
struct TypedValue itp_evaluateExpression(struct Core *core, enum TypeClass typeClass);
struct TypedValue itp_evaluateNumericExpression(struct Core *core, int min, int max);
struct TypedValue itp_evaluateOptionalExpression(struct Core *core, enum TypeClass typeClass);
struct TypedValue itp_evaluateOptionalNumericExpression(struct Core *core, int min, int max);
int itp_isEndOfCommand(struct Interpreter *interpreter);
enum ErrorCode itp_endOfCommand(struct Interpreter *interpreter);

#endif /* interpreter_h */
