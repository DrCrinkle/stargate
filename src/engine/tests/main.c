#include "test_audiodsp.h"
#include "test_csv.h"
#include "test_files.h"
#include "test_plugins.h"


int main(){
    TestAudioDSP();
    TestCSV();
    TestFilesAll();
    TestPlugins();

    return 0;
}
