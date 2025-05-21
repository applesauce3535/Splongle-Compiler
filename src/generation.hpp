#pragma once

#include <unordered_map>

class Generator {

public:
    inline Generator(NodeProg prog)
    : m_prog(std::move(prog))
    {}


    void gen_expr(const NodeExpr& expr) {
        struct ExprVisitor {
            Generator* gen;

            void operator()(const NodeExprId& expr_id) {
                if (!gen->m_symbol_table.contains(expr_id.id.value.value())) {
                    std::cerr << "Undeclared identifier: " << expr_id.id.value.value() << "\n";
                    exit(EXIT_FAILURE);
                }
                std::stringstream offset;
                const auto& var = gen->m_symbol_table.at(expr_id.id.value.value());
                offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_loc - 1) * 8 << "]\n";
                gen->push(offset.str());
            }

            void operator()(const NodeExprIntLit& expr_int) const {
                gen->m_output << "    mov rax, " << expr_int.int_lit.value.value() << "\n";
                gen->push("rax");
            }
            void operator()(const NodeExprDPLit& expr_double) const {
                std::string label = "L" + std::to_string(gen->m_label_counter++); // make a label of L + the current label counter and then post increment it
                gen->m_data << label << ": dq " << expr_double.dp_lit.value.value() << "\n";

                gen->m_output << "    movsd xmm0, [rel " << label << "]\n";
                gen->m_output << "    sub rsp, 8\n"; // this makes space on the stack for a double
                gen->m_output << "    movsd [rsp], xmm0\n"; // this moves the double into wherever the stack pointer is
                gen->m_stack_size++; // explicitly state to increase stack size because we didn't call push()
            }
        };

        ExprVisitor visitor {.gen = this};
        std::visit(visitor, expr.var);
    }

    void gen_stmt(const NodeStmt& stmt) {
        struct StmtVisitor {
            Generator* gen;
            void operator()(const NodeStmtExit& stmt_exit) const{
                gen->gen_expr(stmt_exit.expr);
                gen->m_output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }

            void operator()(const NodeStmtSplinge& stmt_splinge) {
                if (gen->m_symbol_table.contains(stmt_splinge.id.value.value())) {
                    std::cerr << "Identifier already used: " << stmt_splinge.id.value.value() << "\n";
                    exit(EXIT_FAILURE);
                }
                gen->m_symbol_table.insert({stmt_splinge.id.value.value(), Var {.stack_loc = gen->m_stack_size, .type = VarType::Int}});                
                gen->gen_expr(stmt_splinge.expr);
            }

            void operator()(const NodeStmtSplongd& stmt_splongd) {
                if (gen->m_symbol_table.contains(stmt_splongd.id.value.value())) {
                    std::cerr << "Identifier already used: " << stmt_splongd.id.value.value() << "\n";
                    exit(EXIT_FAILURE);
                }
                gen->m_symbol_table.insert({stmt_splongd.id.value.value(), Var {.stack_loc = gen->m_stack_size, .type = VarType::Double}});                
                gen->gen_expr(stmt_splongd.expr);
            }
        };
        StmtVisitor visitor {.gen = this};
        std::visit(visitor, stmt.var);
    }

    [[nodiscard]] std::string gen_prog() {
        for (const NodeStmt& stmt: m_prog.stmts) {
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