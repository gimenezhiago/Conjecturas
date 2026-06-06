#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
#define FIT_MAX         100.0f

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

typedef struct { int face[6][3][3]; } Cubo;

void cubo_init(Cubo *c) { for(int f=0;f<6;f++) for(int i=0;i<3;i++) for(int j=0;j<3;j++) c->face[f][i][j]=f; }
int resolvido(const Cubo *c) { for(int f=0;f<6;f++) for(int i=0;i<3;i++) for(int j=0;j<3;j++) if(c->face[f][i][j]!=f) return 0; return 1; }
static void rot(Cubo*c,int f,int cw){int t[3][3];for(int i=0;i<3;i++)for(int j=0;j<3;j++)t[i][j]=c->face[f][i][j];if(cw){for(int i=0;i<3;i++)for(int j=0;j<3;j++)c->face[f][j][2-i]=t[i][j];}else{for(int i=0;i<3;i++)for(int j=0;j<3;j++)c->face[f][2-j][i]=t[i][j];}}
static void mU(Cubo*c){rot(c,U,1);int t[3];for(int j=0;j<3;j++)t[j]=c->face[F][0][j];for(int j=0;j<3;j++)c->face[F][0][j]=c->face[R][0][j];for(int j=0;j<3;j++)c->face[R][0][j]=c->face[B][0][j];for(int j=0;j<3;j++)c->face[B][0][j]=c->face[L][0][j];for(int j=0;j<3;j++)c->face[L][0][j]=t[j];}
static void mD(Cubo*c){rot(c,D,1);int t[3];for(int j=0;j<3;j++)t[j]=c->face[F][2][j];for(int j=0;j<3;j++)c->face[F][2][j]=c->face[L][2][j];for(int j=0;j<3;j++)c->face[L][2][j]=c->face[B][2][j];for(int j=0;j<3;j++)c->face[B][2][j]=c->face[R][2][j];for(int j=0;j<3;j++)c->face[R][2][j]=t[j];}
static void mF(Cubo*c){rot(c,F,1);int t[3];for(int j=0;j<3;j++)t[j]=c->face[U][2][j];for(int j=0;j<3;j++)c->face[U][2][j]=c->face[L][2-j][2];for(int j=0;j<3;j++)c->face[L][j][2]=c->face[D][0][j];for(int j=0;j<3;j++)c->face[D][0][j]=c->face[R][2-j][0];for(int j=0;j<3;j++)c->face[R][j][0]=t[j];}
static void mB(Cubo*c){rot(c,B,1);int t[3];for(int j=0;j<3;j++)t[j]=c->face[U][0][j];for(int j=0;j<3;j++)c->face[U][0][j]=c->face[R][j][2];for(int j=0;j<3;j++)c->face[R][j][2]=c->face[D][2][2-j];for(int j=0;j<3;j++)c->face[D][2][j]=c->face[L][2-j][0];for(int j=0;j<3;j++)c->face[L][j][0]=t[2-j];}
static void mR(Cubo*c){rot(c,R,1);int t[3];for(int i=0;i<3;i++)t[i]=c->face[U][i][2];for(int i=0;i<3;i++)c->face[U][i][2]=c->face[F][i][2];for(int i=0;i<3;i++)c->face[F][i][2]=c->face[D][i][2];for(int i=0;i<3;i++)c->face[D][i][2]=c->face[B][2-i][0];for(int i=0;i<3;i++)c->face[B][2-i][0]=t[i];}
static void mL(Cubo*c){rot(c,L,1);int t[3];for(int i=0;i<3;i++)t[i]=c->face[U][i][0];for(int i=0;i<3;i++)c->face[U][i][0]=c->face[B][2-i][2];for(int i=0;i<3;i++)c->face[B][2-i][2]=c->face[D][i][0];for(int i=0;i<3;i++)c->face[D][i][0]=c->face[F][i][0];for(int i=0;i<3;i++)c->face[F][i][0]=t[i];}

void aplicar_mov(Cubo*c,int m){void(*tab[6])(Cubo*)={mU,mD,mF,mB,mR,mL};int b=m%6,tp=m/6;if(tp==0)tab[b](c);else if(tp==1){tab[b](c);tab[b](c);tab[b](c);}else{tab[b](c);tab[b](c);}}
void aplicar(Cubo*c,const int *v,int len){for(int i=0;i<len;i++) aplicar_mov(c,v[i]);}

static int eb(const Cubo *c, int i) {
    int c0=c->face[BORDAS[i][0][0]][BORDAS[i][0][1]][BORDAS[i][0][2]];
    int c1=c->face[BORDAS[i][1][0]][BORDAS[i][1][1]][BORDAS[i][1][2]];
    int e0=BCOR[i][0], e1=BCOR[i][1];
    if(c0==e0 && c1==e1) return 0;
    if(c0==e1 && c1==e0) return 2;
    if(c0==e0 || c1==e1) return 1;
    return 3;
}

static int ec(const Cubo *c, int i) {
    int co[3], es[3];
    for(int k=0;k<3;k++){
        co[k]=c->face[CANTOS[i][k][0]][CANTOS[i][k][1]][CANTOS[i][k][2]];
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

float fitness(const Cubo *orig, const int *v, int len) {
    Cubo c = *orig;
    aplicar(&c, v, len);
    if(resolvido(&c)) return FIT_MAX;
    int s = 0;
    for(int i=0;i<12;i++) s += FB[eb(&c,i)];
    for(int i=0;i<8;i++)  s += FC[ec(&c,i)];
    int mx = 12*FB[0] + 8*FC[0];
    int mn = 12*FB[2] + 8*FC[2];
    float r = ((float)(s - mx) / (float)(mn - mx)) * 100.f;
    return r < 0 ? 0 : r > 99 ? 99 : r;
}

typedef struct { int *v; int len; float f; } Ind;

static unsigned int xorshift32(unsigned int *state) { unsigned int x = *state; if(x==0) x = 0x9e3779b1; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return *state = x; }

int cmp_ind(const void *a,const void *b){ const Ind *ia=a,*ib=b; if(ia->f < ib->f) return 1; if(ia->f>ib->f) return -1; return 0; }

void torneio(Ind *pop, int pop_size, int *pa, int *pb, unsigned int *seed) {
    int c[4]; for(int i=0;i<4;i++) c[i] = xorshift32(seed) % pop_size;
    int best1 = c[0], best2 = c[1];
    for(int i=0;i<4;i++){
        int idx = c[i];
        if(pop[idx].f > pop[best1].f){ best2 = best1; best1 = idx; }
        else if(pop[idx].f > pop[best2].f){ best2 = idx; }
    }
    *pa = best1; *pb = best2;
}

Ind cruzar(const Ind *p1, const Ind *p2, unsigned int *seed) {
    int t = (p1->len + p2->len) / 2; if(t<1) t=1;
    Ind f; f.len = t; f.v = malloc(t * sizeof(int));
    for(int i=0;i<t;i++){
        if((i%2)==0) f.v[i] = (i < p1->len) ? p1->v[i] : p1->v[xorshift32(seed) % p1->len];
        else f.v[i] = (i < p2->len) ? p2->v[i] : p2->v[xorshift32(seed) % p2->len];
    }
    f.f = 0;
    return f;
}

void mutar(Ind *ind, unsigned int *seed){ int pos = xorshift32(seed) % ind->len; ind->v[pos] = xorshift32(seed) % NUM_MOV; }

void embaralhar(Cubo *c, int n, unsigned seed){ unsigned int s = seed; for(int i=0;i<n;i++){ int m = xorshift32(&s) % NUM_MOV; aplicar_mov(c,m); } }

int main(int argc, char **argv) {
    int n_embaralha = (argc > 1) ? atoi(argv[1]) : 20;
    unsigned seed   = (argc > 2) ? (unsigned)atoi(argv[2]) : 42;
    if(n_embaralha < 1 || n_embaralha > 1000) { fprintf(stderr,"ERRO: n_embaralha deve ser entre 1 e 1000\n"); return 1; }

    fprintf(stderr, "=== SEQUENCIAL C ===\n");
    fprintf(stderr, "Pop: %d | Cromo: %d | Embaralha: %d | Seed: %u\n\n", TAM_POP, n_embaralha, n_embaralha, seed);

    Cubo cubo; cubo_init(&cubo); embaralhar(&cubo, n_embaralha, seed);

    Ind *pop = malloc(TAM_POP * sizeof(Ind));
    unsigned int s = seed;
    for(int i=0;i<TAM_POP;i++){
        pop[i].len = n_embaralha;
        pop[i].v = malloc(pop[i].len * sizeof(int));
        for(int j=0;j<pop[i].len;j++) pop[i].v[j] = xorshift32(&s) % NUM_MOV;
        pop[i].f = fitness(&cubo, pop[i].v, pop[i].len);
    }
    qsort(pop, TAM_POP, sizeof(Ind), cmp_ind);

    float taxa = TAX_MUT_INI;
    int estag = 0;
    float mg = -1;
    int g_conv = 0;

    for(int g = 1; estag < MAX_ESTAG; g++){
        if(pop[0].f >= FIT_MAX) { g_conv = g; break; }
        Ind *filhos = malloc(TAM_POP * sizeof(Ind));
        unsigned int s2 = seed ^ g;
        int idxf = 0;
        while(idxf < TAM_POP){
            int a,b; torneio(pop, TAM_POP, &a, &b, &s2);
            Ind f = cruzar(&pop[a], &pop[b], &s2);
            if(((float)(xorshift32(&s2) % 10000) / 10000.f) < taxa) mutar(&f, &s2);
            f.f = fitness(&cubo, f.v, f.len);
            filhos[idxf++] = f;
        }
        Ind *combined = malloc((2*TAM_POP)*sizeof(Ind));
        for(int i=0;i<TAM_POP;i++) combined[i]=pop[i];
        for(int i=0;i<TAM_POP;i++) combined[TAM_POP+i]=filhos[i];
        qsort(combined, 2*TAM_POP, sizeof(Ind), cmp_ind);
        for(int i=0;i<TAM_POP;i++){ pop[i]=combined[i]; }
        for(int i=TAM_POP;i<2*TAM_POP;i++) free(combined[i].v);
        free(combined); free(filhos);

        if(pop[0].f > mg) { mg = pop[0].f; estag = 0; taxa = TAX_MUT_INI; }
        else              { estag++; taxa += TAX_MUT_INC; }
        g_conv = g;
    }

    printf("RESULTADO,SequencialC,1,%d,%.4f,%s,%d\n", n_embaralha, pop[0].f, pop[0].f >= FIT_MAX ? "SIM" : "NAO", g_conv);
    fprintf(stderr, "\nMelhor fitness : %.2f/100\n", pop[0].f);
    fprintf(stderr, "Gerações       : %d\n", g_conv);
    fprintf(stderr, "Resolvido      : %s\n", pop[0].f >= FIT_MAX ? "SIM" : "NAO");

    for(int i=0;i<TAM_POP;i++) free(pop[i].v);
    free(pop);
    return 0;
}
