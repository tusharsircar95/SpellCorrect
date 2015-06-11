Version 1
==========
Creates dictionary by scanning the word list provided.
When user inputs word, scans the dictionary and selects the best word based on edit distance and soundex code.
- No preference given to common words
- Previous user inputs don't affect future decisions

RANKING PARAMETERS:
Standard Edit Distance
Soundex Code

Version 2
==========
- Added functionality to read text files(provided while training) so that more common words have higher count. Words with the same score are compared on basis of the word count
- Correct words entered by the user are inserted into TRAI so that over time more common words get higher preference
- Hinglish will be supported if dictionary of Hinglish words is provided

RANKING PARAMETERS:
Standard Edit Distance
Soundex Code
Word Frequency

Version 3
==========
Possible Improvements: 
- Edit distance variation based on errors made via keyboard input
- Bigram approach
- Context based prediction

RANKING PARAMETERS:
Custom Edit Distance (Considers Swaps)
Soundex Code 
Word Frequency
Bigrams

