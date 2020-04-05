#include "word_count_test.h"

int main(int argc, char** argv) {
    DataFrame* df = parse_file("data/100k.txt");
    
    WordCountRower* wcr = new WordCountRower();
    df->pmap(*wcr);
    wcr->print();


    

    // next steps:
    // run the pmap on the network, each node should run pmap on its own dataframe
    // somehow concat the stuff together?
    return 0;
}