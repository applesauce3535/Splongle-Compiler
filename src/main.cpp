#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include <vector>
#include <ostream>
#include "./tokenization.hpp"



std::string tokens_to_asm (const std::vector<Token>& tokens) {
    /*
        This function is responsible or translating splongle tokens into x86 assembly.
        Usage: tokens = a vector of valid tokens gained from tokenizing an input file
    */

    std::stringstream output; // return value
    output << "global _start\n_start:\n";
    for (int i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens.at(i);
        if (token.type == TokenType::exit) { // when we encounter a return token we must ensure there are at most
                                                // two tokens next
            if (i + 1 < tokens.size() && tokens.at(i + 1).type == TokenType::int_lit) {
                if (i + 2 < tokens.size() && tokens.at(i + 2).type == TokenType::splong) {
                    output << "    mov rax, 60\n";
                    output << "    mov rdi, " << tokens.at(i + 1).value.value() << "\n";
                    output << "    syscall";
                }
            }
        }
    }

    return output.str();

}


int main(int argc, char* argv[]) {
    if (argc != 2) { // handling incorrect usage of the splongle compiler

        std::cerr << "Incorrect usage of splongc\n";
        std::cerr << "Call splongc, then provide a splongle source file\n";
        return EXIT_FAILURE;

    }

    std::fstream input(argv[1], std::ios::in); // treat the input file (cl argument 2) as ONLY input
    std::stringstream contents_stream;
    contents_stream << input.rdbuf(); // read the entire input file as a string stream
    std::string contents = contents_stream.str(); // convert that string stream into a string
    input.close();

    std::cout << "File contents: " << contents << "\n";

    Tokenizer tokenizer(std::move(contents)); // std::move improves performance i guess by not making copies

    std::vector<Token> tokens = tokenizer.tokenize();
    std::cout << tokens_to_asm(tokens) << "\n";
    
    std::fstream output("out.asm", std::ios::out); // treat the output assembly file as ONLY output
    output << tokens_to_asm(tokens); // write assembly to the output
    output.close();

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");
    return EXIT_SUCCESS;
}