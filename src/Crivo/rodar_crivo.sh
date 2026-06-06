#!/bin/bash
# =============================================================================
# rodar_crivo.sh
# Compila e executa TesteIntervaloCrivoImpar e TesteIntervaloCrivoImparBITSET
# para múltiplos valores de N e múltiplas rodadas, salvando os resultados em
# resultados.txt (formato TSV pronto para preencher_tabela.py).
# =============================================================================

set -euo pipefail

# ---------- configurações ----------------------------------------------------
SRC_BOOL="TesteIntervaloCrivoImpar.c"
SRC_BITSET="TesteIntervaloCrivoImparBITSET.c"
BIN_BOOL="./TesteIntervaloCrivoImpar"
BIN_BITSET="./TesteIntervaloCrivoImparBITSET"

VALORES_N=(100000000 1000000000 10000000000 100000000000)
RODADAS=10
OMP_THREADS=8          # threads para o BITSET paralelo
SAIDA="resultados.txt" # arquivo lido pelo preencher_tabela.py
# -----------------------------------------------------------------------------

# Detecta comando de tempo disponível
if command -v gtime &>/dev/null; then
    TIME_CMD="gtime"   # macOS com GNU coreutils
else
    TIME_CMD="/usr/bin/time"
fi

echo "=== Compilando ==="
gcc -O3 "$SRC_BOOL"   -o "$BIN_BOOL"   -lm
echo "  [OK] $BIN_BOOL"
gcc -O3 -fopenmp "$SRC_BITSET" -o "$BIN_BITSET" -lm
echo "  [OK] $BIN_BITSET"

# Cabeçalho do arquivo de saída
echo "# programa N rodada tempo_s primos marcacoes mem_kb" > "$SAIDA"

run_programa() {
    local prog="$1"
    local N="$2"
    local rodada="$3"

    # Mede tempo de parede com 'time' builtin; captura stdout do programa
    local tmpout
    tmpout=$(mktemp)

    local t_start t_end tempo
    t_start=$(date +%s%N)
    "$prog" "$N" > "$tmpout"
    t_end=$(date +%s%N)
    tempo=$(echo "scale=6; ($t_end - $t_start) / 1000000000" | bc)

    local primos marcacoes mem_kb
    primos=$(grep  "^PRIMOS="      "$tmpout" | cut -d= -f2)
    marcacoes=$(grep "^MARCACOES=" "$tmpout" | cut -d= -f2)
    mem_kb=$(grep  "^MEM_KB="     "$tmpout" | cut -d= -f2)
    rm -f "$tmpout"

    local nome
    nome=$(basename "$prog")
    echo "$nome $N $rodada $tempo $primos $marcacoes $mem_kb" | tee -a "$SAIDA"
}

echo ""
echo "=== Executando testes ==="
echo ""

for N in "${VALORES_N[@]}"; do
    echo "--- N = $N ---"

    for rodada in $(seq 1 "$RODADAS"); do
        echo -n "  BOOL   rodada $rodada ... "
        run_programa "$BIN_BOOL" "$N" "$rodada"
    done

    for rodada in $(seq 1 "$RODADAS"); do
        echo -n "  BITSET rodada $rodada ... "
        OMP_NUM_THREADS="$OMP_THREADS" run_programa "$BIN_BITSET" "$N" "$rodada"
    done

    echo ""
done

echo "=== Resultados salvos em: $SAIDA ==="
