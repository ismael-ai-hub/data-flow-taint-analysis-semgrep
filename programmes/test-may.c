char *read_user_input(void);
void execute_sql(const char *query);

void test_may(int autorise)
{
    const char *query = "SELECT * FROM users";

    if (!autorise) {
        query = read_user_input();
    }

    execute_sql(query);
}
