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

// don't be scared by any of these pointers. they are placed into the memory arena when parsing happens
// and then the whole arena is freed after code generation
struct NodeBinExprMul {
    // a binary multiplication node consists of a left and right operand
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
    // a whole binary expression can be addition or multiplication
    // TODO: add other arithmetic operations
    std::variant<NodeBinExprAdd*, NodeBinExprSub*, NodeBinExprMul*> var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermDPLit*, NodeTermId*> var;
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

struct NodeSplong {

};


class Parser {

public:
    inline explicit Parser(std::vector<Token> tokens) 
    : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 4) // arena is 4MB
    {}

    std::optional<NodeTerm*> parse_term() {
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

    std::optional<NodeExpr*> parse_expr() {
        auto left_opt = parse_term();
        if (!left_opt.has_value()) { // if no value was created, the expression is incorrect
            return std::nullopt;
        }

        NodeExpr* left_operand = m_allocator.alloc<NodeExpr>();
        left_operand->var = left_opt.value(); // left operand variant is the value from the left optional (constant or variable)

        while (peek().has_value()) {
            TokenType op = peek().value().type;
            if (op == TokenType::add || op == TokenType::sub) {
                consume(); // consume operand
                auto right_opt = parse_term(); // this only parses the next term, not the whole expression
                if (!right_opt.has_value()) {
                    std::cerr << "Expected expression after operator, DOW\n";
                    exit(EXIT_FAILURE);
                }
                auto right_term = right_opt.value();
                NodeExpr* right_operand = m_allocator.alloc<NodeExpr>();
                right_operand->var = right_term;
                auto bin_expr = m_allocator.alloc<NodeBinExpr>(); // allocate room for a bin expression node
                if (op == TokenType::add) { // handle addition
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
            else {
                break; // there are no operators left in the expression
            }
        }
        return left_operand; // final result of any number of binary expressions

    //     if (auto term = parse_term()) {
    //         if (peek().has_value() && peek().value().type == TokenType::add) { // handle addition
    //             auto bin_expr = m_allocator.alloc<NodeBinExpr>(); // allocate appropriate space for bin expression
    //             auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>(); // allocate space for addition
    //             auto left_expr = m_allocator.alloc<NodeExpr>(); // get the left side expression
    //             left_expr->var = term.value();
    //             bin_expr_add->left = left_expr; // put the left expression into the bin expression
    //             consume(); // consume '+'
    //             if (auto right = parse_expr()) {
    //                 bin_expr_add->right = right.value(); // get the right expression
    //                 bin_expr->var = bin_expr_add; 
    //                 auto expr = m_allocator.alloc<NodeExpr>(); // now we know the total size of the expression
    //                 expr->var = bin_expr;
    //                 return expr;
    //             }
    //             else {
    //                 std::cerr << "Expected epxression after '+', DOW\n";
    //                 exit(EXIT_FAILURE);
    //             }
    //         }
    //         else if (peek().has_value() && peek().value().type == TokenType::sub) { // handle subtraction
    //             auto bin_expr = m_allocator.alloc<NodeBinExpr>(); // allocate appropriate space for bin expression
    //             auto bin_expr_sub = m_allocator.alloc<NodeBinExprSub>(); // allocate space for subtraction
    //             auto left_expr = m_allocator.alloc<NodeExpr>(); // get the left side expression
    //             left_expr->var = term.value();
    //             bin_expr_sub->left = left_expr; // put the left expression into the bin expression
    //             consume(); // consume '-'
    //             if (auto right = parse_expr()) {
    //                 bin_expr_sub->right = right.value(); // get the right expression
    //                 bin_expr->var = bin_expr_sub; 
    //                 auto expr = m_allocator.alloc<NodeExpr>(); // now we know the total size of the expression
    //                 expr->var = bin_expr;
    //                 return expr;
    //             }
    //             else {
    //                 std::cerr << "Expected epxression after '-', DOW\n";
    //                 exit(EXIT_FAILURE);
    //             }
    //         }
    //         else {
    //             auto expr = m_allocator.alloc<NodeExpr>();
    //             expr->var = term.value();
    //             return expr;
    //         }    
    //     }
    //     else {
    //         return {};
    //     }
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
                std::cerr << "Invalid expression, MAJOR FUCK UP\n";
                exit(EXIT_FAILURE);
            }
            if (!peek().has_value() || peek().value().type != TokenType::close_paren) { // ensure there is a matching closing parenthesis
                std::cerr << "Expected closing parenthesis for 'exit', MAJOR FUCK UP\n";
                exit(EXIT_FAILURE);
            }
            consume(); // consume ')'

            if (!peek().has_value() || peek().value().type != TokenType::splong) { // this is ensuring 'splong' follows the exit statement  
                std::cerr << "Expected 'splong', MAJOR FUCK UP\n";
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
                std::cerr << "Invalid expression, MAJOR FUCK UP\n";
                exit(EXIT_FAILURE);
            }
            if (!peek().has_value() || peek().value().type != TokenType::splong) { // ensure 'splong' follows identifier declaration
                std::cerr << "Expected 'splong', DOW\n";
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