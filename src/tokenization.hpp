#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

/*
    The different tokens of splongle
*/
enum class TokenType {id, exit, int_lit, splong, open_paren, close_paren, splinge, splongd, dp_lit, assign, 
                      add, mul};


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
                else if (buf == "splinge") { // splinge data type
                    tokens.push_back({.type = TokenType::splinge});
                    buf.clear();
                    continue;
                }
                else if (buf == "splongd") {
                    tokens.push_back({.type = TokenType::splongd});
                    buf.clear();
                    continue;
                }
                else { // make a variable 
                    tokens.push_back({.type = TokenType::id, .value = buf});
                    buf.clear();
                    continue;
                }
            }
            // TODO: I need to check what the data type declared for the variable is, otherwise I think this could put doubles in integer identifiers
            // because there isn't currently type checking
            else if (peek().value() == '.') { // handle double-point literals (64-bits)
                buf.push_back(consume());
                while (peek().has_value() && (std::isdigit(peek().value()) || peek().value() == '.')) {
                    buf.push_back(consume());
                }
                if (std::count(buf.begin(), buf.end(), '.') > 1) {
                    std::cerr << "Floating-point literal " << buf << " has more than 1 decimal point, DOW\n";
                    exit(EXIT_FAILURE);
                }
                size_t dot_index = buf.find('.');
                if (dot_index == std::string::npos) { // handle a missing decimal point
                    std::cerr << "Missing decimal point in double literal: " << buf << ", DOW\n";
                    exit(EXIT_FAILURE);
                }
                if (dot_index + 1 >= buf.length()) { // handle the decimal point being at the end of the number, meaning no digits follow it
                    std::cerr << "Decimal point at end of literal: " << buf << ", DOW\n";
                    exit(EXIT_FAILURE);
                }
                if (!std::isdigit(buf[dot_index + 1])) { // handle if a digit does not follow the decimal point
                    std::cerr << "No digit after decimal point in double literal: " << buf << ", DOW\n";
                    exit(EXIT_FAILURE);
                }
                else { // if it passes all of the checks, tokenize it
                    tokens.push_back({.type = TokenType::dp_lit, .value = buf});
                    buf.clear();
                    continue;
                }
            } 
            else if (std::isdigit(peek().value())) { // handle integer literals (64-bits)
                buf.push_back(consume());
                while (peek().has_value() && (std::isdigit(peek().value()) || peek().value() == '.')) {
                    buf.push_back(consume());
                }
                if (buf.find('.') != std::string::npos) { // if the integer just tokenized has a decimal point make it a double
                    if (std::count(buf.begin(), buf.end(), '.') > 1) {
                        std::cerr << "Floating-point literal " << buf << " has more than 1 decimal point, DOW\n";
                        exit(EXIT_FAILURE);
                    }
                    size_t dot_index = buf.find('.');
                    if (dot_index == std::string::npos) { // handle a missing decimal point
                        std::cerr << "Missing decimal point in double literal: " << buf << ", DOW\n";
                        exit(EXIT_FAILURE);
                    }
                    if (dot_index + 1 >= buf.length()) { // handle the decimal point being at the end of the number, meaning no digits follow it
                        std::cerr << "Decimal point at end of literal: " << buf << ", DOW\n";
                        exit(EXIT_FAILURE);
                    }
                    if (!std::isdigit(buf[dot_index + 1])) { // handle if a digit does not follow the decimal point
                        std::cerr << "No digit after decimal point in double literal: " << buf << ", DOW\n";
                        exit(EXIT_FAILURE);
                    }
                    else { // if it passes all of the checks, tokenize it
                        tokens.push_back({.type = TokenType::dp_lit, .value = buf});
                        buf.clear();
                        continue;
                    }
                }
                else { // otherwise we can say it's an integer
                tokens.push_back({.type = TokenType::int_lit, .value = buf});
                buf.clear();
                continue;
                }
            }
            else if (std::isspace(peek().value())) { // ignore any white space characters
                consume();
                continue;
            }
            else if (peek().value() == '(') { // handle openeing parenthesis
                consume();
                tokens.push_back({.type = TokenType::open_paren});
                continue;
            }
            else if (peek().value() == ')') { // handle closing parenthesis
                consume();
                tokens.push_back({.type = TokenType::close_paren});
                continue;
            }
            else if (peek().value() == '=') { // handle assignment
                consume();
                tokens.push_back({.type = TokenType::assign});
                continue;
            }
            else if (peek().value() == '+') { // handle addition
                consume();
                tokens.push_back({.type = TokenType::add});
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
    
    [[nodiscard]] inline std::optional<char> peek(int offset = 0) const { // [[nodiscard]] = ignore stupid compiler complaints about a function literally doing nothing
        if (m_index + offset >= m_src.length()) {
            return {};
        }
        else {
            return m_src.at(m_index + offset);
        }
    }

    inline char consume() {
        return m_src.at(m_index++);
    }

    const std::string m_src;
    size_t m_index;

};