#include <stdio.h>
#include <stdlib.h>

/* Source : lecture d'une donnee non fiable. */
char *read_user_input(void);

/* Sink : execution d'une requete SQL. */
void execute_sql(const char *query);

void process_request(void)
{
    char *input;
    char query[256];

    /* n1 : acquisition de la donnee utilisateur */
    input = read_user_input();

    /* n2 : construction de la requete SQL */
    snprintf(query, sizeof(query),
             "SELECT * FROM users WHERE name = '%s'",
             input);

    /* n3 : execution de la requete */
    execute_sql(query);
}
