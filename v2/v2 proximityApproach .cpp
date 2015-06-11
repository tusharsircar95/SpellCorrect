#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include <fstream>
#include <dirent.h>
#include<ctype.h>
using namespace std;

/*
1. Load words from a dictionary into a TRAI data structure
2. Each node of TRAI will include the following satellite data:
    - FREQUENCY
    - SOUNDEX CODE
    - WORD

STEPS TO CORRECT SPELLING
1. Get a list of words with similar soundex code
2. Rank this list based on edit distnace, soundex code similarity and frequency
3. Choose from the top few words based on context fitting

POSSIBLE IMPROVEMENTS
1. Modify edit distance that looks at keyboard distance between misspelled letters
2. Use a better training set for the dictionary
3. Language support
*/

//TRAI VARIABLES AND STRUCTURES
// every node in the TRAI structure
struct node
{
    int freq;
    string soundexCode;
    string word;
    node *next[27];
};
struct candidateNode
{
    string word;
    int score;
};

node* newNode(); //get a new initialised pointer to node
node *root = NULL; // pointer to root of the TRAI
vector<string> wordList; // stores all unique words stored in the TRAI
vector<candidateNode> candidateList; // stores all words that are probable corrections

void loadWordList();
void insertWord(string word); // insert a word into TRAI
int searchWord(string word); // returns frequency of that word
string getSoundexCode(string word); // returns soundex code of that word
int getEditDistance(string a,string b); // returns edit distance(standard)
string getCorrection(string word); // traverse TRAI to get a possible correction
void populateWordList(node *root); // loads all words from TRAI into wordList
int soundexComparisonScore(string a,string b); // returns score of comparison based on soundex and -1 if its a bad match
bool scoreComparator(candidateNode a,candidateNode b); // returns true of a has a smaller score
void predictWord(string word); // predict word passed as argument if it doesn't exist
void readDirectory(string directoryName); // read text files to train
int is_txt(string filename); // checks if file is a text file

int main()
{
    // load dictionary into the TRAI
    cout<<"Loading word list...\n";
    loadWordList();
    populateWordList(root);

    cout<<"Done loading word list..."<<wordList.size()<<" words\n\n";
    cout<<"Reading training files...\n";
    readDirectory(".");
    cout<<"Training files read...\n\n";

    while(1)
    {
        string s;
        cout<<"Enter word: ";
        cin>>s;
        predictWord(s);
    }
    return 0;
}


int is_txt(string fullString)
{
    string ending=".txt";
    if (fullString.length() >= ending.length())
    {
        int temp=fullString.compare (fullString.length() - ending.length(), ending.length(), ending);
        if(temp==0) return 1;
        else return 0;
    }
    else
        return 0;
}

void readDirectory(string directoryName)
{
    DIR *dir;
    struct dirent *ent;
    string filenames[10];
    int fileCount = 0;

    //change directory as required
    if ((dir = opendir (directoryName.c_str())) != NULL)
    {
        while ((ent = readdir (dir)) != NULL)
        {
            if(is_txt(ent->d_name))
             {
                 filenames[fileCount++] = ent->d_name;
             }
        }
        closedir (dir);
        cout<<"Files read: "<<fileCount<<endl<<endl;
    }
    else //could not open directory
        cout<<"Error reading files!"<<endl;


    //read and store all the files
    for(int index=0; index<fileCount; index++)
    {
        string s = filenames[index];
        ifstream input(s.c_str());
        string str;
        while(!input.eof())
        {
            input>>str;

            int i=0, j=str.size()-1;

            //removing punctuations from the string, if any
            while(ispunct(str[i]))
                i++;
            while(ispunct(str[j]))
                j--;

            string s = str.substr(i,j-i+1);
            s.erase(s.find_last_not_of(" \n\r\t")+1);
            if(s!="" && s.size()>1 && s.find('-')==string::npos)
                insertWord(s);


        }
        input.close();
    }

}

void predictWord(string s)
{
    candidateList.clear();
    int c = searchWord(s);
    if(c != 0)
    {
        cout<<"Word exists and occurs "<<c<<" times!\n";
        insertWord(s);
    }
    else
    {
        cout<<"Generating candidate list...\n";

        for(vector<string>::iterator it = wordList.begin(); it!=wordList.end(); it++)
        {
            string word = *it;
            //if(word.at(0) != s.at(0))
              //  continue;
            int sxScore;
            if((sxScore=soundexComparisonScore(word,s))!=-1)
            {

                candidateNode cnode;
                cnode.word = word;
                cnode.score = sxScore - (getEditDistance(s,word));
                if(cnode.score>=0)
                    candidateList.push_back(cnode);
            }
        }
        sort(candidateList.begin(),candidateList.end(),scoreComparator);

        // after first filter for the candidate list
        cout<<"Candidate list selected after 1st filter is as follows: \n";
        for(vector<candidateNode>::iterator it = candidateList.begin(); it!=candidateList.end(); it++)
        {
            candidateNode cnode = *it;
            cout<<cnode.word<<" "<<cnode.score<<endl;
        }


        vector<candidateNode> candidateShortList;
        int maxScore = candidateList.front().score;
        // after first filter for the candidate list
        cout<<"\nCandidate list selected after 2nd filter is as follows: \n";
        for(vector<candidateNode>::iterator it = candidateList.begin(); it!=candidateList.end(); it++)
        {
            candidateNode cnode = *it;
            if(cnode.score == maxScore || cnode.score == maxScore-1)
            {
                candidateNode cnodeNew;
                cnodeNew.word = cnode.word;
                cnodeNew.score = searchWord(cnode.word);
                candidateShortList.push_back(cnodeNew);
                cout<<cnodeNew.word<<" "<<cnodeNew.score<<endl;
            }
            else
                break;
        }

         sort(candidateShortList.begin(),candidateShortList.end(),scoreComparator);




        if(candidateShortList.size()!=0)
            cout<<"Suggested Word: "<<candidateShortList.front().word<<endl<<endl;
        else
            cout<<"No suggestion!"<<endl<<endl;
    }
}

bool scoreComparator(candidateNode a,candidateNode b)
{
    return a.score>b.score;
}
int soundexComparisonScore(string a,string b)
{
    string scA = getSoundexCode(a);
    string scB = getSoundexCode(b);
    char codeA = scA.at(0);
    char codeB = scB.at(0);
    int numA = atoi(scA.substr(1,3).c_str());
    int numB = atoi(scB.substr(1,3).c_str());

    if(codeA == codeB) // exact match
    {
        if(numA == numB)
            return 6;
        if(abs(numA - numB)<=2)
            return 3;
    }
    if(codeA != codeB) // first letter mismatches
    {

        if(numA == numB)
            return 4;
        if(abs(numA - numB)<=2)
            return 2;
    }

    return -1;
}

void populateWordList(node *root)
{
    if(root->word != "")
        wordList.push_back(root->word);
    for(int i=1; i<=26; i++)
        if(root->next[i]!=NULL)
            populateWordList(root->next[i]);

}

int getEditDistance(string a,string b)
{
    int alen = a.length();
    int blen = b.length();


    int results[alen+1][blen+1];
    for(int i=0; i<alen+1; i++)
        results[i][0] = i;
    for(int j=0; j<blen+1; j++)
        results[0][j] = j;
    for(int i=1; i<alen+1; i++)
    {
        for(int j=1; j<blen+1; j++)
        {
            int ans = results[i][j-1] + 1;
            ans = min(ans,results[i-1][j] + 1);
            int c = 0;
            if(a.at(i-1) != b.at(j-1))
                c = 1;
            ans = min(ans,results[i-1][j-1]+c);
            results[i][j] = ans;
        }
    }
    return results[alen][blen];

}

string getSoundexCode(string s)
{
     //make string lowercase
     std::transform(s.begin(), s.end(), s.begin(), ::tolower);
     s.erase(s.find_last_not_of(" \n\r\t")+1);

    //cout<<"%"<<s<<endl;
    for(int i=1,l=s.length(); i<l; i++)
    {
        if(s.at(i) == 'a' || s.at(i) == 'e' || s.at(i) == 'i' || s.at(i) == 'o' || s.at(i) == 'u' || s.at(i) == 'h' || s.at(i) == 'w' || s.at(i) == 'y')
            s[i] = '0';
        else if(s.at(i) == 'b' || s.at(i) == 'f' || s.at(i) == 'p' || s.at(i) == 'v')
            s[i] = '1';
        else if(s.at(i) == 'c' || s.at(i) == 'g' || s.at(i) == 'j' || s.at(i) == 'k' || s.at(i) == 'q' || s.at(i) == 's' || s.at(i) == 'x' || s.at(i) == 'z')
            s[i] = '2';
        else if(s.at(i) == 'd' || s.at(i) == 't')
            s[i] = '3';
        else if(s.at(i) == 'l')
            s[i] = '4';
        else if(s.at(i) == 'm' || s.at(i) == 'n')
            s[i] = '5';
        else if(s.at(i) == 'r')
            s[i] = '6';
    }

    char result[10];
    result[0] = s.at(0);
    int index = 1;

    for(int i=1,l=s.length()-1; i<l; i++)
    {
        if(i==l-1 && s.at(i) == '0' && s.at(i+1) !='0')
        {
            result[index++] = s.at(i+1);
            break;
        }
        else if(i==l-1 && s.at(i)!='0')
        {
            result[index++] = s.at(i);
            if(s.at(i+1) != s.at(i))
                result[index++] = s.at(i+1);
            break;

        }
        else if(s.at(i) == '0')
            continue;
        if(s.at(i) == s.at(i+1))
            result[index++] = s.at(i++);
        else
            result[index++] = s.at(i);
    }
    if(index<4)
    {
        while(index<4)
            result[index++] = '0';
    }
    string sc(result);

    return sc.substr(0,4);


}

node* newNode()
{
    node* n = new node;
    for(int i=1; i<=26; i++)
        n->next[i] = NULL;
    n->freq = 0;
    n->soundexCode = "";
    n->word = "";

    return n;
}

void loadWordList()
{
    root = newNode();
    FILE* inputFile;
    inputFile = fopen("wordList.txt","r");
    char c[20];
    string prev = "";
    for(int i=0; i<100000; i++)
    {
        fscanf(inputFile,"%s\n",&c);
        string s(c);
        s.erase(s.find_last_not_of(" \n\r\t")+1);
        if(s == prev)
            break;
        prev = s;
       // cout<<s<<" "<<s.length()<<endl;
        insertWord(s);
    }
}

void insertWord(string word)
{
    //make string lowercase
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);

    node *temp = root;
    for(int i=0; i<word.size(); i++)
    {
        int c = ((int)(word.at(i)))-96;

        if(c>=1 && c<=26)
        {
            if(temp->next[c]==NULL)
                temp->next[c] = newNode();
            temp = temp->next[c];
        }
    }

    temp->freq = temp->freq + 1;
    temp->soundexCode = getSoundexCode(word);
    temp->word = word;


}

int searchWord(string word)
{
     //make string lowercase
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);

    node *temp = root;
    bool wordFound = true;
    for(int i=0; i<word.size(); i++)
    {
        int c = ((int)(word.at(i)))-96;
        if(temp->next[c]==NULL)
        {
            wordFound = false;
            break;
        }
        temp = temp->next[c];
    }

    if(wordFound)
        return temp->freq;
    else
        return 0;

}
