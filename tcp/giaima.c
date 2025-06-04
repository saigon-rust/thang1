#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <zlib.h> // For zlib decompression
#include <stdbool.h> // For boolean type

// --- Base64 Decoding Functions ---

/**
 * @brief Maps a Base64 character to its 6-bit integer value.
 *
 * @param c Base64 character.
 * @return 6-bit integer value, or -1 if the character is invalid.
 */
int base64_char_to_val(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    if (c == '=') return 0; // Padding character
    return -1; // Invalid character
}

/**
 * @brief Decodes a Base64 string to a byte array.
 *
 * @param encoded_data Pointer to the input Base64 string.
 * @param input_length Length of the input Base64 string.
 * @param output_length Pointer to store the length of the decoded byte array.
 * @return Dynamically allocated byte array. Caller must free this memory.
 * Returns NULL on memory allocation failure or invalid input.
 */
unsigned char* base64_decode(const char *encoded_data, size_t input_length, size_t *output_length) {
    if (input_length % 4 != 0) { // Base64 string length must be a multiple of 4
        fprintf(stderr, "Base64 input length is not a multiple of 4.\n");
        return NULL;
    }

    // Calculate potential max output length (before considering padding)
    size_t decoded_max_length = (input_length / 4) * 3;
    unsigned char *decoded_bytes = (unsigned char *)malloc(decoded_max_length);
    if (decoded_bytes == NULL) {
        perror("Failed to allocate memory for decoded bytes");
        return NULL;
    }

    size_t i, j;
    int val[4];
    *output_length = 0; // Initialize output length

    for (i = 0, j = 0; i < input_length; i += 4) {
        // Get 4 Base64 character values
        val[0] = base64_char_to_val(encoded_data[i]);
        val[1] = base64_char_to_val(encoded_data[i+1]);
        val[2] = base64_char_to_val(encoded_data[i+2]);
        val[3] = base64_char_to_val(encoded_data[i+3]);

        // Check for invalid characters
        if (val[0] == -1 || val[1] == -1 || val[2] == -1 || val[3] == -1) {
            fprintf(stderr, "Invalid Base64 character encountered at block starting index %zu.\n", i);
            free(decoded_bytes);
            return NULL;
        }

        // Reconstruct the 24-bit triple
        uint32_t triple = (val[0] << 18) | (val[1] << 12) | (val[2] << 6) | val[3];

        // Extract original 3 bytes
        if (encoded_data[i+2] != '=') { // If not padding, write the third byte
            decoded_bytes[j++] = (triple >> 16) & 0xFF;
            decoded_bytes[j++] = (triple >> 8) & 0xFF;
            decoded_bytes[j++] = triple & 0xFF;
        } else if (encoded_data[i+3] == '=') { // Two padding characters
            decoded_bytes[j++] = (triple >> 16) & 0xFF;
        } else { // One padding character
            decoded_bytes[j++] = (triple >> 16) & 0xFF;
            decoded_bytes[j++] = (triple >> 8) & 0xFF;
        }
    }
    *output_length = j; // Set the actual decoded length

    // Reallocate to the exact size to save memory
    // Only realloc if actual length is less than max allocated, and not zero
    if (*output_length < decoded_max_length && *output_length > 0) {
        unsigned char *final_decoded_bytes = (unsigned char *)realloc(decoded_bytes, *output_length);
        if (final_decoded_bytes == NULL) {
            perror("Failed to reallocate memory for decoded bytes");
            free(decoded_bytes); // Free the original block if realloc fails
            return NULL;
        }
        return final_decoded_bytes;
    }
    return decoded_bytes; // Return original if no realloc needed or output_length is 0
}

// --- Zlib Decompression Function ---

/**
 * @brief Decompresses a zlib-compressed byte array.
 *
 * @param compressed_data Pointer to the compressed byte array.
 * @param compressed_length Length of the compressed byte array.
 * @param output_length Pointer to store the length of the decompressed byte array.
 * @param estimated_uncompressed_size An estimate of the uncompressed size.
 * This helps in initial buffer allocation.
 * @return Dynamically allocated decompressed byte array. Caller must free this memory.
 * Returns NULL on memory allocation failure or decompression error.
 */
unsigned char* zlib_decompress(const unsigned char *compressed_data, unsigned long compressed_length,
                               unsigned long *output_length, unsigned long estimated_uncompressed_size) {
    unsigned long buffer_size = estimated_uncompressed_size;
    if (buffer_size == 0) { // Provide a reasonable default if estimate is 0
        buffer_size = 4 * compressed_length; // A common heuristic for decompression buffer
        if (buffer_size < 1024) buffer_size = 1024; // Ensure a minimum size
    }

    unsigned char *decompressed_data = (unsigned char *)malloc(buffer_size);

    if (decompressed_data == NULL) {
        perror("Failed to allocate memory for decompressed_data");
        return NULL;
    }

    int z_result = uncompress(decompressed_data, &buffer_size, compressed_data, compressed_length);

    // Handle Z_BUF_ERROR by reallocating and retrying
    while (z_result == Z_BUF_ERROR) {
        buffer_size *= 2; // Double the buffer size
        unsigned char *temp_ptr = (unsigned char *)realloc(decompressed_data, buffer_size);
        if (temp_ptr == NULL) {
            perror("Failed to reallocate memory for decompressed_data");
            free(decompressed_data);
            return NULL;
        }
        decompressed_data = temp_ptr;
        z_result = uncompress(decompressed_data, &buffer_size, compressed_data, compressed_length);
    }

    if (z_result != Z_OK) {
        fprintf(stderr, "zlib decompression failed: %d\n", z_result);
        free(decompressed_data);
        return NULL;
    }

    *output_length = buffer_size; // Set the actual decompressed length

    // Reallocate to the exact size to save memory
    // Only realloc if actual length is less than allocated, and not zero
    if (*output_length < buffer_size && *output_length > 0) {
        unsigned char *final_decompressed_data = (unsigned char *)realloc(decompressed_data, *output_length);
        if (final_decompressed_data == NULL) {
            perror("Failed to reallocate memory for decompressed data");
            free(decompressed_data); // Free the original block if realloc fails
            return NULL;
        }
        return final_decompressed_data;
    }
    return decompressed_data; // Return original if no realloc needed or output_length is 0
}


// --- Main Program for Decoding ---
int main() {
    // Define a buffer to store the user's input Base64 string
    // A common Base64 string length is about 1.33 times the original data size.
    // Assuming original data could be up to a few KB, a buffer of 4096 bytes should be sufficient.
    // You might need to adjust this size based on the expected maximum input string length.
    char input_base64_string[4096];
    size_t encoded_base64_length;

    printf("--- DECODING PROCESS ---\n");
    printf("Please enter the Base64 string to decode (max 4095 characters):\n");
    // Read the input string from stdin
    if (fgets(input_base64_string, sizeof(input_base64_string), stdin) == NULL) {
        fprintf(stderr, "Error reading input.\n");
        return 1;
    }

    // Remove trailing newline character if present
    input_base64_string[strcspn(input_base64_string, "\n")] = 0;
    encoded_base64_length = strlen(input_base64_string);

    printf("Input Base64 string length: %zu\n", encoded_base64_length);

    // 1. Decode the Base64 string back to compressed bytes
    size_t decoded_compressed_length;
    unsigned char* decoded_compressed_bytes = base64_decode(input_base64_string, encoded_base64_length, &decoded_compressed_length);
    if (decoded_compressed_bytes == NULL) {
        fprintf(stderr, "Base64 decoding failed.\n");
        return 1;
    }
    printf("Decoded Base64 (compressed) size: %zu bytes\n", decoded_compressed_length);

    // 2. Decompress the byte stream using zlib
    unsigned long decompressed_data_length;
    // We pass 0 as estimated_uncompressed_size, allowing zlib_decompress to determine initial buffer size
    unsigned char* decompressed_bytes = zlib_decompress(decoded_compressed_bytes, decoded_compressed_length, &decompressed_data_length, 0);
    if (decompressed_bytes == NULL) {
        fprintf(stderr, "zlib decompression failed.\n");
        free(decoded_compressed_bytes);
        return 1;
    }
    printf("Decompressed (original) size: %lu bytes\n", decompressed_data_length);

    // 3. Convert the decompressed byte stream back to uint32_t array
    if (decompressed_data_length % sizeof(uint32_t) != 0) {
        fprintf(stderr, "Error: Decompressed data length is not a multiple of uint32_t size.\n");
        free(decoded_compressed_bytes);
        free(decompressed_bytes);
        return 1;
    }

    size_t decoded_num_elements = decompressed_data_length / sizeof(uint32_t);
    uint32_t* decoded_numbers = (uint32_t*)malloc(decoded_num_elements * sizeof(uint32_t));
    if (decoded_numbers == NULL) {
        perror("Failed to allocate memory for decoded_numbers");
        free(decoded_compressed_bytes);
        free(decompressed_bytes);
        return 1;
    }

    // Copy bytes back to uint32_t array (maintaining system's endianness)
    // If you used htonl() during encoding, you would use ntohl() here.
    for (size_t i = 0; i < decoded_num_elements; ++i) {
        memcpy(&decoded_numbers[i], decompressed_bytes + (i * sizeof(uint32_t)), sizeof(uint32_t));
    }

    // --- DISPLAY DECODED NUMBERS ---
    printf("\n--- DECODED NUMBERS ---\n");
    printf("Number of decoded elements: %zu\n", decoded_num_elements);
    for (size_t i = 0; i < decoded_num_elements; ++i) {
        printf("%u ", decoded_numbers[i]);
        if ((i + 1) % 10 == 0) printf("\n"); // Print 10 numbers per line for readability
    }
    printf("\n"); // Newline after printing all numbers

    // --- CLEANUP ---
    free(decoded_compressed_bytes);
    free(decompressed_bytes);
    free(decoded_numbers);

    return 0;
}
