#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int dx[] = {-1, 0, 1, 0};
int dy[] = {0, 1, 0, -1};

typedef struct {
    int local_x, y, dir;
} Ant;

int main(int argc, char **argv) {
    int rank, size;
    
    // Initializare mediu MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = 1000;
    int T = 1000; // Mai putini pasi pentru testare
    
    // Presupunem ca N se imparte exact la numarul de procese pentru simplitate
    int local_N = N / size;
    
    // Alocam subgrila locala + 2 rânduri ghost (sus si jos)
    // Dimensiunea totala: (local_N + 2) randuri de cate N coloane
    int *local_grid = (int *)calloc((local_N + 2) * N, sizeof(int));

    // Identificam vecinii (grila toroidala)
    int up_neighbor = (rank == 0) ? size - 1 : rank - 1;
    int down_neighbor = (rank == size - 1) ? 0 : rank + 1;

    // Initializam o furnica locala in centrul subgrilei procesului curent
    Ant ant = {local_N / 2 + 1, N / 2, 0}; // +1 pentru a sari peste randul ghost de sus

    for (int step = 0; step < T; step++) {
        // 1. COMUNICARE GHOST ROWS cu MPI_Sendrecv
        
        // Trimitem primul rând REAL in sus, primim in ultimul rând GHOST de jos
        MPI_Sendrecv(
            &local_grid[1 * N], N, MPI_INT, up_neighbor, 0,
            &local_grid[(local_N + 1) * N], N, MPI_INT, down_neighbor, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );

        // Trimitem ultimul rând REAL in jos, primim in primul rând GHOST de sus
        MPI_Sendrecv(
            &local_grid[local_N * N], N, MPI_INT, down_neighbor, 1,
            &local_grid[0 * N], N, MPI_INT, up_neighbor, 1,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );

        // 2. ACTUALIZARE AGENT LOCAL
        int idx = ant.local_x * N + ant.y;
        
        if (local_grid[idx] == 0) {
            ant.dir = (ant.dir + 1) % 4; // Rotire dreapta
            local_grid[idx] = 1;
        } else {
            ant.dir = (ant.dir + 3) % 4; // Rotire stanga
            local_grid[idx] = 0;
        }

        // 3. AVANSARE (cu constrangere la domeniul local - fara migrare)
        int next_local_x = ant.local_x + dx[ant.dir];
        int next_y = ant.y + dy[ant.dir];

        // Daca incearca sa iasa din zona reala locala, o blocam temporar
        if (next_local_x >= 1 && next_local_x <= local_N) {
            ant.local_x = next_local_x;
        }
        
        // Pe axa Y se misca liber (wrap-around orizontal)
        ant.y = (next_y + N) % N;
    }

    // Curatare
    free(local_grid);
    
    if (rank == 0) {
        printf("Simulare MPI (Faza 1 - comunicare ghost rows) finalizata pe %d procese!\n", size);
    }
    
    MPI_Finalize();
    return 0;
}
