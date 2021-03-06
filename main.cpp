#include <unistd.h>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <curl/curl.h>
#include "date.hpp"

using namespace std;
using namespace cv;

string getISOCurrentTimestamp ();
void capture (Mat& frame, const int& width);
bool post (const string& url, const string& time, const string& fileName);

/**
 * @argv[1] - URL: string
 * @argv[2] - WIDTH: int
 * @argv[3] - INTERVAL: double
 */
int main (int argc, char* argv[]) {
    if (argc != 4) {
        cout << "<arg1> - URL" << endl;
        cout << "<arg2> - Capture width size" << endl;
        cout << "<arg3> - Capture interval (ms)" << endl;
        return -1;
    }
    const string URL = string(argv[1]);
    const int WIDTH = atoi(argv[2]);
    const double INTERVAL = atof(argv[3]);

    while (waitKey(INTERVAL) == -1) {
        const string TIMESTAMP = getISOCurrentTimestamp();
        const string FILENAME = TIMESTAMP.substr(0, 10) + ".jpg"; // yyyy-mm-dd.jpg
        Mat frame;
        capture(frame, WIDTH);
        cout << "capture done. writing image" << endl;
        imwrite(FILENAME, frame);
        post(URL, TIMESTAMP, FILENAME);
        cout << "post done" << endl;
    }
    system("rm -f *.jpg");
    return 0;
}

string getISOCurrentTimestamp () {
    return date::format("%FT%TZ", date::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()));
}

void capture (Mat& frame, const int& width) {
    VideoCapture cap;
    cap.open(0);
    cap.set(CAP_PROP_FRAME_WIDTH, width);
    cap.set(CAP_PROP_FRAME_HEIGHT, int((float(width) / 4) * 3));
    cap >> frame;
    cap.release();
}

bool post (const string& url, const string& time, const string& fileName) {
    struct curl_httppost* formpost = NULL;
    struct curl_httppost* lastptr = NULL;
    struct curl_slist* headerlist = NULL;
    static const char buf[] = "Expect:";

    curl_global_init(CURL_GLOBAL_ALL);

    curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, "time",
                CURLFORM_COPYCONTENTS, time.c_str(),
                CURLFORM_END);

    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "filename",
                 CURLFORM_COPYCONTENTS, fileName.c_str(),
                 CURLFORM_END);

    // from file
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "files",
                 CURLFORM_FILE, fileName.c_str(),
                 CURLFORM_END);

    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "submit",
                 CURLFORM_COPYCONTENTS, "send",
                 CURLFORM_END);

    CURL* curl = curl_easy_init();

    headerlist = curl_slist_append(headerlist, buf);
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

        CURLcode res = curl_easy_perform(curl);
        while (res != CURLE_OK) {
            res = curl_easy_perform(curl);
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            waitKey(2500);
        }
        curl_easy_cleanup(curl);
        curl_formfree(formpost);
        curl_slist_free_all (headerlist);
    }
    return true;
}