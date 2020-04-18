/*
Data adapter by team BryceZhic (adapted by Caden Shelman and Terry Roy)
Zhichao Chen and Bryce Russell-Benoit

https://github.com/ZhichaoC/A3Prize/tree/master/submission/part1
*/

#include "../dataframe/dataframe.h"
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm> 
#include <vector>

using namespace std;


// credit to https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
// find out the argument of given option
// char* getCmdOption(char** begin, char** end, const string& option)
// {
//     char** itr = find(begin, end, option);

//     if (itr != end && ++itr != end)
//     {
//         return *itr;
//     }

//     cout << "Insufficient number of arguments provided for flag " << option << "\n";
//     exit(1);
// }

// // find out the arguments of given option and modify given value1 and value2
// void getCmdOptions(char** begin, char** end, const string& option, size_t& value1, size_t& value2)
// {
//     char** itr = find(begin, end, option);
//     if (itr != end && ++itr != end)
//     {
//         value1 = atol(*itr);

//         if (++itr != end) {
//             value2 = atol(*itr);
//             return;
//         }
//     }
//     cout << "Insufficient number of arguments provided for flag " << option << "\n";
//     exit(1);
// }

// // determines if the option exist
// bool cmdOptionExists(char** begin, char** end, const string& option)
// {
//     return find(begin, end, option) != end;
// }

// count number of elements in the given line
int count_element(string line) {

    int count = 0;
    int leftBrackets = 0;

    for (int i = 0; i < line.length(); i++) {
        if (line.at(i) == '<') {
            leftBrackets += 1;
        }
        else if (line.at(i) == '>') {
            // increase count only when the outermost < is closed
            if (leftBrackets == 1) {
                count += 1;
            }
            leftBrackets -= 1;
        }
    }

    return count;
}

// find the type of the given input
// if BOOL or empty, return 1
// if INT, return 2
// if FLOAT, return 3
// if STRING, return 4
int compute_type(string input) {

    if (input.length() == 0 || input == "1" || input == "0") {
        return 1;
    }

    bool hasDecimalPoint = false;

    char firstChar = input.at(0);
    int index = 0;
    if (firstChar == '+' || firstChar == '-') {
        index = 1;
    }

    for (index; index < input.length(); index++) {
        char c = input.at(index);
        if (c == '.') {
            if (!hasDecimalPoint) {
                hasDecimalPoint = true;
            }
            else {
                // so that 1.2.3 is a string
                return 4;
            }
        }
        else if (!isdigit(c)) {
            // if not a digit, must be a string
            return 4;
        }
    }

    if (!hasDecimalPoint) {
        return 2;
    }
    return 3;
}

// for a given line, replace invalid value with "" and return
// a list of values (don't care types)
vector<string> parse_line(string line) {
    vector<string> wordList;

    int leftDelimiters = 0;
    int leftDelimiterIndex = 0;
    int rightDelimiterIndex = 0;
    int start = 0;
    int end = 0;

    int quotation = 0;
    int spaceIndex = -1;

    for (int i = 0; i < line.length(); i++) {
        char character = line.at(i);
        if (character == '<') {
            // first time sees <, set index to outermost <
            if (leftDelimiterIndex <= rightDelimiterIndex) {
                leftDelimiterIndex = i;
            }
            leftDelimiters += 1;
        }
        else if (character == '>') {
            // if not outermost >, don't parse the value
            if (leftDelimiters > 1) {
                leftDelimiters -= 1;
                continue;
            }

            rightDelimiterIndex = i;

            // if no content in <>, push ""
            if (!(start > leftDelimiterIndex)) {
                wordList.push_back("");
            }
            else if (spaceIndex > 0) {
                // if <> has space in between characters and no "", push ""
                if (quotation != 2 && end > spaceIndex) {
                    wordList.push_back("");
                }
                else {
                    wordList.push_back(line.substr(start, end - start + 1));
                }
                spaceIndex = -1;
                quotation = 0;
            }
            else {
                wordList.push_back(line.substr(start, end - start + 1));
            }
            leftDelimiters = 0;
        }
        else if (character == '\"') {
            quotation += 1;
        }
        else if (character != ' ') {
            // if first character, set start to i
            if (!(start > leftDelimiterIndex)) {
                start = i;
            }
            end = i;
        }
        else {
            // if space in between characters, set first occurrence of space to i
            if (start > leftDelimiterIndex && rightDelimiterIndex < leftDelimiterIndex && spaceIndex < 0) {
                spaceIndex = i;
            }
        }
    }

    if (leftDelimiters != 0) {
        cout << "Given line is invalid " << line << endl;
        exit(1);
    }

    return wordList;
}

// return an int array of element types in the given line
vector<int> parse_type(string line) {
    // store types for given line
    vector<int> types;
    vector<string> wordList = parse_line(line);
    for (int i = 0; i < wordList.size(); i++) {
        types.push_back(compute_type(wordList[i]));
    }
    return types;
}

// return a list of values of the given line, if the value does
// not match its type, return ""
vector<string> parse_line(string input, vector<int>* types, int columns) {
    
    vector<string> wordList = parse_line(input);

    // if a line has more than columns 
    // (may be the case when this line is not in first 500 lines)
    if (wordList.size() > columns) {
        wordList.erase(wordList.begin() + columns, wordList.end());
    }

    for (int i = 0; i < wordList.size(); i++) {
        int type = compute_type(wordList[i]);
        // invalid type (i.e. a string value for desired type int)
        if (type > types->at(i)) {
            wordList[i] = "";
        }
        else if (type <= 3 && type > 1 && wordList[i].at(0) == '+' && types->at(i) != 4) {
            // erase + if it is float or int
            wordList[i].erase(0, 1);
        }
    }

    // fill up the vectors with given number of columns
    while (wordList.size() < columns) {
        wordList.push_back("");
    }
    return wordList;
}

// print the string format of given type number
void print_type(int type) {
    switch (type) {
    case 1:
        cout << "BOOL\n";
        break;
    case 2:
        cout << "INT\n";
        break;
    case 3:
        cout << "FLOAT\n";
        break;
    case 4:
        cout << "STRING\n";
        break;
    default:
        cout << "Undefined type\n";
        exit(1);
    }
}

// entrance of the program
DataFrame* parseSor(char* filePath, size_t from, size_t len, size_t* highest_user, size_t* highest_project)
{
    // open file
    ifstream* file = new ifstream(filePath);

    int lineNumber = 0;
    int columns = 0;
    vector<int> types;
    string line;

    // read either EOF or first 500, which either comes first
    while ((lineNumber < 500) && getline(*file, line, '\n')) {
         vector<int> currentTypes = parse_type(line);
         int currentColumns = currentTypes.size();

        // compares current row type information with the record,
        // take the most lenient one (or, max of their values)
        if (columns >= currentColumns) {
            for (int i = 0; i < currentColumns; i++) {
                types[i] = max(types[i], currentTypes[i]);
            }
        }
        else {
            for (int i = 0; i < columns; i++) {
                currentTypes[i] = max(types[i], currentTypes[i]);
            }
            types = currentTypes;
            columns = currentColumns;
        }
        lineNumber += 1;
    }

    // move the pointer to the from flag user specified
    file->clear();
    file->seekg(from, file->beg);

    // to avoid reading partial file, ignore the first row
    // if from is not 0
    if (from != 0) {
        getline(*file, line, '\n');
    }

    // store the content of the file
    vector<vector<string>> matrix;

    Schema* sc = new Schema();

    for (int i = 0; i < types.size(); i++) {
        switch (types.at(i)) {
            case 1:
                //cout << "BOOL ";
                sc->add_column('B');
                break;
            case 2:
                //cout << "INT ";
                sc->add_column('I');
                break;
            case 3:
                //cout << "FLOAT ";
                sc->add_column('F');
                break;
            case 4:
                //cout << "STRING ";
                sc->add_column('S');
                break;
            default:
                cout << "Undefined type ";
                exit(1);    
        }
    }

    DataFrame* df = new DataFrame(*sc);
    while (getline(*file, line, '\n')) {
        // if exceed reading length, break
        if (file->tellg() > (len + from)) {
            break;
        }

        vector<string> row = parse_line(line, &types, columns);

        Row* r = new Row(*sc);
        for (int i = 0; i < row.size(); i++) {
            if (r->col_type(i) == 'I') {
                int val = stoi(row.at(i));
                r->set(i, val);
                if (i == 0 && *highest_project < val) *highest_project = val;
                else if (i != 0 && *highest_user < val) *highest_user = val;
            }
            else if (r->col_type(i) == 'F') {
                float f = stof(row.at(i));
                r->set(i, f);
            }
            else if (r->col_type(i) == 'B') {
                bool b = stoi(row.at(i));
                r->set(i, b);
            }
            else if (r->col_type(i) == 'S') {
                String* s = new String((char*)row.at(i).c_str());
                r->set(i, s);
                delete s;
            }
            else {
                printf("wtf\n");
                exit(1);
            }
        }

        df->add_row(*r);
        delete r;
    }

    // close file
    file->close();
    delete sc;
    delete file;
    
    return df;
}

DataFrame* parseSor(char* filePath, size_t from, size_t len) {
    size_t u = 0;
    size_t p = 0;
    return parseSor(filePath, from, len, &u, &p);
}