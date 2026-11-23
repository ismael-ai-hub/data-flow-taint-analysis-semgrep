char *read_user_input(void);
char *escape_sql(const char *input);
void execute_sql(const char *query);

void test_sanitizer(void)
{
    char *input;
    char *safe_input;

    input = read_user_input();
    safe_input = escape_sql(input);

    execute_sql(safe_input);
}
