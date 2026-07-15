#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "cli/InitCommand.hpp"
#include "cli/AddCommand.hpp"
#include "cli/CommitCommand.hpp"
#include "cli/DiffCommand.hpp"
#include "cli/GcCommand.hpp"
#include "cli/CatFileCommand.hpp"
#include "cli/LogCommand.hpp"
#include "cli/StatusCommand.hpp"
#include "cli/BranchCommand.hpp"
#include "cli/CheckoutCommand.hpp"
#include "cli/MergeCommand.hpp"
#include "cli/RestoreCommand.hpp"
#include "cli/SwitchCommand.hpp"

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
        } else if (command_name == "branch") {
            command = make_unique<minigit::BranchCommand>();
        } else if (command_name == "checkout") {
            command = make_unique<minigit::CheckoutCommand>();
        } else if (command_name == "merge") {
            command = make_unique<minigit::MergeCommand>();
        } else if (command_name == "restore") {
            command = make_unique<minigit::RestoreCommand>();
        } else if (command_name == "switch") {
            command = make_unique<minigit::SwitchCommand>();
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
