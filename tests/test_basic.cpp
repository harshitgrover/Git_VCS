#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "core/Repository.h"
#include "core/Blob.h"
#include "core/Diff.h"
#include "core/Chunker.h"
#include "core/Index.h"

using namespace std;
namespace fs = std::filesystem;

class MiniGitTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = "test_repo_cpp";
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
        fs::create_directories(test_dir);
    }

    void TearDown() override {
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    string test_dir;
};

TEST_F(MiniGitTest, RepoInit) {
    auto repo = minigit::Repository::init(test_dir);
    
    fs::path gitdir = fs::path(test_dir) / ".minigit";
    EXPECT_TRUE(fs::is_directory(gitdir));
    EXPECT_TRUE(fs::is_directory(gitdir / "objects"));
    EXPECT_TRUE(fs::is_directory(gitdir / "refs" / "heads"));
    EXPECT_TRUE(fs::is_regular_file(gitdir / "HEAD"));
}

TEST_F(MiniGitTest, AddAndCommit) {
    auto repo = minigit::Repository::init(test_dir);
    
    fs::path file_path = fs::path(test_dir) / "hello.txt";
    ofstream out(file_path);
    out << "hello world!";
    out.close();
    
    repo.add(file_path.string());
    
    fs::path index_path = fs::path(test_dir) / ".minigit" / "index";
    EXPECT_TRUE(fs::exists(index_path));
    
    repo.commit("initial commit");
    
    fs::path ref_path = fs::path(test_dir) / ".minigit" / "refs" / "heads" / "main";
    EXPECT_TRUE(fs::exists(ref_path));
}

TEST_F(MiniGitTest, MyersDiffAlgorithm) {
    minigit::MyersDiffStrategy diff;
    vector<string> old_lines = {"A", "B", "C"};
    vector<string> new_lines = {"A", "X", "C"};
    
    auto edits = diff.compute(old_lines, new_lines);
    
    ASSERT_EQ(edits.size(), 4);
    EXPECT_EQ(edits[0].text, "A");
    EXPECT_EQ(edits[0].type, minigit::EditType::EQUAL);
    EXPECT_EQ(edits[1].text, "B");
    EXPECT_EQ(edits[1].type, minigit::EditType::DELETE);
    EXPECT_EQ(edits[2].text, "X");
    EXPECT_EQ(edits[2].type, minigit::EditType::INSERT);
    EXPECT_EQ(edits[3].text, "C");
    EXPECT_EQ(edits[3].type, minigit::EditType::EQUAL);
}

TEST_F(MiniGitTest, ContentDefinedChunking) {
    auto repo = minigit::Repository::init(test_dir);
    
    string large_data;
    for (int i = 0; i < 10000; ++i) {
        large_data += "abc" + to_string(i);
    }
    
    minigit::Blob blob1(large_data);
    string hash1 = blob1.write(repo);
    
    string mod_data = "X" + large_data.substr(1);
    minigit::Blob blob2(mod_data);
    string hash2 = blob2.write(repo);
    
    auto chunks1 = minigit::Chunker::chunk(large_data);
    auto chunks2 = minigit::Chunker::chunk(mod_data);
    
    EXPECT_GT(chunks1.size(), 2);
    EXPECT_EQ(chunks1.size(), chunks2.size());
    
    int identical = 0;
    for (size_t i = 1; i < chunks1.size(); ++i) {
        if (chunks1[i] == chunks2[i]) identical++;
    }
    
    EXPECT_EQ(identical, chunks1.size() - 1);
}

TEST_F(MiniGitTest, GarbageCollection) {
    auto repo = minigit::Repository::init(test_dir);
    
    string data1;
    for (int i = 0; i < 5000; ++i) data1 += "data";
    minigit::Blob blob1(data1);
    string hash1 = blob1.write(repo);
    
    string data2 = "X" + data1;
    minigit::Blob blob2(data2);
    string hash2 = blob2.write(repo);
    
    minigit::Index idx(repo.getGitDir());
    idx.add("file.txt", hash2);
    idx.write();
    
    int count_before = 0;
    for (auto& entry : fs::recursive_directory_iterator(fs::path(repo.getGitDir()) / "objects")) {
        if (entry.is_regular_file()) count_before++;
    }
    
    string bin_path = (fs::current_path() / "build" / "minigit").string();
    string cmd = "cd " + test_dir + " && " + bin_path + " gc";
    int ret = system(cmd.c_str());
    EXPECT_EQ(ret, 0);
    
    int count_after = 0;
    for (auto& entry : fs::recursive_directory_iterator(fs::path(repo.getGitDir()) / "objects")) {
        if (entry.is_regular_file()) count_after++;
    }
    
    EXPECT_LT(count_after, count_before);
}

TEST_F(MiniGitTest, DiagnosticCommands) {
    auto repo = minigit::Repository::init(test_dir);
    string bin_path = (fs::current_path() / "build" / "minigit").string();
    
    string file1 = (fs::path(test_dir) / "file1.txt").string();
    ofstream out1(file1);
    out1 << "first line\n";
    out1.close();
    
    system(("cd " + test_dir + " && " + bin_path + " add file1.txt").c_str());
    system(("cd " + test_dir + " && " + bin_path + " commit -m \"first commit\"").c_str());
    
    ofstream out2(file1, ios::app);
    out2 << "second line\n";
    out2.close();
    
    system(("cd " + test_dir + " && " + bin_path + " add file1.txt").c_str());
    system(("cd " + test_dir + " && " + bin_path + " commit -m \"second commit\"").c_str());
    
    string log_file = (fs::path(test_dir) / "log.out").string();
    string cmd = "cd " + test_dir + " && " + bin_path + " log > log.out";
    int ret = system(cmd.c_str());
    EXPECT_EQ(ret, 0);
    
    ifstream in(log_file);
    string log_content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    
    EXPECT_NE(log_content.find("second commit"), string::npos);
    EXPECT_NE(log_content.find("first commit"), string::npos);
    
    ofstream out3(file1, ios::app);
    out3 << "untracked line\n";
    out3.close();
    
    string status_file = (fs::path(test_dir) / "status.out").string();
    string cmd_status = "cd " + test_dir + " && " + bin_path + " status > status.out";
    ret = system(cmd_status.c_str());
    EXPECT_EQ(ret, 0);
    
    ifstream in_status(status_file);
    string status_content((istreambuf_iterator<char>(in_status)), istreambuf_iterator<char>());
    EXPECT_NE(status_content.find("modified:   file1.txt"), string::npos);
}
