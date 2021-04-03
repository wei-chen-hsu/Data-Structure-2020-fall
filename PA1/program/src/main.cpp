#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <string>
#include <vector>

using namespace std;

//=================some function=================//

void cal_pageRank(vector<vector<bool> > &pageRelation, vector<int> &C, double d, double stoppingDifference);
void reverseIndex(vector<list<string> > &dictionary);
bool compareString(std::string a, std::string b) { return a < b; }
void searchEngine(double d, double stoppingDifference, vector<list<string> > &dictionary);

//=================main function================//

int main(int argc, char *argv[]) {
    vector<vector<bool> > pageRelation(501, vector<bool>(501, 0));
    vector<list<string> > dictionary;

    //===========================read through all pages, record the pageRelation and the word=======================//

    for (int pageNumber = 0; pageNumber <= 499; pageNumber++) {
        fstream fin("input/web-search-files2/page" + to_string(pageNumber));
        string pageContent;

        //==========build the adjMatrix representing the linked-out pages============//

        while (getline(fin, pageContent)) {
            if (pageContent[0] == '-') {
                break;
            }
            string numericPart_linkOut = pageContent.substr(4, pageContent.size() - 4);
            int substrToNum = stoi(numericPart_linkOut);
            pageRelation[pageNumber][substrToNum] = 1;
        }

        //============read the 20 words of the page, then construct the dictionary=============//

        getline(fin, pageContent);
        int startCharIndex = 0;
        vector<string> wordAlreadyAppearedInThatPage;

        for (int endCharIndex = 0; endCharIndex < pageContent.size(); endCharIndex++) {
            // whenever get a word, loop through the entire
            // dictionary to find whether the word already exists

            if (pageContent[endCharIndex] == ' ') {
                string word = pageContent.substr(startCharIndex, endCharIndex - startCharIndex);

                bool wordAppearedBefore = 0;
                for (int j = 0; j < wordAlreadyAppearedInThatPage.size(); j++) {
                    if (word == wordAlreadyAppearedInThatPage[j]) {
                        wordAppearedBefore = 1;
                        break;
                    }
                }
                if (wordAppearedBefore == 1) {
                    startCharIndex = endCharIndex + 1;
                    continue;
                }
                // if no word in the dictionary
                if (dictionary.size() == 0) {
                    dictionary.resize(1);           // increase the size of the vector
                    dictionary[0].push_back(word);  // warning
                    dictionary[dictionary.size() - 1].push_back("page" + to_string(pageNumber));
                }
                // if already some words in the dictionary
                else {
                    bool word_in_dictionary_flag = 0;
                    for (int i = 0; i < dictionary.size(); i++) {
                        if (word == *dictionary[i].begin()) {
                            dictionary[i].push_back("page" + to_string(pageNumber));
                            wordAlreadyAppearedInThatPage.push_back(word);
                            word_in_dictionary_flag = 1;
                            break;
                        }
                    }
                    if (word_in_dictionary_flag == 0) {
                        dictionary.resize(dictionary.size() + 1);  // increase the size of the vector
                        dictionary[dictionary.size() - 1].push_back(word);
                        wordAlreadyAppearedInThatPage.push_back(word);
                        dictionary[dictionary.size() - 1].push_back("page" + to_string(pageNumber));
                    }
                }
                startCharIndex = endCharIndex + 1;
            }
        }
        fin.close();
    }

    // debug on
    fstream fout_dictionary;
    fout_dictionary.open("dictionary.txt", ios::out);
    for (int i = 0; i < dictionary.size(); i++) {
        for (list<string>::iterator it = dictionary[i].begin(); it != dictionary[i].end(); it++) {
            fout_dictionary << *it << "||";
        }
        fout_dictionary << endl;
    }
    fout_dictionary.close();
    //debug off

    //=====================start calculating the PageRank=========================//

    //===========calculate C[x] first=========//

    vector<int> C(501, 0);
    for (int pageConsidered = 0; pageConsidered <= 499; pageConsidered++) {
        for (int i = 0; i <= 500; i++) {
            if (pageRelation[pageConsidered][i] == 1) {
                C[pageConsidered]++;
            }
        }
    }

    //===calculate the pageRank, sort it, and then write it to the output file with respect to 12 d & diff combinations===//

    cal_pageRank(pageRelation, C, 0.25, 0.001);
    cal_pageRank(pageRelation, C, 0.25, 0.010);
    cal_pageRank(pageRelation, C, 0.25, 0.100);
    cal_pageRank(pageRelation, C, 0.45, 0.001);
    cal_pageRank(pageRelation, C, 0.45, 0.010);
    cal_pageRank(pageRelation, C, 0.45, 0.100);
    cal_pageRank(pageRelation, C, 0.65, 0.001);
    cal_pageRank(pageRelation, C, 0.65, 0.010);
    cal_pageRank(pageRelation, C, 0.65, 0.100);
    cal_pageRank(pageRelation, C, 0.85, 0.001);
    cal_pageRank(pageRelation, C, 0.85, 0.010);
    cal_pageRank(pageRelation, C, 0.85, 0.100);

    //======================reverse index================//

    reverseIndex(dictionary);

    //=====================search engine=================//

    searchEngine(0.25, 0.001, dictionary);
    searchEngine(0.25, 0.010, dictionary);
    searchEngine(0.25, 0.100, dictionary);
    searchEngine(0.45, 0.001, dictionary);
    searchEngine(0.45, 0.010, dictionary);
    searchEngine(0.45, 0.100, dictionary);
    searchEngine(0.65, 0.001, dictionary);
    searchEngine(0.65, 0.010, dictionary);
    searchEngine(0.65, 0.100, dictionary);
    searchEngine(0.85, 0.001, dictionary);
    searchEngine(0.85, 0.010, dictionary);
    searchEngine(0.85, 0.100, dictionary);

    //================================================//

    return 0;
}

//===========================================function content======================================//

void cal_pageRank(vector<vector<bool> > &pageRelation, vector<int> &C, double d, double stoppingDifference) {  //warning
    //----------------------calculate pageRank---------------------------------//
    vector<double> pageRank(501, 1 / 501);
    vector<double> pageRankOld(501, 1 / 501);
    double diff = 0.0;
    do {
        diff = 0.0;
        for (int pageConsidered = 0; pageConsidered <= 500; pageConsidered++) {
            double pageRank_before = pageRank[pageConsidered];
            double cumulative_PR = 0;

            for (int i = 0; i <= 499; i++) {
                if (pageRelation[i][pageConsidered] == 1) {  // if page i points to pageConsidered
                    cumulative_PR += pageRankOld[i] / C[i];
                }
            }
            pageRank[pageConsidered] = (1 - d) / 501 + d * cumulative_PR;
            diff = diff + abs(pageRank_before - pageRank[pageConsidered]);
        }
        for (int i = 0; i < pageRank.size(); i++) {
            pageRankOld[i] = pageRank[i];
        }
    } while (diff >= stoppingDifference);

    //-------------------------sort the pageRank------------------------------//
    vector<int> pageOrder(501, 0);
    for (int i = 0; i <= 500; i++) {
        pageOrder[i] = i;
    }
    for (int i = 1; i <= 500; i++) {
        int key = pageOrder[i];
        int j = i - 1;
        while (j >= 0 && pageRank[key] > pageRank[pageOrder[j]]) {
            pageOrder[j + 1] = pageOrder[j];
            j--;
        }
        pageOrder[j + 1] = key;
    }

    //--------------------------write to output file--------------------------//
    fstream fout;
    string d_str = to_string(d);
    d_str = d_str.substr(2, 2);
    string stoppingDifference_str = to_string(stoppingDifference);
    stoppingDifference_str = stoppingDifference_str.substr(2, 3);
    fout.open("output/pr_" + d_str + "_" + stoppingDifference_str + ".txt", ios::out);

    for (int i = 0; i <= 500; i++) {
        fout << "page" << pageOrder[i] << "    " << C[pageOrder[i]] << "    " << fixed << setprecision(7) << pageRank[pageOrder[i]] << endl;
    }
    fout.close();
}

void reverseIndex(vector<list<string> > &dictionary) {
    //---------store every word in to a new vector---//
    vector<string> everyWord;
    for (int i = 0; i < dictionary.size(); i++) {
        everyWord.push_back(*dictionary[i].begin());
    }

    //----------------sort everyWord----------------------//
    sort(everyWord.begin(), everyWord.end(), compareString);

    //--------------write to output file------------------//
    fstream fout;
    fout.open("output/reverseindex.txt", ios::out);
    for (int i = 0; i < everyWord.size(); i++) {
        for (int j = 0; j < dictionary.size(); j++) {
            if (everyWord[i] == *dictionary[j].begin()) {
                for (list<string>::iterator it = dictionary[j].begin(); it != dictionary[j].end(); it++) {  // print the sorted word and its pages
                    fout << *it << " ";
                }
                fout << endl;
                break;
            }
        }
    }
    fout.close();
}

void searchEngine(double d, double stoppingDifference, vector<list<string> > &dictionary) {
    //========================build the page order first==========================//
    string d_str = to_string(d);
    d_str = d_str.substr(2, 2);
    string stoppingDifference_str = to_string(stoppingDifference);
    stoppingDifference_str = stoppingDifference_str.substr(2, 3);
    fstream fin_page("output/pr_" + d_str + "_" + stoppingDifference_str + ".txt");

    string str;
    vector<int> pageOrder;  // index is the rank; element is the page number
    while (getline(fin_page, str)) {
        for (int i = 0; i < str.size(); i++) {
            if (str[i] == ' ') {
                string numericPart = str.substr(4, i - 4);
                int substrToNum = stoi(numericPart);
                pageOrder.push_back(substrToNum);
                break;
            }
        }
    }

    vector<int> reverse_pageOrder(501, 555);  // index is the page number; element is the rank
    for (int i = 0; i < pageOrder.size(); i++) {
        reverse_pageOrder[pageOrder[i]] = i;
    }

    fin_page.close();

    //======================read list.txt=======================//
    fstream fin_list("input/list.txt");
    fstream fout;
    fout.open("output/result_" + d_str + "_" + stoppingDifference_str + ".txt", ios::out);
    fstream fout_word_searched;
    fout_word_searched.open("word_searched.txt", ios::out);

    string lineOfWords;
    while (getline(fin_list, lineOfWords)) {
        //---------end of reading list.txt---------//
        if (lineOfWords == "*end*") {
            break;
        }

        //-----------get the words of a line----------------//
        int startCharIndex = 0;
        vector<string> word_searched;
        for (int endCharIndex = 0; endCharIndex < lineOfWords.size(); endCharIndex++) {
            if (lineOfWords[endCharIndex] == ' ') {  // multiple words in one line
                string singleWord = lineOfWords.substr(startCharIndex, endCharIndex - startCharIndex);
                word_searched.push_back(singleWord);
                startCharIndex = endCharIndex + 1;
            } else if (endCharIndex == lineOfWords.size() - 1) {                                        // the end of a line
                string singleWord = lineOfWords.substr(startCharIndex, endCharIndex - startCharIndex);  // '\n' after the last word
                word_searched.push_back(singleWord);
            }
        }

        //debug on
        for (int i = 0; i < word_searched.size(); i++) {
            fout_word_searched << word_searched[i] << "||";
        }
        fout_word_searched << endl;
        //debug off

        //------------------------------------------------------------------------//
        if (word_searched.size() == 1) {
            vector<bool> checkPageExist(501, 0);  //index = page nmuber; element = 1 if exist
            bool none_flag = 1;
            for (int i = 0; i < dictionary.size(); i++) {
                if (word_searched[0] == *dictionary[i].begin()) {  // if the word exists in the dictionary
                    none_flag = 0;
                    for (list<string>::iterator it = next(dictionary[i].begin(), 1); it != dictionary[i].end(); it++) {
                        string str = *it;
                        string numericPart = str.substr(4, str.size() - 4);
                        int substrToNum = stoi(numericPart);
                        checkPageExist[substrToNum] = 1;
                    }
                }
            }

            //----------------------------------------------------------------------------//
            vector<int> sort_Rank;
            for (int i = 0; i < checkPageExist.size(); i++) {  // index = page number
                if (checkPageExist[i] == 1) {
                    sort_Rank.push_back(reverse_pageOrder[i]);
                }
            }
            sort(sort_Rank.begin(), sort_Rank.end());
            if (sort_Rank.size() >= 10) {
                for (int i = 0; i < 10; i++) {
                    fout << "page" << pageOrder[sort_Rank[i]] << " ";
                }
                fout << endl;
            } else {
                for (int i = 0; i < sort_Rank.size(); i++) {
                    fout << "page" << pageOrder[sort_Rank[i]] << " ";
                }
                fout << endl;
            }
        }
        // more than 1 word in a line
        else {
            vector<vector<bool> > checkPageExist(word_searched.size(), vector<bool>(501, 0));
            bool none_flag = 1;
            for (int i = 0; i < word_searched.size(); i++) {
                for (int j = 0; j < dictionary.size(); j++) {
                    if (word_searched[i] == *dictionary[j].begin()) {
                        none_flag = 0;
                        for (list<string>::iterator it = next(dictionary[j].begin(), 1); it != dictionary[j].end(); it++) {
                            string str = *it;
                            string numericPart = str.substr(4, str.size() - 4);
                            int substrToNum = stoi(numericPart);
                            checkPageExist[i][substrToNum] = 1;
                        }
                    }
                }
            }
            if (none_flag == 1) {  // neither words are in the dictionary
                fout << "AND none" << endl;
                fout << "OR none" << endl;
                continue;
            }

            //----------------------------------------------------------//
            vector<int> and_sort_Rank;
            vector<int> or_sort_Rank;

            //----------------------check and---------------------------//
            for (int j = 0; j < 501; j++) {
                bool and_success = 1;
                for (int i = 0; i < word_searched.size(); i++) {
                    if (checkPageExist[i][j] == 0) {
                        and_success = 0;
                        break;
                    }
                }
                if (and_success == 1) {
                    and_sort_Rank.push_back(reverse_pageOrder[j]);
                }
            }

            // if there's no "and" situation
            if (and_sort_Rank.size() == 0) {
                fout << "AND none" << endl;
            }
            // there are some "and" situations
            else {
                sort(and_sort_Rank.begin(), and_sort_Rank.end());
                fout << "AND ";
                if (and_sort_Rank.size() >= 10) {
                    for (int i = 0; i < 10; i++) {
                        fout << "page" << pageOrder[and_sort_Rank[i]] << " ";
                    }
                    fout << endl;
                } else {
                    for (int i = 0; i < and_sort_Rank.size(); i++) {
                        fout << "page" << pageOrder[and_sort_Rank[i]] << " ";
                    }
                    fout << endl;
                }
            }

            //----------------------check or----------------------------//

            for (int j = 0; j < 501; j++) {
                bool or_success = 0;
                for (int i = 0; i < word_searched.size(); i++) {
                    if (checkPageExist[i][j] == 1) {
                        or_success = 1;
                        break;
                    }
                }
                if (or_success == 1) {
                    or_sort_Rank.push_back(reverse_pageOrder[j]);
                }
            }

            // if there's no "or" situation
            if (or_sort_Rank.size() == 0) {
                fout << "OR none" << endl;
            }
            // there are some "or" situations
            else {
                sort(or_sort_Rank.begin(), or_sort_Rank.end());
                fout << "OR ";
                if (or_sort_Rank.size() >= 10) {
                    for (int i = 0; i < 10; i++) {
                        fout << "page" << pageOrder[or_sort_Rank[i]] << " ";
                    }
                    fout << endl;
                } else {
                    for (int i = 0; i < or_sort_Rank.size(); i++) {
                        fout << "page" << pageOrder[or_sort_Rank[i]] << " ";
                    }
                    fout << endl;
                }
            }
        }
    }
    fin_list.close();
    fout.close();
    fout_word_searched.close();
}