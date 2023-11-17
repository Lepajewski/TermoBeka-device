#include <cstring>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "argtable3/argtable3.h"

#include "system_manager.h"
#include "logger.h"
#include "tb_event.h"
#include "sd_commands.h"


extern SystemManager sysMgr;


const char * const TAG = "CMDsd";


static esp_err_t send_to_sd_queue(SDEvent *evt) {
    QueueHandle_t *queue = sysMgr.get_sd_queue();
    evt->origin = EventOrigin::SYSTEM_MANAGER;

    if (xQueueSend(*queue, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "cmd sd event send fail");
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t process_path_cmd(SDEventType e_type, const char *path) {
    esp_err_t err = ESP_OK;
    SDEvent evt;
    evt.type = e_type;
    uint16_t path_len = strlen(path) + 1;

    if (path_len < SD_QUEUE_MAX_PAYLOAD) {
        memcpy(evt.payload, path, path_len);
        err = send_to_sd_queue(&evt);
    } else {
        err = ESP_FAIL;
    }

    return err;
}

static esp_err_t process_path_buffer(SDEventType e_type, const char *path, const char *buf) {
    esp_err_t err = ESP_OK;
    SDEvent evt;
    evt.type = e_type;
    uint16_t path_len = strlen(path) + 1;

    if (path_len < SD_QUEUE_MAX_PAYLOAD) {
        snprintf(reinterpret_cast<char *>(evt.payload), SD_QUEUE_MAX_PAYLOAD, "%s %s", path, buf);
        err = send_to_sd_queue(&evt);
    } else {
        err = ESP_FAIL;
    }

    return err;
}


static int cmd_mount_card(int argc, char **argv) {
    TB_ACK(TAG, "mount card");
    SDEvent evt;
    evt.type = SDEventType::MOUNT_CARD;

    return send_to_sd_queue(&evt);
}

static int cmd_unmount_card(int argc, char **argv) {
    TB_ACK(TAG, "unmount card");
    SDEvent evt;
    evt.type = SDEventType::UNMOUNT_CARD;

    return send_to_sd_queue(&evt);
}


static struct {
    struct arg_str *path;
    struct arg_end *end;
} cmd_path;

static void init_cmd_path_args() {
    cmd_path.path = arg_str0(NULL, NULL, "<path>", "path from root (omit \'/\')");
    cmd_path.end = arg_end(1);
}

static int cmd_ls(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &cmd_path);
    if (nerrors != 0) {
        arg_print_errors(stderr, cmd_path.end, argv[0]);
        TB_NAK(TAG, "invalid arg");
        return ESP_FAIL;
    }
    
    TB_ACK(TAG, "ls %s", cmd_path.path->sval[0]);

    return process_path_cmd(SDEventType::LS, cmd_path.path->sval[0]);
}

static int cmd_cat(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &cmd_path);
    if (nerrors != 0) {
        arg_print_errors(stderr, cmd_path.end, argv[0]);
        TB_NAK(TAG, "invalid arg");
        return ESP_FAIL;
    }
    
    TB_ACK(TAG, "cat %s", cmd_path.path->sval[0]);

    return process_path_cmd(SDEventType::CAT, cmd_path.path->sval[0]);
}

static int cmd_mkdir(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &cmd_path);
    if (nerrors != 0) {
        arg_print_errors(stderr, cmd_path.end, argv[0]);
        TB_NAK(TAG, "invalid arg");
        return ESP_FAIL;
    }
    
    TB_ACK(TAG, "mkdir, path: %s", cmd_path.path->sval[0]);

    return process_path_cmd(SDEventType::MKDIR, cmd_path.path->sval[0]);
}

static int cmd_touch(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &cmd_path);
    if (nerrors != 0) {
        arg_print_errors(stderr, cmd_path.end, argv[0]);
        TB_NAK(TAG, "invalid arg");
        return ESP_FAIL;
    }
    
    TB_ACK(TAG, "touch, path: %s", cmd_path.path->sval[0]);

    return process_path_cmd(SDEventType::TOUCH, cmd_path.path->sval[0]);
}

static int cmd_rm_file(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &cmd_path);
    if (nerrors != 0) {
        arg_print_errors(stderr, cmd_path.end, argv[0]);
        TB_NAK(TAG, "invalid arg");
        return ESP_FAIL;
    }
    
    TB_ACK(TAG, "rm_file, path: %s", cmd_path.path->sval[0]);

    return process_path_cmd(SDEventType::RM_FILE, cmd_path.path->sval[0]);
}

static int cmd_rm_dir(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &cmd_path);
    if (nerrors != 0) {
        arg_print_errors(stderr, cmd_path.end, argv[0]);
        TB_NAK(TAG, "invalid arg");
        return ESP_FAIL;
    }
    
    TB_ACK(TAG, "rm_dir, path: %s", cmd_path.path->sval[0]);

    return process_path_cmd(SDEventType::RM_DIR, cmd_path.path->sval[0]);
}


static struct {
    struct arg_str *path;
    struct arg_str *buffer;
    struct arg_end *end;
} cmd_path_buffer;

static void init_cmd_path_buffer_args() {
    cmd_path_buffer.path = arg_str0(NULL, NULL, "<path>", "path from root (omit \'/\')");
    cmd_path_buffer.buffer = arg_str0(NULL, NULL, "<buffer>", "buffer to be saved");
    cmd_path_buffer.end = arg_end(2);
}

static int cmd_save_to_file(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &cmd_path_buffer);
    if (nerrors != 0) {
        arg_print_errors(stderr, cmd_path_buffer.end, argv[0]);
        TB_NAK(TAG, "invalid arg");
        return ESP_FAIL;
    }

    TB_ACK(TAG, "save_to_file %s \'%s\'", cmd_path_buffer.path->sval[0], cmd_path_buffer.buffer->sval[0]);

    return process_path_buffer(SDEventType::SAVE_TO_FILE, cmd_path_buffer.path->sval[0], cmd_path_buffer.buffer->sval[0]);
}


static const esp_console_cmd_t commands[] = {
//    command           help print                      hint        callback            arguments
    { "mount_card",     "mount SD card",                NULL,       &cmd_mount_card,    NULL                },
    { "unmount_card",   "unmount SD card",              NULL,       &cmd_unmount_card,  NULL                },
    { "ls",             "list files on SD card",        NULL,       &cmd_ls,            &cmd_path           },
    { "cat",            "print file contents",          NULL,       &cmd_cat,           &cmd_path           },
    { "mkdir",          "create directory if exists",   NULL,       &cmd_mkdir,         &cmd_path           },
    { "touch",          "touch file",                   NULL,       &cmd_touch,         &cmd_path           },
    { "rm",             "remove file",                  NULL,       &cmd_rm_file,       &cmd_path           },
    { "rmdir",          "remove empty directory",       NULL,       &cmd_rm_dir,        &cmd_path           },
    { "save_to_file",   "append input to file",         NULL,       &cmd_save_to_file,  &cmd_path_buffer    }
};

void register_sd_card() {
    init_cmd_path_args();
    init_cmd_path_buffer_args();

    for (auto i : commands) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&i));
    }
}
