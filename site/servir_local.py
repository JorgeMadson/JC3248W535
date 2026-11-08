#!/usr/bin/env python3
"""Testa o gravador no seu próprio computador, sem depender do GitHub Pages.

Monta uma pasta _site_local/ com o site + os binários e sobe um servidor em
http://localhost:8000 . localhost conta como "contexto seguro", então o
Web Serial funciona e dá para gravar a placa de verdade pelo navegador.

Uso (dentro da pasta do repositório):

    python site/servir_local.py

Opcional, se você já compilou o placar-mesa localmente e quer testá-lo também:

    python site/servir_local.py --placar-build "C:/Users/filho/.rb/pm-build/jc3248w535"

(essa é a pasta com bootloader.bin, partitions.bin e firmware.bin geradas pelo
`python -m platformio run`.)
"""
import argparse
import glob
import hashlib
import http.server
import os
import shutil
import socketserver
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)                     # pasta do repositório
SITE = AQUI
SAIDA = os.path.join(RAIZ, "_site_local")
FABRICA = os.path.join(RAIZ, "backup", "jc3248w535_fabrica_16MB.bin")


def achar_boot_app0():
    padroes = [
        os.path.expanduser("~/.platformio/packages/**/boot_app0.bin"),
        "C:/Users/*/.platformio/packages/**/boot_app0.bin",
    ]
    for p in padroes:
        achados = glob.glob(p, recursive=True)
        if achados:
            return achados[0]
    return None


def gerar_sha256(pasta):
    """Grava SHA256SUMS.txt (formato do sha256sum) com os .bin da pasta."""
    bins = sorted(n for n in os.listdir(pasta) if n.endswith(".bin"))
    if not bins:
        return
    with open(os.path.join(pasta, "SHA256SUMS.txt"), "w") as saida:
        for nome in bins:
            h = hashlib.sha256()
            with open(os.path.join(pasta, nome), "rb") as f:
                for bloco in iter(lambda: f.read(1 << 20), b""):
                    h.update(bloco)
            saida.write("%s  %s\n" % (h.hexdigest(), nome))


def montar(placar_build):
    if os.path.exists(SAIDA):
        shutil.rmtree(SAIDA)
    shutil.copytree(SITE, SAIDA)
    # não precisamos do próprio script no site servido
    s = os.path.join(SAIDA, "servir_local.py")
    if os.path.exists(s):
        os.remove(s)

    # 1) firmware de fábrica
    destino_fab = os.path.join(SAIDA, "firmwares", "fabrica",
                               "jc3248w535_fabrica_16MB.bin")
    if os.path.exists(FABRICA):
        shutil.copy2(FABRICA, destino_fab)
        print("  [ok] firmware de fábrica copiado")
    else:
        print("  [!!] backup de fábrica não encontrado em", FABRICA)

    # 2) firmware do placar-mesa (opcional, se você compilou localmente)
    fw = os.path.join(SAIDA, "firmwares", "placar-mesa")
    if placar_build:
        partes = {
            "bootloader.bin": "bootloader.bin",
            "partitions.bin": "partitions.bin",
            "firmware.bin": "firmware.bin",
        }
        falta = False
        for origem, nome in partes.items():
            caminho = os.path.join(placar_build, origem)
            if os.path.exists(caminho):
                shutil.copy2(caminho, os.path.join(fw, nome))
            else:
                print("  [!!] não achei", caminho)
                falta = True
        boot_app0 = achar_boot_app0()
        if boot_app0:
            shutil.copy2(boot_app0, os.path.join(fw, "boot_app0.bin"))
        else:
            print("  [!!] boot_app0.bin não encontrado (.platformio)")
            falta = True
        if not falta:
            print("  [ok] firmware do placar-mesa copiado")
    else:
        print("  [--] placar-mesa não incluído (use --placar-build para testá-lo)")
        print("       o card do placar vai aparecer, mas sem binário para gravar.")

    # 3) SHA-256 de cada binário (o site mostra para conferência)
    for nome in ("fabrica", "placar-mesa"):
        gerar_sha256(os.path.join(SAIDA, "firmwares", nome))


def servir(porta):
    os.chdir(SAIDA)
    handler = http.server.SimpleHTTPRequestHandler
    with socketserver.TCPServer(("127.0.0.1", porta), handler) as httpd:
        print("\n  Servindo em  http://localhost:%d/\n" % porta)
        print("  Abra esse endereço no Chrome/Edge, conecte a placa e teste.")
        print("  (Ctrl+C para parar.)\n")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n  Parado.")


def main():
    ap = argparse.ArgumentParser(description="Servidor local de teste do gravador.")
    ap.add_argument("--placar-build", default="",
                    help="pasta com bootloader.bin/partitions.bin/firmware.bin do placar-mesa")
    ap.add_argument("--porta", type=int, default=8000)
    args = ap.parse_args()

    print("Montando _site_local/ ...")
    montar(args.placar_build)
    servir(args.porta)


if __name__ == "__main__":
    main()
