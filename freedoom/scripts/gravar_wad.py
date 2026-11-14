# Alvo extra do PlatformIO: `pio run -t gravar_wad`
# Grava wad/freedoom1-e1.wad na partição `wad` (ver particoes.csv).
import os

Import("env")  # noqa: F821

ENDERECO_WAD = "0x310000"
WAD = os.path.join(env.subst("$PROJECT_DIR"), "wad", "freedoom1-e1.wad")  # noqa: F821


def conferir(source, target, env):
    if not os.path.exists(WAD):
        raise SystemExit(
            "Falta o wad/freedoom1-e1.wad. Rode antes: python3 ferramentas/montar_wad.py"
        )


env.AddCustomTarget(  # noqa: F821
    name="gravar_wad",
    dependencies=None,
    actions=[
        conferir,
        '"$PYTHONEXE" "$UPLOADER" --chip esp32s3 --port "$UPLOAD_PORT" '
        "--baud $UPLOAD_SPEED write_flash %s \"%s\"" % (ENDERECO_WAD, WAD),
    ],
    title="Gravar WAD",
    description="Grava o episódio 1 do Freedoom na partição wad",
)
