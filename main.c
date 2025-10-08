#include"try.h"
#include"account.h"
#include"team.h"
#include"mentor.h"

#define MAX_LINE 1048576
// int team_Id;
// int team_Name;

int main()
{
    login_in();
    int ch;
    //here 1. Team
    //     2. Mentor
    //     3. Evaluator
    if(choice==1){
        do{
            printf("\n*********************************************************\n");
            printf("1. View Selected Proposal\n");
            printf("2. Resubmit Proposal\n");
            printf("3. Submit project\n");
            printf("4. View marks\n");
            printf("5. Exit\n");
            scanf("%d",&ch);
            if(ch==1){
                searchSelectedProposal(team_Name,team_Id);
            }
            else if(ch==2){
                // printf("Enter Team Name and ID\n");
                // scanf("%s%d",teamName,&teamId);
                resubmitProposals(team_Name,team_Id);
            }
            else if(ch==3){
                printf("work undergoing\n");
            }
            else if(ch==4){
                searchMarks(team_Name,team_Id);
            }
        }while(ch!=5);
    }
    else if(choice==2){
        do{
            int teamId;
            printf("\n*********************************************************\n");
            printf("1. Select Proposal\n");
            printf("2. View Team Details\n");
            printf("3. Exit\n");
            scanf("%d",&ch);
            if(ch==1){
                int teamId;
                printf("Enter Team ID\n");
                scanf("%d",&teamId);
                selectProposal(teamId,mentor);
            }
            else if(ch==2){
                int teamId;
                printf("Enter Team ID\n");
                scanf("%d",&teamId);
                displayTeamInfo(teamId,mentor);
            }
        }while(ch!=3);
    }
    else if(choice==3){
        printf("work undergoing\n");
    }

    // system("gedit projects.txt &");
    
    return 0;
}