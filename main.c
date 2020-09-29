#include "header.h"
#define N 5
#define CONSUMIDORES 2

sigset_t block_mask;
pthread_mutex_t trinco = PTHREAD_MUTEX_INITIALIZER;  // Trinco tab completation
pthread_mutex_t trinco2 = PTHREAD_MUTEX_INITIALIZER; //
pthread_mutex_t trincoP = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t trincoC = PTHREAD_MUTEX_INITIALIZER;
pthread_t tidC[CONSUMIDORES];
pthread_t *tarefa_dinamica = NULL;
pthread_t ponto3;
sem_t pode_prod,
    pode_cons;

char *path = NULL;
char *line;
char **diretorios = NULL;
char **dicionario = NULL;
static int incremento_dicionario = 0;
char *string; // $PATH
char **myfind = NULL;
static int cnt = 0;
int prodptr = 0, consptr = 0, nItem = 0;

int main(int argc, const char *argv[])
{
    string = malloc((strlen(getenv("PATH")) + 1) * sizeof(char));
    strcpy(string, getenv("PATH"));
    atualizar_caminho();

    /// Bloqueia CTRL+C e CTRL+Z
    struct sigaction sa, sa_orig_int, sa_orig_sigtstp;
    //  Alocação de memoria
    memset(&sa, 0, sizeof(sa));
    //rotina de atendimento será ignorar o sigint and sigtstp
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_SIGINFO;
    /*bloqueia o sinal SIGINT*/
    sigaction(SIGINT, &sa, &sa_orig_int);
    sigaction(SIGTSTP, &sa, &sa_orig_sigtstp);
    //
    rl_attempted_completion_function = character_name_completion;

    int sizePath = parse_path();
    pthread_t tid[sizePath];
    int indices[sizePath], i = 0;

    for (i = 0; i < sizePath; i++)
    {
        indices[i] = i;
        pthread_create(&tid[i], NULL, &preencher_biblioteca, &indices[i]);
    }
    for (i = 0; i < sizePath; i++)
    {
        pthread_join(tid[i], NULL);
    }

    int my;
    while (1)
    {
        line = readline(path);
        if (strcmp(line, "") && strcmp(line, " "))
        {
            if (!strcmp(line, "exit"))
                exit(0);
            add_history(line);
            //cria a estrutura comandos
            CMD *root = parse_line(line);
            //print_command_list(root);
            my = strncmp(root->argv[0], "my", 2);
            if (my != 0)
                exec_comandos(root);
            else
            {
                myexec(root);
            }

            free_command_list(root);
            free(line);
        }
    }

    return 0;
}

void myexec(CMD *root)
{
    if (strcmp(root->argv[0], "myls") == 0)
    {
        DIR *dir;
        struct dirent *entry;
        struct stat sb;
        char pwd[2024] = "\0";
        int param[2] = {0};
        CMD *aux = root;

        int i = 1, p = 0, a, l, al, r;
        // Verificar se passou um diretorio
        while (aux->argv[i] != NULL)
        {
            a = strcmp(aux->argv[i], "-a");
            l = strcmp(aux->argv[i], "-l");
            al = strcmp(aux->argv[i], "-al");
            r = strcmp(aux->argv[i], "-R");
            if (l == 0) // -l
            {
                param[0] = 1;
            }
            else if (a == 0) // -a
            {
                param[1] = 1;
            }
            else if (al == 0) // -al
            {
                param[0] = 1;
                param[1] = 1;
            }
            else if (r == 0) // -R
            {
                param[2] = 1;
            }
            else
            {
                p++;
                strcpy(pwd, aux->argv[i]);
            }
            i++;
        }

        if (p == 0) // Se não passou um diretorio usar o actual
        {
            getcwd(pwd, sizeof(pwd));
        }

        char *aux_pwd = malloc((strlen(pwd) + 1) * sizeof(char));
        strcpy(aux_pwd, pwd);
        //
        if ((dir = opendir(pwd)) == NULL)
            perror("opendir() error");
        else
        {
            char *result = NULL;
            while ((entry = readdir(dir)) != NULL)
            {

                result = malloc((strlen(pwd) + strlen(entry->d_name) + 2) * sizeof(char));
                strcpy(result, pwd);
                strcat(result, "/");
                strcat(result, entry->d_name);

                if (stat(result, &sb) == 0)
                {
                    if (param[0] == 1 && param[1] == 1)
                    { // -al
                        struct passwd *pw = getpwuid(sb.st_uid);
                        struct group *gr = getgrgid(sb.st_gid);

                        printf((sb.st_mode & S_IFDIR) ? "d" : "-");
                        printf((sb.st_mode & S_IRUSR) ? "r" : "-");
                        printf((sb.st_mode & S_IWUSR) ? "w" : "-");
                        printf((sb.st_mode & S_IXUSR) ? "x" : "-");
                        printf((sb.st_mode & S_IRGRP) ? "r" : "-");
                        printf((sb.st_mode & S_IWGRP) ? "w" : "-");
                        printf((sb.st_mode & S_IXGRP) ? "x" : "-");
                        printf((sb.st_mode & S_IROTH) ? "r" : "-");
                        printf((sb.st_mode & S_IWOTH) ? "w" : "-");
                        printf((sb.st_mode & S_IXOTH) ? "x" : "-");
                        printf(" %s", pw->pw_name);
                        printf(" %s", gr->gr_name);
                        if (strncmp(entry->d_name, ".", 1) == 0 || strncmp(entry->d_name, "..", 2) == 0)
                            printf(" %s %s %s ", VERMELHO, entry->d_name, BRANCO);
                        else if (sb.st_mode & S_IFDIR)
                            printf(" %s %s %s", BRANCO, entry->d_name, BRANCO);
                        else if (sb.st_mode & S_IXUSR)
                            printf(" %s %s %s   ", VERDE, entry->d_name, BRANCO);
                        printf("\n");
                    }
                    else if (param[0] == 1)
                    { // -l
                        struct passwd *pw = getpwuid(sb.st_uid);
                        struct group *gr = getgrgid(sb.st_gid);
                        if (sb.st_mode & S_IFDIR)
                            ;
                        else if (sb.st_mode & S_IXUSR)
                        {
                            printf((sb.st_mode & S_IFDIR) ? "d" : "-");
                            printf((sb.st_mode & S_IRUSR) ? "r" : "-");
                            printf((sb.st_mode & S_IWUSR) ? "w" : "-");
                            printf((sb.st_mode & S_IXUSR) ? "x" : "-");
                            printf((sb.st_mode & S_IRGRP) ? "r" : "-");
                            printf((sb.st_mode & S_IWGRP) ? "w" : "-");
                            printf((sb.st_mode & S_IXGRP) ? "x" : "-");
                            printf((sb.st_mode & S_IROTH) ? "r" : "-");
                            printf((sb.st_mode & S_IWOTH) ? "w" : "-");
                            printf((sb.st_mode & S_IXOTH) ? "x" : "-");
                            printf(" %s", pw->pw_name);
                            printf(" %s", gr->gr_name);
                            printf(" %s %s %s   ", VERDE, entry->d_name, BRANCO);
                            printf("\n");
                        }
                    }
                    else if (param[1] == 1)
                    { // -a
                        if (strncmp(entry->d_name, ".", 1) == 0 || strncmp(entry->d_name, "..", 2) == 0)
                            printf(" %s %s %s ", VERMELHO, entry->d_name, BRANCO);
                        else if (sb.st_mode & S_IFDIR)
                            printf("%s %s %s", BRANCO, entry->d_name, BRANCO);
                        else if (sb.st_mode & S_IXUSR)
                            printf("%s %s %s   ", VERDE, entry->d_name, BRANCO);
                        else if (sb.st_mode & S_IFLNK)
                            printf("%s %s %s   ", CYAN, entry->d_name, BRANCO);
                    }
                    else if (param[2] == 1)
                    {
                        //-R
                        if (sb.st_mode)
                        {
                            cnt = 0;
                            free(tarefa_dinamica);
                            tarefa_dinamica = NULL;
                            tarefa_dinamica = realloc(tarefa_dinamica, (cnt + 1) * sizeof(pthread_t));
                            pthread_create(&tarefa_dinamica[cnt], NULL, listdir, aux_pwd);
                            int c = 0;
                            for (c = 0; c <= cnt; c++)
                            {
                                pthread_join(tarefa_dinamica[c], NULL);
                            }
                            printf("\n");
                            closedir(dir);
                            return;
                        }
                    }
                    else
                    {
                        if (strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0)
                        {
                            if (sb.st_mode & S_IFDIR)
                                printf("%s %s %s", AZUL, entry->d_name, BRANCO);
                            else if (sb.st_mode & S_IXUSR)
                                printf("%s %s %s   ", VERDE, entry->d_name, BRANCO);
                            else if (sb.st_mode & S_IFLNK)
                                printf("%s %s %s   ", CYAN, entry->d_name, BRANCO);
                        }
                    }
                }
            }
            printf("\n");
            closedir(dir);
        }
    }
    else if (strcmp(root->argv[0], "myfind") == 0)
    {
        char pwd[2024] = "\0";
        getcwd(pwd, sizeof(pwd));
        char *aux_pwd = malloc((strlen(pwd) + 1) * sizeof(char));
        strcpy(aux_pwd, pwd);

        sem_init(&pode_cons, 0, 1);
        sem_init(&pode_prod, 0, N - 1);

        int i;
        cnt = prodptr = consptr = nItem = 0;

        myfind = realloc(myfind, N * sizeof(char **));

        myfind[prodptr] = malloc((strlen(aux_pwd) + 2) * sizeof(char));
        strcpy(myfind[prodptr], aux_pwd);
        nItem++;
        prodptr = (prodptr + 1) % N;

        pthread_create(&ponto3, NULL, &produtor, aux_pwd);
        for (i = 0; i < CONSUMIDORES; i++)
        {
            pthread_create(&tidC[i], NULL, &consumidor, (void *)root->argv[1]);
        }
        pthread_join(ponto3, NULL);

        for (i = 0; i < CONSUMIDORES; i++)
        {
            pthread_join(tidC[i], NULL);
        }

        printf("\n");
        return;
    }
}

void *preencher_biblioteca(void *pos)
{
    int i = *((int *)pos);
    DIR *dir;
    struct dirent *entry;
    struct stat sb;

    if ((dir = opendir(diretorios[i])) == NULL)
        perror("opendir() error");
    else
    {
        char *result = NULL;
        while ((entry = readdir(dir)) != NULL)
        {
            int ponto = strcmp(entry->d_name, ".");
            int ponto2 = strcmp(entry->d_name, "..");

            if (ponto2 != 0 && ponto != 0)
            {
                result = malloc((strlen(diretorios[i]) + strlen(entry->d_name) + 2) * sizeof(char));
                strcpy(result, diretorios[i]);
                strcat(result, "/");
                strcat(result, entry->d_name);

                if (stat(result, &sb) == 0 && sb.st_mode & S_IXUSR)
                {
                    // ZONA CRITICA
                    pthread_mutex_lock(&trinco);
                    dicionario = realloc(dicionario, (incremento_dicionario + 1) * sizeof(char **));
                    dicionario[incremento_dicionario] = (char *)malloc((strlen(entry->d_name) + 1) * sizeof(char));
                    strcpy(dicionario[incremento_dicionario], entry->d_name);
                    incremento_dicionario++;
                    pthread_mutex_unlock(&trinco);
                    // FIM DA ZONA CRITICA
                }
            }
        }
    }
    closedir(dir);
}

int parse_path()
{
    char *a;
    int n = 0, j = 0;

    a = strtok(string, ":");
    for (n = 0; a; n++)
    {
        diretorios = realloc(diretorios, (n + 1) * sizeof(char **));
        diretorios[n] = (char *)malloc((strlen(a) + 1) * sizeof(char));
        strcpy(diretorios[n], a);
        //printf("\n\t %s", diretorios[n]);
        a = strtok(NULL, ":");
    }

    return n;
}

void exec_comandos(CMD *root)
{
    if (strcmp(root->argv[0], "cd") == 0) /// comando cd
    {
        if (root->argv[1] == NULL)
        {
            chdir("/");
        }
        else
        {
            chdir(root->argv[1]);
        }
        atualizar_caminho();
        return;
    }
    int nComandos = n_comandos(root);
    int i, fp, status = 0;
    int fds[2 * nComandos];
    CMD *aux = root;
    pid_t pid, wpid;

    // criar os pipes
    for (i = 0; i < nComandos - 1; i++)
    {
        if (pipe(fds + i * 2) < 0)
        {
            perror("pipe(fds)");
            exit(1);
        }
    }
    // Executar
    i = 0;
    while (aux != NULL)
    {
        pid = fork();
        if (pid < 0)
        {
            perror("pid");
            exit(1);
        }
        if (pid == 0)
        {
            if (i != 0) // Le do pipe anterior (se não for o primeiro)
            {
                dup2(fds[(i - 1) * 2], 0); // i - 1 (=) pipe anterior (leitura)
            }
            if (i != nComandos - 1) // Escreve para o pipe (se não for o ultimo)
            {
                dup2(fds[i * 2 + 1], 1); // pipe actual (escrita)
            }
            if (aux->infile != NULL)
            {
                fp = open(aux->infile, O_RDONLY, 0400); // owner read
                if (fp == -1)
                {
                    perror("Ficheiro Inexistente");
                    exit(1);
                }
                dup2(fp, 0);
                close(fp);
            }
            if (aux->outfile != NULL)
            {
                fp = open(aux->outfile, O_WRONLY | O_TRUNC | O_CREAT, 0200); // owner write
                if (fp == -1)
                {
                    perror("Erro ao criar o ficheiro");
                    exit(1);
                }
                dup2(fp, 1);
                close(fp);
            }
            if (aux->errfile != NULL)
            {
                fp = open(aux->errfile, O_WRONLY | O_TRUNC | O_CREAT, 0200); // owner write
                if (fp == -1)
                {
                    perror("Erro ao criar o ficheiro");
                    exit(1);
                }
                dup2(fp, 2);
                close(fp);
            }
            execvp(aux->argv[0], aux->argv);
            perror("execvp");
            exit(1);
        }
        if (i != 0)
        {
            close(fds[((i - 1) * 2)]);
        }
        if (i != nComandos - 1)
        {
            close(fds[i * 2 + 1]);
        }
        aux = aux->next;
        i++;
    }
    while (wait(&status) > 0)
        ;
}

int n_comandos(CMD *root)
{
    int n = 0;
    CMD *aux = root;
    while (aux != NULL)
    {
        n++;
        aux = aux->next;
    }
    return n;
}

void atualizar_caminho()
{
    char pwd[2048];
    getcwd(pwd, sizeof(pwd));
    path = malloc((strlen(pwd) + 9) * sizeof(char));
    strcpy(path, "msh$ ~");
    strcat(path, pwd);
    strcat(path, "$ ");
}

char **
character_name_completion(const char *text, int start, int end)
{
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, character_name_generator);
}

char *
character_name_generator(const char *text, int state)
{
    static int list_index, len;
    char *name;

    if (!state)
    {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = dicionario[list_index++]))
    {
        if (strncmp(name, text, len) == 0)
        {
            return strdup(name);
        }
    }

    return NULL;
}

void *listdir(void *name)
{
    char *diretorio = strdup(name);

    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(diretorio)))
        return;

    printf("\n%s: \n", diretorio);
    while ((entry = readdir(dir)) != NULL)
    {
        if (strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0)
        {
            if (entry->d_type == DT_DIR)
            {
                char *path = NULL;
                path = malloc((strlen(diretorio) + strlen(entry->d_name) + 2) * sizeof(char));
                strcpy(path, diretorio);
                strcat(path, "/");
                strcat(path, entry->d_name);
                printf(" %s %s %s", AZUL, entry->d_name, BRANCO);
                // ZONA CRITICA
                pthread_mutex_lock(&trinco);
                cnt++;
                tarefa_dinamica = realloc(tarefa_dinamica, (cnt + 1) * sizeof(pthread_t));
                pthread_create(&tarefa_dinamica[cnt], NULL, &listdir, path);
                pthread_mutex_unlock(&trinco);
                // FIM ZONA CRITICA
            }
            else
            {
                printf(" %s %s %s   ", VERDE, entry->d_name, BRANCO);
            }
        }
    }
    printf("\n");
    closedir(dir);
}

void *produtor(void *name)
{
    char *diretorio = strdup(name);

    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(diretorio)))
        return;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0)
        {
            if (entry->d_type == DT_DIR)
            {
                sem_wait(&pode_prod);
                pthread_mutex_lock(&trincoP);
                myfind[prodptr] = malloc((strlen(diretorio) + strlen(entry->d_name) + 2) * sizeof(char));
                strcpy(myfind[prodptr], diretorio);
                strcat(myfind[prodptr], "/");
                strcat(myfind[prodptr], entry->d_name);
                int aux = prodptr;
                prodptr = (prodptr + 1) % N;
                nItem++;
                pthread_mutex_unlock(&trincoP);
                sem_post(&pode_cons);
                produtor(myfind[aux]);
            }
        }
    }
    closedir(dir);
}

void *consumidor(void *name)
{
    char *fn;
    fn = (char *)name;
    DIR *dir;
    struct dirent *entry;

    int aux = 0;
    while (nItem != 0)
    {
        sem_wait(&pode_cons);
        pthread_mutex_lock(&trincoC);
        char *diretorio = malloc((strlen(myfind[consptr]) + 1) * sizeof(char));
        strcpy(diretorio, myfind[consptr]);
        if (!(dir = opendir(diretorio)))
            return;
        while ((entry = readdir(dir)) != NULL)
        {
            if (strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0)
            {
                if (fn == NULL)
                {
                    printf("\n ./%s", entry->d_name);
                }
                else if (strcmp(entry->d_name, fn) == 0)
                {
                    printf("\n %s", diretorio);
                    printf("\n ./%s", entry->d_name);
                }
            }
        }
        closedir(dir);
        consptr = (consptr + 1) % N;
        nItem--;
        pthread_mutex_unlock(&trincoC);
        sem_post(&pode_prod);
    }
}
