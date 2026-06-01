#include <stdio.h>
#include <stdlib.h>

// Definim directiile: 0=SUS, 1=DREAPTA, 2=JOS, 3=STANGA
int dx[] = {-1, 0, 1, 0};
int dy[] = {0, 1, 0, -1};

// Functie pentru exportul grilei in format PPM
void save_ppm(int *grid, int N, int ant_x, int ant_y, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Eroare la crearea fisierului %s\n", filename);
        return;
    }
    // Header format PPM (P3), latime inaltime, valoare maxima culoare
    fprintf(f, "P3\n%d %d\n255\n", N, N);
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == ant_x && j == ant_y) {
                fprintf(f, "255 0 0 "); // Furnica desenata cu Rosu
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
    // Setam dimensiunile pentru testul recomandat in tema
    int N = 1000; 
    int T = 100000; 

    // Alocam grila ca un array 1D liniarizat (N x N)
    // Folosim calloc pentru a initializa toate celulele cu 0 (Alb)
    int *grid = (int *)calloc(N * N, sizeof(int));

    // Pozitia initiala: centrul grilei
    int ant_x = N / 2;
    int ant_y = N / 2;
    int dir = 0; // Priveste in SUS

    // Simulam T pasi
    for (int step = 0; step < T; step++) {
        int idx = ant_x * N + ant_y;
        
        if (grid[idx] == 0) { // Celula alba
            dir = (dir + 1) % 4; // 90 grade dreapta
            grid[idx] = 1;       // Devine neagra
        } else {              // Celula neagra
            dir = (dir + 3) % 4; // 90 grade stanga (echivalent cu -1 in modul)
            grid[idx] = 0;       // Devine alba
        }

        // Avanseaza un pas
        ant_x += dx[dir];
        ant_y += dy[dir];

        // Pastram furnica in interiorul grilei (grila toroidala)
        ant_x = (ant_x + N) % N;
        ant_y = (ant_y + N) % N;
    }

    // Exportam imaginea finala
    save_ppm(grid, N, ant_x, ant_y, "langton_output.ppm");
    
    free(grid);
    printf("Simularea secventiala s-a incheiat (%d pasi). Imaginea a fost salvata in langton_output.ppm\n", T);
    
    return 0;
}
