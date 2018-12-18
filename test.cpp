#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <glob.h>
#include <signal.h>
#include <sys/times.h>

//#include <sys/stat.h>
//#include <fnmatch.h>
//#include <dirent.h>

using namespace std;

void quarter_pipe (vector < vector<char *> > & argv, int n) {
        for (int i = 0; i < n - 1; i++) {
                int fd[2];
                pipe(fd);
                pid_t pid = fork();
                if (pid == 0) {
                        dup2(fd[1], 1);
                        close(fd[0]);
                        execvp(argv[i][0], &argv[i][0]);
                        exit(0);
                } else {
                        close(fd[1]);
                        dup2(fd[0], 0);
                }
        }
        execvp(argv[n-1][0], &argv[n-1][0]);
        exit(0);
}

int main () {
        signal(SIGINT, SIG_IGN);
        uid_t uid=getuid();
        for(;;) {
                int flag;
                int oshibka = 0;
                char dir[1000];
                char *path;
                path = getcwd(dir, 1000);
                if (uid == 0) {
                        printf("%s! ", path);
                } else {
                        printf("%s> ", path);
                }

        //-------------------------------убираем_лишние_пробелы_и_табуляцию-----------------------------------
                string text;
                getline (cin, text);

                if (cin.eof() && (text.size() == 0)) {
                        printf("\n");
                        exit(0);
                }


                while (text[0] == ' ' || text[0] == '\t') {
                        text.erase(0,1);
                }
                int i = min(text.find(' '),text.find('\t'));
                while (i != -1) {
                        while (text[i+1] == ' ' || text[i+1] == '\t') {
                                text.erase(i+1,1);
                        }
                        i = min(text.find(' ', i+1),text.find('\t', i+1));
                }
                if (text[text.length() - 1] == ' ' || text[text.length() - 1] == '\t') {
                        text.erase(text.length() - 1,1);
                }


                if (text.size() == 0) {
                        continue;
                }

                flag = (text.find("|") == string::npos) ? 0 : 1;
                int number = count(text.begin(), text.end(), '|') + 1;

                i  = min(text.find(' '),text.find('\t'));
                string word;
                vector <string> myvector;
                while(i != -1) {
                        word = text.substr(0,i);
                        myvector.push_back(word);
                        text.erase(0,i+1);
                        i  = min(text.find(' '),text.find('\t'));
                }
                i = text.length();
                if (i > 0) {
                        word = text.substr(0,i);
                        myvector.push_back(word);
                        text.erase(0,i);
                }

                string file_in = "", file_out = "";
                if (flag == 0) {          //------------нет_пайпа-------------
                        if (myvector[0] == "time" && myvector.size() == 1) {
                                fprintf(stderr, "real\t%lf\nuser\t%lf\nsys\t%lf\n", 10000 * (double)(0) / CLOCKS_PER_SEC, 10000 * (double)(0) / CLOCKS_PER_SEC, 10000 * (double)(0) / CLOCKS_PER_SEC);
                                continue;
                        }
                        int l = 0, g = 0, mistake = 0;
                        for (i = 0; i < myvector.size();) {
                                if (myvector[i] == ">") {
                                        if (g != 0)
                                                fprintf(stderr,"microsha: слишком много «>», ну да ладно... \n");
                                        g++;
                                        if (i != myvector.size() - 1)
                                                file_out = myvector[i+1];
                                        else {
                                                fprintf(stderr,"microsha: синтаксическая ошибка рядом с неожиданным маркером «newline» \n");
                                                mistake = 1;
                                                break;
                                        }
                                        if (i == 0) {                       //добавлено
                                                mistake = 1;
                                                break;
                                        }
                                        myvector.erase(myvector.begin() + i);
                                        myvector.erase(myvector.begin() + i);
                                } else
                                        i++;
                        }
                        for (i = 0; i < myvector.size();) {
                                if (myvector[i] == "<") {
                                        if (l != 0)
                                                fprintf(stderr,"microsha: слишком много «<», ну да ладно... \n");
                                        l++;
                                        if (i != myvector.size() - 1)
                                                file_in = myvector[i+1];
                                        else {
                                                fprintf(stderr,"microsha: синтаксическая ошибка рядом с неожиданным маркером «newline» \n");
                                                mistake = 1;
                                                break;
                                        }
                                        if (i == 0) {                           //добавлено
                                                mistake = 1;
                                                break;
                                        }
                                        myvector.erase(myvector.begin() + i);
                                        myvector.erase(myvector.begin() + i);
                                } else
                                        i++;
                        }
                        if (mistake == 1) {
                               continue;
                        }
                        vector <string> victor; //necessary vector
                        for (i = 0; i < myvector.size(); i++) {
                                if (myvector[i].find('*') != string::npos || myvector[i].find('?') != string::npos) {
                                        glob_t result;
                                        int glock = glob(myvector[i].c_str(), GLOB_TILDE, NULL, &result);
                                        if (glock != 0) {
                                                globfree(&result);
                                                perror("microsha: metasymbol function failed");
                                                oshibka = 1;
                                        }
                                        vector <string> files;
                                        for (size_t j = 0; j < result.gl_pathc; j++) {
                                                files.push_back(string(result.gl_pathv[j]));
                                        }
                                        globfree(&result);
                                        for (int k = 0; k < files.size(); k++) {
                                                victor.push_back(files[k]);
                                        }
                                } else {
                                        victor.push_back(myvector[i]);
                                }
                        }
                        vector <char *> argv;
                        for (i = 0; i < victor.size(); i++) {
                                argv.push_back((char *)victor[i].c_str());
                        }
                        argv.push_back(NULL);

                        if (oshibka == 1) {                     // добавлено
                                continue;
                        }

                        if (victor[0] == "exit") {
                                cout << "exit" << endl;
                                exit(0);
                        } else if (victor[0] == "cd") {
                                if (argv[1] != NULL) {          //mb try flag...но тогда не работает "cd .."
                                        chdir(argv[1]);
                                } else {
                                        chdir(::getenv("HOME"));
                                }
                        }
                        pid_t pid = fork();
                        if (pid == 0) {
                                signal(SIGINT, SIG_DFL);
                                if (! file_out.empty()) {
                                        dup2(open(file_out.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0666), 1);
                                }
                                if (! file_in.empty()) {
                                        dup2(open(file_in.c_str(), O_RDWR, 0666), 0);
                                }
                                if (victor[0] == "cd") {
                                } else if (victor[0] == "time") {
                                        execvp(argv[1], &argv[1]);
                                        fprintf(stderr, "microsha: %s: команда не найдена\n", argv[0]);
                                } else {
                                        execvp(argv[0], &argv[0]);
                                        fprintf(stderr, "microsha: %s: команда не найдена\n", argv[0]);
                                }
                                exit(0);
                        } else {
                                tms buf_start;
                                tms buf_end;
                                clock_t wall_start;
                                clock_t wall_end;
                                if (victor[0] == "time") {
                                        wall_start = times(&buf_start);
                                }
                                wait(NULL);
                                if (victor[0] == "time") {
                                        wall_end = times(&buf_end);
                                        fprintf(stderr, "real\t%lf\nuser\t%lf\nsys\t%lf\n", 10000 * (double)(wall_end - wall_start) / CLOCKS_PER_SEC, 10000 * (double)(buf_end.tms_cutime - buf_start.tms_cutime) / CLOCKS_PER_SEC, 10000 * (double)(buf_end.tms_cstime - buf_start.tms_cstime) / CLOCKS_PER_SEC);
                                }
                        }
                        wait(NULL);
                } else {
                        vector < vector <char *> > argv(number);
                        vector<char *> trash;
                        int j = 0, mistake = 0;
                        for (i = 0; i < myvector.size(); i++) {
                                if (myvector[i] == "|") {
                                        argv[j].push_back(NULL);
                                        j++;
                                } else if (myvector[i] == ">") {
                                        if (j == number - 1) {
                                                if (i != myvector.size() - 1 && myvector[i+1] != "|")
                                                        file_out = myvector[i+1];
                                                else {
                                                        fprintf(stderr,"microsha: синтаксическая ошибка рядом с неожиданным маркером «newline» \n");
                                                        mistake = 1;
                                                        break;
                                                }
                                                myvector.erase(myvector.begin() + i + 1);
                                        } else {
                                                fprintf(stderr,"microsha: промах, ну да ладно... \n");
                                                mistake = 1;
                                                continue;
                                        }
                                } else if (myvector[i] == "<") {
                                        if (j == 0) {
                                                if (i != myvector.size() - 1 && myvector[i+1] != "|")
                                                        file_in = myvector[i+1];
                                                else {
                                                        fprintf(stderr,"microsha: синтаксическая ошибка рядом с неожиданным маркером «|» \n");
                                                        mistake = 1;
                                                        break;
                                                }
                                                myvector.erase(myvector.begin() + i + 1);
                                        } else {
                                                fprintf(stderr,"microsha: промах, ну да ладно... \n");
                                                mistake = 1;
                                                continue;
                                        }
                                }
                                else if (myvector[i].find('*') != string::npos || myvector[i].find('?') != string::npos) {
                                        glob_t result;
                                        int glock = glob(myvector[i].c_str(), GLOB_TILDE, NULL, &result);
                                        if (glock != 0) {
                                                globfree(&result);
                                                perror("microsha: metasymbol function failed");
                                        }
                                        for (size_t k = 0; k < result.gl_pathc; k++) {
                                                char *buf = (char *)malloc((2 + strlen(result.gl_pathv[k])) * sizeof(char));
                                                strcpy(buf, result.gl_pathv[k]);
                                                argv[j].push_back(buf);
                                                trash.push_back(buf);
                                        }
                                        globfree(&result);

                                } else {
                                        argv[j].push_back((char *)myvector[i].c_str());
                                }
                        }
                        argv[j].push_back(NULL);
                        for (int i = 0; i < trash.size(); i++) free(trash[i]);
                        if (mistake == 1) {
                               continue;
                        }
                        pid_t pid = fork();
                        if (pid == 0) {
                                signal(SIGINT, SIG_DFL);
                                if (! file_out.empty()) {
                                        close(1);
                                        dup2(open(file_out.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0666), 1);
                                }
                                if (! file_in.empty()) {
                                        close(0);
                                        dup2(open(file_in.c_str(), O_RDWR, 0666), 0);
                                }
                                quarter_pipe(argv, number);
                        } else {
                                wait(NULL);
                        }
                }
        }
        cout << endl;
        wait(NULL);
        return 0;
}
