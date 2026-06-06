#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT_DIR"

REPS=10
SEED=${1:-42}
JSON_FILE="cubo_results.json"
THREADS=(4 8 16 32)
MAX_EMBARALHA=20

echo "Compilando executáveis..."
gcc -O3 -fopenmp -o "$ROOT_DIR/TesteCuboFitness" "$ROOT_DIR/src/Cubo/TesteCuboFitness.c" -lm
gcc -O3 -o "$ROOT_DIR/TesteCuboSequencial" "$ROOT_DIR/src/Cubo/TesteCuboSequencial.c" -lm
gcc -O3 -fopenmp -o "$ROOT_DIR/TesteCuboIlha" "$ROOT_DIR/src/Cubo/TesteCuboIlha.c" -lm

parse_csv() {
    local line="$1"
    IFS=',' read -r _ mode threads emb f solved gens <<< "$line"
    echo "$mode|$threads|$emb|$f|$solved|$gens"
}

results=()
append_result() {
    results+=("$1")
}

run_test() {
    local exe="$1"
    shift
    local output
    output="$($exe "$@" | tail -n 1)"
    parse_csv "$output"
}

for n_embaralha in $(seq 1 $MAX_EMBARALHA); do
    for threads in "${THREADS[@]}"; do
        for rep in $(seq 1 $REPS); do
            echo "[Fitness] n_embaralha=$n_embaralha threads=$threads rep=$rep"
            entry=$(run_test "$ROOT_DIR/TesteCuboFitness" "$n_embaralha" "$threads" "$SEED")
            append_result "$entry|$rep"

            echo "[Ilha] n_embaralha=$n_embaralha threads=$threads rep=$rep"
            entry=$(run_test "$ROOT_DIR/TesteCuboIlha" "$n_embaralha" "$threads" "$SEED")
            append_result "$entry|$rep"
        done
    done
    for rep in $(seq 1 $REPS); do
        echo "[Sequencial] n_embaralha=$n_embaralha rep=$rep"
        entry=$(run_test "$ROOT_DIR/TesteCuboSequencial" "$n_embaralha" "$SEED")
        append_result "$entry|$rep"
    done
done

write_json() {
    local mode="$1"
    local threads="$2"
    local emb="$3"
    local fit="$4"
    local solved="$5"
    local gens="$6"
    local rep="$7"
    printf '    {\n'
    printf '      "mode": "%s",\n' "$mode"
    printf '      "threads": %s,\n' "$threads"
    printf '      "n_embaralha": %s,\n' "$emb"
    printf '      "rep": %s,\n' "$rep"
    printf '      "fitness": %s,\n' "$fit"
    printf '      "solved": "%s",\n' "$solved"
    printf '      "generations": %s\n' "$gens"
    printf '    }'
}

{
    echo '{'
    printf '  "seed": %s,\n' "$SEED"
    printf '  "repetitions": %s,\n' "$REPS"
    printf '  "threads": [%s],\n' "${THREADS[*]// /, }"
    printf '  "n_embaralha": [%s],\n' "$(seq -s ', ' 1 $MAX_EMBARALHA)"
    echo '  "results": ['
    first=1
    for item in "${results[@]}"; do
        IFS='|' read -r mode threads emb fit solved gens rep <<< "$item"
        if [ "$first" -eq 0 ]; then
            echo ','
        fi
        first=0
        write_json "$mode" "$threads" "$emb" "$fit" "$solved" "$gens" "$rep"
    done
    echo
    echo '  ]'
    echo '}'
} > "$JSON_FILE"

echo "JSON salvo em $JSON_FILE"
