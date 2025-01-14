// SAE S3 Système mbash IUT CHARLEMAGNE 2025
// BAUDOIN Mathieu
// EYER Nathan

// Pour compiler : gcc mbash.c -o mbash
// Pour executer : ./mbash

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
    char cmd[1024];

    while(1) {
        fprintf(stderr, "$ ");

        fgets(cmd, sizeof(cmd), stdin);

        if(cmd[0] == '\0' || strcmp(cmd, "\n") == 0) {
            continue;
        }

        if(strcmp(cmd, "exit\n") == 0) {
            break;
        }

        printf("%s", cmd);
    };

    exit(EXIT_SUCCESS);
}



// Structure representant une commande entré par l'utilisateur
struct struct_commande {   
    char *commande;         /* la commande en string */
    int size;               /* taille de la commande*/
    int cursor_pos;         /* position d'un curseur */
};

// Fonctions sur struc_commande --------------------------------------------------------------------

#define ERRCHAR         ( 0)
#define EOF             (-1)

// Recule le curseur de 1
void reculer_cursor(struct source_commande *src) {
    if(src->cursor_pos < 0) {
        return; }

    src->cursor_pos--;
}

// Returne le char actuel et avance le curseur de 1
char lire_char(struct source_commande *src) {
    if(!src || !src->commande) {
        return ERRCHAR;
    }

    src->cursor_pos++ ;

    if(src->cursor_pos >= src->size) {
        src->cursor_pos = src->size;
        return EOF;
    }

    return src->commande[src->cursor_pos];
}

// Retourne le char actuel
char regarder_char(struct source_commande *src) {
    if(!src || !src->commande) {
        return ERRCHAR;
    }

    int pos = src->cursor_pos;

    pos++;

    if(pos >= src->size)
    {
        return EOF;
    }

    return src->commande[pos];
}


void sauter_espaces(struct source_commande *src) {
    char c;

    if(!src || !src->commande) {
        return;
    }

    while( ((c = peek_char(src)) != EOF) && (c == ' ' || c == '\t') ) {
        lire_char(src);
    }
}

// Fin fonctions sur struct_commande --------------------------------------------------------------------------------------------

// Structure representant 1 token issu d'une commande (transformé en source au préalable)
struct token_s {
    struct source_commande *src;       /* source */
    int    text_len;            /* longueur du string */
    char   *text;               /* string du token */
};



char *tok_buf = NULL;
int   tok_bufsize  = 0;
int   tok_bufindex = -1;

/* special token to indicate end of input */
struct token_s eof_token = 
{
    .text_len = 0,
};


void add_to_buf(char c) {
    tok_buf[tok_bufindex++] = c;

    if(tok_bufindex >= tok_bufsize)
    {
        char *tmp = realloc(tok_buf, tok_bufsize*2);

        if(!tmp)
        {
            errno = ENOMEM;
            return;
        }

        tok_buf = tmp;
        tok_bufsize *= 2;
    }
}


struct token_s *create_token(char *str) {
    struct token_s *tok = malloc(sizeof(struct token_s));
    
    if(!tok)
    {
        return NULL;
    }

    memset(tok, 0, sizeof(struct token_s));
    tok->text_len = strlen(str);
    
    char *nstr = malloc(tok->text_len+1);
    
    if(!nstr)
    {
        free(tok);
        return NULL;
    }
    
    strcpy(nstr, str);
    tok->text = nstr;
    
    return tok;
}


void free_token(struct token_s *tok) {
    if(tok->text) {
        free(tok->text);
    }
    free(tok);
}


struct token_s *tokenize(struct source_commande *src) {
    int  endloop = 0;

    if(!src || !src->buffer || !src->bufsize)
    {
        errno = ENODATA;
        return &eof_token;
    }
    
    if(!tok_buf)
    {
        tok_bufsize = 1024;
        tok_buf = malloc(tok_bufsize);
        if(!tok_buf)
        {
            errno = ENOMEM;
            return &eof_token;
        }
    }

    tok_bufindex     = 0;
    tok_buf[0]       = '\0';

    char nc = next_char(src);

    if(nc == ERRCHAR || nc == EOF)
    {
        return &eof_token;
    }

    do
    {
        switch(nc)
        {
            case ' ':
            case '\t':
                if(tok_bufindex > 0)
                {
                    endloop = 1;
                }
                break;
                
            case '\n':
                if(tok_bufindex > 0)
                {
                    unget_char(src);
                }
                else
                {
                    add_to_buf(nc);
                }
                endloop = 1;
                break;
                
            default:
                add_to_buf(nc);
                break;
        }

        if(endloop)
        {
            break;
        }

    } while((nc = next_char(src)) != EOF);

    if(tok_bufindex == 0)
    {
        return &eof_token;
    }
    
    if(tok_bufindex >= tok_bufsize)
    {
        tok_bufindex--;
    }
    tok_buf[tok_bufindex] = '\0';

    struct token_s *tok = create_token(tok_buf);
    if(!tok)
    {
        fprintf(stderr, "error: failed to alloc buffer: %s\n", strerror(errno));
        return &eof_token;
    }

    tok->src = src;
    return tok;
}

struct node_s *parse_command(struct token_s *token){
    if(!token) return NULL; //Verifie qu'il y a bien un parametre

    struct node_s *cmd = new_node(NODE_COMMAND);
    if(!cmd) return NULL; //Verifie qu'il y a bien un cmd

    struct source_s *src = token->src; //Assure un meme flux de donnees

    do{
        if(token->text[0] == '\n') break; //Aucune action si retour a la ligne

        struct node_s *word = new_node(NODE_VAR);
        if(!word) return NULL; //Verifie la presence d'une expression

        set_node_val_str(word, token->text);
        add_child_node(cmd, word);

    } while((token = tokenize(src)) != &eof_token);
    return cmd;
}