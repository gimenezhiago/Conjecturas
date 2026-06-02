#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <random>
#include <tbb/tbb.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/global_control.h>

#define U 0
#define D 1
#define F 2
#define B 3
#define R 4
#define L 5
#define NUM_MOV         18
#define TAM_POP         20000
#define MAX_ESTAG       50
#define TAX_MUT_INI     0.02f
#define TAX_MUT_INC     0.005f
#define TAM_CROMO       50
#define FIT_MAX         100.0f

/* -----------------------------------------------------------------------
 * Pesos da melhor configuração encontrada (artigo SICITE + relatório IC)
 *
 * Para cada peça de CANTO (8 peças):
 *   correto=-1  orientado=-5  permutado=-9  incorreto=-6
 *
 * Para cada peça de BORDA (12 peças):
 *   correto=-96  orientado=-78  permutado=35  incorreto=-6
 *
 * O valor mais determinante é o da borda permutada (+35), pois é o único
 * positivo e permite distinguir cubos mais próximos da solução.
 * ----------------------------------------------------------------------- */
static const int FC[4] = {-1, -5, -9, -6};
static const int FB[4] = {-96, -78, 35, -6};

static const int BORDAS[12][2][3] = {
    {{U,2,1},{F,0,1}}, {{U,1,2},{R,0,1}}, {{U,0,1},{B,0,1}}, {{U,1,0},{L,0,1}},
    {{F,1,2},{R,1,0}}, {{F,1,0},{L,1,2}}, {{B,1,0},{R,1,2}}, {{B,1,2},{L,1,0}},
    {{D,0,1},{F,2,1}}, {{D,1,2},{R,2,1}}, {{D,2,1},{B,2,1}}, {{D,1,0},{L,2,1}}
};
static const int BCOR[12][2] = {
    {U,F},{U,R},{U,B},{U,L},{F,R},{F,L},{B,R},{B,L},{D,F},{D,R},{D,B},{D,L}
};
static const int CANTOS[8][3][3] = {
    {{U,2,2},{F,0,2},{R,0,0}}, {{U,2,0},{F,0,0},{L,0,2}},
    {{U,0,2},{B,0,0},{R,0,2}}, {{U,0,0},{B,0,2},{L,0,0}},
    {{D,0,2},{F,2,2},{R,2,0}}, {{D,0,0},{F,2,0},{L,2,2}},
    {{D,2,2},{B,2,0},{R,2,2}}, {{D,2,0},{B,2,2},{L,2,0}}
};
static const int CCOR[8][3] = {
    {U,F,R},{U,F,L},{U,B,R},{U,B,L},{D,F,R},{D,F,L},{D,B,R},{D,B,L}
};

struct Cubo { int face[6][3][3]; };

void cubo_init(Cubo &c) {
    for(int f=0;f<6;f++) for(int i=0;i<3;i++) for(int j=0;j<3;j++) c.face[f][i][j]=f;
}
bool resolvido(const Cubo &c) {
    for(int f=0;f<6;f++) for(int i=0;i<3;i++) for(int j=0;j<3;j++) if(c.face[f][i][j]!=f) return false;
    return true;
}
static void rot(Cubo&c,int f,int cw){int t[3][3];for(int i=0;i<3;i++)for(int j=0;j<3;j++)t[i][j]=c.face[f][i][j];if(cw){for(int i=0;i<3;i++)for(int j=0;j<3;j++)c.face[f][j][2-i]=t[i][j];}else{for(int i=0;i<3;i++)for(int j=0;j<3;j++)c.face[f][2-j][i]=t[i][j];}}
static void mU(Cubo&c){rot(c,U,1);int t[3];for(int j=0;j<3;j++)t[j]=c.face[F][0][j];for(int j=0;j<3;j++)c.face[F][0][j]=c.face[R][0][j];for(int j=0;j<3;j++)c.face[R][0][j]=c.face[B][0][j];for(int j=0;j<3;j++)c.face[B][0][j]=c.face[L][0][j];for(int j=0;j<3;j++)c.face[L][0][j]=t[j];}
static void mD(Cubo&c){rot(c,D,1);int t[3];for(int j=0;j<3;j++)t[j]=c.face[F][2][j];for(int j=0;j<3;j++)c.face[F][2][j]=c.face[L][2][j];for(int j=0;j<3;j++)c.face[L][2][j]=c.face[B][2][j];for(int j=0;j<3;j++)c.face[B][2][j]=c.face[R][2][j];for(int j=0;j<3;j++)c.face[R][2][j]=t[j];}
static void mF(Cubo&c){rot(c,F,1);int t[3];for(int j=0;j<3;j++)t[j]=c.face[U][2][j];for(int j=0;j<3;j++)c.face[U][2][j]=c.face[L][2-j][2];for(int j=0;j<3;j++)c.face[L][j][2]=c.face[D][0][j];for(int j=0;j<3;j++)c.face[D][0][j]=c.face[R][2-j][0];for(int j=0;j<3;j++)c.face[R][j][0]=t[j];}
static void mB(Cubo&c){rot(c,B,1);int t[3];for(int j=0;j<3;j++)t[j]=c.face[U][0][j];for(int j=0;j<3;j++)c.face[U][0][j]=c.face[R][j][2];for(int j=0;j<3;j++)c.face[R][j][2]=c.face[D][2][2-j];for(int j=0;j<3;j++)c.face[D][2][j]=c.face[L][2-j][0];for(int j=0;j<3;j++)c.face[L][j][0]=t[2-j];}
static void mR(Cubo&c){rot(c,R,1);int t[3];for(int i=0;i<3;i++)t[i]=c.face[U][i][2];for(int i=0;i<3;i++)c.face[U][i][2]=c.face[F][i][2];for(int i=0;i<3;i++)c.face[F][i][2]=c.face[D][i][2];for(int i=0;i<3;i++)c.face[D][i][2]=c.face[B][2-i][0];for(int i=0;i<3;i++)c.face[B][2-i][0]=t[i];}
static void mL(Cubo&c){rot(c,L,1);int t[3];for(int i=0;i<3;i++)t[i]=c.face[U][i][0];for(int i=0;i<3;i++)c.face[U][i][0]=c.face[B][2-i][2];for(int i=0;i<3;i++)c.face[B][2-i][2]=c.face[D][i][0];for(int i=0;i<3;i++)c.face[D][i][0]=c.face[F][i][0];for(int i=0;i<3;i++)c.face[F][i][0]=t[i];}

void aplicar_mov(Cubo&c,int m){void(*tab[6])(Cubo&)={mU,mD,mF,mB,mR,mL};int b=m%6,tp=m/6;if(tp==0)tab[b](c);else if(tp==1){tab[b](c);tab[b](c);tab[b](c);}else{tab[b](c);tab[b](c);}}
void aplicar(Cubo&c,const std::vector<int>&v){for(int m:v)aplicar_mov(c,m);}

/* -----------------------------------------------------------------------
 * Estado de borda i  →  0=correto  1=orientado  2=permutado  3=incorreto
 * ----------------------------------------------------------------------- */
static int eb(const Cubo &c, int i) {
    int c0=c.face[BORDAS[i][0][0]][BORDAS[i][0][1]][BORDAS[i][0][2]];
    int c1=c.face[BORDAS[i][1][0]][BORDAS[i][1][1]][BORDAS[i][1][2]];
    int e0=BCOR[i][0], e1=BCOR[i][1];
    if(c0==e0 && c1==e1) return 0;
    if(c0==e1 && c1==e0) return 2;
    if(c0==e0 || c1==e1) return 1;
    return 3;
}

/* -----------------------------------------------------------------------
 * Estado de canto i  →  0=correto  1=orientado  2=permutado  3=incorreto
 * ----------------------------------------------------------------------- */
static int ec(const Cubo &c, int i) {
    int co[3], es[3];
    for(int k=0;k<3;k++){
        co[k]=c.face[CANTOS[i][k][0]][CANTOS[i][k][1]][CANTOS[i][k][2]];
        es[k]=CCOR[i][k];
    }
    int ct=0;
    for(int k=0;k<3;k++) if(co[k]==es[k]) ct++;
    if(ct==3) return 0;
    int tm=0;
    for(int k=0;k<3;k++) for(int m=0;m<3;m++) if(co[k]==es[m]){tm++;break;}
    if(tm==3 && ct==0) return 2;
    if(ct==1)          return 1;
    return 3;
}

/* -----------------------------------------------------------------------
 * fitness(orig, v)
 *
 * Algoritmo de pontuação conforme descrito nos artigos:
 *   1. Aplica a sequência de movimentos v sobre uma cópia do cubo orig.
 *   2. Se resolvido → retorna FIT_MAX = 100.
 *   3. Percorre as 12 bordas e os 8 cantos acumulando FB[estado] e FC[estado].
 *   4. Normaliza linearmente para [0, 99]:
 *        score_max = 12×FB[0] + 8×FC[0]   (todos corretos)
 *        score_min = 12×FB[2] + 8×FC[2]   (todos permutados)
 *        fitness = (s - score_max) / (score_min - score_max) × 100
 * ----------------------------------------------------------------------- */
float fitness(const Cubo &orig, const std::vector<int> &v) {
    Cubo c = orig;
    aplicar(c, v);
    if(resolvido(c)) return FIT_MAX;
    int s = 0;
    for(int i=0;i<12;i++) s += FB[eb(c,i)];
    for(int i=0;i<8;i++)  s += FC[ec(c,i)];
    int mx = 12*FB[0] + 8*FC[0];
    int mn = 12*FB[2] + 8*FC[2];
    float r = ((float)(s - mx) / (float)(mn - mx)) * 100.f;
    return r < 0 ? 0 : r > 99 ? 99 : r;
}

struct Ind { std::vector<int> v; float f = 0; };

void torneio(const std::vector<Ind>&pop, int&a, int&b, std::mt19937&rng) {
    int c[4];
    for(int i=0;i<4;i++) c[i] = rng() % TAM_POP;
    std::sort(c, c+4, [&](int x, int y){ return pop[x].f > pop[y].f; });
    a = c[0]; b = c[1];
}
Ind cruzar(const Ind&p1, const Ind&p2, std::mt19937&rng) {
    int t = ((int)p1.v.size() + (int)p2.v.size()) / 2; if(t<1) t=1;
    Ind f; f.v.resize(t);
    for(int i=0;i<t;i++)
        f.v[i]=(i%2==0)?((i<(int)p1.v.size())?p1.v[i]:p1.v[rng()%p1.v.size()])
                        :((i<(int)p2.v.size())?p2.v[i]:p2.v[rng()%p2.v.size()]);
    return f;
}
void mutar(Ind &ind, std::mt19937 &rng) { ind.v[rng()%ind.v.size()] = rng()%NUM_MOV; }

void embaralhar(Cubo &c, int n, unsigned seed) {
    std::mt19937 r(seed);
    static const char *nm[18] = {
        "U","D","F","B","R","L","U'","D'","F'","B'","R'","L'","U2","D2","F2","B2","R2","L2"
    };
    fprintf(stderr, "Embaralhamento (%d mov): ", n);
    for(int i=0;i<n;i++){int m=r()%NUM_MOV;aplicar_mov(c,m);fprintf(stderr,"%s ",nm[m]);}
    fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
    /* Uso: ./TesteCuboFitness <n_embaralha> <n_threads> [seed] */
    int n_embaralha   = (argc > 1) ? atoi(argv[1]) : 20;
    unsigned nthreads = (argc > 2) ? (unsigned)atoi(argv[2]) : tbb::info::default_concurrency();
    unsigned seed     = (argc > 3) ? (unsigned)atoi(argv[3]) : 42;

    if(n_embaralha < 1 || n_embaralha > 30) {
        fprintf(stderr, "ERRO: n_embaralha deve ser entre 1 e 30\n");
        return 1;
    }
    if(nthreads < 1) nthreads = 1;

    fprintf(stderr, "=== TBB — FITNESS PARALELO ===\n");
    fprintf(stderr, "Threads: %u | Pop: %d | Cromo: %d | Embaralha: %d | Seed: %u\n\n",
            nthreads, TAM_POP, TAM_CROMO, n_embaralha, seed);

    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, nthreads);

    Cubo cubo; cubo_init(cubo); embaralhar(cubo, n_embaralha, seed);

    std::vector<Ind> pop(TAM_POP);
    tbb::parallel_for(tbb::blocked_range<int>(0, TAM_POP),
        [&](const tbb::blocked_range<int> &r) {
            std::mt19937 rng(std::random_device{}() ^ ((uint32_t)r.begin() * 2654435761u));
            for(int i = r.begin(); i < r.end(); i++) {
                pop[i].v.resize(TAM_CROMO);
                for(auto &m : pop[i].v) m = rng() % NUM_MOV;
                pop[i].f = fitness(cubo, pop[i].v);
            }
        });
    std::sort(pop.begin(), pop.end(), [](const Ind&a, const Ind&b){ return a.f > b.f; });

    float taxa = TAX_MUT_INI;
    int estag = 0;
    float mg = -1;
    int g_conv = 0;

    /* ---------- loop evolutivo paralelo (avaliação de fitness em paralelo) ---------- */
    for(int g = 1; estag < MAX_ESTAG; g++) {
        if(pop[0].f >= FIT_MAX) { g_conv = g; break; }

        std::vector<Ind> filhos(TAM_POP);
        tbb::parallel_for(tbb::blocked_range<int>(0, TAM_POP),
            [&](const tbb::blocked_range<int> &r) {
                std::mt19937 rng(std::random_device{}() ^
                                 ((uint32_t)r.begin() * 2246822519u + (uint32_t)g * 374761393u));
                float tx = taxa;
                for(int i = r.begin(); i < r.end(); i++) {
                    int a, b; torneio(pop, a, b, rng);
                    filhos[i] = cruzar(pop[a], pop[b], rng);
                    if((float)(rng() % 10000) / 10000.f < tx) mutar(filhos[i], rng);
                    filhos[i].f = fitness(cubo, filhos[i].v);
                }
            });

        for(auto &f : filhos) pop.push_back(std::move(f));
        std::sort(pop.begin(), pop.end(), [](const Ind&a, const Ind&b){ return a.f > b.f; });
        pop.resize(TAM_POP);

        if(pop[0].f > mg) { mg = pop[0].f; estag = 0; taxa = TAX_MUT_INI; }
        else              { estag++; taxa += TAX_MUT_INC; }
        g_conv = g;
    }

    printf("RESULTADO,FitnessParalelo,%u,%d,%.4f,%s,%d\n",
           nthreads, n_embaralha, pop[0].f,
           pop[0].f >= FIT_MAX ? "SIM" : "NAO",
           g_conv);

    fprintf(stderr, "\nMelhor fitness : %.2f/100\n", pop[0].f);
    fprintf(stderr, "Gerações       : %d\n", g_conv);
    fprintf(stderr, "Resolvido      : %s\n", pop[0].f >= FIT_MAX ? "SIM" : "NAO");
    return 0;
}
/* Compilar: g++ -O3 -o TesteCuboFitness TesteCuboFitness.cpp -ltbb
   Rodar:    ./TesteCuboFitness <n_mov> <n_threads> [seed]
   Tempo:    medido externamente via perf no script .sh               */
