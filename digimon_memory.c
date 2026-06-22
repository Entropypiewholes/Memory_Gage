#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <storage/storage.h>
#include <toolbox/stream/file_stream.h>
#include <furi_hal.h>

#include <stdio.h>
#include <stdlib.h>

#define TAG "DigimonMemory"
#define CFG_FILENAME "memory.txt"
#define MIN_MEMORY -10
#define MAX_MEMORY 10

typedef struct {
    int8_t memory;
} DigimonMemoryModel;

typedef struct {
    ViewDispatcher* view_dispatcher;
    View* view;
    Storage* storage;
    Gui* gui;
} DigimonMemoryApp;

static int8_t clamp_memory(int8_t value) {
    if(value < MIN_MEMORY) return MIN_MEMORY;
    if(value > MAX_MEMORY) return MAX_MEMORY;
    return value;
}

static void save_memory(Storage* storage, int8_t value) {
    const char* path = APP_DATA_PATH(CFG_FILENAME);
    Stream* stream = file_stream_alloc(storage);
    FuriString* str = furi_string_alloc();
    furi_string_printf(str, "%d\n", value);

    if(file_stream_open(stream, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        stream_write_string(stream, str);
    }

    stream_free(stream);
    furi_string_free(str);
}

static int8_t load_memory(Storage* storage) {
    const char* path = APP_DATA_PATH(CFG_FILENAME);
    Stream* stream = file_stream_alloc(storage);
    FuriString* line = furi_string_alloc();
    int8_t value = 0;

    if(file_stream_open(stream, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        if(stream_read_line(stream, line)) {
            int parsed = atoi(furi_string_get_cstr(line));
            value = clamp_memory((int8_t)parsed);
        }
    }

    stream_free(stream);
    furi_string_free(line);
    return value;
}

static void beep(float frequency, float duration_ms, float volume) {
    if(furi_hal_speaker_acquire(500)) {
        furi_hal_speaker_start(frequency, volume);
        furi_delay_ms((uint32_t)duration_ms);
        furi_hal_speaker_stop();
        furi_hal_speaker_release();
    }
}

static void digimon_memory_draw_callback(Canvas* canvas, void* model) {
    DigimonMemoryModel* m = (DigimonMemoryModel*)model;
    int8_t mem = m->memory;

    canvas_clear(canvas);

    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 0, AlignCenter, AlignTop, "Digimon Memory");

    // Big value + side label
    char value_str[8];
    const char* side_label;
    if(mem > 0) {
        snprintf(value_str, sizeof(value_str), "+%d", mem);
        side_label = "OPP";
    } else if(mem < 0) {
        snprintf(value_str, sizeof(value_str), "%d", mem); // already has minus
        side_label = "YOU";
    } else {
        snprintf(value_str, sizeof(value_str), "0");
        side_label = "0";
    }

    canvas_set_font(canvas, FontBigNumbers);
    // Center the number (move up a little to leave room)
    canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignCenter, value_str);

    // Small side indicator next to or below
    canvas_set_font(canvas, FontSecondary);
    if(mem != 0) {
        canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignTop, side_label);
    }

    // Status line
    char status[32];
    if(mem > 0) {
        snprintf(status, sizeof(status), "Opponent has %d memory", mem);
    } else if(mem < 0) {
        snprintf(status, sizeof(status), "You have %d memory", -mem);
    } else {
        snprintf(status, sizeof(status), "Neutral (0)");
    }
    canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignTop, status);

    // Gauge bar
    const int gauge_y = 55;
    const int left_x = 10;
    const int right_x = 118;

    // Base line
    canvas_draw_line(canvas, left_x, gauge_y, right_x, gauge_y);

    // Ticks and center marker (21 positions: -10 ... +10)
    for(int i = -10; i <= 10; i++) {
        // Use integer step that spans nicely (108 px / 20 intervals = 5.4 -> use 5 + adjust)
        int x = left_x + ((i + 10) * 5) + 4; // tuned offset for visual centering
        if(x > right_x) x = right_x;
        if(x < left_x) x = left_x;

        if(i == 0) {
            // Center neutral mark (taller)
            canvas_draw_line(canvas, x, gauge_y - 3, x, gauge_y + 3);
        } else if(i % 5 == 0) {
            canvas_draw_line(canvas, x, gauge_y - 2, x, gauge_y + 2);
        } else {
            canvas_draw_line(canvas, x, gauge_y - 1, x, gauge_y + 1);
        }
    }

    // Current position indicator (triangle pointing up from below the line for visibility)
    int pos_x = left_x + ((mem + 10) * 5) + 4;
    if(pos_x < left_x) pos_x = left_x;
    if(pos_x > right_x) pos_x = right_x;

    // Small filled marker (disc or triangle)
    canvas_draw_disc(canvas, pos_x, gauge_y - 3, 2);

    // End labels
    //canvas_set_font(canvas, FontSecondary);
    //canvas_draw_str(canvas, 1, gauge_y + 7, "YOU");
    //canvas_draw_str_aligned(canvas, 127, gauge_y + 7, AlignRight, AlignTop, "OPP");

}

static bool digimon_memory_input_callback(InputEvent* event, void* context) {
    DigimonMemoryApp* app = context;
    bool consumed = false;

    if(event->type != InputTypeShort && event->type != InputTypeRepeat) {
        return false;
    }

    with_view_model(
        app->view,
        DigimonMemoryModel * model,
        {
            int8_t before = model->memory;
            if(event->key == InputKeyUp) {
                model->memory = clamp_memory(model->memory + 1);
                if(model->memory != before) {
                    beep(540.0f, 35.0f, 0.6f);
                }
                consumed = true;
            } else if(event->key == InputKeyDown) {
                model->memory = clamp_memory(model->memory - 1);
                if(model->memory != before) {
                    beep(420.0f, 35.0f, 0.6f);
                }
                consumed = true;
            } else if(event->key == InputKeyOk) {
                if(model->memory != 0) {
                    model->memory = 0;
                    beep(880.0f, 60.0f, 0.5f);
                }
                consumed = true;
            } else if(event->key == InputKeyRight) {
                if(model->memory != 3) {
                    model->memory = 3;
                    beep(660.0f, 50.0f, 0.6f);
                }
                consumed = true;
            } else if(event->key == InputKeyLeft) {
                if(model->memory != -3) {
                    model->memory = -3;
                    beep(675.0f, 50.0f, 0.6f);
                }
                consumed = true;
            
            if(model->memory != before) {
                save_memory(app->storage, model->memory);
            }
        },
        consumed);

    return consumed;
}

static bool digimon_memory_navigation_callback(void* context) {
    DigimonMemoryApp* app = context;
    view_dispatcher_stop(app->view_dispatcher);
    return true;
}

static uint32_t digimon_memory_previous_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE; // signals to the view system to exit / go nowhere
}

static DigimonMemoryApp* digimon_memory_alloc(void) {
    DigimonMemoryApp* app = malloc(sizeof(DigimonMemoryApp));
    memset(app, 0, sizeof(DigimonMemoryApp));

    app->storage = furi_record_open(RECORD_STORAGE);
    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, digimon_memory_navigation_callback);

    app->view = view_alloc();
    view_set_context(app->view, app);
    view_allocate_model(app->view, ViewModelTypeLocking, sizeof(DigimonMemoryModel));
    view_set_draw_callback(app->view, digimon_memory_draw_callback);
    view_set_input_callback(app->view, digimon_memory_input_callback);
    view_set_previous_callback(app->view, digimon_memory_previous_callback);

    // Init model from storage
    with_view_model(
        app->view,
        DigimonMemoryModel * model,
        {
            model->memory = load_memory(app->storage);
        },
        true);

    view_dispatcher_add_view(app->view_dispatcher, 0, app->view);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);

    return app;
}

static void digimon_memory_free(DigimonMemoryApp* app) {
    // Persist final value (in case)
    with_view_model(
        app->view,
        DigimonMemoryModel * model,
        {
            save_memory(app->storage, model->memory);
        },
        false);

    view_dispatcher_remove_view(app->view_dispatcher, 0);
    view_free(app->view);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t digimon_memory_app(void* p) {
    UNUSED(p);

    DigimonMemoryApp* app = digimon_memory_alloc();
    view_dispatcher_run(app->view_dispatcher);
    digimon_memory_free(app);

    return 0;
}
