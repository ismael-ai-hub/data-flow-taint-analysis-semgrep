char *read_user_input(void);
void execute_sql(const char *query);

void test_direct(void)
{
    execute_sql(read_user_input());
}
