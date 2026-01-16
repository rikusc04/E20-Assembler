#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void assemble(const char* input_file, const char* output_file);


char *generateOutputFile(const char *inputFile) {
    const char* last_slash = strrchr(inputFile, '/');
    const char* last_backslash = strrchr(inputFile, '\\');
    const char* sep = last_slash;
    if (last_backslash && (!sep || last_backslash > sep)) {
        sep = last_backslash;
    }

    const char* filename = sep ? sep + 1 : inputFile;
    const char* dot = strrchr(filename, '.');
    size_t baseLen = dot ? (size_t)(dot - inputFile) : strlen(inputFile);
    char* output = malloc(baseLen + 5); // ".bin" + '\0'
    if (!output) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }

    snprintf(output, baseLen + 5, "%.*s.bin", (int)baseLen, inputFile);
    return output;
}

int main(int argc, char* argv[]) {
    char* outputFile = NULL;

    if (argc == 2) {
        outputFile = generateOutputFile(argv[1]);
    }
    else if (argc == 3) { // output file name provided
        outputFile = argv[2];
    }
    else {
        fprintf(stderr, "Usage: %s <input.s> [output.bin]\n", argv[0]);
        fprintf(stderr, "  If output.bin is not specified, it will be auto-generated\n");
        fprintf(stderr, "  Example: %s input.s             (produces input.bin)\n", argv[0]);
        fprintf(stderr, "           %s input.s output.bin  (produces output.bin)\n", argv[0]);
        return 1;
    }
    assemble(argv[1], outputFile);

    if (argc == 2) {
        free(outputFile);
    }

}