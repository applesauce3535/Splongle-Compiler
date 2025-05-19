#pragma once

class Generator {

public:
    inline Generator(NodeExit root) // exit is a temporary root
    : m_root(std::move(root))
    {}

    [[nodiscard]] std::string generate() const {
        std::stringstream output; // return value
        output << "global _start\n_start:\n";
        output << "    mov rax, 60\n";
        output << "    mov rdi, " << m_root.expr.int_lit.value.value() << "\n"; // take the exit node's expression's int lit value
        output << "    syscall";
        return output.str();
    }

private:
    const NodeExit m_root;
};