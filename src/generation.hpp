#pragma once

#include <unordered_map>
#include <cassert>
#include <variant>
class Generator {

public:
    inline Generator(NodeProg prog)
    : m_prog(std::move(prog))
    {}

    void gen_term(const NodeTerm* term) {
        struct TermVisitor {
            Generator* gen;
            void operator()(const NodeTermId* term_id) {
                if (!gen->m_symbol_table.contains(term_id->id.value.value())) {
                    std::cerr << "Undeclared identifier: " << term_id->id.value.value() << "\n";
                    exit(EXIT_FAILURE);
                }
                std::stringstream offset;
                const auto& var = gen->m_symbol_table.at(term_id->id.value.value());
                offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_loc - 1) * 8 << "]";
                gen->push(offset.str());
            }
            void operator()(const NodeTermIntLit* term_int) const {
                gen->m_output << "    mov rax, " << term_int->int_lit.value.value() << "\n";
                gen->push("rax");
            }
            void operator()(const NodeTermDPLit* term_double) const {
                std::string label = "L" + std::to_string(gen->m_label_counter++); // make a label of L + the current label counter and then post increment it
                gen->m_data << label << ": dq " << term_double->dp_lit.value.value() << "\n";

                gen->m_output << "    movsd xmm0, [rel " << label << "]\n";
                gen->m_output << "    sub rsp, 8\n"; // this makes space on the stack for a double
                gen->m_output << "    movsd [rsp], xmm0\n"; // this moves the double into wherever the stack pointer is
                gen->m_stack_size++; // explicitly state to increase stack size because we didn't call push()
            }
        };
        TermVisitor visitor({.gen = this});
        std::visit(visitor, term->var);
    }

    void gen_expr(const NodeExpr* expr) {
        struct ExprVisitor {
            Generator* gen;

            void operator()(const NodeTerm* term) const {
                gen->gen_term(term);
            }
            void operator()(const NodeBinExpr* bin_expr) const {
                if (std::holds_alternative<NodeBinExprMul*>(bin_expr->var)) { // this is asking are we doing a multiplication
                    auto* mul_expr = std::get<NodeBinExprMul*>(bin_expr->var); // get the whole expression
                    gen->gen_expr(mul_expr->left); // generate the left operand
                    gen->gen_expr(mul_expr->right); // generate the right operand
                    gen->pop("rcx"); // put the right operand into rcx
                    gen->pop("rax"); // put the left operand into rax
                    gen->m_output << "    imul rax, rcx\n"; // this means rax = rax * rcx
                    gen->push("rax"); // push the new result
                }
                else if (std::holds_alternative<NodeBinExprDiv*>(bin_expr->var)) { // this is asking are we doing a division
                    auto* div_expr = std::get<NodeBinExprDiv*>(bin_expr->var); // get the whole expression
                    gen->gen_expr(div_expr->left); // gen numerator

                    if (auto* right_expr = div_expr->right; // division by 0 check
                        std::holds_alternative<NodeTerm*>(right_expr->var)) {
                        auto* term = std::get<NodeTerm*>(right_expr->var);
                        if (std::holds_alternative<NodeTermIntLit*>(term->var)) {
                            auto* lit = std::get<NodeTermIntLit*>(term->var);
                            if (lit->int_lit.value == "0") {
                                std::cerr << "Division by 0 exception, DOW\n";
                                exit(EXIT_FAILURE);
                            }
                        }
                    }

                    gen->gen_expr(div_expr->right); // gen denominator
                    gen->pop("rcx"); // rcx = denominator
                    gen->pop("rax"); // rax = numerator
                    gen->m_output << "    cqo\n"; // this sign extends rax into rdx, result is a 128-bit integer rdx:rax
                    gen->m_output << "    idiv rcx\n"; // rax = rax / rcx, rdx = rax % rcx
                    gen->push("rax"); // push the new result
                }
                else if (std::holds_alternative<NodeBinExprAdd*>(bin_expr->var)) { // this is asking are we doing an addition
                    auto* add_expr = std::get<NodeBinExprAdd*>(bin_expr->var); // get the whole expression
                    gen->gen_expr(add_expr->left); // generate the left operand
                    gen->gen_expr(add_expr->right); // generate the right operand
                    gen->pop("rcx"); // put the right operand into rcx
                    gen->pop("rax"); // put the left operand into rax
                    gen->m_output << "    add rax, rcx\n"; // this means rax = rax + rcx
                    gen->push("rax"); // push the new result
                }
                else if (std::holds_alternative<NodeBinExprSub*>(bin_expr->var)) { // this is asking are we doing a subtraction
                    auto* sub_expr = std::get<NodeBinExprSub*>(bin_expr->var); // get the whole expression
                    gen->gen_expr(sub_expr->left); // generate the left operand
                    gen->gen_expr(sub_expr->right); // generate the right operand
                    gen->pop("rcx"); // put the right operand into rcx
                    gen->pop("rax"); // put the left operand into rax
                    gen->m_output << "    sub rax, rcx\n"; // this means rax = rax - rcx
                    gen->push("rax"); // push the new result
                }
                else {
                    exit(EXIT_FAILURE);
                }
            }
        };

        ExprVisitor visitor {.gen = this};
        std::visit(visitor, expr->var);
    }

    void gen_stmt(const NodeStmt* stmt) {
        struct StmtVisitor {
            Generator* gen;
            void operator()(const NodeStmtExit* stmt_exit) const{
                gen->gen_expr(stmt_exit->expr);
                gen->m_output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }

            void operator()(const NodeStmtSplinge* stmt_splinge) {
                if (gen->m_symbol_table.contains(stmt_splinge->id.value.value())) {
                    std::cerr << "Identifier already used: " << stmt_splinge->id.value.value() << "\n";
                    exit(EXIT_FAILURE);
                }
                gen->m_symbol_table.insert({stmt_splinge->id.value.value(), Var {.stack_loc = gen->m_stack_size, .type = VarType::Int}});                
                gen->gen_expr(stmt_splinge->expr);
            }

            void operator()(const NodeStmtSplongd* stmt_splongd) {
                if (gen->m_symbol_table.contains(stmt_splongd->id.value.value())) {
                    std::cerr << "Identifier already used: " << stmt_splongd->id.value.value() << "\n";
                    exit(EXIT_FAILURE);
                }
                gen->m_symbol_table.insert({stmt_splongd->id.value.value(), Var {.stack_loc = gen->m_stack_size, .type = VarType::Double}});                
                gen->gen_expr(stmt_splongd->expr);
            }
        };
        StmtVisitor visitor {.gen = this};
        std::visit(visitor, stmt->var);
    }

    [[nodiscard]] std::string gen_prog() {
        for (const NodeStmt* stmt: m_prog.stmts) {
            gen_stmt(stmt);
        }
        // in case there is no explicit exit call, exit without any problems
        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n"; 
        m_output << "    syscall\n";

        // the following prepends the data section for doubles to what was generated above
        std::string output = "section .data\n" + m_data.str();
        output += "\nsection .text\n";
        output += "global _start\n_start:\n";
        output += m_output.str();
        return output;
    }

private:
    void push(const std::string& reg) {
        m_output << "    push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const std::string& reg) {
        m_output << "    pop " << reg << "\n";
        m_stack_size--;
    }

    enum class VarType {Int, Double};

    struct Var {
        size_t stack_loc;
        VarType type;
    };

    const NodeProg m_prog;
    std::stringstream m_output;
    std::stringstream m_data; // this is for double constants
    size_t m_label_counter = 0; // this is for double constants
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_symbol_table {};
};