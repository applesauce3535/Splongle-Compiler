#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include <vector>
#include <ostream>
#include "./arena.hpp"
#include "./tokenization.hpp"
#include "./parser.hpp"
#include "./generation.hpp"



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

    std::cout << "File contents: \n" << contents << "\n";

    Tokenizer tokenizer(std::move(contents)); // std::move improves performance i guess by not making copies

    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_program();
    if (!prog.has_value()) {
        std::cerr << "Invalid program, DOW\n";
        return EXIT_FAILURE;
    }

    Generator generator(prog.value());

    
    std::fstream output("out.asm", std::ios::out); // treat the output assembly file as ONLY output
    output << generator.gen_prog(); // write assembly to the output
    output.close();

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");
    std::stringstream assembly_stream;
    std::fstream ass_file("out.asm", std::ios::in);
    assembly_stream << ass_file.rdbuf();
    std::string assembly = assembly_stream.str();
    ass_file.close();
    std::cout << "Assembly code generated:\n" << assembly << "\n";
    return EXIT_SUCCESS;
}