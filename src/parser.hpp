#pragma once

#include "tokenization.hpp"

struct NodeExpr {
    Token int_lit;
};

struct NodeExit {
    NodeExpr expr;
};

struct NodeSplong {

};


class Parser {

public:
    inline explicit Parser(std::vector<Token> tokens) 
    : m_tokens(std::move(tokens))
    {}

    std::optional<NodeExpr> parse_expr() {
        if (peek().has_value() && peek().value().type == TokenType::int_lit) {
            return NodeExpr{.int_lit = consume()}; // return a NodeExpr with int_lit assigned to the int_lit token it's looking at
        }
        else {
            return {};
        }
    }

    std::optional<NodeExit> parse() {
        std::optional<NodeExit> exit_node;
        while (peek().has_value()) {
            if (peek().value().type == TokenType::exit) {
                consume();
                if (auto node_expr = parse_expr()) { // essentially asks is it true that node_expr has any value in this world
                    exit_node = NodeExit{.expr = node_expr.value()}; // make a NodeExit with expr value of node_expr's value
                }
                else {
                    std::cerr << "invalid expression, MAJOR FUCK UP\n";
                    exit(EXIT_FAILURE);
                }
                if (peek().has_value() && peek().value().type == TokenType::splong) { // this is ensuring 'splong' follows the exit statement  
                    consume();
                }
                else {
                    std::cerr << "invalid expression, MAJOR FUCK UP\n";
                    exit(EXIT_FAILURE);
                }
            }
        }
        m_index = 0;
        return exit_node;
    }

private:

    const std::vector<Token> m_tokens;
    size_t m_index = 0;

    [[nodiscard]] inline std::optional<Token> peek(int ahead = 1) const { // [[nodiscard]] = ignore stupid compiler complaints about a function literally doing nothing
        if (m_index + ahead > m_tokens.size()) {
            return {};
        }
        else {
            return m_tokens.at(m_index);
        }
    }

    inline Token consume() {
        return m_tokens.at(m_index++);
    }


};