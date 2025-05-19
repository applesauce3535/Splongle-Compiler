#pragma once

#include <string>
#include <vector>
#include <iostream>

/*
    The different tokens of splongle
*/
enum class TokenType {exit, int_lit, splong};


struct Token { 
    /*
        a token consists of its token type and an optional value
    */
    TokenType type;
    std::optional<std::string> value {}; // default nothing
    
};

class Tokenizer {

public:

    inline explicit Tokenizer(std::string src):
        m_src(std::move(src)), m_index(0)
    {}

    inline std::vector<Token> tokenize() {
        /* 
            This function is responsible for tokenizing the input file.
            Usage: str = input file as a string
        */
        std::vector<Token> tokens; // used for storing all tokens, return value
        std::string buf; // used for storing current token
        while (peek().has_value()) {
            if (std::isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buf.push_back(consume());
                }
                if (buf == "exit") { // handle exit keyword
                    tokens.push_back({.type = TokenType::exit});
                    buf.clear();
                    continue;
                }
                else if (buf == "splong") { // handle splong end of statement
                    tokens.push_back({.type = TokenType::splong});
                    buf.clear();
                    continue;
                }
                else { // handle non-allowed identifier characters
                    std::cerr << "MAJOR FUCK UP\n";
                    exit(EXIT_FAILURE);
                }
            }
            else if (std::isdigit(peek().value())) { // handle integer literals
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                tokens.push_back({.type = TokenType::int_lit, .value = buf});
                buf.clear();
                continue;
            }
            else if (std::isspace(peek().value())) { // ignore any white space characters
                consume();
                continue;
            }
            else { // handle any undefined tokens of splongle
                std::cerr << "MAJOR FUCK UP\n";
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;
    }

private:
    
    [[nodiscard]] inline std::optional<char> peek(int ahead = 1) const { // [[nodiscard]] = ignore stupid compiler complaints about a function literally doing nothing
        if (m_index + ahead > m_src.length()) {
            return {};
        }
        else {
            return m_src.at(m_index);
        }
    }

    inline char consume() {
        return m_src.at(m_index++);
    }

    const std::string m_src;
    size_t m_index; // why is c++ like this

};