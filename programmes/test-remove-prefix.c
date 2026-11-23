char *read_user_input(void);
char *remove_prefix(const char *input);
void execute_sql(const char *query);

void test_remove_prefix(void)
{
    char *input;

    /* n1 : acquisition de la donnee non fiable */
    input = read_user_input();

    /* n2 : transformation sans garantie de securite */
    input = remove_prefix(input);

    /* n3 : utilisation de la donnee transformee */
    execute_sql(input);
}
