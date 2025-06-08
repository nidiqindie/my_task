#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#define MAX_ID_LENGTH 8
#define MAX_NUM_LENGTH 8

FILE *in, *out;
int line_num = 1;

const char *keywords[] = {
    "const", "var", "procedure", "begin", "end",
    "if", "then", "while", "do", "call", "read", "write"};
int num_keywords = sizeof(keywords) / sizeof(keywords[0]);

int is_keyword(const char *str)
{
    for (int i = 0; i < num_keywords; i++)
    {
        if (strcmp(str, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

void print_token(const char *type, const char *value)
{
    fprintf(out, "(%s,%s)\n", type, value);
}

void print_error(const char *type, const char *value)
{
    fprintf(out, "(%s,%s,行号:%d)\n", type, value, line_num);
}

void skip_whitespace()
{
    int c;
    while ((c = fgetc(in)) != EOF)
    {
        if (c == ' ' || c == '\t' || c == '\r')
            continue;
        if (c == '\n')
            line_num++;
        else
        {
            ungetc(c, in);
            break;
        }
    }
}

void skip_comment()
{
    int c1 = fgetc(in);
    if (c1 == '/')
    { // 单行注释
        while ((c1 = fgetc(in)) != EOF && c1 != '\n')
            ;
        line_num++;
    }
    else if (c1 == '*')
    { // 多行注释
        int prev = 0;
        while ((c1 = fgetc(in)) != EOF)
        {
            if (c1 == '\n')
                line_num++;
            if (prev == '*' && c1 == '/')
                break;
            prev = c1;
        }
        if (c1 == EOF)
        {
            print_error("错误", "未闭合的多行注释");
        }
    }
    else
    {
        // / 运算符
        ungetc(c1, in);
        print_token("运算符", "/");
    }
}

void read_word()
{
    char buffer[100] = {0};
    int len = 0;
    int c;
    while ((c = fgetc(in)) != EOF && (isalpha(c) || isdigit(c) || c == '_'))
    {
        buffer[len++] = c;
    }
    buffer[len] = '\0';
    if (c != EOF)
        ungetc(c, in);

    if (isdigit(buffer[0]))
    {
        if (isalpha(buffer[1]) || buffer[1] == '_')
            print_error("非法字符(串)", buffer);
        else if (strlen(buffer) > MAX_NUM_LENGTH)
            print_error("无符号整数越界", buffer);
        else
            print_token("无符号整数", buffer);
    }
    else
    {
        if (is_keyword(buffer))
            print_token("保留字", buffer);
        else if (strlen(buffer) > MAX_ID_LENGTH)
            print_error("标识符长度超长", buffer);
        else
            print_token("标识符", buffer);
    }
}

void read_symbol()
{
    int c = fgetc(in);
    int next = fgetc(in);
    char buf[3] = {c, next, '\0'};

    if (c == ':' && next == '=')
        print_token("运算符", ":=");
    else if (c == '<' && next == '=')
        print_token("运算符", "<=");
    else if (c == '>' && next == '=')
        print_token("运算符", ">=");
    else if (c == '<' || c == '>' || c == '=' || c == '#' ||
             c == '+' || c == '-' || c == '*' || c == '/')
    {
        ungetc(next, in);
        if (c == '/')
            skip_comment(); // 交由 skip_comment 判断
        else
        {
            char str[2] = {c, '\0'};
            print_token("运算符", str);
        }
    }
    else if (c == ',' || c == ';' || c == '(' || c == ')' || c == '.')
    {
        ungetc(next, in);
        char str[2] = {c, '\0'};
        print_token("界符", str);
    }
    else
    {
        ungetc(next, in);
        char str[2] = {c, '\0'};
        print_error("非法字符(串)", str);
    }
}

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    in = fopen(argv[1], "r");
    out = fopen(argv[2], "w");
    if (!in || !out)
    {
        printf("文件打开失败\n");
        return 1;
    }

    int c;
    while ((c = fgetc(in)) != EOF)
    {
        if (isspace(c))
        {
            if (c == '\n')
                line_num++;
            continue;
        }

        if (isalpha(c) || c == '_')
        {
            ungetc(c, in);
            read_word();
        }
        else if (isdigit(c))
        {
            ungetc(c, in);
            read_word();
        }
        else if (c == '/' || c == ':' || c == '<' || c == '>' ||
                 c == '=' || c == '#' || c == '+' || c == '-' ||
                 c == '*' || c == ',' || c == ';' || c == '(' || c == ')' || c == '.')
        {
            ungetc(c, in);
            read_symbol();
        }
        else
        {
            char str[2] = {c, '\0'};
            print_error("非法字符(串)", str);
        }
    }

    fclose(in);
    fclose(out);
    printf("词法分析完成，结果保存在 %s\n", argv[2]);
    return 0;
}
