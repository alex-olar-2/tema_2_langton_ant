#!/bin/bash

# Pentru ca scriptul e in folderul /scripturi, dar noi il rulam de obicei 
# din radacina proiectului, asigura-te ca il rulezi cu comanda: ./scripturi/run_benchmarks.sh

echo "=== Incepem campania de benchmarking pentru Strong Scaling ==="
echo "Compilam codul sursa direct in folderul executabile/ ..."

# MODIFICARE: Compilam si aruncam executabilul direct in directorul corespunzator
mpicc -Wall -O2 langton_mpi.c -o executabile/langton_mpi

# MODIFICARE: Rezultatele sunt trimise direct in folderul 'rezultate'
OUTPUT_FILE="rezultate/rezultate_benchmark.txt"

echo "Rezultate Benchmarking - Strong Scaling" > $OUTPUT_FILE
echo "Data: $(date)" >> $OUTPUT_FILE
echo "----------------------------------------" >> $OUTPUT_FILE

PROCESE=(1 2 4 8 16)

for p in "${PROCESE[@]}"
do
    echo "Rulez pe $p proces(e)..."
    echo ">> Test cu $p proces(e):" >> $OUTPUT_FILE
    
    # MODIFICARE: Rulam programul citindu-l din folderul executabile/
    mpirun -np $p ./executabile/langton_mpi >> $OUTPUT_FILE
    
    echo "----------------------------------------" >> $OUTPUT_FILE
done

echo "=== Benchmarking complet! ==="
echo "Poti vedea rezultatele ruland: cat $OUTPUT_FILE"
