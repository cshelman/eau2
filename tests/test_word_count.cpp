#include "word_count_test.h"

int main(int argc, char** argv) {
    DataFrame* df = parse_file("data/100k.txt");
    
    WordCountRower* wcr = new WordCountRower();
    df->pmap(*wcr);
    wcr->print();
    
    return 0;
}