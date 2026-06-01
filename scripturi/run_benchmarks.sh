#!/bin/bash

echo "=== Incepem campania de benchmarking pentru Strong Scaling ==="
echo "Compilam codul sursa..."
mpicc -Wall -O2 langton_mpi.c -o langton_mpi

OUTPUT_FILE="rezultate_benchmark.txt"

# Initializam fisierul de rezultate
echo "Rezultate Benchmarking - Strong Scaling" > $OUTPUT_FILE
echo "Data: $(date)" >> $OUTPUT_FILE
echo "----------------------------------------" >> $OUTPUT_FILE

# Lista cu numarul de procese dorite conform temei
PROCESE=(1 2 4 8 16)

for p in "${PROCESE[@]}"
do
    echo "Rulez pe $p proces(e)..."
    echo ">> Test cu $p proces(e):" >> $OUTPUT_FILE
    
    # Rulam programul si adaugam output-ul in fisierul text
    # mpirun va rula executabilul, iar rezultatul (inclusiv timpul masurat) e salvat
    mpirun -np $p ./langton_mpi >> $OUTPUT_FILE
    
    echo "----------------------------------------" >> $OUTPUT_FILE
done

echo "=== Benchmarking complet! ==="
echo "Poti vedea rezultatele ruland: cat $OUTPUT_FILE"
