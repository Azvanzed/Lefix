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
    
    engine::Tokenizer tokenizer(code);
    tokenizer.cleanup();
    auto tokens = tokenizer.tokenize();

    printf("%ld tokens\n", tokens.size());

    engine::IL il(tokens);
    il.analyze();
    il.optimize();

    printf("%ld ils\n", il.getILs().size());

    engine::Assembler assembler(il.getILs());
    assembler.assemble();
    assembler.create("example/main.asm");

    return EXIT_SUCCESS;
}