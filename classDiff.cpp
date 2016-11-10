#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctype.h>
#include <cstring>
#include <vector>
#include <string>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

using namespace std;

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

bool sayYes(string result); 
bool tryAgain(string & result, bool & searchAgain); 
void fixSubject(string & subject);
void fixSpaces(string & str);
string getCourseNum(string str);
vector<double> getHours(string str, int num);

string getCourseSubject(string str);

int main(void)
{
    static const size_t npos = -1;
    // used to denote if searching for class again or not
    bool searchAgain = true;
    bool f_profSort = false;
    int num = 10;

	string subject = "";
	string result = "";
	string searchStr = "";
    string profSortResponse = "";
    const static string profSortStr = "<a id=\"ctl00_C";

	const static char * USER_AGENT_STRING = "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:30.0) Gecko/20100101 Firefox/30.0'";
    // search loop
    while ( searchAgain == true ) {
        
        // get course name from user
        cout << "Enter class name: ";

        cin.clear();
        cin.sync();

        getline(cin,searchStr);

        string classesResponse = "";
        cout << "Enter number of classes to search: ";
        getline(cin,classesResponse);
        num = atoi(classesResponse.c_str());

        // deduce course subject and number
        subject = getCourseSubject(searchStr);
        string classNum = getCourseNum(searchStr);

        // make course name uppercase
        for ( unsigned int i = 0; i < subject.length(); i++ ) {
            subject[i] = toupper(subject[i]);
        }
        // file handling
        CURL *curl_handle;
        static const char *headerfilename = "head.txt";
        FILE *headerfile;

        string begurl = "http://cape.ucsd.edu/responses/Results.aspx?Name=&CourseNumber=";
        string urlStr = begurl + subject + "+" + classNum;

        curl_global_init(CURL_GLOBAL_ALL);
        /* init the curl session */
        curl_handle = curl_easy_init();

		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, USER_AGENT_STRING);
        /* set URL to get */
        curl_easy_setopt(curl_handle, CURLOPT_URL, urlStr.c_str()); curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0); curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, "cookies.txt");

        /* send all data to this function  */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

        /* open the files */
        headerfile = fopen(headerfilename,"w");
        if (headerfile == NULL) {
            curl_easy_cleanup(curl_handle);
            return -1;
        }
        
        /* we want the headers to this file handle */
        curl_easy_setopt(curl_handle,   CURLOPT_WRITEDATA, headerfile);

        /*
        * Notice here that if you want the actual data sent anywhere else but
        * stdout, you should consider using the CURLOPT_WRITEDATA option.  */

        /* get it! */
        curl_easy_perform(curl_handle);

        /* close the header file */
        fclose(headerfile);

        /* cleanup curl stuff */
        curl_easy_cleanup(curl_handle);

        /* place HTML into string */
        
        

        /*
        ifstream in;
        in.open(headerfilename);

        stringstream sstr;
        sstr << in.rdbuf();
        string str = sstr.str();

        // handle page not found error
        if( str.find("<title>UC San Diego : File Not Found (404)</title>") != npos ) {
            cout << "Invalid subject\n";
            tryAgain(result,searchAgain);
            if ( searchAgain ) continue;
            return -1;
        }
        */
        /* -- CUSTOM CLASS SEARCH -- */

        /*
		vector<double> hours = getHours(str,num);
		double totalSum = 0;
		double avg = 0;

		for ( unsigned int i = 0; i < hours.size(); i++ ) {
			totalSum += hours[i];
		}

		if ( hours.size() == 0 ) cout << "no hours\n";
		else avg = totalSum / hours.size();

		cout << "Average Study Hours per week: " << avg << endl;
        */

        /* -- CUSTOM PROF SORT -- */

        // First find string profSortStr, then from that position get
        // professor name and get study hrs/wk

        
        
        /*
        profSortStr

        vector<double> hours = getHours(str,num);
		double totalSum = 0;
		double avg = 0;

		for ( unsigned int i = 0; i < hours.size(); i++ ) {
			totalSum += hours[i];
		}

		if ( hours.size() == 0 ) cout << "no hours\n";
		else avg = totalSum / hours.size();
        */
        if( tryAgain(result,searchAgain) == false ) break;
        else continue;

    }         
    return 0;
}

vector<double> getHours ( string page, int num ) {
	int pos = 0;
	vector<int> studyHourPos;
	vector<double> studyHours;
	int count = 0;
    // average 10 most recent hours
    //pos = page.find("StudyHours\">");

	while ( count != num ) {
		pos = page.find("StudyHours\">", pos+1);
		if ( pos != -1 ) {
			studyHourPos.push_back(pos);
			string hourStr = page.substr(pos+12,4);
			double hour = atof(hourStr.c_str());
			studyHours.push_back(hour);
		}
		count++;
	}
	return studyHours;
}

//TODO: fix magic numbers, add better searching for "yes"
bool sayYes(string result) {
    string yes[5] = {"y", "Y", "yes", "YES", "Yes"};
    
    for ( int i = 0; i < 5; i++ ) {
        if ( result.compare(yes[i]) == 0 )
            return true;
    }
    return false;
}

string getCourseSubject(string str) {

    // get string before first number
    int i = 0;
    while ( isalpha(str[i]) ) {
        i++;
    }
    return str.substr(0,i);
}

bool tryAgain(string & result, bool & searchAgain) {
    cin.clear();
    cout << "Search again? (y/n): ";
    getline(cin, result);
    //cin >> result;

    if ( sayYes(result) ) {
        searchAgain = true;
    }
    else {
        searchAgain = false;
    }
    return searchAgain;
}

void fixSubject(string & subject) {
    string swapBIOL[11] = {"BILD", "BIBC", "BICD", "BIEB", "BIMM", "BIPN", "BISP", "BGGN", "BGJC", "BGRD", "BGSE"};
    string swapNANO = "CENG";
    string swapLING = "LI";
    string swapHIST = "HI";

    if ( !subject.substr(0,2).compare("LI") ) { 
        subject = "LING";
        return;
    }
    if ( !subject.substr(0,2).compare(swapHIST) ) {
        subject = "HIST";
        return;
    }

    if ( !subject.compare(swapNANO) ) {
        subject = "NANO";
        return;
    }
    for ( int i = 0; i < 11; i++ ) {
        if ( !subject.compare(swapBIOL[i]) )
            subject = "BIOL";
    }
}

void fixSpaces(string & str) {
    string newStr="";
    for ( unsigned int i = 0; i < str.length() + 1; i++ ) {
        unsigned char c = str[i];
        if ( !str.substr(i,3).compare("<b>") ) { 
            i += 2;
        }
        if ( !str.substr(i,4).compare("</b>") ){
            i += 3;
        }
        if ( isalpha(c) || isdigit(c) || c == ' ' || c == '.' || c == '!' || c == '?' ) {
            if ( !str.substr(i,4).compare("Note") ) {
                i+=10;
                c = str[i];
            }
            if ( c != ' ' || str[i+1] != ' ' )
                newStr.push_back(c);
        }
    }
    // fix newline spacing
    unsigned int len = 80;
    while ( len < newStr.length() ) {
        if ( newStr[len] != '\n' ) {
            size_t pos = len;
            // find first occurence of space before position 80(or multiples)
            while ( newStr[pos] != ' ' ) {
                pos--;
            }
            if ( pos > 0 && pos < newStr.length() && newStr[pos] == ' ' ) {
                newStr[pos] = '\n';
            }
        }
        len += 80;
    }
    str = newStr;
}
/* input: string in form SUBJNUM (ex: MATH20D)
 * output: string representation of class number
 */
string getCourseNum(string str){

    int strlen = str.length();
	int i = 0;

    while ( !isdigit(str[i]) && i < strlen ) i++;

    return str.substr(i);
}
