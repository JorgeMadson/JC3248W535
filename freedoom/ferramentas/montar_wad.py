#!/usr/bin/env python3
"""Monta o WAD que vai na flash da placa: Freedoom Fase 1, só o episódio 1.

O freedoom1.wad inteiro tem 28,8 MB e a flash da JC3248W535 tem 16 MB. Este
script baixa o Freedoom oficial, confere o SHA-256 e gera um WAD menor, no
mesmo formato do Doom "shareware" (só E1M1..E1M9), que cabe na partição `wad`.

O que sai:
  - mapas dos episódios 2, 3 e 4 e as músicas deles;
  - texturas de parede que o episódio 1 não usa (e os pedaços delas);
  - demos gravadas em mapas que não existem mais.
O que fica igual: sprites, pisos, sons, menus, fontes, mapas do episódio 1.

Uso (dentro de freedoom/):
    python3 ferramentas/montar_wad.py
    python3 ferramentas/montar_wad.py --origem /caminho/freedoom1.wad

Só usa a biblioteca padrão do Python.
"""
import argparse
import hashlib
import io
import struct
import sys
import urllib.request
import zipfile
from pathlib import Path

VERSAO = "0.13.0"
URL = (
    "https://github.com/freedoom/freedoom/releases/download/"
    f"v{VERSAO}/freedoom-{VERSAO}.zip"
)
SHA256_ZIP = "3f9b264f3e3ce503b4fb7f6bdcb1f419d93c7b546f4df3e874dd878db9688f59"

RAIZ = Path(__file__).resolve().parent.parent
CACHE = RAIZ / ".cache"
SAIDA_PADRAO = RAIZ / "wad" / "freedoom1-e1.wad"

# Tamanho da partição `wad` em particoes.csv. O WAD tem que caber nela.
TAMANHO_PARTICAO = 0xCF0000

EPISODIO = 1
LUMPS_DE_MAPA = (
    "THINGS", "LINEDEFS", "SIDEDEFS", "VERTEXES", "SEGS",
    "SSECTORS", "NODES", "SECTORS", "REJECT", "BLOCKMAP",
)

# Interruptores do episódio 1 (p_switch.c). No modo shareware o motor exige
# que todos existam, mesmo os que nenhum mapa usa.
INTERRUPTORES_EP1 = (
    "BRCOM", "BRN1", "BRN2", "BRNGN", "BROWN", "COMM", "COMP", "DIRT", "EXIT",
    "GRAY", "GRAY1", "METAL", "PIPE", "SLAD", "STARG", "STON1", "STON2",
    "STONE", "STRTN",
)

# Texturas animadas (p_spec.c): (primeira, última). O motor anima o intervalo
# inteiro pela ordem do TEXTURE1, então ou fica tudo ou não fica nada.
ANIMACOES = (
    ("BLODGR1", "BLODGR4"), ("SLADRIP1", "SLADRIP3"), ("BLODRIP1", "BLODRIP4"),
    ("FIREWALA", "FIREWALL"), ("GSTFONT1", "GSTFONT3"), ("FIRELAV3", "FIRELAVA"),
    ("FIREMAG1", "FIREMAG3"), ("FIREBLU1", "FIREBLU2"), ("ROCKRED1", "ROCKRED3"),
    ("BFALL1", "BFALL4"), ("SFALL1", "SFALL4"), ("WFALL1", "WFALL4"),
    ("DBRAIN1", "DBRAIN4"),
)

# Lumps que o motor abre pelo nome no modo shareware. Se faltar um, o jogo
# para com erro, então o script confere antes de gravar.
OBRIGATORIOS = (
    "PLAYPAL", "COLORMAP", "TEXTURE1", "PNAMES", "TITLEPIC", "CREDIT",
    "HELP1", "HELP2", "WIMAP0", "STBAR", "M_DOOM", "DEMO1", "DEMO2", "DEMO3",
    "D_E1M1", "D_INTER", "D_INTRO", "D_VICTOR", "S_START", "S_END",
    "F_START", "F_END", "FREEDOOM",
)


def nome(b):
    return b.split(b"\0")[0].decode("latin1").upper()


def nome8(s):
    return s.encode("latin1").ljust(8, b"\0")


def ler_wad(dados):
    if dados[:4] not in (b"IWAD", b"PWAD"):
        sys.exit("Isso não é um arquivo WAD.")
    n, pos = struct.unpack_from("<ii", dados, 4)
    lumps = []
    for i in range(n):
        p, t, nm = struct.unpack_from("<ii8s", dados, pos + i * 16)
        lumps.append((nome(nm), dados[p:p + t]))
    return lumps


def obter_freedoom1(origem):
    if origem:
        return Path(origem).read_bytes()
    CACHE.mkdir(exist_ok=True)
    zip_local = CACHE / f"freedoom-{VERSAO}.zip"
    if not zip_local.exists():
        print(f"Baixando {URL} ...")
        with urllib.request.urlopen(URL) as r:
            zip_local.write_bytes(r.read())
    h = hashlib.sha256(zip_local.read_bytes()).hexdigest()
    if h != SHA256_ZIP:
        zip_local.unlink()
        sys.exit(f"SHA-256 do zip não confere ({h}). Download apagado.")
    with zipfile.ZipFile(zip_local) as z:
        pasta = f"freedoom-{VERSAO}/"
        # A licença (BSD) pede que o texto acompanhe o WAD.
        (SAIDA_PADRAO.parent).mkdir(exist_ok=True)
        for doc in ("COPYING.txt", "CREDITS.txt"):
            (SAIDA_PADRAO.parent / f"FREEDOOM-{doc}").write_bytes(z.read(pasta + doc))
        return z.read(pasta + "freedoom1.wad")


def ler_texturas(lump):
    """Devolve [(nome, cabecalho_22_bytes, [(x, y, indice_patch)])]."""
    (n,) = struct.unpack_from("<i", lump, 0)
    texturas = []
    for i in range(n):
        (pos,) = struct.unpack_from("<i", lump, 4 + i * 4)
        nm = nome(lump[pos:pos + 8])
        (qtd,) = struct.unpack_from("<h", lump, pos + 20)
        pedacos = []
        for j in range(qtd):
            x, y, patch = struct.unpack_from("<hhh", lump, pos + 22 + j * 10)
            pedacos.append((x, y, patch))
        texturas.append((nm, lump[pos:pos + 20], pedacos))
    return texturas


def escrever_texturas(texturas):
    tabela = io.BytesIO()
    corpo = io.BytesIO()
    base = 4 + 4 * len(texturas)
    for nm, cab, pedacos in texturas:
        tabela.write(struct.pack("<i", base + corpo.tell()))
        corpo.write(cab)
        corpo.write(struct.pack("<h", len(pedacos)))
        for x, y, patch in pedacos:
            corpo.write(struct.pack("<hhhhh", x, y, patch, 1, 0))
    return struct.pack("<i", len(texturas)) + tabela.getvalue() + corpo.getvalue()


def montar(lumps):
    # ---- 1. Separa os mapas ----
    mapas = {}  # "E1M1" -> {"SIDEDEFS": bytes, ...}
    i = 0
    while i < len(lumps):
        nm = lumps[i][0]
        if len(nm) == 4 and nm[0] == "E" and nm[2] == "M" and nm[1].isdigit():
            mapas[nm] = {}
            j = i + 1
            while j < len(lumps) and lumps[j][0] in LUMPS_DE_MAPA:
                mapas[nm][lumps[j][0]] = lumps[j][1]
                j += 1
            i = j
        else:
            i += 1

    # ---- 2. Texturas que o episódio usa ----
    por_nome = {nm: d for nm, d in lumps}
    texturas = ler_texturas(por_nome["TEXTURE1"])
    if "TEXTURE2" in por_nome:
        texturas += ler_texturas(por_nome["TEXTURE2"])
    ordem = [t[0] for t in texturas]
    existe = set(ordem)

    usadas = {ordem[0], "SKY1"}  # a textura 0 é o "sem textura" do motor
    for nm, partes in mapas.items():
        if int(nm[1]) != EPISODIO:
            continue
        lados = partes["SIDEDEFS"]
        for k in range(0, len(lados), 30):
            for desloc in (4, 12, 20):
                t = nome(lados[k + desloc:k + desloc + 8])
                if t and t != "-":
                    usadas.add(t)
    for s in INTERRUPTORES_EP1:
        usadas.update(("SW1" + s, "SW2" + s))
    # o par de qualquer interruptor usado também fica
    for t in list(usadas):
        if t.startswith("SW1"):
            usadas.add("SW2" + t[3:])
        elif t.startswith("SW2"):
            usadas.add("SW1" + t[3:])
    for primeira, ultima in ANIMACOES:
        if primeira in existe and ultima in existe:
            a, b = ordem.index(primeira), ordem.index(ultima)
            faixa = ordem[a:b + 1]
            if usadas.intersection(faixa):
                usadas.update(faixa)

    faltam = sorted(t for t in usadas if t not in existe)
    if faltam:
        print("Aviso: texturas pedidas que não existem no WAD:", ", ".join(faltam))

    # ---- 3. Refaz TEXTURE1 e PNAMES só com o que ficou ----
    pnames = por_nome["PNAMES"]
    (n_patches,) = struct.unpack_from("<i", pnames, 0)
    patches = [nome(pnames[4 + k * 8:12 + k * 8]) for k in range(n_patches)]

    novas, novo_indice, novos_patches = [], {}, []
    for nm, cab, pedacos in texturas:
        if nm not in usadas:
            continue
        convertidos = []
        for x, y, p in pedacos:
            pn = patches[p]
            if pn not in novo_indice:
                novo_indice[pn] = len(novos_patches)
                novos_patches.append(pn)
            convertidos.append((x, y, novo_indice[pn]))
        novas.append((nm, cab, convertidos))
    novo_texture1 = escrever_texturas(novas)
    novo_pnames = struct.pack("<i", len(novos_patches)) + b"".join(
        nome8(p) for p in novos_patches
    )
    patches_usados = set(novos_patches)

    # ---- 4. Passa pelos lumps na ordem original e decide o que fica ----
    saida = []
    dentro_de_p = False
    demo_valida = None
    i = 0
    while i < len(lumps):
        nm, dados = lumps[i]
        if nm in mapas:
            fica = int(nm[1]) == EPISODIO
            j = i + 1
            while j < len(lumps) and lumps[j][0] in LUMPS_DE_MAPA:
                j += 1
            if fica:
                saida.extend(lumps[i:j])
            i = j
            continue
        i += 1

        if nm == "P_START":
            dentro_de_p = True
        elif nm == "P_END":
            dentro_de_p = False
        elif dentro_de_p and not (nm.endswith("_START") or nm.endswith("_END")):
            if nm not in patches_usados:
                continue

        if nm == "TEXTURE1":
            dados = novo_texture1
        elif nm == "TEXTURE2":
            continue
        elif nm == "PNAMES":
            dados = novo_pnames
        elif nm.startswith("D_E") and nm[3:4].isdigit() and nm[4:5] == "M":
            if int(nm[3]) != EPISODIO:
                continue
        elif nm.startswith("DEMO") and len(dados) > 4:
            # byte 2 = episódio em que a demo foi gravada
            if dados[2] != EPISODIO:
                continue
            demo_valida = dados
        saida.append((nm, dados))

    # O modo shareware toca DEMO1, DEMO2 e DEMO3 em sequência. As que saíram
    # viram cópias da que ficou (mesmos bytes no arquivo, não gasta espaço).
    if demo_valida is not None:
        tem = {nm for nm, _ in saida}
        for d in ("DEMO1", "DEMO2", "DEMO3"):
            if d not in tem:
                saida.append((d, demo_valida))

    tem = {nm for nm, _ in saida}
    faltam = [x for x in OBRIGATORIOS if x not in tem]
    faltam += [f"E{EPISODIO}M{m}" for m in range(1, 10) if f"E{EPISODIO}M{m}" not in tem]
    if faltam:
        sys.exit("Faltam lumps obrigatórios no resultado: " + ", ".join(faltam))
    if any(nm[:1] == "E" and nm[2:3] == "M" and nm[1:2] in "234" and len(nm) == 4 for nm in tem):
        sys.exit("Sobrou mapa de outro episódio (o motor sairia do modo shareware).")

    resumo = {
        "texturas": (len(novas), len(texturas)),
        "patches": (len(novos_patches), n_patches),
    }
    return saida, resumo


def escrever_wad(lumps, destino):
    corpo = io.BytesIO()
    corpo.write(b"\0" * 12)
    diretorio = []
    ja_gravado = {}  # mesmos bytes -> mesma posição (demos repetidas)
    for nm, dados in lumps:
        chave = hashlib.sha1(dados).digest() if len(dados) > 64 else None
        if chave in ja_gravado:
            pos = ja_gravado[chave]
        else:
            # Alinha em 4 bytes: o motor lê os lumps direto da flash mapeada.
            corpo.write(b"\0" * (-corpo.tell() % 4))
            pos = corpo.tell()
            corpo.write(dados)
            if chave:
                ja_gravado[chave] = pos
        diretorio.append(struct.pack("<ii8s", pos if dados else 0, len(dados), nome8(nm)))
    corpo.write(b"\0" * (-corpo.tell() % 4))
    pos_dir = corpo.tell()
    corpo.write(b"".join(diretorio))
    corpo.seek(0)
    corpo.write(b"IWAD" + struct.pack("<ii", len(lumps), pos_dir))
    destino.parent.mkdir(exist_ok=True)
    destino.write_bytes(corpo.getvalue())
    return len(corpo.getvalue())


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--origem", help="freedoom1.wad já baixado (pula o download)")
    ap.add_argument("--saida", default=str(SAIDA_PADRAO))
    args = ap.parse_args()

    original = obter_freedoom1(args.origem)
    lumps = ler_wad(original)
    saida, resumo = montar(lumps)
    tamanho = escrever_wad(saida, Path(args.saida))

    mb = 1024 * 1024
    print(f"Original : {len(original) / mb:6.2f} MB, {len(lumps)} lumps")
    print(f"Resultado: {tamanho / mb:6.2f} MB, {len(saida)} lumps -> {args.saida}")
    print("Texturas : {} de {}".format(*resumo["texturas"]))
    print("Patches  : {} de {}".format(*resumo["patches"]))
    print(f"Partição : {TAMANHO_PARTICAO / mb:6.2f} MB "
          f"(sobram {(TAMANHO_PARTICAO - tamanho) / mb:.2f} MB)")
    if tamanho > TAMANHO_PARTICAO:
        sys.exit("ERRO: o WAD não cabe na partição `wad`.")


if __name__ == "__main__":
    main()
