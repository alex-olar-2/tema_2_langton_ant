#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ANTS_PER_PROC 1000

int dx[] = {-1, 0, 1, 0};
int dy[] = {0, 1, 0, -1};

typedef struct {
    int local_x, y, dir;
} Ant;

// Functie pentru salvarea grilei complete
void save_frame(int *global_grid, int N, int step) {
    char filename[64];
    sprintf(filename, "frame_%05d.ppm", step);
    
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "P3\n%d %d\n255\n", N, N);
    for (int i = 0; i < N * N; i++) {
        if (global_grid[i] == 0) {
            fprintf(f, "255 255 255 "); // Alb
        } else {
            fprintf(f, "0 0 0 ");       // Negru
        }
        // Un newline din cand in cand pentru lizibilitatea fisierului
        if ((i + 1) % N == 0) fprintf(f, "\n");
    }
    fclose(f);
}

int main(int argc, char **argv) {
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = 100; 
    int T = 12000; // Suficient pentru a vedea formarea "highway-ului"
    int K = 100;   // Colectam date o data la K pasi
    
    int local_N = N / size;
    int *local_grid = (int *)calloc((local_N + 2) * N, sizeof(int));
    int *global_grid = NULL;

    if (rank == 0) {
        global_grid = (int *)malloc(N * N * sizeof(int));
    }

    int up_neighbor = (rank == 0) ? size - 1 : rank - 1;
    int down_neighbor = (rank == size - 1) ? 0 : rank + 1;

    MPI_Datatype MPI_ANT;
    MPI_Type_contiguous(3, MPI_INT, &MPI_ANT);
    MPI_Type_commit(&MPI_ANT);

    Ant local_ants[MAX_ANTS_PER_PROC];
    int num_local_ants = 0;

    // Doar procesul 0 initiaza o furnica (ea va migra ulterior in celelalte procese)
    if (rank == 0) {
        local_ants[0] = (Ant){local_N / 2 + 1, N / 2, 0};
        num_local_ants = 1;
    }

    Ant send_up[MAX_ANTS_PER_PROC], send_down[MAX_ANTS_PER_PROC];

// Sincronizam toate procesele inainte de a porni cronometrul
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime(); // Pornim cronometrul

    for (int step = 0; step <= T; step++) {
        
        // --- NOU: Colectarea periodica ---
        if (step % K == 0) {
            // Trimitem doar zona reala: &local_grid[1 * N]
            MPI_Gather(&local_grid[1 * N], local_N * N, MPI_INT, 
                       global_grid, local_N * N, MPI_INT, 
                       0, MPI_COMM_WORLD);
            
            if (rank == 0) {
                save_frame(global_grid, N, step);
                printf("Salvat cadru la pasul %d\n", step);
            }
        }

        // 1. EXCHANGE GHOST ROWS
        MPI_Sendrecv(&local_grid[1 * N], N, MPI_INT, up_neighbor, 0,
                     &local_grid[(local_N + 1) * N], N, MPI_INT, down_neighbor, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Sendrecv(&local_grid[local_N * N], N, MPI_INT, down_neighbor, 1,
                     &local_grid[0 * N], N, MPI_INT, up_neighbor, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 2. ACTUALIZARE AGENȚI
        int count_up = 0, count_down = 0;
        for (int i = num_local_ants - 1; i >= 0; i--) {
            int idx = local_ants[i].local_x * N + local_ants[i].y;
            
            if (local_grid[idx] == 0) {
                local_ants[i].dir = (local_ants[i].dir + 1) % 4;
                local_grid[idx] = 1;
            } else {
                local_ants[i].dir = (local_ants[i].dir + 3) % 4;
                local_grid[idx] = 0;
            }

            local_ants[i].local_x += dx[local_ants[i].dir];
            local_ants[i].y = (local_ants[i].y + dy[local_ants[i].dir] + N) % N;

            // 3. VERIFICARE MIGRARE
            if (local_ants[i].local_x == 0) { 
                local_ants[i].local_x = local_N; 
                send_up[count_up++] = local_ants[i];
                local_ants[i] = local_ants[--num_local_ants];
            } 
            else if (local_ants[i].local_x == local_N + 1) { 
                local_ants[i].local_x = 1; 
                send_down[count_down++] = local_ants[i];
                local_ants[i] = local_ants[--num_local_ants];
            }
        }

        // 4. PROTOCOLUL DE MIGRARE
        MPI_Request reqs[4];
        int req_count = 0;

        MPI_Isend(send_up, count_up, MPI_ANT, up_neighbor, 2, MPI_COMM_WORLD, &reqs[req_count++]);
        MPI_Isend(send_down, count_down, MPI_ANT, down_neighbor, 3, MPI_COMM_WORLD, &reqs[req_count++]);

        for(int src = 0; src < 2; src++) {
            int target_neighbor = (src == 0) ? up_neighbor : down_neighbor;
            int tag = (src == 0) ? 3 : 2; 
            
            MPI_Status status;
            int incoming_count;
            
            MPI_Probe(target_neighbor, tag, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_ANT, &incoming_count);

            if (incoming_count > 0) {
                MPI_Recv(&local_ants[num_local_ants], incoming_count, MPI_ANT, 
                         target_neighbor, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                num_local_ants += incoming_count;
            } else {
                MPI_Recv(NULL, 0, MPI_ANT, target_neighbor, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        MPI_Waitall(req_count, reqs, MPI_STATUSES_IGNORE);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if (rank == 0) {
        printf("Simulare finalizata! Timp executie: %f secunde pe %d procese.\n", end_time - start_time, size);
    }

    if (rank == 0) free(global_grid);
    free(local_grid);
    MPI_Type_free(&MPI_ANT);
    
    if (rank == 0) printf("Simulare MPI finalizata!\n");
    
    MPI_Finalize();
    return 0;
}
