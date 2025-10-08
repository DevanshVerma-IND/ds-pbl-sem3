#ifndef MENTOR_H
#define MENTOR_H
#define MAX_LINE 1048576
#include"try.h"

void selectProposal(int teamID, const char *mentorName);
void displayTeamInfo(int teamID, const char *mentorName);

void selectProposal(int teamID, const char *mentorName) {
    char filename[] = "projects.txt";
    FILE *fp = fopen(filename, "r");
    FILE *temp = fopen("temp.txt", "w");
    char line[MAX_LINE];
    char proposals[5][MAX_LINE];
    char currentMentor[MAX_LINE] = "";
    int currentID = -1;
    int found = 0;
    int proposalCount = 0;

    if (!fp || !temp) {
        printf("Error opening files!\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        // Detect Mentor Name
        if (strncmp(line, "Mentor Name:", 12) == 0) {
            sscanf(line, "Mentor Name: %[^\n]", currentMentor);
        }

        // Detect Team ID
        if (strncmp(line, "Team ID:", 8) == 0) {
            sscanf(line, "Team ID: %d", &currentID);
            proposalCount = 0;
        }

        // Write line to temp first (we'll modify if needed)
        fputs(line, temp);

        // If both mentor and team match
        if (currentID == teamID && strcmp(currentMentor, mentorName) == 0) {

            // Capture proposals
            if (strstr(line, "1.") && proposalCount == 0) {
                strcpy(proposals[0], line + 4); proposalCount++;
            } else if (strstr(line, "2.") && proposalCount == 1) {
                strcpy(proposals[1], line + 4); proposalCount++;
            } else if (strstr(line, "3.") && proposalCount == 2) {
                strcpy(proposals[2], line + 4); proposalCount++;
            } else if (strstr(line, "4.") && proposalCount == 3) {
                strcpy(proposals[3], line + 4); proposalCount++;
            } else if (strstr(line, "5.") && proposalCount == 4) {
                strcpy(proposals[4], line + 4); proposalCount++;
            }

            // When Selected Proposal line is found
            if (strstr(line, "Selected Proposal:") != NULL) {
                printf("\nProposals for Team ID %d (Mentor: %s):\n", teamID, mentorName);
                for (int i = 0; i < 5; i++)
                    printf("%d.%s", i + 1, proposals[i]);

                int choice;
                printf("\nEnter the number of the proposal to select (1-5): ");
                scanf("%d", &choice);
                getchar();

                if (choice < 1 || choice > 5) {
                    printf("Invalid choice!\n");
                    fclose(fp);
                    fclose(temp);
                    remove("temp.txt");
                    return;
                }

                // Move back and overwrite the selected proposal line
                fseek(temp, -strlen(line), SEEK_CUR);
                fprintf(temp, " Selected Proposal: %s", proposals[choice - 1]);
                found = 1;
            }
        }

        // Reset after a full record (for next block)
        if (strstr(line, "==============================")) {
            currentMentor[0] = '\0';
            currentID = -1;
        }
    }

    fclose(fp);
    fclose(temp);

    if (found) {
        remove(filename);
        rename("temp.txt", filename);
        printf("\nSelected Proposal updated successfully!\n");
    } else {
        remove("temp.txt");
        printf("No matching record found for Mentor '%s' and Team ID %d.\n", mentorName, teamID);
    }
}

void displayTeamInfo(int teamID, const char *mentorName) {
    char filename[] = "projects.txt";
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        printf("Could not open file %s\n", filename);
        return;
    }

    char line[MAX_LINE];
    char currentMentor[MAX_LINE] = "";
    int currentID = -1;
    int found = 0;
    int printFlag = 0;

    while (fgets(line, sizeof(line), fp)) {
        // Capture Mentor Name
        if (strncmp(line, "Mentor Name:", 12) == 0) {
            sscanf(line, "Mentor Name: %[^\n]", currentMentor);
        }

        // Capture Team ID
        if (strncmp(line, "Team ID:", 8) == 0) {
            sscanf(line, "Team ID: %d", &currentID);
            // Check both Mentor and Team ID
            if (currentID == teamID && strcmp(currentMentor, mentorName) == 0) {
                found = 1;
                printFlag = 1;
                printf("\n=== Team Details for Mentor '%s', Team ID %d ===\n\n", mentorName, teamID);
            }
        }

        // Print only when both match
        if (printFlag)
            printf("%s", line);

        // End of this team's record
        if (strstr(line, "==============================") != NULL) {
            if (printFlag)
                break; // stop printing after one block
            // reset for next block
            currentMentor[0] = '\0';
            currentID = -1;
        }
    }

    if (!found)
        printf("\nNo record found for Mentor '%s' and Team ID %d.\n", mentorName, teamID);

    fclose(fp);
}

#endif