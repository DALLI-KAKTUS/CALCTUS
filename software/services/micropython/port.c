// MicroPython platform hooks. No OS-specific calls: same source on both targets.
#include <string.h>

#include "py/builtin.h"
#include "py/compile.h"
#include "py/lexer.h"
#include "py/parse.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "py/objstr.h"

extern void calctus_python_output(const char* str, size_t length);

void mp_hal_stdout_tx_strn_cooked(const char* str, size_t length) {
    calctus_python_output(str, length);
}

// Bounded root-only SD file object: text mode, 4095-byte maximum, commit on
// close. No host filesystem, POSIX or desktop-only behavior leaks into VM.
#define CALCTUS_FILE_MAX 4095
typedef struct {
    mp_obj_base_t base;
    char name[32];
    char data[CALCTUS_FILE_MAX + 1];
    size_t length;
    size_t position;
    bool readable;
    bool writable;
    bool closed;
} calctus_file_t;

const mp_obj_type_t calctus_file_type;
extern int calctus_python_file_read(const char* name, char* data, size_t capacity,
                                    size_t* length);
extern int calctus_python_file_write(const char* name, const char* data, size_t length);

static calctus_file_t* file_obj(mp_obj_t self_in) {
    calctus_file_t* self = MP_OBJ_TO_PTR(self_in);
    if (self->closed) mp_raise_ValueError(MP_ERROR_TEXT("I/O operation on closed file"));
    return self;
}

static mp_obj_t file_read(size_t n_args, const mp_obj_t* args) {
    calctus_file_t* self = file_obj(args[0]);
    if (!self->readable) mp_raise_OSError(MP_EPERM);
    size_t count = self->length - self->position;
    if (n_args > 1 && mp_obj_get_int(args[1]) >= 0 &&
        (size_t)mp_obj_get_int(args[1]) < count) count = mp_obj_get_int(args[1]);
    mp_obj_t result = mp_obj_new_str(self->data + self->position, count);
    self->position += count;
    return result;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(file_read_obj, 1, 2, file_read);

static mp_obj_t file_readline(mp_obj_t self_in) {
    calctus_file_t* self = file_obj(self_in);
    if (!self->readable) mp_raise_OSError(MP_EPERM);
    size_t end = self->position;
    while (end < self->length && self->data[end++] != '\n') {}
    mp_obj_t result = mp_obj_new_str(self->data + self->position, end - self->position);
    self->position = end;
    return result;
}
static MP_DEFINE_CONST_FUN_OBJ_1(file_readline_obj, file_readline);

static mp_obj_t file_write(mp_obj_t self_in, mp_obj_t value) {
    calctus_file_t* self = file_obj(self_in);
    if (!self->writable) mp_raise_OSError(MP_EPERM);
    size_t length;
    const char* data = mp_obj_str_get_data(value, &length);
    if (length > CALCTUS_FILE_MAX - self->position)
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("file exceeds 4095 bytes"));
    memcpy(self->data + self->position, data, length);
    self->position += length;
    if (self->position > self->length) self->length = self->position;
    self->data[self->length] = '\0';
    return mp_obj_new_int_from_uint(length);
}
static MP_DEFINE_CONST_FUN_OBJ_2(file_write_obj, file_write);

static mp_obj_t file_tell(mp_obj_t self_in) {
    return mp_obj_new_int_from_uint(file_obj(self_in)->position);
}
static MP_DEFINE_CONST_FUN_OBJ_1(file_tell_obj, file_tell);

static mp_obj_t file_seek(size_t n_args, const mp_obj_t* args) {
    calctus_file_t* self = file_obj(args[0]);
    mp_int_t offset = mp_obj_get_int(args[1]);
    mp_int_t whence = n_args > 2 ? mp_obj_get_int(args[2]) : 0;
    mp_int_t base = whence == 0 ? 0 : whence == 1 ? self->position :
                    whence == 2 ? self->length : -1;
    if (base < 0 || base + offset < 0 || base + offset > CALCTUS_FILE_MAX)
        mp_raise_ValueError(MP_ERROR_TEXT("invalid seek"));
    self->position = base + offset;
    return mp_obj_new_int_from_uint(self->position);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(file_seek_obj, 2, 3, file_seek);

static mp_obj_t file_close(mp_obj_t self_in) {
    calctus_file_t* self = MP_OBJ_TO_PTR(self_in);
    if (!self->closed && self->writable &&
        !calctus_python_file_write(self->name, self->data, self->length))
        mp_raise_OSError(MP_EIO);
    self->closed = true;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(file_close_obj, file_close);

static mp_obj_t file_flush(mp_obj_t self_in) {
    calctus_file_t* self = file_obj(self_in);
    if (self->writable &&
        !calctus_python_file_write(self->name, self->data, self->length))
        mp_raise_OSError(MP_EIO);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(file_flush_obj, file_flush);

static mp_obj_t file_enter(mp_obj_t self_in) {
    (void)file_obj(self_in);
    return self_in;
}
static MP_DEFINE_CONST_FUN_OBJ_1(file_enter_obj, file_enter);

static mp_obj_t file_exit(size_t n_args, const mp_obj_t* args) {
    (void)n_args;
    return file_close(args[0]);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(file_exit_obj, 4, 4, file_exit);

static const mp_rom_map_elem_t file_locals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&file_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readline), MP_ROM_PTR(&file_readline_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&file_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_tell), MP_ROM_PTR(&file_tell_obj) },
    { MP_ROM_QSTR(MP_QSTR_seek), MP_ROM_PTR(&file_seek_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&file_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&file_flush_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&file_enter_obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&file_exit_obj) },
};
static MP_DEFINE_CONST_DICT(file_locals, file_locals_table);
MP_DEFINE_CONST_OBJ_TYPE(calctus_file_type, MP_QSTR_io, MP_TYPE_FLAG_NONE,
    locals_dict, &file_locals);

static mp_obj_t calctus_open(size_t n_args, const mp_obj_t* args, mp_map_t* kwargs) {
    if (kwargs->used) mp_raise_TypeError(MP_ERROR_TEXT("open() keyword arguments unsupported"));
    if (n_args < 1 || n_args > 2) mp_raise_TypeError(MP_ERROR_TEXT("open(path[, mode])"));
    size_t name_len;
    const char* name = mp_obj_str_get_data(args[0], &name_len);
    if (!name_len || name_len >= sizeof(((calctus_file_t*)0)->name) ||
        memchr(name, '/', name_len) || memchr(name, '\\', name_len))
        mp_raise_ValueError(MP_ERROR_TEXT("root filename required"));
    char filename[32];
    memcpy(filename, name, name_len);
    filename[name_len] = '\0';
    const char* mode = "r";
    size_t mode_len;
    if (n_args == 2) mode = mp_obj_str_get_data(args[1], &mode_len);
    else mode_len = 1;
    if (mode_len != 1 || (mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a'))
        mp_raise_ValueError(MP_ERROR_TEXT("mode must be r, w or a"));
    const char kind = mode[0];
    calctus_file_t* self = mp_obj_malloc(calctus_file_t, &calctus_file_type);
    memset((char*)self + sizeof(self->base), 0, sizeof(*self) - sizeof(self->base));
    memcpy(self->name, filename, name_len + 1);
    self->readable = kind == 'r';
    self->writable = kind != 'r';
    self->closed = false;
    if (kind == 'r' || kind == 'a') {
        if (!calctus_python_file_read(self->name, self->data, sizeof(self->data),
                                      &self->length)) {
            if (kind == 'r') mp_raise_OSError(MP_ENOENT);
            self->length = 0;
        }
    }
    self->data[self->length] = '\0';
    self->position = kind == 'a' ? self->length : 0;
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, calctus_open);

// Embed portunda POSIX/VFS yok. Python import yolu yalnız HAL'in SD kökündeki
// ASCII .py dosyalarıyla eşleşir; platformun gerçek dosya yolu VM'ye girmez.
static char import_source[4096];
extern int calctus_python_import_read(const char* name, char* data,
                                      size_t capacity, size_t* length);

static bool import_name(const char* path) {
    if (!path || strchr(path, '/') || strchr(path, '\\')) return false;
    size_t size = strlen(path);
    return size > 3 && size < 32 && strcmp(path + size - 3, ".py") == 0;
}

mp_import_stat_t mp_import_stat(const char* path) {
    size_t length = 0;
    return import_name(path) &&
           calctus_python_import_read(path, import_source, sizeof import_source, &length)
           ? MP_IMPORT_STAT_FILE : MP_IMPORT_STAT_NO_EXIST;
}

mp_lexer_t* mp_lexer_new_from_file(qstr filename) {
    const char* path = qstr_str(filename);
    size_t length = 0;
    if (!import_name(path) ||
        !calctus_python_import_read(path, import_source, sizeof import_source, &length)) {
        mp_raise_OSError(MP_ENOENT);
    }
    return mp_lexer_new_from_str_len(filename, import_source, length, 0);
}

int calctus_python_exec(const char* source) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_lexer_t* lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, source,
                                                     strlen(source), 0);
        qstr source_name = lex->source_name;
        mp_parse_tree_t tree = mp_parse(lex, MP_PARSE_FILE_INPUT);
        mp_obj_t function = mp_compile(&tree, source_name, true);
        mp_call_function_0(function);
        nlr_pop();
        return 1;
    }
    mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    return 0;
}

extern int calctus_python_limit_reached(void);
void calctus_python_vm_hook(void) {
    if (calctus_python_limit_reached()) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("execution limit"));
    }
}
