#define GOOGLE_STRIP_LOG 1

#include <glog/logging.h>

int main(int argc, char* argv[]) {
    // Initialize Google’s logging library.
    google::InitGoogleLogging(argv[0]);

    VLOG(2) << "stuffs";
}
