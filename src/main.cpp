#include <stdio.h>
#include <iostream>

#include "io.hpp"
#include "engine.hpp"
#include "assert.hpp"

int main() {
    io::File file("example/main.lx");
    ASSERT(file, "Failed to open file");

    auto code = file.read<string>();
    ASSERT(!code.empty(), "Failed to read file");
    
    printf("Step 1:\n");
    engine::Tokenizer tokenizer(code);
    
    printf("\t- Cleaning\n");
    tokenizer.cleanup();

    printf("\t- Tokenizing\n");
    tokenizer.tokenize();

    printf("Step 2:\n");
    engine::IL il(tokenizer.getTokens());
    printf("\t- Analyzing\n");
    il.analyze();

    printf("\t- Optimizing\n");
    il.optimize();

    printf("Step 3:\n");
    engine::Assembler assembler(il.getILs());
    printf("\t- Translating\n");
    assembler.translate();

    printf("\t- Optimizing\n");
    assembler.optimize();
    
    printf("\t- Assembling\n");
    assembler.assemble();

    printf("\t- Saving\n");
    assembler.create("example/main.asm");

    return EXIT_SUCCESS;
}