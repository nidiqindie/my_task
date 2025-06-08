#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#define MAX_TOKENS 500
#define MAX_TOKEN_LEN 50
#define if_only_one_error 0
char A = 0;
int sum_error_num = 0;
typedef struct
{
    char type[MAX_TOKEN_LEN];
    char value[MAX_TOKEN_LEN];
} Token;

Token tokens[MAX_TOKENS];
int token_count = 0;
int current_token_index = 0;

// 函数声明
void program();
void block();
void const_declaration();
void const_definition();
void var_declaration();
void procedure_declaration();
void procedure_header();
int statement();
void assignment_statement();
void compound_statement();
void condition();
void expression();
void term();
void factor();
void condition_statement();
void call_statement();
void while_statement();
void read_statement();
void write_statement();
int check(const char *type, const char *value);
int check_type(const char *type);
// 辅助函数
void match(const char *expected_type, const char *expected_value)
{
    if (current_token_index >= token_count)
    {
        fprintf(stderr, "语法错误: 预期 %s %s，但遇到文件结束\n", expected_type, expected_value);
#if if_only_one_error
        exit(1);
#else
        A = 1;
        sum_error_num++;
        return;
#endif
    }

    Token current = tokens[current_token_index];
    if (strcmp(current.type, expected_type) != 0 ||
        (expected_value != NULL && strcmp(current.value, expected_value) != 0))
    {
        fprintf(stderr, "语法错误: 第 %d 个token，预期 %s %s，但遇到 %s %s\n",
                current_token_index + 1, expected_type, expected_value, current.type, current.value);
#if if_only_one_error
        exit(1);
#else
        A = 1;
        sum_error_num++;
        if (check("运算符", ":="))
        {
            current_token_index++;
        }
        if (strcmp(":=", expected_value) == 0)
        {
            if (strcmp("=", current.value) == 0)
            {
                current_token_index++;
            }
        }

        return;
#endif
    }
    current_token_index++;
}

void match_type(const char *expected_type)
{
    if (current_token_index >= token_count)
    {
        fprintf(stderr, "语法错误: 预期 %s，但遇到文件结束\n", expected_type);
#if if_only_one_error
        exit(1);
#else
        A = 1;
        sum_error_num++;
        return;
#endif
    }

    Token current = tokens[current_token_index];
    if (strcmp(current.type, expected_type) != 0)
    {
        fprintf(stderr, "语法错误: 第 %d 个token，预期 %s，但遇到 %s %s\n",
                current_token_index + 1, expected_type, current.type, current.value);
#if if_only_one_error
        exit(1);
#else
        A = 1;
        sum_error_num++;
        return;
#endif
    }
    current_token_index++;
}

int check(const char *type, const char *value)
{
    if (current_token_index >= token_count)
        return 0;
    Token current = tokens[current_token_index];
    return strcmp(current.type, type) == 0 && (value == NULL || strcmp(current.value, value) == 0);
}

int check_type(const char *type)
{
    if (current_token_index >= token_count)
        return 0;
    return strcmp(tokens[current_token_index].type, type) == 0;
}
// 检查上一个token是否匹配指定类型和值
int check_prev_token(const char *type, const char *value)
{
    if (current_token_index - 1 >= 0)
    {
        Token prev = tokens[current_token_index - 1];
        return strcmp(prev.type, type) == 0 && strcmp(prev.value, value) == 0;
    }
    return 0;
}
void load_tokens(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("打开文件失败");
        exit(1);
    }

    char line[100];
    while (fgets(line, sizeof(line), file))
    {
        if (line[0] != '(')
            continue;

        char *type_start = strchr(line, ',') - 1;
        char *value_end = strrchr(line, ')');

        if (!type_start || !value_end)
        {
            fprintf(stderr, "无效的token格式: %s", line);
            continue;
        }

        // 提取类型
        char *comma = strchr(line, ',');
        *comma = '\0';
        char *type = line + 1;

        // 提取值
        char *value = comma + 1;
        *value_end = '\0';

        // 去除值中的空格
        if (value[0] == ' ')
            value++;

        // 存储token
        strncpy(tokens[token_count].type, type, MAX_TOKEN_LEN - 1);
        strncpy(tokens[token_count].value, value, MAX_TOKEN_LEN - 1);
        tokens[token_count].type[MAX_TOKEN_LEN - 1] = '\0';
        tokens[token_count].value[MAX_TOKEN_LEN - 1] = '\0';

        token_count++;
        if (token_count >= MAX_TOKENS)
        {
            fprintf(stderr, "超出最大token数量限制\n");
            break;
        }
    }
    fclose(file);
}

// 语法规则实现
void program()
{
    block();
    match("界符", ".");
}

void block()
{
    if (check("保留字", "const"))
    {
        const_declaration();
    }
    if (check("保留字", "var"))
    {
        var_declaration();
    }
    while (check("保留字", "procedure"))
    {
        procedure_declaration();
    }
    statement();
}

void const_declaration()
{
    match("保留字", "const");
    const_definition();
    while (check("界符", ","))
    {
        match("界符", ",");
        const_definition();
    }
    match("界符", ";");
}

void const_definition()
{
    match_type("标识符");
    match("运算符", "=");
    match_type("无符号整数");
}

void var_declaration()
{
    match("保留字", "var");
    match_type("标识符");
    while (check("界符", ","))
    {
        match("界符", ",");
        match_type("标识符");
        if (check_type("标识符"))
        {
            match("界符", ",");
            current_token_index++;
        }
    }
    match("界符", ";");
}

void procedure_declaration()
{
    procedure_header();
    block();
    match("界符", ";");
}

void procedure_header()
{
    match("保留字", "procedure");
    int a = 0;
    if (check("保留字", "procedure"))
    {
        a = sum_error_num;
        match_type("标识符");
        current_token_index = current_token_index + (sum_error_num - a);
        a = 0;
    }

    while (check_type("标识符"))
    {
        if (a == 0)
        {
            a = sum_error_num;
            match_type("标识符");
            current_token_index = current_token_index + (sum_error_num - a);
            a = 1;
        }
        else
        {
            match("界符", ";");
            current_token_index++;
        }
    }

    match("界符", ";");
}
// 过程函数，返回值表示后面本应不应该有分号，0为不应该，1为应该。
int statement()
{
    if (check_type("标识符"))
    {
        assignment_statement();
        return 1;
    }
    else if (check("保留字", "if"))
    {
        condition_statement();
        return 0;
    }
    else if (check("保留字", "while"))
    {
        while_statement();
        return 0;
    }
    else if (check("保留字", "call"))
    {
        call_statement();
        return 1;
    }
    else if (check("保留字", "read"))
    {
        read_statement();
        return 1;
    }
    else if (check("保留字", "write"))
    {
        write_statement();
        return 1;
    }
    else if (check("保留字", "begin"))
    {
        compound_statement();
        return 0;
    }
    // 空语句不做任何处理
}

void assignment_statement()
{
    match_type("标识符");
    match("运算符", ":=");
    expression();
}

void compound_statement()
{
    // match("保留字", "begin");
    // statement();
    // while (check("界符", ";"))
    // {
    //     match("界符", ";");
    //     statement();
    // }
    // match("保留字", "end");

    match("保留字", "begin");
    // 在当前语句后检查是否还有下一条语句
    statement();
    while (1)
    {
        if (check("界符", ";"))
        {
            // 如果有分号，继续解析下一条语句
            match("界符", ";");
            statement();
        }
        else if (check("保留字", "end"))
        {
            // 如果是最后一个语句，前面不能有分号
            if (!check_prev_token("界符", ";") && !check_prev_token("保留字", "end"))
            {
                printf("语法错误: 缺少分号\n");
                sum_error_num++;
            }
            match("保留字", "end");
            break;
        }
        else
        {
            // 如果没有分号且不是end，可以考虑允许继续解析下一条语句（容错处理）
            // 或者报告错误并尝试恢复
            printf("语法错误: 期待 ';' 或 'end'，但遇到 '%s %s'\n",
                   tokens[current_token_index].type, tokens[current_token_index].value);
            sum_error_num++;
            // 尝试找到下一个可能的语句开始点
            while (current_token_index < token_count &&
                   !check_type("保留字") &&
                   strcmp(tokens[current_token_index].value, "end") != 0)
            {
                current_token_index++;
            }
            if (current_token_index < token_count &&
                strcmp(tokens[current_token_index].value, "end") == 0)
            {
                match("保留字", "end");
                break;
            }
            else
            {
                // 如果没有找到end，继续解析下一条语句
                statement();
            }
        }
    }
}

void condition()
{
    if (check("保留字", "odd"))
    {
        match("保留字", "odd");
        expression();
    }
    else
    {
        expression();
        if (check("运算符", "=") || check("运算符", "#") ||
            check("运算符", "<") || check("运算符", "<=") ||
            check("运算符", ">") || check("运算符", ">="))
        {
            current_token_index++;
        }
        else
        {
            fprintf(stderr, "语法错误: 预期关系运算符\n");
#if if_only_one_error
            exit(1);
#else
            A = 1;
            sum_error_num++;
#endif
        }
        expression();
    }
}

void expression()
{
    if (check("运算符", "+") || check("运算符", "-"))
    {
        current_token_index++;
    }
    term();
    while (check("运算符", "+") || check("运算符", "-"))
    {
        current_token_index++;
        term();
    }
}

void term()
{
    factor();
    while (check("运算符", "*") || check("运算符", "/"))
    {
        current_token_index++;
        factor();
    }
}

void factor()
{
    if (check_type("标识符"))
    {
        match_type("标识符");
    }
    else if (check_type("无符号整数"))
    {
        match_type("无符号整数");
    }
    else if (check("界符", "("))
    {
        match("界符", "(");
        expression();
        match("界符", ")");
    }
    else
    {
        fprintf(stderr, "语法错误: 预期标识符、无符号整数或表达式\n");
#if if_only_one_error
        exit(1);
#else
        A = 1;
        sum_error_num++;
#endif
    }
}

void condition_statement()
{
    match("保留字", "if");
    condition();
    match("保留字", "then");
    statement();
}

void call_statement()
{
    match("保留字", "call");
    match_type("标识符");
}

void while_statement()
{
    match("保留字", "while");
    condition();
    match("保留字", "do");
    statement();
}

void read_statement()
{
    match("保留字", "read");
    match("界符", "(");
    match_type("标识符");
    while (check("界符", ","))
    {
        match("界符", ",");
        match_type("标识符");
    }
    match("界符", ")");
}

void write_statement()
{
    match("保留字", "write");
    match("界符", "(");
    expression();
    while (check("界符", ","))
    {
        match("界符", ",");
        expression();
    }
    match("界符", ")");
}

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    if (argc != 2)
    {
        fprintf(stderr, "用法: %s <token文件>\n", argv[0]);
        return 1;
    }
    printf("%s", argv[1]);
    load_tokens(argv[1]);
    program();

    if (current_token_index < token_count)
    {
        Token extra = tokens[current_token_index];
        fprintf(stderr, "语法错误: 分析结束后还有未处理的token: %s %s\n", extra.type, extra.value);
        sum_error_num++;
    }
    if (A == 0)
    {
        printf("语法分析成功! 程序符合PL/0语法规范。\n");
    }
    printf("\n*********** \n语法分析结束，一共有%d个错误。\n*********** \n", sum_error_num);
    return 0;
}