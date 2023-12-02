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
    
    printf("Tokenizing code\n");
    engine::Tokenizer tokenizer(code);
    tokenizer.cleanup();
    tokenizer.tokenize();

    printf("Analyzing tokens\n");
    engine::IL il(tokenizer.getTokens());
    il.analyze();
    il.optimize();

    printf("Assembling IL\n");
    engine::Assembler assembler(il.getILs());
    assembler.translate();
    assembler.assemble();
    assembler.create("example/main.asm");

    return EXIT_SUCCESS;
}