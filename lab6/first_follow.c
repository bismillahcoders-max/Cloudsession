/*
 * Lab 6 - FIRST & FOLLOW Sets
 * Compiler Construction, Forman Christian College
 *
 * Computes and displays the FIRST and FOLLOW set of every non-terminal
 * for one of five hardcoded grammars selected by the user.
 *
 * Grammar notation used in the tables below:
 *   - symbols are separated by spaces, so multi-character symbols such
 *     as "id" and "E'" are single tokens
 *   - alternatives are separated by '|'
 *   - "ε" (or "#") stands for the empty string
 *   - any symbol that appears on a left-hand side is a non-terminal,
 *     every other symbol is a terminal
 *   - the left-hand side of the first rule is the start symbol
 *
 * Build: cc -std=c99 -Wall -Wextra -o first_follow first_follow.c
 * Run:   ./first_follow
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 64
#define MAX_PRODS   64
#define MAX_RHS     16
#define MAX_NAME    16
#define MAX_LINE    256
#define NUM_GRAMMARS 5

#define EPSILON    "ε"
#define END_MARKER "$"

typedef struct {
    char name[MAX_NAME];
    int  isTerminal;
} Symbol;

/* One alternative of a rule, e.g. E' -> + T E' */
typedef struct {
    int lhs;            /* index of the left-hand non-terminal            */
    int rhs[MAX_RHS];   /* indices of the right-hand symbols              */
    int len;            /* number of right-hand symbols, 0 means A -> ε   */
} Production;

/* A set of symbol indices that remembers insertion order for printing */
typedef struct {
    int items[MAX_SYMBOLS];
    int count;
} Set;

typedef struct {
    const char *rules[8];   /* NULL-terminated list of rules */
} Grammar;

static const Grammar GRAMMARS[NUM_GRAMMARS] = {
    { { "E -> T E'", "E' -> + T E' | ε", "T -> F T'", "T' -> * F T' | ε",
        "F -> ( E ) | id", NULL } },
    { { "S -> A B", "A -> a A | ε", "B -> b B | c", NULL } },
    { { "S -> ( S ) S | ε", NULL } },
    { { "S -> a B | b A", "A -> c A | a", "B -> d B | b", NULL } },
    { { "S -> A a | b", "A -> c A | ε", NULL } },
};

static Symbol     symbols[MAX_SYMBOLS];
static int        numSymbols;
static Production prods[MAX_PRODS];
static int        numProds;
static int        startSymbol;
static int        epsilonIdx;
static int        endIdx;

static Set first[MAX_SYMBOLS];
static Set follow[MAX_SYMBOLS];

/* Per-pass recursion state: 0 = not visited, 1 = on the call stack, 2 = done */
static int state[MAX_SYMBOLS];
/* Set to 1 whenever any FIRST or FOLLOW set grows during a pass */
static int changed;

/* ---------------------------------------------------------------- sets */

static int setContains(const Set *s, int sym)
{
    for (int i = 0; i < s->count; i++)
        if (s->items[i] == sym)
            return 1;
    return 0;
}

static void setAdd(Set *s, int sym)
{
    if (!setContains(s, sym)) {
        s->items[s->count++] = sym;
        changed = 1;
    }
}

/* dst = dst U (src - {ε}) */
static void setUnionNoEpsilon(Set *dst, const Set *src)
{
    for (int i = 0; i < src->count; i++)
        if (src->items[i] != epsilonIdx)
            setAdd(dst, src->items[i]);
}

static void printSet(const Set *s)
{
    printf("{ ");
    for (int i = 0; i < s->count; i++)
        printf("%s%s", i ? ", " : "", symbols[s->items[i]].name);
    printf(" }\n");
}

/* ------------------------------------------------- grammar tokenizing */

static int findSymbol(const char *name)
{
    for (int i = 0; i < numSymbols; i++)
        if (strcmp(symbols[i].name, name) == 0)
            return i;
    return -1;
}

static int addSymbol(const char *name, int isTerminal)
{
    int idx = findSymbol(name);
    if (idx >= 0)
        return idx;
    if (numSymbols == MAX_SYMBOLS) {
        fprintf(stderr, "Too many symbols\n");
        exit(1);
    }
    snprintf(symbols[numSymbols].name, MAX_NAME, "%s", name);
    symbols[numSymbols].isTerminal = isTerminal;
    return numSymbols++;
}

/* Splits "A -> X Y | Z" into one Production per alternative */
static void parseRule(const char *rule)
{
    char buf[MAX_LINE];
    char name[MAX_NAME];
    snprintf(buf, sizeof buf, "%s", rule);

    char *arrow = strstr(buf, "->");
    if (arrow == NULL) {
        fprintf(stderr, "Bad rule (missing ->): %s\n", rule);
        exit(1);
    }
    *arrow = '\0';
    sscanf(buf, "%15s", name);
    int lhs = findSymbol(name);

    char *alt = arrow + 2;
    while (alt != NULL) {
        char *bar = strchr(alt, '|');
        if (bar != NULL)
            *bar = '\0';

        if (numProds == MAX_PRODS) {
            fprintf(stderr, "Too many productions\n");
            exit(1);
        }
        Production *p = &prods[numProds++];
        p->lhs = lhs;
        p->len = 0;

        char *s = alt;
        int n;
        while (sscanf(s, "%15s%n", name, &n) == 1) {
            s += n;
            if (strcmp(name, EPSILON) == 0 || strcmp(name, "#") == 0)
                continue;               /* ε contributes no symbols */
            if (p->len == MAX_RHS) {
                fprintf(stderr, "Production too long: %s\n", rule);
                exit(1);
            }
            /* Not a left-hand side anywhere, so it is a terminal */
            p->rhs[p->len++] = addSymbol(name, 1);
        }
        alt = (bar != NULL) ? bar + 1 : NULL;
    }
}

static void loadGrammar(const Grammar *g)
{
    char name[MAX_NAME];

    numSymbols = 0;
    numProds = 0;
    memset(first, 0, sizeof first);
    memset(follow, 0, sizeof follow);

    epsilonIdx = addSymbol(EPSILON, 1);
    endIdx     = addSymbol(END_MARKER, 1);

    /* Pass 1: every left-hand side is a non-terminal */
    for (int i = 0; g->rules[i] != NULL; i++) {
        sscanf(g->rules[i], "%15s", name);
        addSymbol(name, 0);
    }
    sscanf(g->rules[0], "%15s", name);
    startSymbol = findSymbol(name);

    /* Pass 2: tokenize right-hand sides */
    for (int i = 0; g->rules[i] != NULL; i++)
        parseRule(g->rules[i]);
}

/* ------------------------------------------------------------- FIRST */

static void computeFirst(int X)
{
    /* Rule 1: FIRST(a) = { a } for a terminal a */
    if (symbols[X].isTerminal) {
        setAdd(&first[X], X);
        return;
    }
    /* Already finished this pass, or X is on the call stack (recursion) */
    if (state[X] != 0)
        return;
    state[X] = 1;

    for (int i = 0; i < numProds; i++) {
        if (prods[i].lhs != X)
            continue;

        /* Rule 2: X -> Y1 Y2 ... Yk */
        int allNullable = 1;
        for (int j = 0; j < prods[i].len; j++) {
            int Y = prods[i].rhs[j];
            computeFirst(Y);
            setUnionNoEpsilon(&first[X], &first[Y]);
            if (!setContains(&first[Y], epsilonIdx)) {
                allNullable = 0;
                break;
            }
        }
        /* X -> ε, or every Yi can derive ε */
        if (allNullable)
            setAdd(&first[X], epsilonIdx);
    }
    state[X] = 2;
}

/* Repeats passes until no FIRST set changes, so results are complete
 * even when a recursive call had to stop at a symbol on the stack. */
static void computeAllFirst(void)
{
    do {
        changed = 0;
        memset(state, 0, sizeof state);
        /* Terminals too, since FOLLOW reads FIRST(β) for any symbol in β */
        for (int i = 0; i < numSymbols; i++)
            computeFirst(i);
    } while (changed);
}

/* ------------------------------------------------------------ FOLLOW */

static void computeFollow(int B)
{
    if (state[B] != 0)
        return;
    state[B] = 1;

    /* Rule 1: $ is in FOLLOW(S) for the start symbol S */
    if (B == startSymbol)
        setAdd(&follow[B], endIdx);

    /* Rule 2: look at every occurrence A -> α B β */
    for (int i = 0; i < numProds; i++) {
        const Production *p = &prods[i];
        for (int j = 0; j < p->len; j++) {
            if (p->rhs[j] != B)
                continue;

            /* Add FIRST(β) - {ε} */
            int betaNullable = 1;
            for (int k = j + 1; k < p->len; k++) {
                int Y = p->rhs[k];
                setUnionNoEpsilon(&follow[B], &first[Y]);
                if (!setContains(&first[Y], epsilonIdx)) {
                    betaNullable = 0;
                    break;
                }
            }
            /* β is empty or β =>* ε: add FOLLOW(A) */
            if (betaNullable && p->lhs != B) {
                computeFollow(p->lhs);
                setUnionNoEpsilon(&follow[B], &follow[p->lhs]);
            }
        }
    }
    state[B] = 2;
}

static void computeAllFollow(void)
{
    do {
        changed = 0;
        memset(state, 0, sizeof state);
        for (int i = 0; i < numSymbols; i++)
            if (!symbols[i].isTerminal)
                computeFollow(i);
    } while (changed);
}

/* ----------------------------------------------------------- display */

static void printGrammar(void)
{
    for (int a = 0; a < numSymbols; a++) {
        if (symbols[a].isTerminal)
            continue;
        printf("  %-3s ->", symbols[a].name);
        int alt = 0;
        for (int i = 0; i < numProds; i++) {
            if (prods[i].lhs != a)
                continue;
            if (alt++)
                printf(" |");
            if (prods[i].len == 0)
                printf(" %s", EPSILON);
            for (int j = 0; j < prods[i].len; j++)
                printf(" %s", symbols[prods[i].rhs[j]].name);
        }
        printf("\n");
    }
}

static void printSymbols(int terminals)
{
    int n = 0;
    printf("{ ");
    for (int i = 0; i < numSymbols; i++) {
        if (i == epsilonIdx || i == endIdx || symbols[i].isTerminal != terminals)
            continue;
        printf("%s%s", n++ ? ", " : "", symbols[i].name);
    }
    printf(" }\n");
}

static void printMenu(void)
{
    printf("==============================================\n");
    printf("   Lab 6 - FIRST & FOLLOW Sets Calculator\n");
    printf("==============================================\n");
    for (int g = 0; g < NUM_GRAMMARS; g++) {
        printf("Grammar %d:  ", g + 1);
        for (int i = 0; GRAMMARS[g].rules[i] != NULL; i++)
            printf("%s%s", i ? ";  " : "", GRAMMARS[g].rules[i]);
        printf("\n");
    }
    printf("\n");
}

static int readChoice(void)
{
    char line[MAX_LINE];
    for (;;) {
        printf("Select Grammar (1-%d): ", NUM_GRAMMARS);
        fflush(stdout);
        if (fgets(line, sizeof line, stdin) == NULL)
            return -1;
        char *end;
        long choice = strtol(line, &end, 10);
        if (end != line && choice >= 1 && choice <= NUM_GRAMMARS)
            return (int)choice;
        printf("Invalid choice. Enter a number from 1 to %d.\n", NUM_GRAMMARS);
    }
}

int main(void)
{
    printMenu();
    int choice = readChoice();
    if (choice < 0)
        return 1;

    loadGrammar(&GRAMMARS[choice - 1]);
    computeAllFirst();
    computeAllFollow();

    printf("\nGrammar %d:\n", choice);
    printGrammar();
    printf("\nNon-terminals : ");
    printSymbols(0);
    printf("Terminals     : ");
    printSymbols(1);
    printf("Start symbol  : %s\n", symbols[startSymbol].name);

    printf("\n--- FIRST sets ---\n");
    for (int i = 0; i < numSymbols; i++) {
        if (symbols[i].isTerminal)
            continue;
        printf("FIRST(%s) = ", symbols[i].name);
        printSet(&first[i]);
    }

    printf("\n--- FOLLOW sets ---\n");
    for (int i = 0; i < numSymbols; i++) {
        if (symbols[i].isTerminal)
            continue;
        printf("FOLLOW(%s) = ", symbols[i].name);
        printSet(&follow[i]);
    }
    return 0;
}
