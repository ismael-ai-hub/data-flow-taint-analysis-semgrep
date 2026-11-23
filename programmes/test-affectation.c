char *read_user_input(void);
void execute_sql(const char *query);

void test_affectation(void)
{
    char *input;
    char *query;

    input = read_user_input();
    query = input;

    execute_sql(query);
}
