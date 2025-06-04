#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <zlib.h> // For zlib compression

// --- Base64 Encoding Functions (simplified, for demonstration) ---
// Note: In a real application, consider a more robust Base64 library.

static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Encodes a block of 3 bytes to 4 Base64 characters
void base64_encode_block(const unsigned char *in, unsigned char *out, int len) {
    out[0] = base64_chars[in[0] >> 2];
    out[1] = base64_chars[((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4)];
    out[2] = base64_chars[((in[1] & 0x0F) << 2) | ((in[2] & 0xC0) >> 6)];
    out[3] = base64_chars[in[2] & 0x3F];

    if (len < 3) out[3] = '=';
    if (len < 2) out[2] = '=';
}

// Encodes a byte array to a Base64 string
// Returns a dynamically allocated string, caller must free it.
char* base64_encode(const unsigned char *data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = (char *)malloc(output_length + 1);
    if (encoded_data == NULL) return NULL;

    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }

    // Add padding
    if (input_length % 3 == 1) {
        encoded_data[j - 1] = '=';
        encoded_data[j - 2] = '=';
    } else if (input_length % 3 == 2) {
        encoded_data[j - 1] = '=';
    }
    encoded_data[j] = '\0'; // Null-terminate

    return encoded_data;
}

// --- Main Program ---
int main() {
    uint32_t original_numbers[] = {
        23, 325553, 23, 322, 1, 3, 23, 23, 21, 13, 23, 5, 23, 231, 523, 23, 12, 123,
        1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000,
        1234567, 87654321, 2147483647, 0, 1, 2, 10, 100, 1000, 100000, 10000000,
        123, 456, 789, 987, 654, 321, 1234, 5678, 9012, 3456, 7890, 1000, 1000, 1000,
        1234567, 1234567, 1234567, 1234567,
        50000, 60000, 70000, 80000, 90000, 100000, 110000, 120000,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        12345, 67890, 123456, 789012, 12345678, 90123456, 100, 200, 300, 400
    };

    size_t num_elements = sizeof(original_numbers) / sizeof(original_numbers[0]);
    size_t uncompressed_size = num_elements * sizeof(uint32_t);

    // 1. Chuyển đổi mảng uint32_t thành luồng byte
    unsigned char* uncompressed_bytes = (unsigned char*)malloc(uncompressed_size);
    if (uncompressed_bytes == NULL) {
        perror("Failed to allocate memory for uncompressed_bytes");
        return 1;
    }

    // Copy numbers to byte array (assuming current system's endianness)
    // For cross-platform compatibility, consider explicit endianness conversion.
    for (size_t i = 0; i < num_elements; ++i) {
        // Cast to unsigned char* and copy bytes directly
        memcpy(uncompressed_bytes + (i * sizeof(uint32_t)), &original_numbers[i], sizeof(uint32_t));
    }

    printf("Original uncompressed size: %zu bytes\n", uncompressed_size);

    // 2. Nén luồng byte bằng zlib
    // Ước tính kích thước buffer nén an toàn: data_size + 0.1% * data_size + 12 (hoặc 13)
    unsigned long compressed_buffer_size = compressBound(uncompressed_size);
    unsigned char* compressed_bytes = (unsigned char*)malloc(compressed_buffer_size);
    if (compressed_bytes == NULL) {
        perror("Failed to allocate memory for compressed_bytes");
        free(uncompressed_bytes);
        return 1;
    }

    int z_result = compress(compressed_bytes, &compressed_buffer_size, uncompressed_bytes, uncompressed_size);

    if (z_result != Z_OK) {
        fprintf(stderr, "zlib compression failed: %d\n", z_result);
        free(uncompressed_bytes);
        free(compressed_bytes);
        return 1;
    }

    printf("Compressed size: %lu bytes\n", compressed_buffer_size);

    // 3. Mã hóa luồng byte đã nén sang Base64
    char* encoded_base64_string = base64_encode(compressed_bytes, compressed_buffer_size);

    if (encoded_base64_string == NULL) {
        perror("Failed to allocate memory for Base64 string");
        free(uncompressed_bytes);
        free(compressed_bytes);
        return 1;
    }

    printf("Encoded Base64 string length: %zu\n", strlen(encoded_base64_string));
    printf("Encoded Base64 string: %s\n", encoded_base64_string);

    // Giải phóng bộ nhớ
    free(uncompressed_bytes);
    free(compressed_bytes);
    free(encoded_base64_string);

    return 0;
}