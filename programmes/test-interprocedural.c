char *read_user_input(void);
void execute_sql(const char *query);

void envoyer_requete(const char *query)
{
    execute_sql(query);
}

void traiter(void)
{
    char *input = read_user_input();
    envoyer_requete(input);
}
