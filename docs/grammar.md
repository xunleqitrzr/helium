$$
\begin{align}
    [\text{Prog}] &\to [\text{Stmt}]^*
    \\
    [\text{Stmt}] &\to 
    \begin {cases}
        \text{exit}([\text{Expr}]); \\
        \text{var}\space \text{identifier} = [\text{Expr}]; \\
        \text{ident} \ = \ [\text{Expr}]; \\
        \text{if}\ ([\text{Expr}]) \ [\text{Scope}] \ [\text{IfPred}] \\
        [\text{Scope}] \\
    \end {cases}
    \\
    [\text{Scope}] &\to 
    \begin{cases}
        \{[\text{Stmt}]^*\} \\
    \end{cases}
    \\
    [\text{IfPred}] &\to
    \begin{cases}
        \text{elif([Expr])} \ \text{[Scope]} \ [\text{IfPred}] \\
        \text{else} \ [\text{Scope}] \\
        \epsilon \\
    \end{cases}
    \\
    [\text{Expr}] &\to 
    \begin {cases}
        [\text{Term}] \\
        [\text{BinExpr}]
    \end {cases}
    \\
    [\text{BinExpr}] &\to
    \begin {cases}
        [\text{Expr}] * [\text{Expr}] & \text{prec} = 1 \\
        [\text{Expr}] \div [\text{Expr}] & \text{prec} = 1 \\
        [\text{Expr}] + [\text{Expr}] & \text{prec} = 0 \\
        [\text{Expr}] - [\text{Expr}] & \text{prec} = 0 \\
    \end {cases}
    \\
    [\text{Term}] &\to 
    \begin {cases}
        \text{int-lit} \\
        \text{identifier} \\
        ([\text{Expr}])
    \end{cases}
\end{align}
$$