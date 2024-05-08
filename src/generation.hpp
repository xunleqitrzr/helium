#pragma once

#include <string>
#include <cassert>

#include "./parser.hpp"

class Generator {
public:
    inline explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {

    }

    void gen_term(const NodeTerm* term) {
        struct TermVisitor {
            Generator* gen;

            void operator()(const NodeTermIntLit* term_int_lit) const {
                gen->m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << "\n";
                gen->push("rax");
            }

            void operator()(const NodeTermIdent* term_ident) const {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var& var) {
                    return var.name == term_ident->ident.value.value();
                });

                if (it == gen->m_vars.cend()) {
                    std::cerr << "Undeclared identifier: " << term_ident->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                std::stringstream offset;
                offset << "QWORD [rsp + " << ((gen->m_stack_size - (*it).stack_loc - 1) * 8) << "]";
                gen->push(offset.str());
            }

            void operator()(const NodeTermParen* term_paren) const {
                gen->gen_expr(term_paren->expr);
            }

        };

        TermVisitor visitor({.gen = this});
        std::visit(visitor, term->var);
    }

    void gen_bin_expr(const NodeBinExpr* bin_expr) {
        struct BinExprVisitor {
            Generator* gen;

            void operator()(const NodeBinExprAdd* add) const {
                gen->gen_expr(add->rhs);
                gen->gen_expr(add->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    add rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const NodeBinExprSub* sub) const {
                gen->gen_expr(sub->rhs);
                gen->gen_expr(sub->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    sub rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const NodeBinExprMulti* multi) const {
                gen->gen_expr(multi->rhs);
                gen->gen_expr(multi->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    mul rbx\n";
                gen->push("rax");
            }

            void operator()(const NodeBinExprDiv* div) const {
                gen->gen_expr(div->rhs);
                gen->gen_expr(div->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    div rbx\n";
                gen->push("rax");
            }
        };

        BinExprVisitor visitor{.gen = this};
        std::visit(visitor, bin_expr->var);

    }

    void gen_expr(const NodeExpr* expr) {
        struct ExprVisitor {
            Generator* gen;

            void operator()(const NodeTerm* term) const {
                gen->gen_term(term);
            }

            void operator()(const NodeBinExpr* bin_expr) const {
                gen->gen_bin_expr(bin_expr);
            }
        };

        ExprVisitor visitor{.gen = this};
        std::visit(visitor, expr->var);
    }

    void gen_stmt(const NodeStmt* stmt) {
        struct StmtVisitor {
            Generator* gen;

            void operator()(const NodeStmtExit* stmt_exit) const {

                gen->gen_expr(stmt_exit->expr);
                gen->m_output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }

            void operator()(const NodeStmtVar* stmt_var) const {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var& var) {
                    return var.name == stmt_var->ident.value.value();
                });
                if (it != gen->m_vars.cend()) {
                    std::cerr << "Identifier already declared: " << stmt_var->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }

                gen->m_vars.push_back({.name = stmt_var->ident.value.value(), .stack_loc  = gen->m_stack_size});
                gen->gen_expr(stmt_var->expr);
            }

            void operator()(const NodeStmtScope* scope) const {
                gen->begin_scope();
                for (const NodeStmt* stmt: scope->stmts) {
                    gen->gen_stmt(stmt);
                }
                gen->end_scope();
            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt->var);
    }

    [[nodiscard]] std::string gen_prog() {
        m_output << "global _start\n_start:\n";

        for (const NodeStmt* stmt: m_prog.stmts) {
            gen_stmt(stmt);
        }

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

    void begin_scope() {
        m_scopes.push_back(m_vars.size());
    }

    void end_scope() {
        size_t pop_count = m_vars.size() - m_scopes.back();
        m_output << "    add rsp, " << pop_count * 8 << "\n";
        m_stack_size -= pop_count;

        for (int i = 0; i < pop_count; i++) {
            m_vars.pop_back();
        }
        m_scopes.pop_back();
    }

    struct Var {
        std::string name;
        size_t stack_loc;
    };

    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::vector<Var> m_vars{};
    std::vector<size_t> m_scopes{};
};
