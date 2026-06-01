#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ANTS_PER_PROC 1000

int dx[] = {-1, 0, 1, 0};
int dy[] = {0, 1, 0, -1};

typedef struct {
    int local_x, y, dir;
} Ant;

int main(int argc, char **argv) {
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = 100; // Grila mai mica pentru teste mai rapide
    int T = 5000;
    int local_N = N / size;
    
    int *local_grid = (int *)calloc((local_N + 2) * N, sizeof(int));

    int up_neighbor = (rank == 0) ? size - 1 : rank - 1;
    int down_neighbor = (rank == size - 1) ? 0 : rank + 1;

    // Definim un tip de date MPI pentru structura Ant
    MPI_Datatype MPI_ANT;
    MPI_Type_contiguous(3, MPI_INT, &MPI_ANT);
    MPI_Type_commit(&MPI_ANT);

    Ant local_ants[MAX_ANTS_PER_PROC];
    int num_local_ants = 0;

    // Initializam o furnica pe fiecare proces
    local_ants[0] = (Ant){local_N / 2 + 1, N / 2, 0};
    num_local_ants = 1;

    // Buffere pentru migratie
    Ant send_up[MAX_ANTS_PER_PROC], send_down[MAX_ANTS_PER_PROC];

    for (int step = 0; step < T; step++) {
        // 1. EXCHANGE GHOST ROWS
        MPI_Sendrecv(&local_grid[1 * N], N, MPI_INT, up_neighbor, 0,
                     &local_grid[(local_N + 1) * N], N, MPI_INT, down_neighbor, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Sendrecv(&local_grid[local_N * N], N, MPI_INT, down_neighbor, 1,
                     &local_grid[0 * N], N, MPI_INT, up_neighbor, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 2. ACTUALIZARE AGENȚI [cite: 30]
        int count_up = 0, count_down = 0;
        
        // Parcurgem invers pentru a putea sterge elemente in siguranta
        for (int i = num_local_ants - 1; i >= 0; i--) {
            int idx = local_ants[i].local_x * N + local_ants[i].y;
            
            // Regula Langton [cite: 31]
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
            if (local_ants[i].local_x == 0) { // A iesit pe sus
                local_ants[i].local_x = local_N; // Se va pozitiona la baza vecinului de sus
                send_up[count_up++] = local_ants[i]; // 
                local_ants[i] = local_ants[--num_local_ants]; // Stergem din lista locala
            } 
            else if (local_ants[i].local_x == local_N + 1) { // A iesit pe jos
                local_ants[i].local_x = 1; // Se va pozitiona la varful vecinului de jos
                send_down[count_down++] = local_ants[i]; // 
                local_ants[i] = local_ants[--num_local_ants];
            }
        }

        // 4. PROTOCOLUL DE MIGRARE [cite: 35]
        MPI_Request reqs[4];
        int req_count = 0;

        // Trimitem asincron furnicile care pleaca [cite: 36, 53]
        MPI_Isend(send_up, count_up, MPI_ANT, up_neighbor, 2, MPI_COMM_WORLD, &reqs[req_count++]);
        MPI_Isend(send_down, count_down, MPI_ANT, down_neighbor, 3, MPI_COMM_WORLD, &reqs[req_count++]);

        // Receptionam folosind Probe 
        for(int src = 0; src < 2; src++) {
            int target_neighbor = (src == 0) ? up_neighbor : down_neighbor;
            int tag = (src == 0) ? 3 : 2; // Daca primim de la UP, el a trimis cu tag-ul de DOWN (3)
            
            MPI_Status status;
            int incoming_count;
            
            // Asteptam sa vedem daca vine un mesaj 
            MPI_Probe(target_neighbor, tag, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_ANT, &incoming_count);

            if (incoming_count > 0) {
                MPI_Recv(&local_ants[num_local_ants], incoming_count, MPI_ANT, 
                         target_neighbor, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // [cite: 36]
                num_local_ants += incoming_count; // [cite: 37]
            } else {
                // Receptionam mesajul gol ca sa curatam bufferul intern MPI
                MPI_Recv(NULL, 0, MPI_ANT, target_neighbor, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        // Ne asiguram ca expedierile s-au incheiat inainte de urmatorul pas
        MPI_Waitall(req_count, reqs, MPI_STATUSES_IGNORE);
    }

    free(local_grid);
    MPI_Type_free(&MPI_ANT);
    
    if (rank == 0) {
        printf("Simulare MPI (Faza 2 - Migrare finalizata) cu succes!\n");
    }
    
    MPI_Finalize();
    return 0;
}
