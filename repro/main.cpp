#define GOOGLE_STRIP_LOG 1

#include <glog/logging.h>

int main(int argc, char* argv[]) {
    VLOG(2) << "stuffs";
}
