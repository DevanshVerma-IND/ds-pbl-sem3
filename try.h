#ifndef TRY_H
#define TRY_H
#define MAX_LINE 1048576

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

struct project{
    int semester;
    char subject[20];
    char t_name[20];
    int t_id;

    char m_name[20];

    char t_member1[20];
    char t_member2[20];
    char t_member3[20];
    char t_member4[20];

    char proposal1[30];
    char proposal2[30];
    char proposal3[30];
    char proposal4[30];
    char proposal5[30];

    char s_proposal[30];

    int marks;
    time_t time;
    int is_late;
} p1;

#endif