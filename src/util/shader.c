#include "util/shader.h"

#include <stdlib.h>

/* clang-format off */
#ifndef SHADER_BIN_DIR
#define SHADER_BIN_DIR build/shaders
#endif

#define PATH_JOIN(dir, file) dir/file
#define STR_(x) #x
#define STR(x) STR_(x)
#define EMBED_PATH(file) STR(PATH_JOIN(SHADER_BIN_DIR, file))

const unsigned char shader_vertex_spv_data[] = {
#embed EMBED_PATH(shader.vert.spv)
};
const uint32_t shader_vertex_spv_size = sizeof(shader_vertex_spv_data);

const unsigned char shader_fragment_spv_data[] = {
#embed EMBED_PATH(shader.frag.spv)
};
const uint32_t shader_fragment_spv_size = sizeof(shader_fragment_spv_data);
/* clang-format on */

void *shader_get_vertex_spv_data(uint32_t *size) {
    if (size != NULL) {
        *size = shader_vertex_spv_size;
    }

    return (void *)shader_vertex_spv_data;
}

void *shader_get_fragment_spv_data(uint32_t *size) {
    if (size != NULL) {
        *size = shader_fragment_spv_size;
    }

    return (void *)shader_fragment_spv_data;
}
