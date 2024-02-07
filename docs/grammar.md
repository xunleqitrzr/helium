$$
\begin{align}
    [\text{prog}] &\to [\text{stmt}]^*
    \\
    [\text{stmt}] &\to 
    \begin {cases}
        \text{exit}([\text{expr}]); \\
        \text{var}\space \text{identifier} = [\text{expr}];
    \end {cases}
    \\
    [\text{expr}] &\to 
    \begin {cases}
        \text{int\_lit} \\
        \text{identifier}
    \end {cases}
\end{align}
$$