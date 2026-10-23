#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace color {
constexpr const char* red = "\x1b[1;31m";
constexpr const char* green = "\x1b[32m";
constexpr const char* yellow = "\x1b[33m";
constexpr const char* cyan = "\x1b[36m";
constexpr const char* magenta = "\x1b[35m";
constexpr const char* orange = "\x1b[93m";
constexpr const char* grey = "\x1b[90m";
constexpr const char* boldYellow = "\x1b[1;33m";
constexpr const char* boldCyan = "\x1b[1;36m";
constexpr const char* reset = "\x1b[0m";
}

const char* const dictionaryFile = "Dictionary.txt";
const char* const statsFile = "SearchStats.txt";

std::string toLowerCase(std::string word) {
    for (char& ch : word)
        if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
    return word;
}

bool validWord(const std::string& word) {
    return !word.empty() && word.size() < 100 &&
        std::all_of(word.begin(), word.end(), [](char c) { return c >= 'a' && c <= 'z'; });
}

// Each node owns its children. Destroying a trie releases all nodes automatically.
class Trie {
    struct Node {
        std::array<std::unique_ptr<Node>, 26> children{};
        bool isEndOfWord = false;
    };
    Node root;

    const Node* searchPrefix(const std::string& prefix) const {
        const Node* node = &root;
        for (char ch : prefix) {
            if (ch < 'a' || ch > 'z' || !node->children[ch - 'a']) return nullptr;
            node = node->children[ch - 'a'].get();
        }
        return node;
    }

    static void collectWords(const Node& node, std::string& word,
                             std::vector<std::string>& words) {
        if (node.isEndOfWord) words.push_back(word);
        for (std::size_t i = 0; i < node.children.size(); ++i) {
            if (node.children[i]) {
                word.push_back(static_cast<char>('a' + i));
                collectWords(*node.children[i], word, words);
                word.pop_back();
            }
        }
    }

    static bool deleteWordHelper(Node& node, const std::string& word, std::size_t depth) {
        if (depth == word.size()) node.isEndOfWord = false;
        else {
            auto& child = node.children[word[depth] - 'a'];
            if (child && deleteWordHelper(*child, word, depth + 1)) child.reset();
        }
        return !node.isEndOfWord && std::all_of(node.children.begin(), node.children.end(),
            [](const std::unique_ptr<Node>& child) { return !child; });
    }

public:
    void insert(const std::string& word) {
        if (!validWord(word)) return;
        Node* node = &root;
        for (char ch : word) {
            auto& child = node->children[ch - 'a'];
            if (!child) child = std::make_unique<Node>();
            node = child.get();
        }
        node->isEndOfWord = true;
    }

    bool searchWord(const std::string& word) const {
        const Node* node = searchPrefix(word);
        return node && node->isEndOfWord;
    }

    void deleteWord(const std::string& word) {
        if (searchWord(word)) deleteWordHelper(root, word, 0);
    }

    std::vector<std::string> wordsWithPrefix(std::string prefix = "") const {
        std::vector<std::string> words;
        const Node* node = searchPrefix(prefix);
        if (node) collectWords(*node, prefix, words);
        return words;
    }
};

struct WordFrequency {
    std::string word;
    unsigned long long frequency;
};

class DictionaryApp {
    Trie trie;
    std::vector<std::string> sessionWords;
    std::vector<std::string> deletedSessionWords;
    std::vector<WordFrequency> wordFreqList;

    static void message(const char* tint, const std::string& text) {
        std::cout << tint << text << '\n' << color::reset;
    }

    static void printWords(const std::vector<std::string>& words, const char* tint = color::cyan) {
        for (const auto& word : words) message(tint, " - " + word);
    }

    static bool readWord(const char* prompt, std::string& word) {
        std::cout << color::grey << prompt << color::reset;
        if (!std::getline(std::cin, word)) return false;
        word = toLowerCase(word);
        if (!validWord(word)) {
            message(color::red, "Enter a word or prefix containing 1-99 letters (a-z) only.");
            return false;
        }
        return true;
    }

    bool saveWordToFile(const std::string& word) {
        // Add a separator even if the existing file has no final newline.
        std::ifstream existing(dictionaryFile, std::ios::binary | std::ios::ate);
        bool needsNewline = false;
        if (existing && existing.tellg() > 0) {
            existing.seekg(-1, std::ios::end);
            char last = '\n';
            existing.get(last);
            needsNewline = last != '\n';
        }
        existing.close();
        std::ofstream file(dictionaryFile, std::ios::app);
        if (needsNewline) file << '\n';
        file << word << '\n';
        file.close();
        if (file.fail()) {
            message(color::red, "Error writing dictionary file!");
            return false;
        }
        return true;
    }

    bool removeWordFromFile(const std::string& word) {
        std::ifstream file(dictionaryFile);
        if (!file) {
            message(color::red, "Error opening dictionary file!");
            return false;
        }
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (toLowerCase(line) != word) lines.push_back(line);
        }
        if (file.bad()) {
            message(color::red, "Error reading dictionary file!");
            return false;
        }
        file.close();
        std::ofstream output(dictionaryFile, std::ios::trunc);
        for (const auto& kept : lines) output << kept << '\n';
        output.close();
        if (output.fail()) {
            message(color::red, "Error writing dictionary file!");
            return false;
        }
        return true;
    }

    void loadDictionary() {
        std::ifstream file(dictionaryFile);
        if (!file) {
            message(color::orange, "Warning: Dictionary file not found. Proceeding with an empty trie.");
            return;
        }
        std::string word;
        while (std::getline(file, word)) {
            if (!word.empty() && word.back() == '\r') word.pop_back();
            trie.insert(toLowerCase(word));
        }
        message(color::green, "Dictionary loaded successfully!");
    }

    void loadSearchStats() {
        std::ifstream file(statsFile);
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream record(line);
            std::string word;
            long long frequency = 0;
            if (record >> word >> frequency && validWord(word) && frequency > 0)
                wordFreqList.push_back({word, static_cast<unsigned long long>(frequency)});
        }
    }

    void saveSearchStats() const {
        std::ofstream file(statsFile);
        for (const auto& entry : wordFreqList) file << entry.word << ' ' << entry.frequency << '\n';
        file.close();
        if (file.fail()) message(color::red, "Error saving search statistics!");
    }

    void autoSuggest(const std::string& prefix) {
        const auto words = trie.wordsWithPrefix(prefix);
        if (words.empty()) {
            message(color::red, "No suggestions found.");
            return;
        }
        auto entry = std::find_if(wordFreqList.begin(), wordFreqList.end(),
            [&prefix](const WordFrequency& item) { return item.word == prefix; });
        if (entry == wordFreqList.end()) wordFreqList.push_back({prefix, 1});
        else ++entry->frequency;
        message(color::green, "Suggestions:");
        printWords(words);
    }

    void showShortestLongestWord() const {
        const auto words = trie.wordsWithPrefix();
        if (words.empty()) {
            message(color::red, "Trie is empty.");
            return;
        }
        const auto limits = std::minmax_element(words.begin(), words.end(),
            [](const std::string& a, const std::string& b) { return a.size() < b.size(); });
        message(color::magenta, "Shortest word(s):");
        for (const auto& word : words)
            if (word.size() == limits.first->size()) message(color::cyan, " - " + word);
        message(color::magenta, "Longest word(s):");
        for (const auto& word : words)
            if (word.size() == limits.second->size()) message(color::cyan, " - " + word);
    }

    void deleteWord(const std::string& word) {
        if (!trie.searchWord(word)) {
            message(color::red, "Word not found in Trie.");
            return;
        }
        std::cout << color::orange << "Are you sure you want to delete \"" << word
                  << "\"? (y/n): " << color::reset;
        std::string confirm;
        if (!std::getline(std::cin, confirm) || (confirm != "y" && confirm != "Y")) {
            message(color::yellow, "Deletion cancelled.");
            return;
        }
        if (!removeWordFromFile(word)) return;
        trie.deleteWord(word);
        const auto added = std::find(sessionWords.begin(), sessionWords.end(), word);
        if (added != sessionWords.end()) sessionWords.erase(added);
        deletedSessionWords.push_back(word);
        message(color::green, "Word deleted successfully from Trie.");
    }

    void undoDelete() {
        if (deletedSessionWords.empty()) {
            message(color::yellow, "No deleted words to undo.");
            return;
        }
        const std::string word = deletedSessionWords.back();
        if (!saveWordToFile(word)) return;
        trie.insert(word);
        sessionWords.push_back(word);
        deletedSessionWords.pop_back();
        message(color::green, "Successfully restored \"" + word + "\" to Trie and Dictionary.");
    }

public:
    void run() {
        loadDictionary();
        loadSearchStats();
        while (std::cin) {
            std::cout << '\n' << color::boldCyan << "--- Auto-Suggest System ---\n"
                      << color::reset << "\x1b[38;5;208m"
                      << "1. Add a new word\n"
                      << "2. Search by prefix (Auto-suggestions)\n"
                      << "3. Display all words\n"
                      << "4. Show recently added words\n"
                      << "5. Show shortest & longest word\n"
                      << "6. Delete a word\n"
                      << "7. Show recently deleted words\n"
                      << "8. Undo last deleted word\n"
                      << "9. Show most frequently searched words\n"
                      << "10. Exit\n" << color::reset << "Enter your choice: ";
            std::string input, word;
            if (!std::getline(std::cin, input)) break;
            std::istringstream choiceInput(input);
            int choice = 0;
            char extra = 0;
            if (!(choiceInput >> choice) || (choiceInput >> extra)) choice = 0;
            switch (choice) {
            case 1:
                if (readWord("Enter word to add: ", word) && saveWordToFile(word)) {
                    trie.insert(word);
                    sessionWords.push_back(word);
                    message(color::green, "Word added successfully!");
                }
                break;
            case 2:
                if (readWord("Enter prefix: ", word)) autoSuggest(word);
                break;
            case 3:
                message(color::orange, "All words in dictionary:");
                printWords(trie.wordsWithPrefix());
                break;
            case 4:
                if (sessionWords.empty()) message(color::yellow, "No words added during this session.");
                else {
                    message(color::boldYellow, "Recently Added Words (This Session):");
                    printWords(sessionWords);
                }
                break;
            case 5: showShortestLongestWord(); break;
            case 6:
                if (readWord("Enter word to delete: ", word)) deleteWord(word);
                break;
            case 7:
                if (deletedSessionWords.empty()) message(color::yellow, "No words deleted during this session.");
                else {
                    message(color::red, "Recently Deleted Words (This Session):");
                    printWords(deletedSessionWords, "\x1b[31m");
                }
                break;
            case 8: undoDelete(); break;
            case 9:
                if (wordFreqList.empty()) message(color::yellow, "No search history found.");
                else {
                    message(color::boldYellow, "Most Frequently Searched Words:");
                    for (const auto& entry : wordFreqList)
                        message(color::cyan, " - " + entry.word + " (" +
                            std::to_string(entry.frequency) + " times)");
                }
                break;
            case 10:
                saveSearchStats();
                message(color::boldYellow, "PROGRAM EXITED SUCCESSFULLY.");
                return;
            default: message(color::red, "Invalid choice! Please try again.");
            }
        }
        saveSearchStats();
        message(color::boldYellow, "PROGRAM EXITED SUCCESSFULLY.");
    }
};

int main() {
    DictionaryApp app;
    app.run();
}
