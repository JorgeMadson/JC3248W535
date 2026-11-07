# Testes nativos no Windows: linka libstdc++/libgcc estáticas para o .exe não
# pegar uma DLL incompatível que esteja no PATH (Git, Neovim...).
Import("env")  # noqa: F821

env.Append(LINKFLAGS=["-static"])  # noqa: F821
