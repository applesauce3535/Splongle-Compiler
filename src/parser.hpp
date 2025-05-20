#pragma once

#include "tokenization.hpp"
#include <variant>

struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprId {
    Token id;
};

struct NodeExpr {
    std::variant<NodeExprIntLit, NodeExprId> var; 
};

struct NodeStmtSplinge {
    Token id;
    NodeExpr expr;
};

struct NodeStmtExit {
    NodeExpr expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit, NodeStmtSplinge> var;
};

struct NodeProg{
    std::vector<NodeStmt> stmts;
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
            return NodeExpr{.var = NodeExprIntLit{.int_lit = consume()}}; // return a NodeExpr with int_lit assigned to the int_lit token it's looking at
        }
        else if (peek().has_value() && peek().value().type == TokenType::id) {
            return NodeExpr {.var = NodeExprId {.id = consume()}}; // return a NodeExpr with id assigned to the id it's looking at
        }
        else {
            return {};
        }
    }

    std::optional<NodeStmt> parse_stmt() {
        if (peek().has_value() && peek().value().type == TokenType::exit && peek(1).has_value() &&
            peek(1).value().type == TokenType::open_paren) { // ensure exit function is used with parentheses
            consume(); // consume exit
            consume(); // consume opening parenthesis
            NodeStmtExit stmt_exit;
            if (auto node_expr = parse_expr()) { // essentially asks is it true that node_expr has any value in this world
                stmt_exit = {.expr = node_expr.value()}; // make a NodeExit with expr value of node_expr's value
            }
            else {
                std::cerr << "Invalid expression, MAJOR FUCK UP\n";
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::close_paren) {
                consume();
            }
            else {
                std::cerr << "Expected closing parenthesis for 'exit', MAJOR FUCK UP\n";
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::splong) { // this is ensuring 'splong' follows the exit statement  
                consume();
            }
            else {
                std::cerr << "Expected 'splong', MAJOR FUCK UP\n";
                exit(EXIT_FAILURE);
            }
            return NodeStmt {.var = stmt_exit};
        }
        else if (peek().has_value() && peek().value().type == TokenType::splinge &&
                peek(1).has_value() && peek(1).value().type == TokenType::id &&
                peek(2).has_value() && peek(2).value().type == TokenType::assign) { // check that the token is a splinge data type, it has an id, and is assigned a value
            consume(); // consume splinge
            auto stmt_splinge = NodeStmtSplinge {.id = consume()}; // the splinge statement's id is the next consumed value
            consume(); // consume '='
            if (auto expr = parse_expr()) {
                stmt_splinge.expr = expr.value(); // the splinge's value should either be an int literal or a valid identifier
            }
            else {
                std::cerr << "Invalid expression, MAJOR FUCK UP\n";
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::splong) {
                consume();
            }
            else {
                std::cerr << "Expected 'splong', DOW\n";
                exit(EXIT_FAILURE);
            }
            return NodeStmt {.var = stmt_splinge};
        }
        else {
            return {};
        }
    }

    std::optional<NodeProg> parse_program() {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value()); // parse all the statements and put them into the program vector
            }
            else {
                std::cerr << "Invalid statement, DOW\n";
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

private:

    const std::vector<Token> m_tokens;
    size_t m_index = 0;

    [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const { // [[nodiscard]] = ignore stupid compiler complaints about a function literally doing nothing
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        else {
            return m_tokens.at(m_index + offset);
        }
    }

    inline Token consume() {
        return m_tokens.at(m_index++);
    }

};