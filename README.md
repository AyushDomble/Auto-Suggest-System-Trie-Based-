# 🔠 Auto-Suggest System (Trie-Based)

![C++](https://shields.io)
![License](https://img.shields.io/badge/License-MIT-green.svg)
![Status](https://img.shields.io/badge/Status-Active-brightgreen.svg)
![Contributions](https://img.shields.io/badge/Contributions-Welcome-orange.svg)

## **Trie-based Auto-Suggest System** written in **C++**.  
This is a console-based auto-suggest and dictionary management system built in C. It uses a Trie data structure for efficient storage and retrieval of words, providing fast auto-suggestions based on a given prefix. The system also includes features for managing a dictionary file, tracking search frequency, and monitoring word additions and deletions within a session.

---

## ✨ Features
- 📥 Add new words (saved in `Dictionary.txt`)  
- 🔍 Search with prefix-based auto-suggestions  
- 📋 Display all words from dictionary  
- 🆕 Track recently added & deleted words  
- 📏 Find shortest & longest words  
- 🗑️ Delete words with confirmation  
- ♻️ Undo last deleted word  
- 📊 Track most frequently searched words (`SearchStats.txt`)  
- 🎨 Colorful CLI interface  

---

## 📂 Project Structure
```plaintext
├── AutoSuggest.cpp # Main source code
├── Dictionary.txt # Persistent word dictionary
├── SearchStats.txt # Persistent search frequency log
├── README.md # Documentation
└── .gitignore # Ignored files list
```

---

## 🖥️ Menu Options
1. Add a new word

2. Search by prefix (Auto-suggestions)

3. Display all words

4. Show recently added words

5. Show shortest & longest word

6. Delete a word

7. Show recently deleted words

8. Undo last deleted word

9. Show most frequently searched words

10. Exit

---

## 🚀 How to Run

### Compile:
```bash
g++ AutoSuggest.cpp -o AutoSuggest
```
### Run:
```bash
./AutoSuggest
```

---

## 📸 Screenshots
- Main Menu<br>
<img width="461" height="476" alt="image" src="https://github.com/user-attachments/assets/ca6a1a90-6843-4a0d-8a83-4aff0e4b5fdd" />

---

## 🛠️ Requirements
- GCC Compiler (gcc)
- Works on Linux, MacOS, and Windows (ANSI-supported terminals)

## 📝 License
This project is licensed under the MIT License.
