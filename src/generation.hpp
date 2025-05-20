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
                if (!gen->m_vars.contains(expr_id.id.value.value())) {
                    std::cerr << "Undeclared identifier: " << expr_id.id.value.value() << "\n";
                    exit(EXIT_FAILURE);
                }
                std::stringstream offset;
                const auto& var = gen->m_vars.at(expr_id.id.value.value());
                offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_loc - 1) * 8 << "]\n";
                gen->push(offset.str());
            }

            void operator()(const NodeExprIntLit& expr_int) const {
                gen->m_output << "    mov rax, " << expr_int.int_lit.value.value() << "\n";
                gen->push("rax");
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
                if (gen->m_vars.contains(stmt_splinge.id.value.value())) {
                    std::cerr << "Identifier already used: " << stmt_splinge.id.value.value() << "\n";
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.insert({stmt_splinge.id.value.value(), Var {.stack_loc = gen->m_stack_size}});                
                gen->gen_expr(stmt_splinge.expr);
            }
        };
        StmtVisitor visitor {.gen = this};
        std::visit(visitor, stmt.var);
    }

    [[nodiscard]] std::string gen_prog() {
        m_output << "global _start\n_start:\n";

        for (const NodeStmt& stmt: m_prog.stmts) {
            gen_stmt(stmt);
        }
        // in case there is no explicit exit call, exit without any problems
        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n"; 
        m_output << "    syscall\n";
        return m_output.str();
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

    struct Var {
        size_t stack_loc;
    };

    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_vars {};
};