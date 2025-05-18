#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include <vector>
#include <ostream>
enum class TokenType { 

    /*
        The different tokens of splongle
    */

    _return,
    int_lit,
    splong

};

struct Token { 

    /*
        a token consists of its token type and an optional value
    */

    TokenType type;
    std::optional<std::string> value {}; // default nothing
    
};


std::vector<Token> tokenize(const std::string& str) {

    /* 
        This function is responsible for tokenizing the input file.
        Usage: str = input file as a string
    */

    std::vector<Token> tokens; // used for storing all tokens, return value
    std::string buf; // used for storing current token
    
    for (int i = 0; i < str.length(); ++i) {

        if (i < str.length() && std::isalpha(str.at(i))) { // identifiers must begin with a character

            buf.push_back(str.at(i));
            ++i;

            while (i < str.length() && std::isalnum(str.at(i))) { // continually add to the buffer until reaching
                                              // a terminating character
                buf.push_back(str.at(i));
                ++i;

            }

            --i; // we will end one character ahead of the token, so take one step back
            
            if (buf == "return") { // if the token we looked at is return, push the return token
        
                tokens.push_back({.type = TokenType::_return});
                buf.clear();
                continue;

            }

            else if (buf == "splong") { // if the token was splong, push the splong token

                tokens.push_back({.type = TokenType::splong});
                buf.clear();
                continue;

            }

            else { // handle unknown identifiers and keywords
                
                std::cerr << "MAJOR FUCK UP\n";
                exit(EXIT_FAILURE);

            }
        }

        else if (std::isdigit(str.at(i))) { // handling integer literals

            buf.push_back(str.at(i));
            ++i;

            while (i < str.length() && std::isdigit(str.at(i))) {

                buf.push_back(str.at(i));
                ++i;

            }

            --i; // again must go back one
            tokens.push_back({.type = TokenType::int_lit, .value = buf});
            buf.clear();

        }

        else if (i < str.length() && std::isspace(str.at(i))) { // ignore any kind of white space

            continue;

        }

        else { // if nothing above happened then there's a major fuck up

            std::cerr << "MAJOR FUCK UP\n";
            exit(EXIT_FAILURE);

        }
    }

    return tokens;

}

std::string tokens_to_asm (const std::vector<Token>& tokens) {
    /*
        This function is responsible or translating splongle tokens into x86 assembly.
        Usage: tokens = a vector of valid tokens gained from tokenizing an input file
    */

    std::stringstream output; // return value
    output << "global _start\n_start:\n";
    for (int i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens.at(i);
        if (token.type == TokenType::_return) { // when we encounter a return token we must ensure there are at most
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

    std::vector<Token> tokens = tokenize(contents);
    std::cout << tokens_to_asm(tokens) << "\n";
    
    std::fstream output("out.asm", std::ios::out); // treat the output assembly file as ONLY output
    output << tokens_to_asm(tokens); // write assembly to the output
    output.close();
    system("nasm -f win64 out.asm");
    system("ld -o out out.obj");
    return EXIT_SUCCESS;
}