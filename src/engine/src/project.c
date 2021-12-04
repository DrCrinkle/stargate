#include <time.h>

#include "stargate.h"
#include "daw.h"
#include "files.h"
#include "wave_edit.h"


void v_open_tracks(){
    v_we_open_tracks();
}

void v_open_project(const char* a_project_folder, int a_first_load){
#if SG_OS == _OS_LINUX
    struct timespec f_start, f_finish;
    clock_gettime(CLOCK_REALTIME, &f_start);
#endif

    char stargate_dot_project[1024];
    sprintf(
        stargate_dot_project,
        "%s%sstargate.project",
        a_project_folder,
        PATH_SEP
    );
    if(!i_file_exists(stargate_dot_project)){
        log_error(
            "Project folder %s does not contain a stargate.project file, "
            "it is not a Stargate DAW project, exiting.",
            a_project_folder
        );
        exit(321);
    }
    log_info("Setting files and folders");
    sprintf(STARGATE->project_folder, "%s", a_project_folder);
    sprintf(
        STARGATE->plugins_folder,
        "%s%sprojects%splugins%s",
        STARGATE->project_folder,
        PATH_SEP,
        PATH_SEP,
        PATH_SEP
    );
    sprintf(
        STARGATE->samples_folder,
        "%s%saudio%ssamples",
        STARGATE->project_folder,
        PATH_SEP,
        PATH_SEP
    );  //No trailing slash
    sprintf(
        STARGATE->samplegraph_folder,
        "%s%saudio%ssamplegraph",
        STARGATE->project_folder,
        PATH_SEP,
        PATH_SEP
    );  //No trailing slash

    sprintf(
        STARGATE->audio_pool->samples_folder,
        "%s",
        STARGATE->samples_folder
    );

    sprintf(
        STARGATE->audio_pool_file,
        "%s%saudio%saudio_pool",
        STARGATE->project_folder,
        PATH_SEP,
        PATH_SEP
    );
    sprintf(
        STARGATE->audio_folder,
        "%s%saudio",
        STARGATE->project_folder,
        PATH_SEP
    );
    sprintf(
        STARGATE->audio_tmp_folder,
        "%s%saudio%sfiles%stmp%s",
        STARGATE->project_folder,
        PATH_SEP,
        PATH_SEP,
        PATH_SEP,
        PATH_SEP
    );

    if(a_first_load && i_file_exists(STARGATE->audio_pool_file)){
        log_info("Loading wave pool");
        v_audio_pool_add_items(
            STARGATE->audio_pool,
            STARGATE->audio_pool_file,
            STARGATE->audio_folder
        );
    }

    log_info("Opening wave editor project");
    v_we_open_project();
    log_info("Opening DAW project");
    v_daw_open_project(a_first_load);
    log_info("Finished opening projects");

#if SG_OS == _OS_LINUX
    clock_gettime(CLOCK_REALTIME, &f_finish);
    v_print_benchmark("v_open_project", f_start, f_finish);
#endif
}


