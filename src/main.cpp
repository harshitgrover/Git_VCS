#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "cli/InitCommand.h"
#include "cli/AddCommand.h"
#include "cli/CommitCommand.h"
#include "cli/DiffCommand.h"
#include "cli/GcCommand.h"
#include "cli/CatFileCommand.h"
#include "cli/LogCommand.h"
#include "cli/StatusCommand.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: minigit <command>\n";
        return 1;
    }
    
    string command_name = argv[1];
    vector<string> args;
    for (int i = 2; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    
    try {
        unique_ptr<minigit::Command> command;
        
        if (command_name == "init") {
            command = make_unique<minigit::InitCommand>();
        } else if (command_name == "add") {
            command = make_unique<minigit::AddCommand>();
        } else if (command_name == "commit") {
            command = make_unique<minigit::CommitCommand>();
        } else if (command_name == "diff") {
            command = make_unique<minigit::DiffCommand>();
        } else if (command_name == "gc") {
            command = make_unique<minigit::GcCommand>();
        } else if (command_name == "cat-file") {
            command = make_unique<minigit::CatFileCommand>();
        } else if (command_name == "log") {
            command = make_unique<minigit::LogCommand>();
        } else if (command_name == "status") {
            command = make_unique<minigit::StatusCommand>();
        } else {
            cerr << "Unknown command: " << command_name << "\n";
            return 1;
        }
        
        command->execute(args);
        
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
