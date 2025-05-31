#pragma once

#include "tokenization.hpp"
#include <variant>
#include <optional>
#include "./arena.hpp"

struct NodeTermIntLit {
    // for when an integer literal is used
    Token int_lit;
};

struct NodeTermDPLit {
    // for when a double-point literal is used
    Token dp_lit;
};

struct NodeTermId {
    // for when an identifier is used
    Token id;
}; 


struct NodeExpr;

struct NodeTermExpr {
    // used for holding parenthesizd expressions
    NodeExpr* expr;
};

// don't be scared by any of these pointers. they are placed into the memory arena when parsing happens
// and then the whole arena is freed after code generation
struct NodeBinExprMul {
    // a binary multiplication node consists of a left and right operand
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprDiv {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprAdd {
    // a binary addition node consists of a left and right operand
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprSub {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExpr {
    // a whole binary expression can be any arithmetic operation
    std::variant<NodeBinExprAdd*, NodeBinExprSub*, NodeBinExprMul*, NodeBinExprDiv*> var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermDPLit*, NodeTermId*, NodeTermExpr*> var;
};

struct NodeExpr {
    // an expression can be an integer literal, an identifier, a double-point literal,
    // or a binary expression consisting of two expressions, which is what var is
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtSplinge {
    // a splinge will be an identifier and will also contain an expression as its value
    Token id;
    NodeExpr* expr;
};

struct NodeStmtSplongd {
    // a splongd will be an identifier and will also contain an expression as its value
    Token id;
    NodeExpr* expr;
};

struct NodeStmtExit {
    // the exit() statement must contain an expression in the parentheses
    NodeExpr* expr;
};

struct NodeStmt {
    // a statement can either be exit() or a splinge (that is int) declaration
    // TODO: implement more built-ins and data types
    std::variant<NodeStmtExit*, NodeStmtSplinge*, NodeStmtSplongd*> var;
};

struct NodeProg{
    // this is just saying the whole program should be a list of statements
    std::vector<NodeStmt*> stmts;
};



class Parser {

public:
    inline explicit Parser(std::vector<Token> tokens) 
    : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 4) // arena is 4MB
    {}

    // function to define operator precedence
    int precedence(TokenType op) {
        switch (op) {
            case TokenType::mul:
            case TokenType::div:
                return 2;
            case TokenType::add:
            case TokenType::sub:
                return 1;
            default:
                return 0;
        }
    }

    std::optional<NodeTerm*> parse_term() {
        if (!peek().has_value()) {
            return std::nullopt;
        }
        if (peek()->type == TokenType::open_paren) {
            consume(); // consume '('
            auto expr = parse_expr(); // parse the inner expression before anything else with default precedence
            if (!expr.has_value()) { // make sure something is actually after the '('
                std::cerr << "Expected expression after '(', DOW\n";
                exit(EXIT_FAILURE);
            }
            if (!peek().has_value() || peek()->type != TokenType::close_paren) { // make sure there is a matching ')'
                std::cerr << "Expected ')' after expression, DOW\n";
                exit(EXIT_FAILURE);
            }
            consume(); // consume ')'

            // the following wraps the whole expression in the parentheses as a term
            auto term_expr = m_allocator.alloc<NodeTermExpr>();
            term_expr->expr = expr.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_expr;

            return term;
        }
        if (peek().has_value() && peek().value().type == TokenType::int_lit) {
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>(); // allocate size for an integer literal term
            term_int_lit->int_lit = consume(); // the integer literal is the consumed token
            auto term = m_allocator.alloc<NodeTerm>(); // allocate size for a term
            term->var = term_int_lit; // the variant for the NodeTerm becomes an int literal
            return term; 
        }
        else if (peek().has_value() && peek().value().type == TokenType::dp_lit) {
            auto term_dp_lit = m_allocator.alloc<NodeTermDPLit>(); // allocate size for a double-point literal term
            term_dp_lit->dp_lit = consume(); // the double-point literal is the consumed token
            auto term = m_allocator.alloc<NodeTerm>(); // allocate size for a term
            term->var = term_dp_lit; // the variant for the NodeTerm becomes a double-point literal
            return term; 
        }
        else if (peek().has_value() && peek().value().type == TokenType::id) {
            auto term_id = m_allocator.alloc<NodeTermId>(); // allocate size for an identifier term
            term_id->id = consume(); // the id is the consumed token
            auto term = m_allocator.alloc<NodeTerm>(); // allocate size for a term
            term->var = term_id; // the variant for the NodeTerm becomes an id
            return term;
        }
        else {
            return {};
        }
    }

    std::optional<NodeExpr*> parse_expr(int min_prec = 0) {
        auto left_opt = parse_term();
        if (!left_opt.has_value()) { // if no value was created, the expression is incorrect
            return std::nullopt;
        }

        NodeExpr* left_operand = m_allocator.alloc<NodeExpr>();
        left_operand->var = left_opt.value(); // left operand variant is the value from the left optional (constant or variable)

        while (peek().has_value()) {
            TokenType op = peek()->type; // look at what the operator is
            int prec = precedence(op); // get the precedence of the operator
            if (prec == 0 || prec < min_prec) {
                break;
            }
            consume(); // consume operator
            int next_min_prec = prec + 1;
            auto right_opt = parse_expr(next_min_prec);
            if (!right_opt) {
                std::cerr << "Expected expression after operator, DOW\n";
                exit(EXIT_FAILURE);
            }
            NodeExpr* right_operand = right_opt.value();
            auto bin_expr = m_allocator.alloc<NodeBinExpr>(); // allocate room for a bin expression node
            if (op == TokenType::mul) { // handle multiplication
                auto bin_expr_mul = m_allocator.alloc<NodeBinExprMul>();
                bin_expr_mul->left = left_operand; // the bin expression's left operand value
                bin_expr_mul->right = right_operand; // its right operand value
                bin_expr->var = bin_expr_mul;
            }
            else if (op == TokenType::div) { // handle division
                auto bin_expr_div = m_allocator.alloc<NodeBinExprDiv>();
                bin_expr_div->left = left_operand; // the bin expression's left operand value
                bin_expr_div->right = right_operand; // its right operand value
                bin_expr->var = bin_expr_div;
            }
            else if (op == TokenType::add) { // handle addition
                auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
                bin_expr_add->left = left_operand; // the bin expression's left operand value
                bin_expr_add->right = right_operand; // its right operand value
                bin_expr->var = bin_expr_add;
            }
            else if (op == TokenType::sub) { // handle subtraction
                auto bin_expr_sub = m_allocator.alloc<NodeBinExprSub>();
                bin_expr_sub->left = left_operand;
                bin_expr_sub->right = right_operand;
                bin_expr->var = bin_expr_sub;
            }
            left_operand = m_allocator.alloc<NodeExpr>();
            left_operand->var = bin_expr;
        }
        return left_operand; // final result of any number of binary expressions
    }

    std::optional<NodeStmt*> parse_stmt() {
        if (peek().has_value() && peek().value().type == TokenType::exit && peek(1).has_value() &&
            peek(1).value().type == TokenType::open_paren) { // ensure exit function is used with parentheses
            consume(); // consume 'exit'
            consume(); // consume '('
            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (auto node_expr = parse_expr()) { // essentially asks is it true that node_expr has any value in this world
                stmt_exit->expr = node_expr.value(); // make a NodeStmtExit with expr value of node_expr's value
            }
            else {
                std::cerr << "Invalid expression, DOW\n";
                exit(EXIT_FAILURE);
            }
            if (!peek().has_value() || peek().value().type != TokenType::close_paren) { // ensure there is a matching closing parenthesis
                std::cerr << "Expected closing parenthesis for 'exit', DOW\n";
                exit(EXIT_FAILURE);
            }
            consume(); // consume ')'

            if (!peek().has_value() || peek().value().type != TokenType::splong) { // this is ensuring 'splong' follows the exit statement  
                std::cerr << "Expected 'splong', DOW\n";
                exit(EXIT_FAILURE);
            }
            consume(); // consume 'splong'
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        }
        else if (peek().has_value() && peek().value().type == TokenType::splinge &&
                peek(1).has_value() && peek(1).value().type == TokenType::id &&
                peek(2).has_value() && peek(2).value().type == TokenType::assign) { // check that the token is a splinge data type, it has an id, and is assigned a value
            consume(); // consume splinge
            auto stmt_splinge = m_allocator.alloc<NodeStmtSplinge>();
            stmt_splinge->id = consume(); // getting the identifier
            consume(); // consume '='
            if (auto expr = parse_expr()) {
                stmt_splinge->expr = expr.value(); // the splinge's value should either be an int literal or a valid identifier
            }
            else {
                std::cerr << "Invalid expression, DOW\n";
                exit(EXIT_FAILURE);
            }
            if (!peek().has_value() || peek().value().type != TokenType::splong) { // ensure 'splong' follows identifier declaration
                std::cerr << "Expected 'splong', DOW (this is the first expected splong)\n";
                exit(EXIT_FAILURE);
            }
            consume(); // consume 'splong'
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_splinge;
            return stmt;
        }
        else if (peek().has_value() && peek().value().type == TokenType::splongd &&
                peek(1).has_value() && peek(1).value().type == TokenType::id &&
                peek(2).has_value() && peek(2).value().type == TokenType::assign) { // check that the token is a splongd data type, it has an id, and is assigned a value
            consume(); // consume 'splongd'
            auto stmt_splongd = m_allocator.alloc<NodeStmtSplongd>();
            stmt_splongd->id = consume(); // getting the identifier
            consume(); // conusme the '='
            if (auto expr = parse_expr()) {
                stmt_splongd->expr = expr.value(); // the splongd's value should either be a double-point literal or a valid id
            }
            else {
                std::cerr << "Invalid expression, DOW\n";
                exit(EXIT_FAILURE);
            }
            if (!peek().has_value() || peek().value().type != TokenType::splong) {
                std::cerr << "Expected 'splong', DOW\n";
                exit(EXIT_FAILURE);
            }
            consume(); // consume 'splong'
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_splongd;
            return stmt;
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

    ArenaAllocator m_allocator;
};