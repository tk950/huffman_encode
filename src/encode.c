#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "encode.h"

#define NSYMBOLS 256

static int symbol_count[NSYMBOLS];

// 以下このソースで有効なstatic関数のプロトタイプ宣言

// ファイルを読み込み、static配列の値を更新する関数
static void count_symbols(const char *filename);

// symbol_count をリセットする関数
static void reset_count(void);

// 与えられた引数でNode構造体を作成し、そのアドレスを返す関数
static Node *create_node(int symbol, int count, Node *left, Node *right);

// Node構造体へのポインタが並んだ配列から、最小カウントを持つ構造体をポップしてくる関数
// n は 配列の実効的な長さを格納する変数を指している（popするたびに更新される）
static Node *pop_min(int *n, Node *nodep[]);

// ハフマン木を構成する関数
static Node *build_tree(void);

// 以下 static関数の実装
static void count_symbols(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "error: cannot open %s\n", filename);
        exit(1);
    }

    // 1Byteずつ読み込み、カウントする
    int charcter;
    while ((charcter = fgetc(fp)) != EOF)
    {
        symbol_count[charcter]++;
    }

    fclose(fp);
}
static void reset_count(void)
{
    for (int i = 0; i < NSYMBOLS; i++)
        symbol_count[i] = 0;
}

static Node *create_node(int symbol, int count, Node *left, Node *right)
{
    Node *ret = (Node *)malloc(sizeof(Node));
    *ret = (Node){.symbol = symbol, .count = count, .left = left, .right = right};
    return ret;
}

static Node *pop_min(int *n, Node *nodep[])
{
    // カウントが最小のノードを見つけてくる
    int argmin = 0;
    for (int i = 0; i < *n; i++)
    {
        if (nodep[i]->count < nodep[argmin]->count)
        {
            argmin = i;
        }
    }

    Node *node_min = nodep[argmin];

    // 見つかったノード以降の配列を前につめていく
    for (int i = argmin; i < (*n) - 1; i++)
    {
        nodep[i] = nodep[i + 1];
    }
    // 合計ノード数を一つ減らす
    (*n)--;

    return node_min;
}

static Node *build_tree(void)
{
    int n = 0;
    Node *nodep[NSYMBOLS];

    for (int i = 0; i < NSYMBOLS; i++)
    {
        // カウントの存在しなかったシンボルには何もしない
        if (symbol_count[i] == 0)
            continue;

        nodep[n++] = create_node(i, symbol_count[i], NULL, NULL);
    }

    const int dummy = -1; // ダミー用のsymbol を用意しておく
    while (n >= 2)
    {
        Node *node1 = pop_min(&n, nodep);
        Node *node2 = pop_min(&n, nodep);
        // 選ばれた2つのノードを元に統合ノードを新規作成
        nodep[n++] = create_node(dummy, node1->count + node2->count, node1, node2); //後置演算
    }

    // 木を返す
    return (n == 0) ? NULL : nodep[0];
}

// 深さ優先で木を走査する
void traverse_tree(const int depth, const Node *np)
{
    static unsigned int code = 0;
    if (np == NULL || np->left == NULL)
    {
        if (np != NULL)
        {
            char symbol_c[5];
            switch (np->symbol)
            {
            case '\n':
                strcpy(symbol_c, "LF");
                break;
            case ' ':
                strcpy(symbol_c, " ");
                break;
            default:
                symbol_c[0] = (char)np->symbol;
                symbol_c[1] = 0;
                break;
            }
            char code_c[33];
            for (int k = 1; k <= depth; ++k)
            {
                if ((code >> (depth - k)) & 1)
                {
                    code_c[k - 1] = '1';
                }
                else
                {
                    code_c[k - 1] = '0';
                }
            }
            code_c[depth] = 0;
            printf("\'%s\': %s: %d\n", symbol_c, code_c, np->count);
            while (code & 1)
            {
                code = code >> 1;
            }
        }
        return;
    }
    code = code << 1;
    printf("+--%d--", code & 1);
    traverse_tree(depth + 1, np->left);
    for (int i = 0; i < depth; ++i)
    {
        if ((code >> (depth - i)) & 1)
        {
            printf("      ");
        }
        else
        {
            printf("|     ");
        }
    }
    code = code + 1;
    printf("+--%d--", code & 1);
    traverse_tree(depth + 1, np->right);
}

Node *encode(const char *filename)
{
    reset_count();
    count_symbols(filename);
    Node *root = build_tree();

    if (root == NULL)
    {
        fprintf(stderr, "A tree has not been constructed.\n");
    }

    return root;
}
