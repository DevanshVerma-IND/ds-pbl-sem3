#ifndef TEAM_H
#define TEAM_H
#define MAX_LINE 1048576
#include"try.h"

void searchSelectedProposal(const char *input_name, int input_id);
void searchMarks(const char *input_name, int input_id);
void resubmitProposals();

void searchSelectedProposal(const char *input_name, int input_id) {
    char filename[50];
    strcpy(filename,"projects.txt");
    FILE *fp;
    char line[256];
    char current_team[50] = "";
    int current_id = -1;
    char selected_proposal[100] = "";
    int found = 0;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Could not open %s\n", filename);
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Team Name:", 10) == 0)
            sscanf(line, "Team Name: %[^\n]", current_team);

        else if (strncmp(line, "Team ID:", 8) == 0)
            sscanf(line, "Team ID: %d", &current_id);

        if (strcmp(current_team, input_name) == 0 && current_id == input_id) {
            if (strncmp(line, " Selected Proposal:", 19) == 0) {
                sscanf(line, " Selected Proposal: %[^\n]", selected_proposal);
                found = 1;
                break;
            }
        }
    }

    fclose(fp);

    if (found)
        printf("\nSelected Proposal for %s (ID: %d): %s\n", input_name, input_id, selected_proposal);
    else
        printf("\nNo record found for Team Name '%s' and ID %d\n", input_name, input_id);
}

void searchMarks(const char *input_name, int input_id) {
    char filename[50];
    strcpy(filename,"projects.txt");
    FILE *fp;
    char line[256];
    char current_team[50] = "";
    int current_id = -1;
    int marks = -1;
    int found = 0;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Could not open %s\n", filename);
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Team Name:", 10) == 0)
            sscanf(line, "Team Name: %[^\n]", current_team);

        else if (strncmp(line, "Team ID:", 8) == 0)
            sscanf(line, "Team ID: %d", &current_id);

        if (strcmp(current_team, input_name) == 0 && current_id == input_id) {
            if (strncmp(line, "Marks:", 6) == 0) {
                sscanf(line, "Marks: %d", &marks);
                found = 1;
                break;
            }
        }
    }

    fclose(fp);

    if (found)
        printf("\nMarks for %s (ID: %d): %d\n", input_name, input_id, marks);
    else
        printf("\nNo record found for Team Name '%s' and ID %d\n", input_name, input_id);
}

void resubmitProposals(const char *input_name, int input_id) {
    char filename[50];
    strcpy(filename,"projects.txt");
    FILE *fp, *temp;
    char line[256];
    int target_id=input_id, current_id = -1;
    int found = 0;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Could not open %s\n", filename);
        return;
    }

    temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        printf("Could not create temporary file.\n");
        fclose(fp);
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Team ID:", 8) == 0) {
            sscanf(line, "Team ID: %d", &current_id);
        }
        if (current_id == target_id && strstr(line, "Proposals:") != NULL) {
            found = 1;

            fprintf(temp, "Proposals:\n");

            char newProp[5][50];
            printf("\nEnter 5 new proposals:\n");
            getchar();
            for (int i = 0; i < 5; i++) {
                printf("Proposal %d: ", i + 1);
                fgets(newProp[i], sizeof(newProp[i]), stdin);
                newProp[i][strcspn(newProp[i], "\n")] = '\0';
                fprintf(temp, "   %d. %s\n", i + 1, newProp[i]);
            }

            continue;
        }

        if (found && strstr(line, "Selected Proposal:") == NULL && 
            (line[0] == ' ' || line[0] == '\t')) {
            continue;
        }

        if (found && strstr(line, "Selected Proposal:") != NULL) {
            fprintf(temp, "%s", line);
            found = 0;
            continue;
        }

        fprintf(temp, "%s", line);
    }

    fclose(fp);
    fclose(temp);

    remove(filename);
    rename("temp.txt", filename);

    printf("\nProposals updated successfully for Team ID: %d\n", target_id);
}

#endif