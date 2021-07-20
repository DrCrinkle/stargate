#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "csv/2d.h"
#include "csv/split.h"
#include "files.h"
#include "file/path.h"
#include "hardware/config.h"

int OUTPUT_CH_COUNT = 2;
int MAIN_OUT_L = 0;
int MAIN_OUT_R = 1;
int THREAD_AFFINITY = 0;
int THREAD_AFFINITY_SET = 0;
int AUDIO_INPUT_TRACK_COUNT = 0;
int NO_HARDWARE = 0;


NO_OPTIMIZATION char* default_device_file_path(){
    char* result = (char*)malloc(2048);

    char * f_home = get_home_dir();

    printf("using home folder: %s\n", f_home);

    char * path_list[4] = {
        f_home, STARGATE_VERSION, "config", "device.txt"
    };

    path_join(result, 4, path_list);

    return result;
}

NO_OPTIMIZATION struct HardwareConfig* load_hardware_config(
    char* config_path
){
    struct HardwareConfig* result = (struct HardwareConfig*)malloc(
        sizeof(struct HardwareConfig)
    );
    *result = (struct HardwareConfig){
        .sample_rate = 44100.0,
        .host_api_name = "",
        .device_name = "",
        .input_name = "",
        .audio_input_count = 0,
        .audio_output_count = 2,
        .frame_count = DEFAULT_FRAMES_PER_BUFFER,
        .performance = 0,
        .thread_affinity = 0,
        .thread_count = 0,
        .midi_in_device_count = 0,
    };

    char * f_key_char = (char*)malloc(sizeof(char) * TINY_STRING);
    char * f_value_char = (char*)malloc(sizeof(char) * TINY_STRING);

    if(!i_file_exists(config_path)){
        printf("%s does not exist\n", config_path);
        return NULL;
    }
    printf("%s exists, loading\n", config_path);

    t_2d_char_array * f_current_string = g_get_2d_array_from_file(
        config_path,
        LARGE_STRING
    );
    while(1)
    {
        v_iterate_2d_char_array(f_current_string);
        if(f_current_string->eof){
            break;
        }
        if(
            !strcmp(f_current_string->current_str, "")
            ||
            f_current_string->eol
        ){
            continue;
        }

        strcpy(f_key_char, f_current_string->current_str);
        v_iterate_2d_char_array_to_next_line(f_current_string);
        strcpy(f_value_char, f_current_string->current_str);

        if(!strcmp(f_key_char, "hostApi")){
            strncpy(result->host_api_name, f_value_char, 127);
        } else if(!strcmp(f_key_char, "name")){
            strncpy(result->device_name, f_value_char, 255);
        } else if(!strcmp(f_key_char, "inputName")){
            strncpy(result->input_name, f_value_char, 255);
            printf("input name: %s\n", result->input_name);
        } else if(!strcmp(f_key_char, "bufferSize")){
            result->frame_count = atoi(f_value_char);
            printf("bufferSize: %i\n", result->frame_count);
        } else if(!strcmp(f_key_char, "audioEngine")){
            int f_engine = atoi(f_value_char);
            printf("audioEngine: %i\n", f_engine);
            if(f_engine == 4 || f_engine == 5 || f_engine == 7){
                NO_HARDWARE = 1;
            } else {
                //NO_HARDWARE = 0;
            }
        } else if(!strcmp(f_key_char, "sampleRate")){
            result->sample_rate = atof(f_value_char);
            printf("sampleRate: %i\n", (int)result->sample_rate);
        } else if(!strcmp(f_key_char, "threads")){
            result->thread_count = atoi(f_value_char);
            if(result->thread_count > 8){
                result->thread_count = 8;
            } else if(result->thread_count < 0){
                result->thread_count = 0;
            }
            printf("threads: %i\n", result->thread_count);
        } else if(!strcmp(f_key_char, "threadAffinity")){
            result->thread_affinity = atoi(f_value_char);
            THREAD_AFFINITY = result->thread_affinity;
            printf("threadAffinity: %i\n", result->thread_affinity);
        } else if(!strcmp(f_key_char, "performance")){
            result->performance = atoi(f_value_char);

            printf("performance: %i\n", result->performance);
        } else if(!strcmp(f_key_char, "midiInDevice")){
            snprintf(
                result->midi_in_device_names[result->midi_in_device_count],
                127,
                "%s",
                f_value_char
            );
            result->midi_in_device_count++;
        } else if(!strcmp(f_key_char, "audioInputs")){
            result->audio_input_count = atoi(f_value_char);
            printf("audioInputs: %s\n", f_value_char);
            assert(
                result->audio_input_count >= 0
                &&
                result->audio_input_count <= 128
            );
            AUDIO_INPUT_TRACK_COUNT = result->audio_input_count;
        } else if(!strcmp(f_key_char, "audioOutputs")){
            t_line_split * f_line = g_split_line(
                '|',
                f_value_char
            );
            if(f_line->count != 3){
                printf(
                    "audioOutputs: invalid value: '%s'\n",
                    f_value_char
                );
                return NULL;
            }
            printf("audioOutputs: %s\n", f_value_char);

            result->audio_output_count = atoi(f_line->str_arr[0]);
            assert(
                result->audio_output_count >= 1
                &&
                result->audio_output_count <= 128
            );
            OUTPUT_CH_COUNT = result->audio_output_count;
            MAIN_OUT_L = atoi(f_line->str_arr[1]);
            MAIN_OUT_R = atoi(f_line->str_arr[2]);
            assert(
                MAIN_OUT_L >= 0
                &&
                MAIN_OUT_L < result->audio_output_count
            );
            assert(
                MAIN_OUT_R >= 0
                &&
                MAIN_OUT_R < result->audio_output_count
            );

            v_free_split_line(f_line);
        } else {
            printf("Unknown key|value pair: %s|%s\n",
                f_key_char, f_value_char);
        }
    }

    free(f_key_char);
    free(f_value_char);

    g_free_2d_char_array(f_current_string);

    return result;
}
