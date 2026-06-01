# Tema 2: Simularea unui automat celular - Furnica lui Langton în 2D (MPI)

Acest proiect implementează simularea automatului celular bidimensional „Furnica lui Langton”, creat de Christopher Langton în 1986. Proiectul conține atât o versiune secvențială (cu suport pentru mai mulți agenți), cât și o versiune paralelă și distribuită dezvoltată în C folosind standardul MPI (Message Passing Interface).

Scopul principal al paralelizării este accelerarea calculului pe grile mari (ex. 10.000 x 10.000 celule) prin partiționarea domeniului (1D) și gestionarea dinamică a migrării agenților (furnicilor) între procese.

---

## 📁 Structura Proiectului

Proiectul este organizat pentru a menține spațiul de lucru curat, separând codul sursă de fișierele generate (binare, imagini, texte):

* `langton_seq.c` - Codul sursă pentru versiunea secvențială (suportă CLI și multi-agent).
* `langton_mpi.c` - Codul sursă pentru versiunea paralelă cu partiționare 1D, Ghost Rows și migrare asincronă via MPI.
* `executabile/` - Directorul unde sunt salvate binarele după compilare.
* `imagini/` - Directorul de output pentru cadrele generate de simulare (format `.ppm`).
* `rezultate/` - Directorul de output pentru fișierele text cu datele de benchmarking.
* `scripturi/` - Directorul conținând scripturi utilitare (ex. `run_benchmarks.sh`).

---

## ⚙️ Cerințe Preliminare (Prerequisites)

Pentru a compila și rula acest proiect pe un sistem Linux, ai nevoie de următoarele pachete instalate:

* **GCC (GNU Compiler Collection):** Pentru compilarea versiunii secvențiale.
* **OpenMPI sau MPICH:** Implementarea standardului MPI pentru rularea distribuită (`mpicc`, `mpirun`).
* **Git:** Pentru versionarea codului.

---

## 🛠️ Instalare și Compilare

Clonarea repository-ului și crearea directoarelor necesare (dacă nu au fost deja create):
```bash
git clone git@github.com:UsernameTau/tema_2_langton_ant.git
cd tema_2_langton_ant
mkdir -p executabile imagini rezultate scripturi
```

---

### Compilarea versiunii secvențiale:

```bash
gcc -Wall -O2 langton_seq.c -o executabile/langton_seq

```

### Compilarea versiunii paralele (MPI):

```bash
mpicc -Wall -O2 langton_mpi.c -o executabile/langton_mpi

```

---

## 🚀 Rularea Proiectului

### 1. Versiunea Secvențială

Versiunea secvențială acceptă argumente din linia de comandă (CLI) pentru parametrizare:

* `-n` : Dimensiunea grilei (N x N)
* `-t` : Numărul de pași ai simulării
* `-ants` : Numărul de furnici generate aleatoriu pe grilă

**Exemplu de rulare:**

```bash
./executabile/langton_seq -n 500 -t 50000 -ants 10

```

*Rezultatul va fi generat sub forma unui fișier `imagini/langton_multi.ppm`.*

### 2. Versiunea Paralelă (MPI)

Versiunea paralelă partiționează automat grila pe rânduri (1D) în funcție de numărul de procese specificat prin flag-ul `-np`.

**Exemplu de rulare pe 4 procese:**

```bash
mpirun -np 4 ./executabile/langton_mpi

```

*Programul va exporta periodic cadre în format `.ppm` în folderul `imagini/` (pentru reconstrucția animației) și va afișa timpul total de execuție în consolă.*

---

## 📊 Benchmarking și Testare Performanță

Pentru automatizarea testelor de scalabilitate (Strong Scaling), proiectul include un script de Bash care rulează simularea MPI pe 1, 2, 4, 8 și 16 procese succesiv, măsurând timpii.

Înainte de prima rulare, asigură-te că scriptul are permisiuni de execuție:

```bash
chmod +x scripturi/run_benchmarks.sh

```

**Rularea suitei de teste:**

```bash
./scripturi/run_benchmarks.sh

```

*Rezultatele testelor vor fi formatate și salvate în fișierul `rezultate/rezultate_benchmark.txt`.*

---

**Notă tehnică:** Se recomandă vizualizarea cadrelor `.ppm` folosind utilitare standard Linux precum `display` (ImageMagick), `eog` sau convertoare online.

```

```
