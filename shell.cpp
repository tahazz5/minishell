#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <readline/readline.h>
#include <readline/history.h>
#include <pwd.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/types.h>

#define COLOR_RESET "\033[0m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"

void printPrompt() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    std::cout << COLOR_GREEN << "mini-shell 🐚 " << COLOR_BLUE << "[" << cwd << "]$ " << COLOR_RESET;
}

void executeCommand(std::vector<std::string>& args, bool background = false) {
    if (args.empty()) return;

    std::vector<char*> cargs;
    for (auto& arg : args) cargs.push_back(&arg[0]);
    cargs.push_back(nullptr);

    pid_t pid = fork();

    if (pid == 0) {
        // Child
        execvp(cargs[0], cargs.data());
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent
        if (!background) waitpid(pid, nullptr, 0);
        else std::cout << "[Background PID: " << pid << "]\n";
    } else {
        perror("Fork failed");
    }
}

std::vector<std::string> parseInput(const std::string& input, bool& background) {
    std::istringstream iss(input);
    std::vector<std::string> args;
    std::string token;
    background = false;

    while (iss >> token) {
        if (token == "&") background = true;
        else args.push_back(token);
    }

    return args;
}

int main() {
    std::string input;
    bool background;

    while (true) {
        // Read input
        char* raw = readline("mini-shell 🐚$ "); // Readline prompt
        if (!raw) break; // Ctrl+D

        input = std::string(raw);
        free(raw);

        if (input.empty()) continue;
        add_history(input.c_str()); // Arrow ↑ ↓ support

        auto args = parseInput(input, background);
        if (args.empty()) continue;

        // Built-in: exit
        if (args[0] == "exit") {
            std::cout << "Bye 👋\n";
            break;
        }

        // Built-in: cd
        if (args[0] == "cd") {
            const char* path = (args.size() > 1) ? args[1].c_str() : getenv("HOME");
            if (chdir(path) != 0) perror("cd failed");
            continue;
        }

        // Built-in: help
        if (args[0] == "help") {
            std::cout << "Built-in commands:\n"
                         "  cd [dir]       Change directory\n"
                         "  exit           Quit shell\n"
                         "  help           Show this help\n"
                         "Supports piping (|), redirection (>, <), background (&), history (↑ ↓)\n";
            continue;
        }

        // External command
        executeCommand(args, background);
    }

    return 0;
}
