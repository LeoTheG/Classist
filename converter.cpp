#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include "gumbo.h"

using namespace std;

string getCourseSubject(string str);
string getCourseNumber(string str);
void generateFile(string s, string n);

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

struct classInfo{
    string subject;
    string num;
};

class converter {
    public:
        converter();
    private:
        string url;
        classInfo* getUserInput();
        classInfo* cI;
        float getAvgStudyHrs(string s, int numSearches = 10);
        string getOutput(string fileName);
};

converter::converter(){
    cI = getUserInput();
    //cerr << "Got subject:" << cI->subject << "\nGot number:"<<cI->num<<endl;
    generateFile(cI->subject,cI->num);
    //ofstream out("output.txt");
    string formattedStr = getOutput("head.txt");
    //out << getOutput("head.txt") << endl;
    //out.close();
    cerr<<"Avg study hours/wk:"<<getAvgStudyHrs(formattedStr)<<endl;
}
// calculate average study hours
float converter::getAvgStudyHrs(string s, int numSearches){

    size_t pos = 0;
    float sum = 0;
    int count = 0;
    string subject = cI->subject;
    string num = cI->num;

    // gets study hours/wk by looking at number after second '%'
    while ( pos != -1 && count < numSearches ) {
        pos = s.find("%",pos) + 1;
        pos = s.find("%",pos);
        if ( pos != -1 ) {
            pos++;
            // skip over spaces after '%' until reach number
            while ( pos < s.length() && s.at(pos)==' ') pos++;
            sum += stof(s.substr(pos).c_str());
            cerr << "Got num:"<<stof(s.substr(pos).c_str())<<endl;
            count++;
        }
    }
    return (float)sum/count;
}
// gets input from user and inputs into struct
classInfo* converter::getUserInput(){
    string input = "";
    cout << "Enter class: ";
    getline(cin,input); 
    classInfo* userClassInfo = new classInfo();
    userClassInfo->subject = getCourseSubject(input);
    userClassInfo->num = getCourseNumber(input);
    return userClassInfo;
}
string getCourseSubject(string str){
    int count = 0;
    while ( count < str.length() && isalpha(str.at(count)) )
        count++;
    return str.substr(0,count); 
}
string getCourseNumber(string str){
    int alphaCount = 0;

    while ( alphaCount < str.length() && isalpha(str.at(alphaCount)) )
        alphaCount++;

    int numCount = alphaCount;
    while ( numCount < str.length() && str.at(numCount) == ' ') numCount++;

    string num = str.substr(numCount);

    return num;

}

void generateFile(string subject, string classNum){
    // file handling
    CURL *curl_handle;
    static const char *headerfilename = "head.txt";
    FILE *headerfile;
    const static char * USER_AGENT_STRING = "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:30.0) Gecko/20100101 Firefox/30.0'";

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
        return;
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

}

static std::string cleantext(GumboNode* node) {
  if (node->type == GUMBO_NODE_TEXT) {
    return std::string(node->v.text.text);
  } else if (node->type == GUMBO_NODE_ELEMENT &&
             node->v.element.tag != GUMBO_TAG_SCRIPT &&
             node->v.element.tag != GUMBO_TAG_STYLE) {
    std::string contents = "";
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
      const std::string text = cleantext((GumboNode*) children->data[i]);
      if (i != 0 && !text.empty()) {
        contents.append(" ");
      }
      contents.append(text);
    }
    return contents;
  } else {
    return "";
  }
}

// Returns string containing parsed text
string converter::getOutput(string fileName){ 

  const char* filename = fileName.c_str();

  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (!in) {
    std::cout << "File " << filename << " not found!\n";
    exit(EXIT_FAILURE);
  }

  std::string contents;
  in.seekg(0, std::ios::end);
  contents.resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&contents[0], contents.size());
  in.close();

  GumboOutput* output = gumbo_parse(contents.c_str());
  string ans = cleantext(output->root);

  int pos = 0;
  //string s = "CSE 12";
  string s = cI->subject + " " + cI->num;
  unsigned int len = s.length();
  int count = 0;

  // find every instance of subject that is not actual subject
  // ex: CSE123 showing up in searches for CSE12
  pos = ans.find(s,pos);
  while( pos != -1 ) {//  && count < 100 ) {
  //while( ans.at(pos = ans.find(s,pos) + len) != ' '  ) { 
     //remove both %s
      //cerr << "Found string: 
      if( isdigit(ans.at(pos+len)) ) {
     //if ( ans.at(pos+len) != ' ' ) {
         //cerr << "it's a number:" << ans.substr(pos+len-10, 20) << endl;
         //cerr << "=" << ans.at(pos+len) << endl;
         pos = ans.find("%",pos); 
         if ( pos == -1 ) break;
         ans = ans.replace(pos,1," ");
         //cerr << "Deleting % - 1" << endl;

         pos = ans.find("%",pos); 
         if ( pos == -1 ) break;
         ans = ans.replace(pos,1," "); 
         //cerr << "Deleting % - 2" << endl;
     }
     else {
        //cerr << "NOT a number:\n" << ans.at(pos+len) << endl;
     }
     pos = ans.find(s,pos+1);
     //count++;
  }

  //std::cerr << cleantext(output->root) << std::endl;
  gumbo_destroy_output(&kGumboDefaultOptions, output);
  //cerr << "===\n===\n===" << endl;
  //cerr << ans << endl;
  return ans;
}

int main(int argc, char** argv){
    converter * conv = new converter();
}
