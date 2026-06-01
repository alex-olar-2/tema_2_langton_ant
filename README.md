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
