// SAE S3 Système mbash IUT CHARLEMAGNE 2025
// BAUDOIN Mathieu
// EYER Nathan

// Pour compiler : gcc mbash.c -o mbash
// Pour executer : ./mbash

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>



// Structure representant 1 commande entré par l'utilisateur
struct struct_commande {   
    char *commande;         /* la commande en string */
    int taille;               /* taille de la commande*/
    int cursor_pos;         /* position d'un curseur */
};

// Structure representant 1 token issu d'une commande (transformé en struct_commande au préalable)
struct struct_token {
    struct struct_commande *src;       /* source */
    int    taille;            /* longueur du string */
    char   *texte;               /* string du token */
};


// Fonctions sur struct_commande --------------------------------------------------------------------

#define ERRCHAR         ( 0)
#define EOF             (-1)

// Recule le curseur de 1
void reculer_cursor(struct struct_commande *src) {
    if(src->cursor_pos < 0) {
        return; }

    src->cursor_pos--;
}

// Returne le char actuel et avance le curseur de 1
char lire_char(struct struct_commande *src) {
    if(!src || !src->commande) {
        return ERRCHAR;
    }

    src->cursor_pos++ ;

    if(src->cursor_pos >= src->taille) {
        src->cursor_pos = src->taille;
        return EOF;
    }

    return src->commande[src->cursor_pos];
}

// Retourne le char actuel
char regarder_char(struct struct_commande *src) {
    if(!src || !src->commande) {
        return ERRCHAR;
    }

    int pos = src->cursor_pos;

    pos++;

    if(pos >= src->taille) {
        return EOF;
    }

    return src->commande[pos];
}


void sauter_espaces(struct struct_commande *src) {
    char c;

    if(!src || !src->commande) {
        return;
    }

    while( ((c = regarder_char(src)) != EOF) && (c == ' ' || c == '\t') ) {
        lire_char(src);
    }
}

// Fin fonctions sur struct_commande --------------------------------------------------------------------------------------------


// Fonctions sur struct_token ----------------------------------------------------------------------------------------------

char *token_buffer = NULL;
int   token_buffersize  = 0;
int   token_bufferindex = -1;

/* token End Of File */
struct struct_token eof_token =  {
    .taille = 0,
};


// Créer un token a partir d'un mot (une fonction ou un argument)
struct struct_token* creer_token(char *str) {

    struct struct_token *tok = malloc(sizeof(struct struct_token));
    tok->taille = strlen(str);
    tok->texte = str;
    
    return tok;
}

// Ajoute un caractere a token_buffer
void add_to_buf(char c) {
    token_buffer[token_bufferindex++] = c;

    if(token_bufferindex >= token_buffersize) {
        char* tmp = realloc(token_buffer, token_buffersize*2);

        token_buffer = tmp;
        token_buffersize *= 2;
    }
}

// Retourne 
struct struct_token* tokenize(struct struct_commande *src) {
    bool endloop = false;

    if(!src || !src->commande || !src->taille) {
        return &eof_token;
    }
    
    if(!token_buffer) {
        token_buffersize = 1024;
        token_buffer = malloc(token_buffersize);
    }

    token_bufferindex     = 0;
    token_buffer[0]       = '\0';

    char nc = lire_char(src);

    if(nc == EOF) return &eof_token;

    do {
        switch(nc) {
            case ' ':

            case '\t':
                if(token_bufferindex > 0) endloop = true;
                break;
                
            case '\n':
                if(token_bufferindex > 0) reculer_cursor(src);
                else add_to_buf(nc);

                endloop = 1;
                break;
                
            default:
                add_to_buf(nc);
                break;
        }
        if(endloop) break;

    } while((nc = lire_char(src)) != EOF);

    // if(token_bufferindex == 0) return &eof_token;
    // if(token_bufferindex >= token_buffersize) token_bufferindex--;

    token_buffer[token_bufferindex] = '\0';
    struct struct_token *tok = creer_token(token_buffer);
    tok->src = src;

    return tok;
}


// Partie sur le ABSTRACT SYNTAX TREE ------------------------------------------------------------------------------------------

enum node_type_e
{
    NODE_COMMAND,           /* simple command */
    NODE_VAR,               /* variable name (or simply, a word) */
};

enum val_type_e
{
    VAL_SINT = 1,       /* signed int */
    VAL_UINT,           /* unsigned int */
    VAL_SLLONG,         /* signed long long */
    VAL_ULLONG,         /* unsigned long long */
    VAL_FLOAT,          /* floating point */
    VAL_LDOUBLE,        /* long double */
    VAL_CHR,            /* char */
    VAL_STR,            /* str (char pointer) */
};

union symval_u
{
    long               sint;
    unsigned long      uint;
    long long          sllong;
    unsigned long long ullong;
    double             sfloat;
    long double        ldouble;
    char               chr;
    char              *str;
};

struct node_s {
    enum   node_type_e type;    /* type of this node */
    enum   val_type_e val_type; /* type of this node's val field */
    union  symval_u val;        /* value of this node */
    int    enfants;            /* number of child nodes */
    struct node_s *premier_enfant; /* first child node */
    struct node_s *next_frere, *prev_frere; /*
                                                 * if this is a child node, keep
                                                 * pointers to prev/next freres
                                                 */
};

// Creer un nouveau node et le retourne
struct node_s *new_node(enum node_type_e type) {
    struct node_s *node = malloc(sizeof(struct node_s));
    node->type = type;
    return node;
}

// Ajoute un noeuad enfant a un noeud parent
void add_child_node(struct node_s *parent, struct node_s *enfant) {

    if(!parent || !enfant) return;


    if(!parent->premier_enfant) parent->premier_enfant = enfant;
    else {
        struct node_s *frere = parent->premier_enfant;
    
        while(frere->next_frere) frere = frere->next_frere;

        frere->next_frere = enfant;
        enfant->prev_frere = frere;
    }

    parent->enfants++;
}


// Setter
void set_valeur_node(struct node_s *node, char *val) {

    node->val_type = VAL_STR;
    if(!val) node->val.str = NULL;
    else {
        char *val2 = malloc(strlen(val)+1);
        strcpy(val2, val);
        node->val.str = val2;
    }
}

// Prend une commande et retourne un arbre AST
struct node_s* parse_command(struct struct_token *token){
    if(!token) return NULL; //Verifie qu'il y a bien un parametre

    struct node_s *cmd = new_node(NODE_COMMAND);
    if(!cmd) return NULL; //Verifie qu'il y a bien un cmd

    struct struct_commande *src = token->src; //Assure un meme flux de donnees

    do{
        if(token->texte[0] == '\n') break; //Aucune action si retour a la ligne

        struct node_s *word = new_node(NODE_VAR);
        if(!word) return NULL; //Verifie la presence d'une expression

        set_valeur_node(word, token->texte);
        add_child_node(cmd, word);

    } while((token = tokenize(src)) != &eof_token);
    return cmd;
}

// Cherche la path d'un fichier    A NETTOYER
char* search_path(char* file)
{
    char *PATH = getenv("PATH");
    char *p    = PATH;
    char *p2;
    
    while(p && *p)
    {
        p2 = p;

        while(*p2 && *p2 != ':')
        {
            p2++;
        }
        
    int  plen = p2-p;
        if(!plen)
        {
            plen = 1;
        }
        
        int  alen = strlen(file);
        char path[plen+1+alen+1];
        
    strncpy(path, p, p2-p);
        path[p2-p] = '\0';
        
    if(p2[-1] != '/')
        {
            strcat(path, "/");
        }

        strcat(path, file);
        
    struct stat st;
        if(stat(path, &st) == 0)
        {
            if(!S_ISREG(st.st_mode))
            {
                errno = ENOENT;
                p = p2;
                if(*p2 == ':')
                {
                    p++;
                }
                continue;
            }

            p = malloc(strlen(path)+1);
            if(!p)
            {
                return NULL;
            }
            
        strcpy(p, path);
            return p;
        }
        else    /* file not found */
        {
            p = p2;
            if(*p2 == ':')
            {
                p++;
            }
        }
    }

    errno = ENOENT;
    return NULL;
}

// execute une commande comme si entré dans un shell
int do_exec_cmd(int argc, char **argv)
{
    if(strchr(argv[0], '/'))
    {
        execv(argv[0], argv);
    }
    else
    {
        char *path = search_path(argv[0]);
        if(!path)
        {
            return 0;
        }
        execv(path, argv);
        free(path);
    }
    return 0;
}

// Prend un noeud parent et l'execute
int do_simple_command(struct node_s *node) {

    if(!node) return 0;

    struct node_s *child = node->premier_enfant;

    if(!child) return 0;
    
    int argc = 0;
    long max_args = 255;
    char *argv[max_args+1];     /* keep 1 for the terminating NULL arg */
    char *str;
    
    while(child) {
        str = child->val.str;
        argv[argc] = malloc(strlen(str)+1);
        
        if(!argv[argc]) return 0;

        
        strcpy(argv[argc], str);
        if(++argc >= max_args) break;

        child = child->next_frere;
    }
    argv[argc] = NULL;

    pid_t child_pid = 0;
    if((child_pid = fork()) == 0) {
        do_exec_cmd(argc, argv);
        fprintf(stderr, "error: failed to execute command: %s\n", strerror(errno));
        if(errno == ENOEXEC)
        {
            exit(126);
        }
        else if(errno == ENOENT)
        {
            exit(127);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }
    else if(child_pid < 0)
    {
        fprintf(stderr, "error: failed to fork command: %s\n", strerror(errno));
        return 0;
    }

    int status = 0;
    waitpid(child_pid, &status, 0);
    
    return 1;
}


// Prend une commande (struct_commande) et l'execute
int parse_and_execute(struct struct_commande *src)
{
    sauter_espaces(src);

    struct struct_token *tok = tokenize(src);

    if(tok == &eof_token)
    {
        return 0;
    }

    while(tok && tok != &eof_token)
    {
        struct node_s *cmd = parse_command(tok);

        if(!cmd)
        {
            break;
        }

        do_simple_command(cmd);
        tok = tokenize(src);
    }

    return 1;
}

// MAIN ----------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    char cmd[1024];

    while(1) {
        fprintf(stderr, "$ ");

        fgets(cmd, sizeof(cmd), stdin);

        if(cmd[0] == '\0' || strcmp(cmd, "\n") == 0) continue;

        if(strcmp(cmd, "exit\n") == 0) break;

        if(strncmp(cmd, "cd ", 3) == 0){
            cmd[strlen(cmd)-1] = '\0';
            if(chdir(cmd+3) != 0) perror("cd");
            continue;
        }

        struct struct_commande src;
        src.commande   = cmd;
        src.taille  = strlen(cmd);
        src.cursor_pos   = -1 ;
        parse_and_execute(&src);
    };

    exit(EXIT_SUCCESS);
}