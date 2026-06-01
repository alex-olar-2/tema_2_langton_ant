#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int dx[] = {-1, 0, 1, 0}; // SUS, DREAPTA, JOS, STANGA
int dy[] = {0, 1, 0, -1};

typedef struct {
    int x, y, dir;
} Ant;

void save_ppm(int *grid, int N, Ant *ants, int num_ants, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Eroare: Nu s-a putut deschide %s. Exista folderul 'imagini'?\n", filename);
        return;
    }
    
    fprintf(f, "P3\n%d %d\n255\n", N, N);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int has_ant = 0;
            for(int a = 0; a < num_ants; a++) {
                if(ants[a].x == i && ants[a].y == j) {
                    has_ant = 1;
                    break;
                }
            }

            if (has_ant) {
                fprintf(f, "255 0 0 "); // Furnicile sunt rosii
            } else if (grid[i * N + j] == 0) {
                fprintf(f, "255 255 255 "); // Alb
            } else {
                fprintf(f, "0 0 0 "); // Negru
            }
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

int main(int argc, char **argv) {
    int N = 1000; 
    int T = 100000;
    int num_ants = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            N = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            T = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-ants") == 0 && i + 1 < argc) {
            num_ants = atoi(argv[++i]);
        }
    }

    printf("Pornire simulare: Grila %dx%d, %d pasi, %d furnici\n", N, N, T, num_ants);

    int *grid = (int *)calloc(N * N, sizeof(int));
    Ant *ants = (Ant *)malloc(num_ants * sizeof(Ant));

    srand(time(NULL));
    for (int i = 0; i < num_ants; i++) {
        ants[i].x = rand() % N;
        ants[i].y = rand() % N;
        ants[i].dir = rand() % 4;
    }

    for (int step = 0; step < T; step++) {
        for (int i = 0; i < num_ants; i++) {
            int idx = ants[i].x * N + ants[i].y;
            
            if (grid[idx] == 0) {
                ants[i].dir = (ants[i].dir + 1) % 4; 
                grid[idx] = 1;
            } else {
                ants[i].dir = (ants[i].dir + 3) % 4; 
                grid[idx] = 0;
            }

            ants[i].x += dx[ants[i].dir];
            ants[i].y += dy[ants[i].dir];

            ants[i].x = (ants[i].x + N) % N;
            ants[i].y = (ants[i].y + N) % N;
        }
    }

    // MODIFICARE: Calea de iesire este redirectionata catre folderul 'imagini'
    save_ppm(grid, N, ants, num_ants, "imagini/langton_multi.ppm");
    
    free(grid);
    free(ants);
    printf("Simulare completata. Rezultatul e in imagini/langton_multi.ppm\n");
    
    return 0;
}
