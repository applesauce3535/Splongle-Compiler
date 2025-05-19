#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include <vector>
#include <ostream>
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

    std::cout << "File contents: " << contents << "\n";

    Tokenizer tokenizer(std::move(contents)); // std::move improves performance i guess by not making copies

    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeExit> tree = parser.parse();
    if (!tree.has_value()) {
        std::cerr << "No exit statement found, MAJOR FUCK UP\n";
        return EXIT_FAILURE;
    }

    Generator generator(tree.value());

    
    std::fstream output("out.asm", std::ios::out); // treat the output assembly file as ONLY output
    output << generator.generate(); // write assembly to the output
    output.close();

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");
    return EXIT_SUCCESS;
}