#ifndef ACCOUNT_H
#define ACCOUNT_H
#define MAX_LINE 1048576
#include"try.h"

void login_in();
void team_login();
void ev_login();
void team_account();

int choice;
int team_Id;
char team_Name[50];
char mentor[50];
char evaluator[50];

int verifyLogin(const char *filename, int id, int password) {
    FILE *fp;
    char line[100];
    int file_id = -1;
    int file_password = -1;
    int found = 0;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Could not open %s\n", filename);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Team ID:", 8) == 0) {
            sscanf(line, "Team ID: %d", &file_id);
        }
        else if (strncmp(line, "Password:", 9) == 0) {
            sscanf(line, "Password: %d", &file_password);
            if (file_id == id && file_password == password) {
                found = 1;
                break;
            }
        }
    }
    fclose(fp);
    return found;
}
int verifyLoginEM(const char *filename, int id, int password) {
    FILE *fp;
    char line[100];
    int file_id = -1;
    int file_password = -1;
    int found = 0;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Could not open %s\n", filename);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "ID:", 3) == 0) {
           sscanf(line, "ID: %d", &file_id);
        }
        else if (strncmp(line, "Password:", 9) == 0) {
            sscanf(line, "Password: %d", &file_password);
            if (file_id == id && file_password == password) {
                found = 1;
                break;
            }
        }
    }
    fclose(fp);
    return found;
}

void team_login(){
    int id;
    int password;
    printf("enter team name, id and password\n");
    scanf("%s%d%d",team_Name,&id,&password);
    team_Id=id;
    int res=verifyLogin("team_password.txt", id, password);
    if(!res){
        printf("Incorrect entry\n");
        exit(1);
    }
}

void mentor_login(){
    int id;
    int password;
    printf("enter name, id and password\n");
    scanf("%s%d%d",mentor,&id,&password);
    int res=verifyLoginEM("mentor_password.txt", id, password);
    if(!res){
        printf("Incorrect entry\n");
        exit(1);
    }
}

void ev_login(){
    int id;
    int password;
    printf("enter id and password\n");
    scanf("%s%d%d",mentor,&id,&password);
    int res=verifyLoginEM("evaluator_password.txt", id, password);
    if(!res){
        printf("Incorrect entry\n");
        exit(1);
    }
}

void team_account(){

    int password;

    FILE *fp;
    fp = fopen("projects.txt", "a");
    if (fp == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    printf("Enter semester: ");
    scanf("%d", &p1.semester);
    getchar();

    printf("Enter subject: ");
    fgets(p1.subject, sizeof(p1.subject), stdin);
    p1.subject[strcspn(p1.subject, "\n")] = '\0';

    printf("Enter mentor name: ");
    fgets(p1.m_name, sizeof(p1.m_name), stdin);
    p1.m_name[strcspn(p1.m_name, "\n")] = '\0';

    printf("Enter team name: ");
    fgets(p1.t_name, sizeof(p1.t_name), stdin);
    strcpy(team_Name,p1.t_name);
    p1.t_name[strcspn(p1.t_name, "\n")] = '\0';

    printf("Enter team ID: ");
    scanf("%d", &p1.t_id);
    team_Id=p1.t_id;
    getchar();

    printf("Enter password: ");
    scanf("%d", &password);
    getchar();

    printf("Enter Team Member 1(Leader) name: ");
    fgets(p1.t_member1, sizeof(p1.t_member1), stdin);
    p1.t_member1[strcspn(p1.t_member1, "\n")] = '\0';

    printf("Enter Team Member 2 name: ");
    fgets(p1.t_member2, sizeof(p1.t_member2), stdin);
    p1.t_member2[strcspn(p1.t_member2, "\n")] = '\0';

    printf("Enter Team Member 3 name: ");
    fgets(p1.t_member3, sizeof(p1.t_member3), stdin);
    p1.t_member3[strcspn(p1.t_member3, "\n")] = '\0';

    printf("Enter Team Member 4 name: ");
    fgets(p1.t_member4, sizeof(p1.t_member4), stdin);
    p1.t_member4[strcspn(p1.t_member4, "\n")] = '\0';

    printf("Enter Proposal 1: ");
    fgets(p1.proposal1, sizeof(p1.proposal1), stdin);
    p1.proposal1[strcspn(p1.proposal1, "\n")] = '\0';

    printf("Enter Proposal 2: ");
    fgets(p1.proposal2, sizeof(p1.proposal2), stdin);
    p1.proposal2[strcspn(p1.proposal2, "\n")] = '\0';

    printf("Enter Proposal 3: ");
    fgets(p1.proposal3, sizeof(p1.proposal3), stdin);
    p1.proposal3[strcspn(p1.proposal3, "\n")] = '\0';

    printf("Enter Proposal 4: ");
    fgets(p1.proposal4, sizeof(p1.proposal4), stdin);
    p1.proposal4[strcspn(p1.proposal4, "\n")] = '\0';

    printf("Enter Proposal 5: ");
    fgets(p1.proposal5, sizeof(p1.proposal5), stdin);
    p1.proposal5[strcspn(p1.proposal5, "\n")] = '\0';

    p1.marks = 0;
    p1.is_late = 1;
    p1.time = time(NULL);

    fprintf(fp, "==============================\n");
    fprintf(fp, "Semester: %d\n", p1.semester);
    fprintf(fp, "subject: %s\n", p1.subject);
    fprintf(fp, "Mentor Name: %s\n", p1.m_name);
    fprintf(fp, "Team Name: %s\n", p1.t_name);
    fprintf(fp, "Team ID: %d\n", p1.t_id);
    fprintf(fp, "Team Members:\n");
    fprintf(fp, "  Leader: %s\n", p1.t_member1);
    fprintf(fp, "  Member2: %s\n", p1.t_member2);
    fprintf(fp, "  Member3: %s\n", p1.t_member3);
    fprintf(fp, "  Member4: %s\n", p1.t_member4);
    fprintf(fp, "Proposals:\n ");
    fprintf(fp, "  1. %s\n ", p1.proposal1);
    fprintf(fp, "  2. %s\n ", p1.proposal2);
    fprintf(fp, "  3. %s\n ", p1.proposal3);
    fprintf(fp, "  4. %s\n ", p1.proposal4);
    fprintf(fp, "  5. %s\n ", p1.proposal5);
    fprintf(fp, "Selected Proposal: \n\n");
    // fprintf(fp, "Timestamp: %s", ctime(&p1.time));
    fprintf(fp, "Timestamp: \n");
    fprintf(fp, "Marks: %d\n", p1.marks);
    fprintf(fp, "Late: %s\n", p1.is_late ? "Yes" : "No");
    fprintf(fp, "==============================\n\n");

    fclose(fp);

    printf("\nProject details saved successfully in 'projects.txt'\n");

    FILE *ps;
    ps = fopen("team_password.txt", "a");
    if (ps == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(ps, "Team ID: %d\n", p1.t_id);
    fprintf(ps, "Password: %d\n\n", password);
    fclose(ps);
}

void login_in(){
    // int choice;
    printf("Select 1/2/3:\n1. Team\n2. Mentor\n3. Evaluator\n");
    scanf("%d",&choice);

    switch (choice){
        case 1:
        char ch;
        printf("already have an account ?(y for yes / N for no): ");
        getchar();
        scanf("%c",&ch);
        if(ch=='y'){
            team_login();
        }
        else if(ch=='N'){
            team_account();
        }
        else if(ch!='y' && ch!='N'){
            while(ch!='y' && ch!='N'){
                printf("Invalid entry\nRetry\n");
                printf("already have an account ?(y for yes / N for no): ");
                getchar();
                scanf("%c",&ch);
                if(ch=='y'){
                    team_login();
                }
                else if(ch=='N'){
                    team_account();
                }
            
            }
        }
        break;
        case 2:
        mentor_login();
        break;
        case 3:
        ev_login();
        break;
    }
}


#endif