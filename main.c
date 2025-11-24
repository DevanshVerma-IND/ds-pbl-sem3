#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <mysql/mysql.h>
#include <openssl/sha.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

// gcc main.c -o main `mysql_config --cflags --libs` -lssl -lcrypto
// ./main

//////////////////////////////////////////////
#define HOST "127.0.0.1"            //////////
#define USER "root"                 //////////
#define PASS "<ASDZXC>!!@sql123"    //////////
#define DB   "team_management"      //////////
//////////////////////////////////////////////

// ---------- COLOR + STYLE DEFINES (CYBERPUNK, 256-color) ----------

#define ESC         "\x1b"

// FULL BACKGROUND: reset always re-applies BG
#define CLR_BG "\x1b[48;2;0;11;24m"
#define CLR_RESET "\x1b[0m\x1b[48;2;0;11;24m"
#define CLR_DIM     "\x1b[2m"

// Foreground colors (multi-color scheme)
#define CLR_TITLE   "\x1b[38;5;213m"    // neon pink
#define CLR_SUB     "\x1b[38;5;45m"     // cyan
#define CLR_MENU    "\x1b[38;5;50m"     // aqua green
#define CLR_PROMPT  "\x1b[38;5;190m"    // lime yellow
#define CLR_INFO    "\x1b[38;5;81m"     // soft blue
#define CLR_WARN    "\x1b[38;5;226m"    // yellow
#define CLR_ERROR   "\x1b[38;5;203m"    // red

// “Glow” accents (rainbow-ish)
#define CLR_GLOW1   "\x1b[38;5;201m"    // magenta
#define CLR_GLOW2   "\x1b[38;5;45m"     // cyan
#define CLR_GLOW3   "\x1b[38;5;118m"    // neon green
#define CLR_GLOW4   "\x1b[38;5;208m"    // orange

MYSQL *conn;

struct team {
    int id;             // numeric part of team_id (e.g. T1001 -> 1001)
    int semester;
    char subject[256];
    char mentor[256];
    char team_id[256];
    char name[256];
    int marks;
};

int position;

//////////////////////////////////////////////////////
// PROTOTYPES
//////////////////////////////////////////////////////

// Cyberpunk UI helpers
void clear_screen();
void hide_cursor();
void show_cursor();
void cpms_sleep_ms(int ms);
void print_neon_separator();
void print_neon_separator_thin();
void print_cyber_title_bar(const char *title);
void show_startup_banner();
void print_cyber_panel_header(const char *label);

// Animation helpers
void print_rainbow_border_frame(int width, int offset);
void print_scrolling_intro_frame(const char *text, int width, int frame);

// Auth / password
void hash_user_password(const char *password, char *output);
void get_hidden_password(char *password, int max_len);
int  login(char *logged_in_id, char *logged_in_name, char *status_out);

// TEAM / MENTOR / EVALUATOR functions
void input_proposals(const char *team_id);
void submit_project_url(const char *team_id);
void view_selected_proposal(const char *team_id);
void view_marks(const char *team_id);
void Show_Team_Details(const char *team_id);

int  check_team_for_mentor(const char *team_id, const char *mentor_name_logged_in);
void select_proposal(const char *mentor_name_logged_in);
void reject_all_proposals(const char *mentor_name_logged_in);
void Show_Team_Details_for_mentor(const char *mentor_name_logged_in);

// Evaluator queue
typedef struct EvalNode {
    int semester;
    char team_id[20];
    char team_name[100];
    int is_late;
    char project_url[255];
    struct EvalNode* next;
} EvalNode;

typedef struct {
    EvalNode* front;
    EvalNode* rear;
} EvalQueue;

void enqueue(EvalQueue* q, int semester, const char* team_id, const char* team_name, int is_late, const char* project_url);
EvalNode* dequeue(EvalQueue* q);
int  isQueueEmpty(EvalQueue* q);

void extractProjectsBySemester(const char* evaluator_id);
void viewExtractedList();
void evaluateBySemester();

// Global dynamic deadline
void setGlobalDeadline();
void showGlobalDeadline();

// Reporting utilities (struct array + merge sort)
int  loadAllTeams(struct team arr[], int maxSize);            // only completed + evaluated
int  loadAllTeamsAnyStatus(struct team arr[], int maxSize);   // ALL teams (for binary search)
int  cmpMarksDesc(struct team a, struct team b);
int  cmpIDAsc(struct team a, struct team b);

void merge(struct team arr[], int left, int mid, int right,
           int (*cmp)(struct team, struct team));
void mergeSort(struct team arr[], int left, int right,
               int (*cmp)(struct team, struct team));

// Reports
void semesterReport();
void mentorReport();
void subjectReport();
void leaderboard();

// Binary search for View Team Details by ID
int  binarySearchTeam(struct team arr[], int left, int right, int targetID);
void viewTeamDetailsByID();

// Reports menu
void reportsMenu();

//////////////////////////////////////////////////////
// CYBERPUNK UI HELPERS
//////////////////////////////////////////////////////

void clear_screen() {
    // Clear and move cursor home, apply full BG
    printf(ESC "[2J" ESC "[H" CLR_BG);
}

void hide_cursor() {
    printf(ESC "[?25l");
}

void show_cursor() {
    printf(ESC "[?25h");
}

void cpms_sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

// Moving rainbow border segment (used for animation & separators)
void print_rainbow_border_frame(int width, int offset) {
    int colors[] = {196, 202, 208, 214, 220, 190, 82, 46, 51, 39, 27, 93};
    int colorCount = sizeof(colors) / sizeof(colors[0]);

    printf(CLR_BG);
    for (int i = 0; i < width; ++i) {
        int code = colors[(i + offset) % colorCount];
        printf("\x1b[38;5;%dm═", code);
    }
    printf(CLR_RESET "\n");
}

// Thin rainbow line
void print_rainbow_border_frame_thin(int width, int offset) {
    int colors[] = {45, 51, 39, 27, 93, 201};
    int colorCount = sizeof(colors) / sizeof(colors[0]);

    printf(CLR_BG);
    for (int i = 0; i < width; ++i) {
        int code = colors[(i + offset) % colorCount];
        printf("\x1b[38;5;%dm─", code);
    }
    printf(CLR_RESET "\n");
}

// Static thick separator
void print_neon_separator() {
    print_rainbow_border_frame(74, 0);
}

// Static thin separator
void print_neon_separator_thin() {
    print_rainbow_border_frame_thin(60, 0);
}

void print_cyber_title_bar(const char *title) {
    print_neon_separator();
    printf(CLR_BG CLR_TITLE "  CPMS " CLR_SUB "// " CLR_INFO "COLLEGE PROJECT MANAGEMENT SYSTEM" CLR_RESET "\n");
    printf(CLR_BG CLR_MENU "  » %s\n" CLR_RESET, title);
    print_neon_separator();
}

void print_cyber_panel_header(const char *label) {
    printf(CLR_BG CLR_GLOW2 " ┌─[ " CLR_TITLE "%s" CLR_GLOW2 " ]", label);
    printf("───────────────────────────────────────┐\n" CLR_RESET);
}

// Scrolling intro line
void print_scrolling_intro_frame(const char *text, int width, int frame) {
    int len = (int)strlen(text);
    int total = width + len;
    int pos = total - (frame % total);  // virtual scroll position

    printf(CLR_BG CLR_INFO " ");
    for (int i = 0; i < width; ++i) {
        int txt_index = i - (pos - width);
        if (txt_index >= 0 && txt_index < len)
            putchar(text[txt_index]);
        else
            putchar(' ');
    }
    printf(" " CLR_RESET "\n");
}

// Animated cyberpunk startup banner:
// - Moving rainbow border
// - Scrolling intro text
void show_startup_banner() {
    clear_screen();
    hide_cursor();

    const char *intro =
        "Made by Devansh and others(only on paper) !!";

    const int width  = 70;
    const int frames = 45;

    for (int f = 0; f < frames; ++f) {
        clear_screen();

        // Top moving rainbow border
        print_rainbow_border_frame(width, f);

        // Title block
        printf(CLR_BG CLR_GLOW2 " ██████╗    ██████╗     ███╗   ███╗    ███████╗\n" CLR_RESET);
        printf(CLR_BG CLR_GLOW2 "██╔════╝    ██╔══██╗    ████╗ ████║    ██╔════╝\n" CLR_RESET);
        printf(CLR_BG CLR_GLOW2 "██║         ██████╔╝    ██╔████╔██║    ███████╗\n" CLR_RESET);
        printf(CLR_BG CLR_GLOW2 "██║         ██╔═══╝     ██║╚██╔╝██║    ╚════██║\n" CLR_RESET);
        printf(CLR_BG CLR_GLOW2 "╚██████╗    ██║         ██║ ╚═╝ ██║    ███████║\n" CLR_RESET);
        printf(CLR_BG CLR_GLOW2 " ╚═════╝    ╚═╝         ╚═╝     ╚═╝    ╚══════╝\n" CLR_RESET);



        printf("\n");
        printf(CLR_BG CLR_SUB "   CPMS" CLR_RESET CLR_BG "  –  " CLR_INFO "COLLEGE PROJECT MANAGEMENT SYSTEM\n" CLR_RESET);
        printf("\n");

        // Moving rainbow line under title
        print_rainbow_border_frame_thin(width, frames - f);

        // Scrolling intro text
        print_scrolling_intro_frame(intro, width, f);

        // Bottom moving border
        print_rainbow_border_frame(width, frames - f);

        fflush(stdout);
        cpms_sleep_ms(80);
    }

    // Final static frame
    clear_screen();
    print_rainbow_border_frame(width, 0);
    printf(CLR_BG CLR_TITLE "   CPMS // COLLEGE PROJECT MANAGEMENT SYSTEM\n" CLR_RESET);
    print_rainbow_border_frame(width, 10);

    // printf(CLR_DIM "\n  hello to the neon grid. Use the menus below to manage teams,\n");
    // printf("  mentors, deadlines and evaluations in full cyberpunk CLI style.\n\n" CLR_RESET);

    show_cursor();
}

//////////////////////////////////////////////////////
// PASSWORD HASH
//////////////////////////////////////////////////////

void hash_user_password(const char *password, char *output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)password, strlen(password), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(output + (i * 2), "%02x", hash[i]);
    output[64] = '\0';
}

//////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////

int main() {
    conn = mysql_init(NULL);
    if (conn == NULL) {
        printf(CLR_ERROR "Error: %s\n" CLR_RESET, mysql_error(conn));
        exit(1);
    }

    if (!mysql_real_connect(conn, HOST, USER, PASS, DB, 0, NULL, 0)) {
        printf(CLR_ERROR "Connection Failed! %s\n" CLR_RESET, mysql_error(conn));
        exit(1);
    }

    show_startup_banner();
    printf(CLR_INFO " Connected to database successfully!\n" CLR_RESET);

    while (1) {
        char main_choice;

        // Cyberpunk MAIN MENU
        print_cyber_title_bar("MAIN MENU");
        printf(CLR_MENU "  [A] " CLR_RESET "Reports Menu (No Login Required)\n");
        printf(CLR_MENU "  [B] " CLR_RESET "Login as Team / Mentor / Evaluator\n");
        printf(CLR_MENU "  [Q] " CLR_RESET "Quit\n");
        print_neon_separator_thin();
        printf(CLR_PROMPT "  ⮞ Enter choice " CLR_RESET "➜ ");

        // Read a single char and validate
        do {
            if (scanf(" %c", &main_choice) != 1) {
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF);
                printf(CLR_ERROR "  Invalid input. Enter A/a, B/b or Q/q: " CLR_RESET);
                continue;
            }
            getchar(); // consume newline if present
            if (main_choice != 'A' && main_choice != 'a' &&
                main_choice != 'B' && main_choice != 'b' &&
                main_choice != 'Q' && main_choice != 'q') {
                printf(CLR_ERROR "  Invalid choice. Enter A/a, B/b or Q/q: " CLR_RESET);
            } else {
                break;
            }
        } while (1);

        if (main_choice == 'Q' || main_choice == 'q') {
            printf(CLR_WARN "\n  Exiting CPMS. See you in the neon grid again!\n\n" CLR_RESET);
            break;
        }

        if (main_choice == 'A' || main_choice == 'a') {
            // Reports menu (no login)
            reportsMenu();
            continue;
        }

        // B / b => Login flow
        char logged_in_id[50];   // stores ID for team OR mentor OR evaluator
        char logged_in_name[50]; // for mentor name
        char status[20];

        if (!login(logged_in_id, logged_in_name, status)) {
            printf(CLR_ERROR " Login failed! Invalid ID, password, or status.\n" CLR_RESET);
            continue;   // back to main menu
        }

        printf("\n" CLR_INFO " Login successful as '%s'!\n" CLR_RESET, status);

        // TEAM MENU
        if (position == 1) {
            char team_id[50];
            int choice;
            strcpy(team_id, logged_in_id);  // team_id = login ID

            while (1) {
                print_cyber_title_bar("TEAM MENU");
                printf(CLR_INFO "  Team: " CLR_SUB "%s\n" CLR_RESET, team_id);
                print_neon_separator_thin();
                printf(CLR_MENU "  1 " CLR_RESET "Input Proposals\n");
                printf(CLR_MENU "  2 " CLR_RESET "Submit Project URL\n");
                printf(CLR_MENU "  3 " CLR_RESET "View Selected Proposal\n");
                printf(CLR_MENU "  4 " CLR_RESET "Show Team Details\n");
                printf(CLR_MENU "  5 " CLR_RESET "View Marks\n");
                printf(CLR_MENU "  6 " CLR_RESET "Back to Main Menu\n");
                print_neon_separator_thin();
                printf(CLR_PROMPT "  ⮞ Enter choice " CLR_RESET "➜ ");

                if (scanf("%d", &choice) != 1) {
                    int ch;
                    while ((ch = getchar()) != '\n' && ch != EOF);
                    printf(CLR_ERROR " Invalid choice.\n" CLR_RESET);
                    continue;
                }
                getchar();

                switch (choice) {
                    case 1:
                        input_proposals(team_id);
                        break;
                    case 2:
                        submit_project_url(team_id);
                        break;
                    case 3:
                        view_selected_proposal(team_id);
                        break;
                    case 4:
                        Show_Team_Details(team_id);
                        break;
                    case 5:
                        view_marks(team_id);
                        break;
                    case 6:
                        goto back_to_main;
                    default:
                        printf(CLR_ERROR " Invalid choice.\n" CLR_RESET);
                }
            }
        }

        // MENTOR MENU
        if (position == 2) {
            char mentor_name_logged_in[50];
            int choice;
            strcpy(mentor_name_logged_in, logged_in_name); // store mentor NAME

            while (1) {
                print_cyber_title_bar("MENTOR MENU");
                printf(CLR_INFO "  Mentor: " CLR_SUB "%s\n" CLR_RESET, mentor_name_logged_in);
                print_neon_separator_thin();
                printf(CLR_MENU "  1 " CLR_RESET "Select Proposal\n");
                printf(CLR_MENU "  2 " CLR_RESET "Reject all Proposals\n");
                printf(CLR_MENU "  3 " CLR_RESET "Show Team Details\n");
                printf(CLR_MENU "  4 " CLR_RESET "Back to Main Menu\n");
                print_neon_separator_thin();
                printf(CLR_PROMPT "  ⮞ Enter choice " CLR_RESET "➜ ");

                if (scanf("%d", &choice) != 1) {
                    int ch;
                    while ((ch = getchar()) != '\n' && ch != EOF);
                    printf(CLR_ERROR " Invalid choice.\n" CLR_RESET);
                    continue;
                }
                getchar();

                switch (choice) {
                    case 1:
                        select_proposal(mentor_name_logged_in);
                        break;
                    case 2:
                        reject_all_proposals(mentor_name_logged_in);
                        break;
                    case 3:
                        Show_Team_Details_for_mentor(mentor_name_logged_in);
                        break;
                    case 4:
                        goto back_to_main;
                    default:
                        printf(CLR_ERROR " Invalid choice.\n" CLR_RESET);
                }
            }
        }

        // EVALUATOR MENU
        if (position == 3) {
            char evaluator_id[50];
            int choice;
            strcpy(evaluator_id, logged_in_id);  // evaluator_id = login ID

            while (1) {
                print_cyber_title_bar("EVALUATOR MENU");
                printf(CLR_INFO "  Evaluator: " CLR_SUB "%s\n" CLR_RESET, evaluator_id);
                print_neon_separator_thin();
                printf(CLR_MENU "  1 " CLR_RESET "Extract Projects by Semester\n");
                printf(CLR_MENU "  2 " CLR_RESET "View Extracted List\n");
                printf(CLR_MENU "  3 " CLR_RESET "Evaluate by Semester\n");
                printf(CLR_MENU "  4 " CLR_RESET "Set Global Submission Deadline\n");
                printf(CLR_MENU "  5 " CLR_RESET "Show Current Global Deadline (Approx)\n");
                printf(CLR_MENU "  6 " CLR_RESET "Back to Main Menu\n");
                print_neon_separator_thin();
                printf(CLR_PROMPT "  ⮞ Enter choice " CLR_RESET "➜ ");

                if (scanf("%d", &choice) != 1) {
                    int ch;
                    while ((ch = getchar()) != '\n' && ch != EOF);
                    printf(CLR_ERROR " Invalid choice.\n" CLR_RESET);
                    continue;
                }
                getchar();

                switch (choice) {
                    case 1:
                        extractProjectsBySemester(evaluator_id);
                        break;
                    case 2:
                        viewExtractedList();
                        break;
                    case 3:
                        evaluateBySemester();
                        break;
                    case 4:
                        setGlobalDeadline();
                        break;
                    case 5:
                        showGlobalDeadline();
                        break;
                    case 6:
                        goto back_to_main;
                    default:
                        printf(CLR_ERROR " Invalid choice.\n" CLR_RESET);
                }
            }
        }

    back_to_main:
        ; // label needs a statement
    }

    mysql_close(conn);
    return 0;
}

//////////////////////////////////////////////////////
// LOGIN + PASSWORD INPUT
//////////////////////////////////////////////////////

void get_hidden_password(char *password, int max_len) {
    struct termios oldt, newt;
    int i = 0;
    char c;

    // Turn off echo
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (i < max_len - 1 && (c = getchar()) != '\n' && c != EOF) {
        password[i++] = c;
    }
    password[i] = '\0';

    // Turn echo back on
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}

int login(char *logged_in_id, char *logged_in_name, char *status_out) {
    char password[100];
    char hashed_pass[65];
    char query[512];
    int status_choice;

    print_cyber_title_bar("LOGIN");
    printf(CLR_PROMPT "  Select your status:\n" CLR_RESET);
    printf(CLR_MENU   "   1 " CLR_RESET "Team\n");
    printf(CLR_MENU   "   2 " CLR_RESET "Mentor\n");
    printf(CLR_MENU   "   3 " CLR_RESET "Evaluator\n");
    print_neon_separator_thin();
    printf(CLR_PROMPT "  ⮞ Enter choice " CLR_RESET "➜ ");

    if (scanf("%d", &status_choice) != 1) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        printf(CLR_ERROR " Invalid input!\n" CLR_RESET);
        return 0;
    }
    position = status_choice;
    getchar(); // clear newline

    const char *status = NULL;
    switch (status_choice) {
        case 1: status = "team"; break;
        case 2: status = "mentor"; break;
        case 3: status = "evaluator"; break;
        default:
            printf(CLR_ERROR " Invalid status!\n" CLR_RESET);
            return 0;
    }
    strcpy(status_out, status);

    printf(CLR_PROMPT "  User ID " CLR_RESET "➜ ");
    scanf("%s", logged_in_id);
    getchar();

    printf(CLR_PROMPT "  Password " CLR_RESET "🔒 ");
    get_hidden_password(password, sizeof(password));
    hash_user_password(password, hashed_pass);

    snprintf(query, sizeof(query),
             "SELECT name FROM users WHERE id='%s' AND password='%s' AND status='%s';",
             logged_in_id, hashed_pass, status);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " MySQL Query Error: %s\n" CLR_RESET, mysql_error(conn));
        return 0;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        printf(CLR_ERROR " Failed to fetch results: %s\n" CLR_RESET, mysql_error(conn));
        return 0;
    }

    MYSQL_ROW row = mysql_fetch_row(res);

    if (row) {
        if (status_choice == 2) {
            strcpy(logged_in_name, row[0]); // Mentor → use NAME
        } else {
            strcpy(logged_in_name, logged_in_id); // Team/Evaluator → ID
        }

        mysql_free_result(res);
        return 1;
    } else {
        printf(CLR_ERROR " Invalid ID or Password!\n" CLR_RESET);
        mysql_free_result(res);
        return 0;
    }
}

//////////////////////////////////////////////////////
// TEAM FUNCTIONS
//////////////////////////////////////////////////////

void input_proposals(const char *team_id) {
    char p1[255], p2[255], p3[255], p4[255], p5[255];
    print_cyber_panel_header("INPUT PROPOSALS");
    printf(CLR_INFO " Team: %s\n" CLR_RESET, team_id);
    print_neon_separator_thin();

    printf(CLR_PROMPT " Proposal 1 " CLR_RESET "➜ ");
    scanf(" %[^\n]", p1);
    printf(CLR_PROMPT " Proposal 2 " CLR_RESET "➜ ");
    scanf(" %[^\n]", p2);
    printf(CLR_PROMPT " Proposal 3 " CLR_RESET "➜ ");
    scanf(" %[^\n]", p3);
    printf(CLR_PROMPT " Proposal 4 " CLR_RESET "➜ ");
    scanf(" %[^\n]", p4);
    printf(CLR_PROMPT " Proposal 5 " CLR_RESET "➜ ");
    scanf(" %[^\n]", p5);

    char query[2000];
    sprintf(query,
        "UPDATE team_detail SET proposal1='%s', proposal2='%s', proposal3='%s', proposal4='%s', proposal5='%s' WHERE team_id='%s'",
        p1, p2, p3, p4, p5, team_id);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " Error updating proposals: %s\n" CLR_RESET, mysql_error(conn));
    } else {
        printf(CLR_INFO " Proposals updated successfully!\n" CLR_RESET);
    }
}

// DYNAMIC GLOBAL DEADLINE SUPPORT

void submit_project_url(const char *team_id) {
    char project_url[255];
    print_cyber_panel_header("SUBMIT PROJECT URL");
    printf(CLR_INFO " Team: %s\n" CLR_RESET, team_id);
    print_neon_separator_thin();
    printf(CLR_PROMPT " URL " CLR_RESET "➜ ");
    scanf("%s", project_url);

    // Current submission time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char submission_time[25];
    strftime(submission_time, sizeof(submission_time), "%Y-%m-%d %H:%M:%S", t);

    // Get deadline from DB (submission_deadline column)
    char query[512];
    snprintf(query, sizeof(query),
             "SELECT submission_deadline FROM team_detail WHERE team_id='%s';",
             team_id);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " Database error (fetch deadline): %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        printf(CLR_ERROR " Failed to fetch deadline: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    char deadline_str[32] = {0};
    int has_deadline = 0;

    if (row && row[0]) {
        strncpy(deadline_str, row[0], sizeof(deadline_str)-1);
        has_deadline = 1;
    }
    mysql_free_result(res);

    int is_late = 0;

    if (has_deadline) {
        struct tm deadline_tm;
        memset(&deadline_tm, 0, sizeof(deadline_tm));
        if (strptime(deadline_str, "%Y-%m-%d %H:%M:%S", &deadline_tm) == NULL) {
            printf(CLR_WARN " Warning: Invalid deadline format in database. Treating as 'no deadline'.\n" CLR_RESET);
        } else {
            time_t deadline_time = mktime(&deadline_tm);
            is_late = (difftime(now, deadline_time) > 0) ? 1 : 0;
        }
    } else {
        printf(CLR_WARN "\n No submission deadline set in database yet.\n" CLR_RESET);
        printf(CLR_INFO " Treating this submission as ON-TIME.\n" CLR_RESET);
    }

    // Update submission info in DB
    char update_query[1000];
    snprintf(update_query, sizeof(update_query),
        "UPDATE team_detail "
        "SET project_url='%s', submission_time='%s', is_late=%d "
        "WHERE team_id='%s';",
        project_url, submission_time, is_late, team_id);

    if (mysql_query(conn, update_query)) {
        printf(CLR_ERROR " Database error (update submission): %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    printf(CLR_INFO "\n Project submitted successfully at %s\n" CLR_RESET, submission_time);
    if (has_deadline) {
        if (is_late)
            printf(CLR_ERROR " Submission is LATE " CLR_RESET "(Deadline was: %s)\n", deadline_str);
        else
            printf(CLR_INFO " Submission is ON TIME " CLR_RESET "(Before deadline: %s)\n", deadline_str);
    } else {
        printf(CLR_INFO " No deadline was configured in the system.\n" CLR_RESET);
    }
}

void view_selected_proposal(const char *team_id) {
    char query[512];

    snprintf(query, sizeof(query),
             "SELECT selected_proposal, is_selected FROM team_detail WHERE team_id='%s';",
             team_id);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " MySQL Query Error: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        printf(CLR_ERROR " Failed to retrieve result: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        printf(CLR_WARN " No team found with ID '%s'.\n" CLR_RESET, team_id);
        mysql_free_result(res);
        return;
    }

    print_cyber_panel_header("SELECTED PROPOSAL");
    if (row[1] && atoi(row[1]) == 1) {
        printf(CLR_INFO " Team: %s\n" CLR_RESET, team_id);
        print_neon_separator_thin();
        printf(" %s\n", row[0] ? row[0] : "NULL");
    } else {
        printf(CLR_WARN " No proposal has been selected yet for Team ID '%s'.\n" CLR_RESET, team_id);
    }

    mysql_free_result(res);
}

void view_marks(const char *team_id) {
    char query[512];

    snprintf(query, sizeof(query),
             "SELECT marks FROM team_detail WHERE team_id='%s';",
             team_id);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " MySQL Query Error: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        printf(CLR_ERROR " Failed to retrieve result: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row || !row[0]) {
        printf(CLR_WARN " No marks available yet for Team ID '%s'.\n" CLR_RESET, team_id);
        mysql_free_result(res);
        return;
    }

    print_cyber_panel_header("MARKS");
    printf(CLR_INFO " Team: %s\n" CLR_RESET, team_id);
    print_neon_separator_thin();
    printf(" Marks: %s\n", row[0]);
    mysql_free_result(res);
}

void Show_Team_Details(const char *team_id) {
    char query[256];
    snprintf(query, sizeof(query),
             "SELECT * FROM team_detail WHERE team_id = '%s';",
             team_id);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " MySQL query error: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL) {
        printf(CLR_WARN " No results found or query failed.\n" CLR_RESET);
        return;
    }

    MYSQL_ROW row;
    MYSQL_FIELD *fields;
    unsigned int num_fields = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    print_cyber_panel_header("TEAM DETAILS");
    printf(CLR_INFO " Team ID: %s\n" CLR_RESET, team_id);
    print_neon_separator_thin();

    while ((row = mysql_fetch_row(res))) {
        for (unsigned int i = 0; i < num_fields; i++) {
            printf(" %s: %s\n", fields[i].name, row[i] ? row[i] : "NULL");
        }
        print_neon_separator_thin();
    }

    mysql_free_result(res);
}

//////////////////////////////////////////////////////
// MENTOR FUNCTIONS
//////////////////////////////////////////////////////

int check_team_for_mentor(const char *team_id, const char *mentor_name_logged_in) {
    char query[300];

    snprintf(query, sizeof(query),
             "SELECT COUNT(*) FROM team_detail "
             "WHERE team_id='%s' AND mentor=\"%s\";",
             team_id, mentor_name_logged_in);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " Database error: %s\n" CLR_RESET, mysql_error(conn));
        return 0;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        printf(CLR_ERROR " Error fetching result: %s\n" CLR_RESET, mysql_error(conn));
        return 0;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    int allowed = row && atoi(row[0]) > 0;

    if (!allowed) {
        printf(CLR_ERROR "\n Access Denied! '%s' is NOT mentor of Team '%s'.\n\n" CLR_RESET,
               mentor_name_logged_in, team_id);
    }

    mysql_free_result(res);
    return allowed;
}

void select_proposal(const char *mentor_name_logged_in) {
    char team_id[50];
    print_cyber_panel_header("SELECT PROPOSAL");
    printf(CLR_PROMPT " Team ID " CLR_RESET "➜ ");
    scanf("%s", team_id);

    if (!check_team_for_mentor(team_id, mentor_name_logged_in)) return;

    int proposal_num;
    printf(CLR_PROMPT " Proposal number (1-5) " CLR_RESET "➜ ");
    scanf("%d", &proposal_num);

    if (proposal_num < 1 || proposal_num > 5) {
        printf(CLR_ERROR " Invalid proposal number!\n" CLR_RESET);
        return;
    }

    char proposal_col[20], query[500];
    sprintf(proposal_col, "proposal%d", proposal_num);

    sprintf(query,
            "SELECT %s FROM team_detail WHERE team_id='%s';",
            proposal_col, team_id);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " Error: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);

    if (!row || !row[0] || strlen(row[0]) == 0) {
        printf(CLR_WARN " Proposal %d is empty or not found!\n" CLR_RESET, proposal_num);
        mysql_free_result(res);
        return;
    }

    char selected_proposal[255];
    strcpy(selected_proposal, row[0]);
    mysql_free_result(res);

    snprintf(query, sizeof(query),
             "UPDATE team_detail SET is_selected=1, selected_proposal=\"%s\" "
             "WHERE team_id='%s';",
             selected_proposal, team_id);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " Error updating selected proposal: %s\n" CLR_RESET, mysql_error(conn));
    } else {
        printf(CLR_INFO "\n Proposal %d selected successfully for Team %s!\n" CLR_RESET,
               proposal_num, team_id);
    }
}

void reject_all_proposals(const char *mentor_name_logged_in) {
    char team_id[50];
    print_cyber_panel_header("REJECT ALL PROPOSALS");
    printf(CLR_PROMPT " Team ID " CLR_RESET "➜ ");
    scanf("%s", team_id);

    if (!check_team_for_mentor(team_id, mentor_name_logged_in)) return;

    char query[300];
    sprintf(query,
            "UPDATE team_detail SET is_selected=0, selected_proposal=NULL "
            "WHERE team_id='%s';",
            team_id);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " Error: %s\n" CLR_RESET, mysql_error(conn));
    } else {
        printf(CLR_INFO "\n All proposals rejected for Team %s!\n" CLR_RESET, team_id);
    }
}

void Show_Team_Details_for_mentor(const char *mentor_name_logged_in) {
    char team_id[50];
    print_cyber_panel_header("TEAM DETAILS (MENTOR VIEW)");
    printf(CLR_PROMPT " Team ID " CLR_RESET "➜ ");
    scanf("%s", team_id);

    if (!check_team_for_mentor(team_id, mentor_name_logged_in)) return;

    char query[300];
    snprintf(query, sizeof(query),
             "SELECT * FROM team_detail WHERE team_id='%s';",
             team_id);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " MySQL query error: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        printf(CLR_WARN " No data found!\n" CLR_RESET);
        return;
    }

    MYSQL_ROW row;
    MYSQL_FIELD *fields = mysql_fetch_fields(res);
    unsigned int num_fields = mysql_num_fields(res);

    print_neon_separator_thin();
    while ((row = mysql_fetch_row(res))) {
        for (unsigned int i = 0; i < num_fields; i++) {
            printf(" %s: %s\n", fields[i].name, row[i] ? row[i] : "NULL");
        }
        print_neon_separator_thin();
    }

    mysql_free_result(res);
}

//////////////////////////////////////////////////////
// EVALUATION QUEUE (LINKED LIST)
//////////////////////////////////////////////////////

EvalQueue evalQueue = {NULL, NULL};

void enqueue(EvalQueue* q, int semester, const char* team_id, const char* team_name, int is_late, const char* project_url) {
    EvalNode* newNode = (EvalNode*)malloc(sizeof(EvalNode));
    newNode->semester = semester;
    strcpy(newNode->team_id, team_id);
    strcpy(newNode->team_name, team_name);
    newNode->is_late = is_late;
    strcpy(newNode->project_url, project_url);
    newNode->next = NULL;

    if (q->rear == NULL)
        q->front = q->rear = newNode;
    else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

EvalNode* dequeue(EvalQueue* q) {
    if (q->front == NULL) return NULL;
    EvalNode* temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    return temp;
}

int isQueueEmpty(EvalQueue* q) {
    return q->front == NULL;
}

void extractProjectsBySemester(const char* evaluator_id) {
    int semester;
    print_cyber_panel_header("EXTRACT PROJECTS");
    printf(CLR_PROMPT " Semester " CLR_RESET "➜ ");
    scanf("%d", &semester);
    getchar();

    // Clear previous queue
    while (!isQueueEmpty(&evalQueue)) {
        EvalNode* temp = dequeue(&evalQueue);
        free(temp);
    }
    evalQueue.front = evalQueue.rear = NULL;

    char query[512];
    snprintf(query, sizeof(query),
    "SELECT semester, team_id, team_name, IFNULL(is_late,0), IFNULL(project_url,'N/A') "
    "FROM team_detail "
    "WHERE semester=%d "
    "AND LOWER(TRIM(evaluator)) = LOWER(TRIM((SELECT name FROM users WHERE id='%s'))) "
    "AND (marks IS NULL OR marks=0) "
    "ORDER BY IFNULL(submission_time, NOW()) ASC;",
    semester, evaluator_id);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " MySQL Error: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        printf(CLR_ERROR " Failed to fetch result: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_ROW row;
    int count = 0;

    while ((row = mysql_fetch_row(res))) {
        int sem = atoi(row[0]);
        const char* team_id = row[1];
        const char* team_name = row[2];
        int is_late = atoi(row[3]);
        const char* project_url = row[4] ? row[4] : "Not Submitted";

        enqueue(&evalQueue, sem, team_id, team_name, is_late, project_url);
        count++;
    }

    mysql_free_result(res);

    if (count > 0)
        printf(CLR_INFO "\n %d projects extracted successfully for evaluation (Semester %d)\n" CLR_RESET, count, semester);
    else
        printf(CLR_WARN "\n No pending projects found for this semester.\n" CLR_RESET);
}

void viewExtractedList() {
    if (isQueueEmpty(&evalQueue)) {
        printf(CLR_WARN "\n No projects currently extracted. Please extract projects first.\n" CLR_RESET);
        return;
    }

    printf(CLR_BG CLR_GLOW2 "\n╔════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                            EXTRACTED PROJECT QUEUE                             ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════════╝\n" CLR_RESET);
    
    printf(CLR_INFO "%-10s %-12s %-20s %-10s %-35s\n" CLR_RESET, 
           "Semester", "Team ID", "Team Name", "Late?", "Project URL");
    print_neon_separator_thin();

    EvalNode* temp = evalQueue.front;
    int count = 0;
    while (temp != NULL) {
        printf("%-10d %-12s %-20s %-10s %-35s\n",
               temp->semester,
               temp->team_id,
               temp->team_name,
               temp->is_late ? "YES" : "NO",
               temp->project_url);
        temp = temp->next;
        count++;
    }
    print_neon_separator_thin();
    printf(CLR_INFO " Total projects in queue: %d\n\n" CLR_RESET, count);
}

void evaluateBySemester() {
    if (isQueueEmpty(&evalQueue)) {
        printf(CLR_WARN "\n No projects available for evaluation.\n" CLR_RESET);
        printf(CLR_INFO "   Please use 'Extract Projects by Semester' first.\n" CLR_RESET);
        return;
    }

    char choice;
    int evaluated_count = 0;
    
    printf(CLR_BG CLR_GLOW3 "\n╔════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                            PROJECT EVALUATION MODE                             ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════════╝\n" CLR_RESET);

    while (!isQueueEmpty(&evalQueue)) {
        EvalNode* project = dequeue(&evalQueue);

        printf(CLR_GLOW2 "\n┌────────────────────────────────────────────────────────────────┐\n" CLR_RESET);
        printf("│ " CLR_TITLE "PROJECT DETAILS" CLR_RESET "                                              │\n");
        printf(CLR_GLOW2 "├────────────────────────────────────────────────────────────────┤\n" CLR_RESET);
        printf("│ Semester      : %-42d │\n", project->semester);
        printf("│ Team ID       : %-42s │\n", project->team_id);
        printf("│ Team Name     : %-42s │\n", project->team_name);
        printf("│ Late Status   : %-42s │\n", project->is_late ? "LATE SUBMISSION" : "ON TIME");
        printf("│ Project URL   : %-42s │\n", project->project_url);
        printf(CLR_GLOW2 "└────────────────────────────────────────────────────────────────┘\n" CLR_RESET);

        int marks;
        do {
            printf(CLR_PROMPT "\n Enter marks (0-100) " CLR_RESET "➜ ");
            if (scanf("%d", &marks) != 1) {
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF);
                printf(CLR_ERROR " Invalid input! Try again.\n" CLR_RESET);
                marks = -1;
                continue;
            }
            getchar();
            
            if (marks < 0 || marks > 100) {
                printf(CLR_ERROR " Invalid marks! Please enter a value between 0 and 100.\n" CLR_RESET);
            }
        } while (marks < 0 || marks > 100);

        // Update marks in database
        char updateQuery[256];
        snprintf(updateQuery, sizeof(updateQuery),
            "UPDATE team_detail SET marks=%d WHERE team_id='%s';",
            marks, project->team_id);

        if (mysql_query(conn, updateQuery)) {
            printf(CLR_ERROR " Failed to update marks: %s\n" CLR_RESET, mysql_error(conn));
        } else {
            printf(CLR_INFO " Marks (%d/100) for team '%s' updated successfully.\n" CLR_RESET, marks, project->team_id);
            evaluated_count++;
        }

        free(project);

        if (!isQueueEmpty(&evalQueue)) {
            printf(CLR_PROMPT "\n Evaluate next project? (y/n) " CLR_RESET "➜ ");
            if (scanf(" %c", &choice) != 1) {
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF);
                choice = 'n';
            }
            getchar();

            if (choice == 'n' || choice == 'N') {
                printf(CLR_WARN "\n ⏸ Evaluation paused. %d project(s) evaluated.\n" CLR_RESET, evaluated_count);
                printf(CLR_INFO "   Remaining projects are still in the queue.\n" CLR_RESET);
                break;
            }
        }
    }

    if (isQueueEmpty(&evalQueue)) {
        printf(CLR_BG CLR_GLOW4 "\n╔════════════════════════════════════════════════════════════════════════════════╗\n");
        printf("║             ALL PROJECTS OF THE SEMESTER HAVE BEEN EVALUATED!                  ║\n");
        printf("╚════════════════════════════════════════════════════════════════════════════════╝\n" CLR_RESET);
        printf(CLR_INFO " Total projects evaluated: %d\n\n" CLR_RESET, evaluated_count);
    }
}

//////////////////////////////////////////////////////
// GLOBAL DYNAMIC DEADLINE (Evaluator)
//////////////////////////////////////////////////////

void setGlobalDeadline() {
    char deadline_str[32];

    print_cyber_panel_header("SET GLOBAL DEADLINE");
    printf(CLR_PROMPT " New deadline (YYYY-MM-DD HH:MM:SS) " CLR_RESET "➜ ");
    scanf(" %[^\n]", deadline_str);

    if (strlen(deadline_str) < 19) {
        printf(CLR_WARN " Warning: Deadline string looks too short. Expected 'YYYY-MM-DD HH:MM:SS'.\n" CLR_RESET);
    }

    struct tm test_tm;
    memset(&test_tm, 0, sizeof(test_tm));
    if (strptime(deadline_str, "%Y-%m-%d %H:%M:%S", &test_tm) == NULL) {
        printf(CLR_ERROR " Error: Invalid datetime format! Please follow 'YYYY-MM-DD HH:MM:SS'.\n" CLR_RESET);
        return;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE team_detail SET submission_deadline='%s' "
             "WHERE project_url IS NULL;",
             deadline_str);

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " MySQL Error updating global deadline: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    my_ulonglong affected = mysql_affected_rows(conn);
    printf(CLR_INFO "\n Global deadline set to: %s\n" CLR_RESET, deadline_str);
    printf(CLR_INFO " Rows updated (teams without submission yet): %lu\n" CLR_RESET, (unsigned long)affected);
}

void showGlobalDeadline() {
    char query[256] =
        "SELECT submission_deadline "
        "FROM team_detail "
        "WHERE submission_deadline IS NOT NULL "
        "ORDER BY submission_deadline DESC "
        "LIMIT 1;";

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " MySQL Error fetching global deadline: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        printf(CLR_ERROR " Error retrieving deadline: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row || !row[0]) {
        printf(CLR_WARN "\n No global submission deadline is set yet.\n" CLR_RESET);
    } else {
        print_cyber_panel_header("CURRENT GLOBAL DEADLINE");
        printf(" %s\n", row[0]);
    }

    mysql_free_result(res);
}

//////////////////////////////////////////////////////
// REPORTING UTILITIES (struct array + merge sort)
//////////////////////////////////////////////////////

int loadAllTeams(struct team arr[], int maxSize) {
    char query[1024];
    snprintf(query, sizeof(query),
        "SELECT semester, subject, mentor, team_id, team_name, marks "
        "FROM team_detail "
        "WHERE project_url IS NOT NULL AND marks IS NOT NULL;"
    );

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " Database Error: %s\n" CLR_RESET, mysql_error(conn));
        return 0;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        printf(CLR_ERROR " Result Error: %s\n" CLR_RESET, mysql_error(conn));
        return 0;
    }

    int count = 0;
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(res)) && count < maxSize) {
        arr[count].semester = atoi(row[0]);
        strcpy(arr[count].subject, row[1]);
        strcpy(arr[count].mentor,  row[2]);
        strcpy(arr[count].team_id, row[3]);
        strcpy(arr[count].name,    row[4]);
        arr[count].marks    = atoi(row[5]);

        char *idstr = arr[count].team_id;
        int i, j = 0;
        char digits[32];
        for (i = 0; idstr[i] != '\0'; i++) {
            if (isdigit((unsigned char)idstr[i])) {
                digits[j++] = idstr[i];
            }
        }
        digits[j] = '\0';
        arr[count].id = (j > 0) ? atoi(digits) : 0;

        count++;
    }

    mysql_free_result(res);
    return count;
}

int loadAllTeamsAnyStatus(struct team arr[], int maxSize) {
    char query[1024];
    snprintf(query, sizeof(query),
        "SELECT semester, subject, mentor, team_id, team_name, IFNULL(marks,0) "
        "FROM team_detail;"
    );

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " Database Error: %s\n" CLR_RESET, mysql_error(conn));
        return 0;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        printf(CLR_ERROR " Result Error: %s\n" CLR_RESET, mysql_error(conn));
        return 0;
    }

    int count = 0;
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(res)) && count < maxSize) {
        arr[count].semester = atoi(row[0]);
        strcpy(arr[count].subject, row[1]);
        strcpy(arr[count].mentor,  row[2]);
        strcpy(arr[count].team_id, row[3]);
        strcpy(arr[count].name,    row[4]);
        arr[count].marks    = atoi(row[5]);

        char *idstr = arr[count].team_id;
        int i, j = 0;
        char digits[32];
        for (i = 0; idstr[i] != '\0'; i++) {
            if (isdigit((unsigned char)idstr[i])) {
                digits[j++] = idstr[i];
            }
        }
        digits[j] = '\0';
        arr[count].id = (j > 0) ? atoi(digits) : 0;

        count++;
    }

    mysql_free_result(res);
    return count;
}

int cmpMarksDesc(struct team a, struct team b) {
    return b.marks - a.marks;
}

int cmpIDAsc(struct team a, struct team b) {
    return a.id - b.id;
}

void merge(struct team arr[], int left, int mid, int right,
           int (*cmp)(struct team, struct team)) {

    int n1 = mid - left + 1;
    int n2 = right - mid;

    struct team L[n1], R[n2];
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        if (cmp(L[i], R[j]) <= 0)
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSort(struct team arr[], int left, int right,
               int (*cmp)(struct team, struct team)) {
    if (left < right) {
        int mid = (left + right) / 2;
        mergeSort(arr, left, mid, cmp);
        mergeSort(arr, mid + 1, right, cmp);
        merge(arr, left, mid, right, cmp);
    }
}

//////////////////////////////////////////////////////
// REPORT FUNCTIONS (with neon styling)
//////////////////////////////////////////////////////

void semesterReport() {
    int sem;
    print_cyber_panel_header("SEMESTER REPORT");
    printf(CLR_PROMPT " Semester " CLR_RESET "➜ ");
    scanf("%d", &sem);
    getchar();

    struct team arr[500], semArr[500];
    int n = loadAllTeams(arr, 500);

    int count = 0;
    for (int i = 0; i < n; i++)
        if (arr[i].semester == sem)
            semArr[count++] = arr[i];

    if (count == 0) {
        printf(CLR_WARN "\n No teams found for semester %d (with completed submissions and marks).\n" CLR_RESET, sem);
        return;
    }

    mergeSort(semArr, 0, count - 1, cmpMarksDesc);

    print_neon_separator();
    printf(CLR_TITLE "  SEMESTER %d – RANKED TEAMS\n" CLR_RESET, sem);
    print_neon_separator_thin();

    for (int i = 0; i < count; i++) {
        const char *col = (i == 0) ? CLR_GLOW1 :
                          (i == 1) ? CLR_GLOW2 :
                          (i == 2) ? CLR_GLOW3 : CLR_INFO;
        printf("%s %2d. %s (%s) — %d marks\n" CLR_RESET,
               col, i+1, semArr[i].team_id, semArr[i].name, semArr[i].marks);
    }

    int highest = semArr[0].marks;
    int lowest  = semArr[count - 1].marks;

    float avg = 0;
    for (int i = 0; i < count; i++) avg += semArr[i].marks;
    avg /= count;

    print_neon_separator_thin();
    printf(CLR_SUB "  Summary\n" CLR_RESET);
    printf("  Total Teams : %d\n", count);
    printf("  Highest     : %d (%s)\n", highest, semArr[0].team_id);
    printf("  Lowest      : %d (%s)\n", lowest, semArr[count - 1].team_id);
    printf("  Average     : %.2f\n", avg);
    print_neon_separator();
}

void mentorReport() {
    char mentor[256];
    print_cyber_panel_header("MENTOR REPORT");
    printf(CLR_PROMPT " Mentor name " CLR_RESET "➜ ");
    scanf(" %[^\n]", mentor);

    struct team arr[500], res[500];
    int n = loadAllTeams(arr, 500);

    int count = 0;
    for (int i = 0; i < n; i++)
        if (strcasecmp(arr[i].mentor, mentor) == 0)
            res[count++] = arr[i];

    if (count == 0) {
        printf(CLR_WARN "\n No teams found for mentor %s (with completed submissions and marks).\n" CLR_RESET, mentor);
        return;
    }

    mergeSort(res, 0, count - 1, cmpMarksDesc);

    print_neon_separator();
    printf(CLR_TITLE "  MENTOR REPORT – %s\n" CLR_RESET, mentor);
    print_neon_separator_thin();

    for (int i = 0; i < count; i++) {
        const char *col = (i == 0) ? CLR_GLOW1 :
                          (i == 1) ? CLR_GLOW2 :
                          (i == 2) ? CLR_GLOW3 : CLR_INFO;
        printf("%s %2d. %s (%s) — %d marks\n" CLR_RESET,
               col, i+1, res[i].team_id, res[i].name, res[i].marks);
    }

    int highest = res[0].marks;
    int lowest  = res[count - 1].marks;

    float avg = 0;
    for (int i = 0; i < count; i++) avg += res[i].marks;
    avg /= count;

    print_neon_separator_thin();
    printf(CLR_SUB "  Summary\n" CLR_RESET);
    printf("  Total Teams : %d\n", count);
    printf("  Highest     : %d (%s)\n", highest, res[0].team_id);
    printf("  Lowest      : %d (%s)\n", lowest, res[count - 1].team_id);
    printf("  Average     : %.2f\n", avg);
    print_neon_separator();
}

void subjectReport() {
    char subject[256];
    print_cyber_panel_header("SUBJECT REPORT");
    printf(CLR_PROMPT " Subject " CLR_RESET "➜ ");
    scanf(" %[^\n]", subject);

    struct team arr[500], res[500];
    int n = loadAllTeams(arr, 500);

    int count = 0;
    for (int i = 0; i < n; i++)
        if (strcasecmp(arr[i].subject, subject) == 0)
            res[count++] = arr[i];

    if (count == 0) {
        printf(CLR_WARN "\n No teams found for subject %s (with completed submissions and marks).\n" CLR_RESET, subject);
        return;
    }

    mergeSort(res, 0, count - 1, cmpMarksDesc);

    print_neon_separator();
    printf(CLR_TITLE "  SUBJECT REPORT – %s\n" CLR_RESET, subject);
    print_neon_separator_thin();

    for (int i = 0; i < count; i++) {
        const char *col = (i == 0) ? CLR_GLOW1 :
                          (i == 1) ? CLR_GLOW2 :
                          (i == 2) ? CLR_GLOW3 : CLR_INFO;
        printf("%s %2d. %s (%s) — %d marks\n" CLR_RESET,
               col, i+1, res[i].team_id, res[i].name, res[i].marks);
    }

    int highest = res[0].marks;
    int lowest  = res[count - 1].marks;

    float avg = 0;
    for (int i = 0; i < count; i++) avg += res[i].marks;
    avg /= count;

    print_neon_separator_thin();
    printf(CLR_SUB "  Summary\n" CLR_RESET);
    printf("  Total Teams : %d\n", count);
    printf("  Highest     : %d (%s)\n", highest, res[0].team_id);
    printf("  Lowest      : %d (%s)\n", lowest, res[count - 1].team_id);
    printf("  Average     : %.2f\n", avg);
    print_neon_separator();
}

void leaderboard() {
    struct team arr[500];
    int count = loadAllTeams(arr, 500);

    if (count == 0) {
        printf(CLR_WARN " No team data available (no completed submissions with marks).\n" CLR_RESET);
        return;
    }

    mergeSort(arr, 0, count - 1, cmpMarksDesc);

    print_neon_separator();
    printf(CLR_TITLE "  GLOBAL LEADERBOARD\n" CLR_RESET);
    print_neon_separator_thin();

    for (int i = 0; i < count; i++) {
        const char *col;
        if (i == 0)      col = CLR_GLOW1;
        else if (i == 1) col = CLR_GLOW2;
        else if (i == 2) col = CLR_GLOW3;
        else             col = CLR_INFO;

        printf("%s %2d. %s (%s, Sem %d) — %d marks\n" CLR_RESET,
               col, i+1, arr[i].team_id, arr[i].name, arr[i].semester, arr[i].marks);
    }
    print_neon_separator();
}

//////////////////////////////////////////////////////
// BINARY SEARCH + VIEW TEAM DETAILS BY ID
//////////////////////////////////////////////////////

int binarySearchTeam(struct team arr[], int left, int right, int targetID) {
    while (left <= right) {
        int mid = (left + right) / 2;

        if (arr[mid].id == targetID)
            return mid;
        else if (arr[mid].id < targetID)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return -1;
}

void viewTeamDetailsByID() {
    struct team arr[500];
    int n = loadAllTeamsAnyStatus(arr, 500);

    if (n == 0) {
        printf(CLR_WARN "\n No team data available in database.\n" CLR_RESET);
        return;
    }

    mergeSort(arr, 0, n - 1, cmpIDAsc);

    char inputID[50];
    print_cyber_panel_header("VIEW TEAM BY ID");
    printf(CLR_PROMPT " Team ID (e.g., T1001) " CLR_RESET "➜ ");
    scanf("%s", inputID);

    char digits[50];
    int j = 0;
    for (int i = 0; inputID[i] != '\0'; i++) {
        if (inputID[i] >= '0' && inputID[i] <= '9')
            digits[j++] = inputID[i];
    }
    digits[j] = '\0';

    int targetID = atoi(digits);

    int index = binarySearchTeam(arr, 0, n - 1, targetID);

    if (index == -1) {
        printf(CLR_WARN "\n Team ID '%s' not found in in-memory list.\n" CLR_RESET, inputID);
        printf(CLR_INFO " Still trying to fetch directly from database...\n" CLR_RESET);
    }

    char query[512];
    snprintf(query, sizeof(query),
        "SELECT semester, subject, team_id, team_name, "
        "proposal1, proposal2, proposal3, proposal4, proposal5, "
        "project_url, IFNULL(marks, 'Not Given') "
        "FROM team_detail WHERE team_id='%s';",
        inputID
    );

    if (mysql_query(conn, query)) {
        printf(CLR_ERROR " MySQL Error: %s\n" CLR_RESET, mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        printf(CLR_WARN "\n Team details not found.\n" CLR_RESET);
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);

    if (!row) {
        printf(CLR_WARN "\n Team details not found.\n" CLR_RESET);
        mysql_free_result(res);
        return;
    }

    print_neon_separator();
    printf(CLR_TITLE "  TEAM DETAILS\n" CLR_RESET);
    print_neon_separator_thin();
    printf(" Team ID        : %s\n", row[2]);
    printf(" Team Name      : %s\n", row[3]);
    printf(" Semester       : %s\n", row[0]);
    printf(" Subject        : %s\n", row[1]);
    printf("-----------------------------------\n");
    printf(" Proposal 1     : %s\n", row[4] ? row[4] : "NULL");
    printf(" Proposal 2     : %s\n", row[5] ? row[5] : "NULL");
    printf(" Proposal 3     : %s\n", row[6] ? row[6] : "NULL");
    printf(" Proposal 4     : %s\n", row[7] ? row[7] : "NULL");
    printf(" Proposal 5     : %s\n", row[8] ? row[8] : "NULL");
    printf("-----------------------------------\n");
    printf(" Project URL    : %s\n", row[9] ? row[9] : "Not Submitted");
    printf(" Marks Obtained : %s\n", row[10] ? row[10] : "Not Given");
    print_neon_separator();

    mysql_free_result(res);
}

//////////////////////////////////////////////////////
// REPORTS MENU
//////////////////////////////////////////////////////

void reportsMenu() {
    int choice;

    while (1) {
        print_cyber_title_bar("REPORTS MENU // PUBLIC ANALYTICS");
        printf(CLR_MENU "  1 " CLR_RESET "Semester-Wise Report\n");
        printf(CLR_MENU "  2 " CLR_RESET "Mentor-Wise Report\n");
        printf(CLR_MENU "  3 " CLR_RESET "Subject-Wise Report\n");
        printf(CLR_MENU "  4 " CLR_RESET "Global Leaderboard\n");
        printf(CLR_MENU "  5 " CLR_RESET "View Team Details by ID\n");
        printf(CLR_MENU "  6 " CLR_RESET "Back to Main Menu\n");
        print_neon_separator_thin();
        printf(CLR_PROMPT "  ⮞ Enter choice " CLR_RESET "➜ ");

        if (scanf("%d", &choice) != 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            printf(CLR_ERROR " Invalid input.\n" CLR_RESET);
            continue;
        }
        getchar();

        switch (choice) {
            case 1: semesterReport(); break;
            case 2: mentorReport(); break;
            case 3: subjectReport(); break;
            case 4: leaderboard(); break;
            case 5: viewTeamDetailsByID(); break;
            case 6: return;
            default: printf(CLR_ERROR " Invalid choice!\n" CLR_RESET);
        }
    }
}
